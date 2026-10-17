// SoundManager.cpp — CSound. Ver SoundManager.h.

#include "stdafx.h"
#include "Sound/SoundManager.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <math.h>

extern "C" void DbgLogPublic(const char* msg);

CSound gSound;

namespace {
constexpr int kMaxLevel = 9;
}

long CSound::LevelToDirectSound(int level)
{
    // Fórmula del DLL (CSound::UpdateSoundVolumeLevel): 0 es mudo y cada nivel
    // suma 6,25 dB hasta llegar a 0 dB (sin atenuación) en el 9.
    if (level <= 0) return DSBVOLUME_MIN;
    long vol = 625 * (level - 1) - 5000;
    return vol > DSBVOLUME_MAX ? DSBVOLUME_MAX : vol;
}

float CSound::LevelToGain(int level)
{
    // La misma curva que LevelToDirectSound, en ganancia lineal.
    if (level <= 0) return 0.0f;
    const double db = (625.0 * (level - 1) - 5000.0) / 100.0;
    return (float)pow(10.0, db / 20.0);
}

void CSound::SetLevels(int soundLevel, int musicLevel)
{
    auto clamp = [](int level) {
        if (level < 0) return kMaxLevel;
        return level > kMaxLevel ? kMaxLevel : level;
    };
    m_SoundLevel = clamp(soundLevel);
    m_MusicLevel = clamp(musicLevel);

    for (int buffer = 0; buffer < 420; ++buffer)
        ApplySoundVolume(buffer);
    if (m_Music)
        ma_sound_set_volume(static_cast<ma_sound*>(m_Music), LevelToGain(m_MusicLevel));
}

void CSound::ApplySoundVolume(int buffer)
{
    if (!g_EnableSound || buffer < 0 || buffer >= 420) return;
    for (int channel = 0; channel < 4; ++channel) {
        if (g_lpDSBuffer[buffer][channel])
            g_lpDSBuffer[buffer][channel]->SetVolume(LevelToDirectSound(m_SoundLevel));
    }
}

bool CSound::EnsureEngine()
{
    if (m_Engine) return true;

    ma_engine* engine = new ma_engine;
    ma_engine_config config = ma_engine_config_init();
    // The WebAudio backend pulls audio on the game thread (ScriptProcessorNode).
    // A 10 ms period underruns as soon as a frame runs long, which is the
    // crackle, and resampling every 22050 effect up to 48000 is most of that
    // callback. Mix at the WAV rate. 2048 frames is about 90 ms, enough to
    // ride out a hitch. If the browser refuses 22050, fall back to its rate
    // with the same frame count.
    config.sampleRate = 22050;
    config.periodSizeInFrames = 2048;
    config.periodSizeInMilliseconds = 0;
    if (ma_engine_init(&config, engine) != MA_SUCCESS) {
        delete engine;
        engine = new ma_engine;
        config.sampleRate = 0;
        config.periodSizeInFrames = 4096;
        if (ma_engine_init(&config, engine) != MA_SUCCESS) {
            delete engine;
            DbgLogPublic("Music: no se pudo iniciar miniaudio");
            return false;
        }
    }
    m_Engine = engine;
    ma_engine_set_volume(engine, m_Audible ? 1.0f : 0.0f);
    return true;
}

void CSound::SetAudible(bool on)
{
    m_Audible = on;
    if (m_Engine)
        ma_engine_set_volume(static_cast<ma_engine*>(m_Engine), on ? 1.0f : 0.0f);
}

bool CSound::PlayMusic(const char* fileName)
{
    StopMusic();
    if (!fileName || !EnsureEngine()) return false;

    // Streaming: el archivo se decodifica de a poco, sin cargarlo entero.
    ma_sound* sound = new ma_sound;
    const ma_uint32 flags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION;
    if (ma_sound_init_from_file(static_cast<ma_engine*>(m_Engine), fileName, flags,
                                nullptr, nullptr, sound) != MA_SUCCESS) {
        delete sound;
        return false;
    }
    ma_sound_set_volume(sound, LevelToGain(m_MusicLevel));
    ma_sound_start(sound);
    m_Music = sound;

    char line[300];
    wsprintfA(line, "Music: %s", fileName);
    DbgLogPublic(line);
    return true;
}

void CSound::StopMusic()
{
    if (!m_Music) return;
    ma_sound* sound = static_cast<ma_sound*>(m_Music);
    ma_sound_stop(sound);
    ma_sound_uninit(sound);
    delete sound;
    m_Music = nullptr;
}

void CSound::SetMusicStoppedByUser(bool stopped)
{
    m_UserStopped = stopped;
    if (stopped) StopMusic();
}

namespace {
struct PcmVoice {
    const void* owner;
    ma_audio_buffer* buffer;
    ma_sound* sound;
    unsigned char* pcm;
    int bytes;
    int rate;
    int channels;
    int bits;
    long volume;
    float spatial;
    bool playing;
    bool looping;
};

// One slot per loaded effect. Stealing a slot uninits the graph node on the
// same thread that fills the speakers, which is the cutoff that jumped
// from sound to sound.
constexpr int kVoiceCount = 128;
PcmVoice g_voices[kVoiceCount];

float DirectSoundToGain(long dsVolume)
{
    if (dsVolume <= -10000) return 0.0f;
    float gain = (float)pow(10.0, ((double)dsVolume / 100.0) / 20.0);
    return gain > 1.0f ? 1.0f : gain;
}

void ApplyVoiceGain(PcmVoice* voice)
{
    if (!voice || !voice->sound) return;
    float gain = DirectSoundToGain(voice->volume) * voice->spatial;
    if (gain < 0.0f) gain = 0.0f;
    if (gain > 1.0f) gain = 1.0f;
    ma_sound_set_volume(voice->sound, gain);
}

void ReleaseVoice(PcmVoice* voice)
{
    if (!voice) return;
    if (voice->sound) {
        ma_sound_uninit(voice->sound);
        delete voice->sound;
        voice->sound = nullptr;
    }
    if (voice->buffer) {
        ma_audio_buffer_uninit(voice->buffer);
        delete voice->buffer;
        voice->buffer = nullptr;
    }
    free(voice->pcm);
    voice->pcm = nullptr;
    voice->owner = nullptr;
    voice->playing = false;
    voice->bytes = 0;
}

PcmVoice* FindVoice(const void* owner)
{
    if (!owner) return nullptr;
    for (int i = 0; i < kVoiceCount; ++i)
        if (g_voices[i].owner == owner) return &g_voices[i];
    return nullptr;
}

bool VoiceAudible(const PcmVoice* voice)
{
    return voice && voice->sound && ma_sound_is_playing(voice->sound);
}

PcmVoice* AllocVoice(const void* owner)
{
    PcmVoice* slot = FindVoice(owner);
    if (slot) return slot;
    for (int i = 0; i < kVoiceCount; ++i)
        if (!g_voices[i].owner) return &g_voices[i];
    for (int i = 0; i < kVoiceCount; ++i) {
        if (!VoiceAudible(&g_voices[i]) && !g_voices[i].looping) {
            ReleaseVoice(&g_voices[i]);
            return &g_voices[i];
        }
    }
    for (int i = 0; i < kVoiceCount; ++i) {
        if (!g_voices[i].looping) {
            ReleaseVoice(&g_voices[i]);
            return &g_voices[i];
        }
    }
    ReleaseVoice(&g_voices[0]);
    return &g_voices[0];
}
}

bool CSound::PlayPcm(const void* owner, const void* pcm, int bytes, int rate, int channels, int bits,
                     long dsVolume, float spatialGain, int loop)
{
    if (!owner || !pcm || bytes <= 0 || rate <= 0 || channels <= 0) return false;
    int sampleBytes = bits <= 8 ? 1 : 2;
    int frameBytes = sampleBytes * channels;
    if (frameBytes <= 0 || bytes < frameBytes) return false;
    if (spatialGain < 0.0f) spatialGain = 0.0f;
    if (spatialGain > 1.0f) spatialGain = 1.0f;
    if (!EnsureEngine()) return false;

    PcmVoice* voice = AllocVoice(owner);
    bool same = voice->owner == owner && voice->sound && voice->pcm &&
                voice->bytes == bytes && voice->rate == rate &&
                voice->channels == channels && voice->bits == bits;
    // IDirectSoundBuffer::Play on a buffer that is already playing keeps the
    // cursor. Seeking back to 0 here restarted footsteps, UI ticks and zone
    // sounds on every call, which is the stutter that moved from place to place.
    if (same && voice->sound && ma_sound_is_playing(voice->sound)) {
        voice->volume = dsVolume;
        voice->spatial = spatialGain;
        voice->looping = loop != 0;
        ma_sound_set_looping(voice->sound, loop ? MA_TRUE : MA_FALSE);
        ApplyVoiceGain(voice);
        voice->playing = true;
        return true;
    }

    if (!same) {
        ReleaseVoice(voice);
        voice->pcm = (unsigned char*)malloc((size_t)bytes);
        if (!voice->pcm) return false;
        memcpy(voice->pcm, pcm, (size_t)bytes);
        voice->bytes = bytes;
        voice->rate = rate;
        voice->channels = channels;
        voice->bits = bits;
        voice->owner = owner;

        ma_audio_buffer_config cfg = ma_audio_buffer_config_init(
            bits <= 8 ? ma_format_u8 : ma_format_s16,
            (ma_uint32)channels,
            (ma_uint64)(bytes / frameBytes),
            voice->pcm,
            nullptr);
        // config_init leaves sampleRate at 0, and the engine then plays the
        // frames at the device rate (usually 48000). The WAVs are 22050.
        cfg.sampleRate = (ma_uint32)rate;
        voice->buffer = new ma_audio_buffer;
        if (ma_audio_buffer_init(&cfg, voice->buffer) != MA_SUCCESS) {
            ReleaseVoice(voice);
            return false;
        }
        voice->sound = new ma_sound;
        if (ma_sound_init_from_data_source(static_cast<ma_engine*>(m_Engine), voice->buffer,
                                           MA_SOUND_FLAG_NO_SPATIALIZATION,
                                           nullptr, voice->sound) != MA_SUCCESS) {
            ReleaseVoice(voice);
            return false;
        }
    } else if (voice->sound) {
        ma_sound_seek_to_pcm_frame(voice->sound, 0);
    }

    voice->owner = owner;
    voice->volume = dsVolume;
    voice->spatial = spatialGain;
    voice->looping = loop != 0;
    voice->playing = true;
    ma_sound_set_looping(voice->sound, loop ? MA_TRUE : MA_FALSE);
    ApplyVoiceGain(voice);
    ma_sound_start(voice->sound);
    return true;
}

void CSound::StopPcm(const void* owner)
{
    PcmVoice* voice = FindVoice(owner);
    if (!voice || !voice->sound) return;
    ma_sound_stop(voice->sound);
    voice->playing = false;
}

void CSound::SetPcmVolume(const void* owner, long dsVolume)
{
    PcmVoice* voice = FindVoice(owner);
    if (!voice) return;
    voice->volume = dsVolume;
    ApplyVoiceGain(voice);
}

void CSound::SetPcmPosition(const void* owner, float x, float y, float z)
{
    PcmVoice* voice = FindVoice(owner);
    if (!voice) return;
    float d2 = x * x + y * y + z * z;
    float gain = 1.0f;
    if (d2 > 1.0f) {
        gain = 1.0f / sqrtf(d2);
        if (gain < 0.03f) gain = 0.0f;
    }
    voice->spatial = gain;
    ApplyVoiceGain(voice);
}

void CSound::Shutdown()
{
    StopMusic();
    for (int i = 0; i < kVoiceCount; ++i)
        ReleaseVoice(&g_voices[i]);
    if (m_Engine) {
        ma_engine* engine = static_cast<ma_engine*>(m_Engine);
        ma_engine_uninit(engine);
        delete engine;
        m_Engine = nullptr;
    }
}
