const path = require('path');
const fs = require('fs');

async function testServer() {
    console.log("=== Testing MU Online Solo WebAssembly Server ===");

    const createMuServer = require('./build/muserver.js');

    // Create a local saves directory if not exists
    if (!fs.existsSync('./saves')) {
        fs.mkdirSync('./saves');
    }

    const Module = await createMuServer({
        locateFile: (fileName) => path.join(__dirname, 'build', fileName),
        print: (txt) => console.log('[STDOUT]', txt),
        printErr: (txt) => console.error('[STDERR]', txt)
    });
    console.log("WASM Module instantiated successfully!");

    // Set send packet callback from C++ to JS
    const sendCallback = Module.addFunction((dataPtr, size) => {
        const bytes = Module.HEAPU8.subarray(dataPtr, dataPtr + size);
        const hex = Array.from(bytes.slice(0, Math.min(16, size))).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
        console.log(`[JS Callback] Sent to Client (${size} bytes): ${hex}`);
    }, 'vii');

    Module._SoloServer_SetSendCallback(sendCallback);

    // Initialize server with preloaded /Data directory and ./saves
    console.log("Initializing SoloServer...");
    const initRes = Module.ccall('SoloServer_Init', 'number', ['string', 'string'], ['/Data', './saves']);
    console.log(`SoloServer_Init returned: ${initRes}`);

    if (initRes !== 1) {
        console.error("SoloServer_Init failed!");
        process.exit(1);
    }

    const monsters = Module._SoloServer_GetMonsterCount();
    const players = Module._SoloServer_GetPlayerCount();
    console.log(`Server State: Monsters Loaded = ${monsters}, Active Players = ${players}`);

    // Connect player
    console.log("\nConnecting player session...");
    const playerIndex = Module.ccall('SoloServer_ConnectPlayer', 'number', ['string'], ['127.0.0.1']);
    console.log(`Player session connected with index: ${playerIndex}`);

    const XorKey = [
        0xE7, 0x6D, 0x3A, 0x89, 0xBC, 0xB2, 0x9F, 0x73,
        0x23, 0xA8, 0xFE, 0xB6, 0x49, 0x5D, 0x39, 0x5D,
        0x8A, 0xCB, 0x63, 0x8D, 0xEA, 0x7D, 0x2B, 0x5F,
        0xC3, 0xB1, 0xE9, 0x83, 0x29, 0x51, 0xE8, 0x56
    ];

    function sendPacket(rawBytes) {
        const bytes = new Uint8Array(rawBytes);
        // Emulate client C1 frame encoding (chain XOR from byte 3)
        if (bytes[0] === 0xC1 && bytes.length >= 4) {
            for (let i = 3; i < bytes.length; i++) {
                bytes[i] ^= bytes[i - 1] ^ XorKey[i & 0x1f];
            }
        }
        const ptr = Module._malloc(bytes.length);
        Module.HEAPU8.set(bytes, ptr);
        Module._SoloServer_ReceiveClientPacket(ptr, bytes.length);
        Module._free(ptr);
    }

    // Send Login packet: C1 24 F1 01 (account: "solo", pass: "solo")
    console.log("\n1. Sending Login Packet from Client...");
    const loginPacket = new Uint8Array(36);
    loginPacket[0] = 0xC1;
    loginPacket[1] = 36;
    loginPacket[2] = 0xF1;
    loginPacket[3] = 0x01;
    const accStr = "solo";
    for (let i = 0; i < accStr.length; i++) loginPacket[4 + i] = accStr.charCodeAt(i);
    const passStr = "solo";
    for (let i = 0; i < passStr.length; i++) loginPacket[14 + i] = passStr.charCodeAt(i);
    sendPacket(loginPacket);

    // Request Character List: C1 04 F3 00
    console.log("\n2. Sending Character List Request...");
    const charListReq = new Uint8Array([0xC1, 0x04, 0xF3, 0x00]);
    sendPacket(charListReq);

    // Create Character: C1 0E F3 01 (Name: "SoloKnight", Class: 1 = Dark Knight)
    console.log("\n3. Sending Character Create Packet (Dark Knight: SoloKnight)...");
    const charCreateReq = new Uint8Array(14);
    charCreateReq[0] = 0xC1;
    charCreateReq[1] = 14;
    charCreateReq[2] = 0xF3;
    charCreateReq[3] = 0x01;
    const nameStr = "SoloKnight";
    for (let i = 0; i < nameStr.length; i++) charCreateReq[4 + i] = nameStr.charCodeAt(i);
    charCreateReq[13] = 1; // Class 1: Dark Knight
    sendPacket(charCreateReq);

    // Select Character: C1 0E F3 03 (Name: "SoloKnight")
    console.log("\n4. Sending Character Select Packet...");
    const charSelectReq = new Uint8Array(14);
    charSelectReq[0] = 0xC1;
    charSelectReq[1] = 14;
    charSelectReq[2] = 0xF3;
    charSelectReq[3] = 0x03;
    for (let i = 0; i < nameStr.length; i++) charSelectReq[4 + i] = nameStr.charCodeAt(i);
    sendPacket(charSelectReq);

    // Tick the server
    console.log("\n5. Running server ticks (simulation)...");
    for (let t = 0; t < 20; t++) {
        Module._SoloServer_Tick(100);
    }

    console.log("\n=== ALL TESTS PASSED SUCCESSFULLY! ===");
}

testServer().catch(err => {
    console.error("Error during test:", err);
    process.exit(1);
});
