// Web Worker hosting the compiled MU Online Solo GameServer WebAssembly module
importScripts('muserver.js');

let Module = null;
let tickTimer = null;
let lastTickTime = performance.now();

function log(msg, color = '#60a5fa') {
    self.postMessage({ type: 'LOG', message: msg, color: color });
}

// ============================================================================
// Cryptographic Vault for Save Games
// Standard Web Crypto API: AES-256-GCM + SHA-256 Tamper Detection Checksum
// ============================================================================
const VAULT_MAGIC = "MUSAV1";
const VAULT_SECRET = "MU_ONLINE_SOLO_CLASSIC_097K_WASM_VAULT_KEY";
const VAULT_SALT = new Uint8Array([
    0x4D, 0x55, 0x5F, 0x53, 0x4F, 0x4C, 0x4F, 0x5F,
    0x53, 0x41, 0x4C, 0x54, 0x30, 0x39, 0x37, 0x4B
]); // "MU_SOLO_SALT097K"

let cachedCryptoKey = null;

async function getVaultKey() {
    if (cachedCryptoKey) return cachedCryptoKey;
    const enc = new TextEncoder();
    const keyMaterial = await self.crypto.subtle.importKey(
        "raw",
        enc.encode(VAULT_SECRET),
        { name: "PBKDF2" },
        false,
        ["deriveKey"]
    );
    cachedCryptoKey = await self.crypto.subtle.deriveKey(
        {
            name: "PBKDF2",
            salt: VAULT_SALT,
            iterations: 10000,
            hash: "SHA-256"
        },
        keyMaterial,
        { name: "AES-GCM", length: 256 },
        false,
        ["encrypt", "decrypt"]
    );
    return cachedCryptoKey;
}

function readVirtualSaveFiles() {
    const saves = {};
    if (!Module || !Module.FS) return saves;

    try {
        if (Module.FS.analyzePath('/saves').exists) {
            const files = Module.FS.readdir('/saves').filter(f => f !== '.' && f !== '..');
            for (const file of files) {
                const data = Module.FS.readFile('/saves/' + file);
                let binary = '';
                const len = data.byteLength;
                for (let i = 0; i < len; i++) {
                    binary += String.fromCharCode(data[i]);
                }
                saves[file] = btoa(binary);
            }
        }
    } catch (err) {
        log(`[Worker] Error reading /saves: ${err.message}`, '#f87171');
    }
    return saves;
}

function writeVirtualSaveFiles(savesMap) {
    if (!Module || !Module.FS || !savesMap) return 0;

    let count = 0;
    try {
        if (!Module.FS.analyzePath('/saves').exists) {
            Module.FS.mkdir('/saves');
        }

        for (const [file, b64] of Object.entries(savesMap)) {
            const binary = atob(b64);
            const bytes = new Uint8Array(binary.length);
            for (let i = 0; i < binary.length; i++) {
                bytes[i] = binary.charCodeAt(i);
            }
            Module.FS.writeFile('/saves/' + file, bytes);
            count++;
        }
        log(`[Worker] Відновлено ${count} файл(ів) у /saves`, '#34d399');
    } catch (err) {
        log(`[Worker] Error writing /saves: ${err.message}`, '#f87171');
    }
    return count;
}

async function exportEncryptedSaves() {
    const rawFiles = readVirtualSaveFiles();
    const fileCount = Object.keys(rawFiles).length;
    if (fileCount === 0) {
        return null;
    }

    try {
        const key = await getVaultKey();
        const jsonStr = JSON.stringify(rawFiles);
        const iv = self.crypto.getRandomValues(new Uint8Array(12));
        const encoded = new TextEncoder().encode(jsonStr);

        const ciphertextBuffer = await self.crypto.subtle.encrypt(
            { name: "AES-GCM", iv: iv },
            key,
            encoded
        );

        const cipherBytes = new Uint8Array(ciphertextBuffer);

        // Calculate SHA-256 integrity checksum over ciphertext
        const hashBuffer = await self.crypto.subtle.digest("SHA-256", cipherBytes);
        const hashHex = Array.from(new Uint8Array(hashBuffer))
            .map(b => b.toString(16).padStart(2, '0'))
            .join('');

        let cipherBinary = '';
        for (let i = 0; i < cipherBytes.length; i++) cipherBinary += String.fromCharCode(cipherBytes[i]);

        let ivBinary = '';
        for (let i = 0; i < iv.length; i++) ivBinary += String.fromCharCode(iv[i]);

        return {
            magic: VAULT_MAGIC,
            version: 1,
            encrypted: true,
            algorithm: "AES-GCM-256",
            timestamp: Date.now(),
            filesCount: fileCount,
            iv: btoa(ivBinary),
            checksum: hashHex,
            payload: btoa(cipherBinary)
        };
    } catch (err) {
        log(`[Vault] Помилка шифрування сейву: ${err.message}`, '#ef4444');
        return null;
    }
}

async function importEncryptedSaves(container) {
    if (!container) return 0;

    try {
        // 1. Backward compatibility: handle legacy unencrypted save
        if (!container.magic && typeof container === 'object') {
            log('[Vault] Знайдено нешифрований сейв (legacy). Відновлюємо...', '#fbbf24');
            return writeVirtualSaveFiles(container);
        }

        if (container.magic !== VAULT_MAGIC) {
            throw new Error(`Невідомий формат файлу (очікувався заголовок ${VAULT_MAGIC})`);
        }

        // 2. Decode ciphertext bytes
        const cipherBinary = atob(container.payload);
        const cipherBytes = new Uint8Array(cipherBinary.length);
        for (let i = 0; i < cipherBinary.length; i++) {
            cipherBytes[i] = cipherBinary.charCodeAt(i);
        }

        // 3. Verify SHA-256 integrity checksum
        const hashBuffer = await self.crypto.subtle.digest("SHA-256", cipherBytes);
        const hashHex = Array.from(new Uint8Array(hashBuffer))
            .map(b => b.toString(16).padStart(2, '0'))
            .join('');

        if (hashHex !== container.checksum) {
            throw new Error("Сейв пошкоджено або змінено в текстовому редакторі (невідповідність контрольної суми SHA-256)!");
        }

        // 4. Decode IV
        const ivBinary = atob(container.iv);
        const iv = new Uint8Array(ivBinary.length);
        for (let i = 0; i < ivBinary.length; i++) {
            iv[i] = ivBinary.charCodeAt(i);
        }

        // 5. Decrypt using AES-GCM
        const key = await getVaultKey();
        let decryptedBuffer;
        try {
            decryptedBuffer = await self.crypto.subtle.decrypt(
                { name: "AES-GCM", iv: iv },
                key,
                cipherBytes
            );
        } catch (decryptErr) {
            throw new Error("Помилка автентифікації AES-GCM: вміст сейву було модифіковано або підроблено!");
        }

        const jsonStr = new TextDecoder().decode(decryptedBuffer);
        const rawFiles = JSON.parse(jsonStr);

        return writeVirtualSaveFiles(rawFiles);
    } catch (err) {
        log(`[Vault] ❌ Помилка дешифрування: ${err.message}`, '#ef4444');
        self.postMessage({ type: 'SAVE_INTEGRITY_ERROR', message: err.message });
        return 0;
    }
}

async function initServer(initialSaves = null) {
    try {
        log('[Worker] Instantiating MU WebAssembly GameServer...', '#93c5fd');

        Module = await createMuServer({
            locateFile: (filename) => filename,
            print: (text) => log(`[STDOUT] ${text}`, '#94a3b8'),
            printErr: (text) => log(`[STDERR] ${text}`, '#f87171')
        });

        // Restore initial saves into virtual FS before server boot if provided
        if (initialSaves) {
            await importEncryptedSaves(initialSaves);
        }

        // Register outgoing packet callback (Server -> Client)
        const sendCallback = Module.addFunction((ptr, size) => {
            const bytes = new Uint8Array(Module.HEAPU8.subarray(ptr, ptr + size));
            self.postMessage({
                type: 'PACKET_FROM_SERVER',
                data: Array.from(bytes),
                size: size
            });
        }, 'vii');

        Module._SoloServer_SetSendCallback(sendCallback);

        // Register save event callback (Server -> Local Storage auto-save)
        const saveCallback = Module.addFunction(() => {
            exportEncryptedSaves().then(currentSaves => {
                if (currentSaves) {
                    self.postMessage({
                        type: 'AUTO_SAVE_DATA',
                        saves: currentSaves,
                        timestamp: Date.now()
                    });
                }
            });
        }, 'v');

        Module._SoloServer_SetSaveCallback(saveCallback);

        log('[Worker] Initializing Game World and Data Files (/Data)...', '#a78bfa');
        const initResult = Module.ccall('SoloServer_Init', 'number', ['string', 'string'], ['/Data', '/saves']);

        if (initResult !== 1) {
            log('[Worker] ERROR: SoloServer_Init failed!', '#ef4444');
            self.postMessage({ type: 'ERROR', message: 'Server initialization failed' });
            return;
        }

        const monsters = Module._SoloServer_GetMonsterCount();
        const players = Module._SoloServer_GetPlayerCount();

        log(`[Worker] Server running! Total Monsters: ${monsters}, Players: ${players}`, '#34d399');

        // Start server simulation tick loop (20 TPS / 50ms)
        lastTickTime = performance.now();
        tickTimer = setInterval(() => {
            const now = performance.now();
            const elapsedMs = Math.min(200, Math.round(now - lastTickTime));
            lastTickTime = now;
            if (Module) {
                Module._SoloServer_Tick(elapsedMs);
            }
        }, 50);

        self.postMessage({
            type: 'SERVER_READY',
            monsters: monsters,
            players: players
        });
    } catch (err) {
        log(`[Worker] Critical Exception: ${err.message}`, '#ef4444');
        self.postMessage({ type: 'ERROR', message: err.message });
    }
}

self.onmessage = async function (e) {
    const { action, payload } = e.data;

    if (!Module && action !== 'INIT') {
        return;
    }

    switch (action) {
        case 'INIT': {
            await initServer(payload?.saves);
            break;
        }

        case 'CONNECT_PLAYER': {
            const ip = payload?.ip || '127.0.0.1';
            const playerIndex = Module.ccall('SoloServer_ConnectPlayer', 'number', ['string'], [ip]);
            self.postMessage({
                type: 'PLAYER_CONNECTED',
                playerIndex: playerIndex,
                monsters: Module._SoloServer_GetMonsterCount(),
                players: Module._SoloServer_GetPlayerCount()
            });
            break;
        }

        case 'DISCONNECT_PLAYER': {
            const index = payload?.index || 0;
            Module._SoloServer_DisconnectPlayer(index);
            self.postMessage({
                type: 'PLAYER_DISCONNECTED',
                index: index
            });
            break;
        }

        case 'SEND_PACKET': {
            const rawBytes = new Uint8Array(payload.bytes);
            const ptr = Module._malloc(rawBytes.length);
            Module.HEAPU8.set(rawBytes, ptr);
            Module._SoloServer_ReceiveClientPacket(ptr, rawBytes.length);
            Module._free(ptr);
            break;
        }

        case 'EXPORT_SAVES': {
            const saves = await exportEncryptedSaves();
            self.postMessage({
                type: 'SAVES_EXPORTED',
                saves: saves,
                timestamp: Date.now()
            });
            break;
        }

        case 'IMPORT_SAVES': {
            const count = await importEncryptedSaves(payload?.saves);
            self.postMessage({
                type: 'SAVES_IMPORTED',
                count: count
            });
            break;
        }

        case 'GET_STATS': {
            self.postMessage({
                type: 'STATS_UPDATE',
                monsters: Module._SoloServer_GetMonsterCount(),
                players: Module._SoloServer_GetPlayerCount()
            });
            break;
        }
    }
};
