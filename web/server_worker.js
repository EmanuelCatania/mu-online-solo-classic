// Web Worker hosting the compiled MU Online Solo GameServer WebAssembly module
importScripts('muserver.js');

let Module = null;
let tickTimer = null;
let lastTickTime = performance.now();

function log(msg, color = '#60a5fa') {
    self.postMessage({ type: 'LOG', message: msg, color: color });
}

async function initServer() {
    try {
        log('[Worker] Instantiating MU WebAssembly GameServer...', '#93c5fd');

        Module = await createMuServer({
            locateFile: (filename) => filename,
            print: (text) => log(`[STDOUT] ${text}`, '#94a3b8'),
            printErr: (text) => log(`[STDERR] ${text}`, '#f87171')
        });

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

self.onmessage = function (e) {
    const { action, payload } = e.data;

    if (!Module && action !== 'INIT') {
        return;
    }

    switch (action) {
        case 'INIT': {
            initServer();
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
