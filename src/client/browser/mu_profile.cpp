// Reads Config.ini from MEMFS. The browser stubs used to ignore the file,
// so language, font, sound and resolution never left their defaults.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <string>
#include <vector>

static void slashes(char* s) {
    for (; s && *s; ++s)
        if (*s == '\\') *s = '/';
}

static bool same(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        ++a;
        ++b;
    }
    return *a == 0 && *b == 0;
}

static void trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i) s.erase(0, i);
}

static bool read_lines(const char* file, std::vector<std::string>& lines) {
    char path[512];
    if (!file || !file[0]) return false;
    strncpy(path, file, sizeof(path) - 1);
    path[sizeof(path) - 1] = 0;
    slashes(path);
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    std::string cur;
    int c;
    int n = 0;
    while ((c = fgetc(f)) != EOF && n < 1024 * 1024) {
        ++n;
        if (c == '\n') {
            lines.push_back(cur);
            cur.clear();
        } else {
            cur.push_back((char)c);
        }
    }
    if (!cur.empty()) lines.push_back(cur);
    fclose(f);
    if (!lines.empty() && lines[0].size() >= 3 &&
        (unsigned char)lines[0][0] == 0xEF &&
        (unsigned char)lines[0][1] == 0xBB &&
        (unsigned char)lines[0][2] == 0xBF) {
        lines[0].erase(0, 3);
    }
    return true;
}

static bool section_name(const std::string& line, std::string& name) {
    if (line.size() < 3 || line[0] != '[') return false;
    size_t end = line.find(']');
    if (end == std::string::npos || end < 2) return false;
    name = line.substr(1, end - 1);
    trim(name);
    return !name.empty();
}

static bool key_value(const std::string& line, std::string& key, std::string& value) {
    if (line.empty() || line[0] == ';' || line[0] == '[') return false;
    size_t eq = line.find('=');
    if (eq == std::string::npos) return false;
    key = line.substr(0, eq);
    value = line.substr(eq + 1);
    trim(key);
    trim(value);
    return !key.empty();
}

static unsigned copy_out(char* out, unsigned n, const char* s) {
    if (!out || n == 0) return 0;
    const char* src = s ? s : "";
    unsigned i = 0;
    for (; i + 1 < n && src[i]; ++i) out[i] = src[i];
    out[i] = 0;
    return i;
}

extern "C" int mu_file_exists(const char* path) {
    char buf[512];
    if (!path || !path[0]) return 0;
    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    slashes(buf);
    struct stat st;
    if (stat(buf, &st) != 0) return 0;
    return S_ISREG(st.st_mode) ? 1 : 0;
}

extern "C" unsigned mu_profile_string(const char* sec, const char* key, const char* def,
                                      char* out, unsigned n, const char* file) {
    const char* fallback = def ? def : "";
    if (!sec || !key || !file) return copy_out(out, n, fallback);
    std::vector<std::string> lines;
    if (!read_lines(file, lines)) return copy_out(out, n, fallback);
    std::string section;
    bool in = false;
    for (const std::string& raw : lines) {
        std::string line = raw;
        trim(line);
        std::string name;
        if (section_name(line, name)) {
            in = same(name.c_str(), sec);
            continue;
        }
        if (!in) continue;
        std::string k, v;
        if (key_value(line, k, v) && same(k.c_str(), key))
            return copy_out(out, n, v.c_str());
    }
    return copy_out(out, n, fallback);
}

extern "C" unsigned mu_profile_section(const char* sec, char* out, unsigned n, const char* file) {
    if (out && n) out[0] = 0;
    if (!sec || !file || !out || n < 3) return 0;
    std::vector<std::string> lines;
    if (!read_lines(file, lines)) return 0;
    std::string section;
    bool in = false;
    bool found = false;
    unsigned used = 0;
    for (const std::string& raw : lines) {
        std::string line = raw;
        trim(line);
        std::string name;
        if (section_name(line, name)) {
            if (in) break;
            in = same(name.c_str(), sec);
            found = found || in;
            continue;
        }
        if (!in) continue;
        std::string k, v;
        if (!key_value(line, k, v)) continue;
        std::string item = k + "=" + v;
        if (used + item.size() + 2 >= n) {
            out[used] = 0;
            return n > 2 ? n - 2 : 0;
        }
        memcpy(out + used, item.c_str(), item.size() + 1);
        used += (unsigned)item.size() + 1;
    }
    if (!found) return 0;
    if (used + 1 < n) out[used] = 0;
    return used ? used : 1;
}

extern "C" int mu_profile_write(const char* sec, const char* key, const char* val, const char* file) {
    if (!sec || !key || !file || !file[0]) return 0;
    std::vector<std::string> lines;
    read_lines(file, lines);
    int sec_line = -1;
    int sec_end = (int)lines.size();
    int key_line = -1;
    bool in = false;
    for (int i = 0; i < (int)lines.size(); ++i) {
        std::string line = lines[i];
        trim(line);
        std::string name;
        if (section_name(line, name)) {
            if (in) {
                sec_end = i;
                break;
            }
            in = same(name.c_str(), sec);
            if (in) sec_line = i;
            continue;
        }
        std::string k, v;
        if (in && key_value(line, k, v) && same(k.c_str(), key)) key_line = i;
    }
    if (key_line >= 0) {
        if (val) lines[key_line] = std::string(key) + "=" + val;
        else lines.erase(lines.begin() + key_line);
    } else if (val) {
        const std::string item = std::string(key) + "=" + val;
        if (sec_line >= 0) lines.insert(lines.begin() + sec_end, item);
        else {
            lines.push_back(std::string("[") + sec + "]");
            lines.push_back(item);
        }
    }
    char path[512];
    strncpy(path, file, sizeof(path) - 1);
    path[sizeof(path) - 1] = 0;
    slashes(path);
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    for (size_t i = 0; i < lines.size(); ++i) {
        fwrite(lines[i].data(), 1, lines[i].size(), f);
        fputc('\n', f);
    }
    fclose(f);
    return 1;
}
