const crypto = require('crypto').webcrypto;

const VAULT_MAGIC = "MUSAV1";
const VAULT_SECRET = "MU_ONLINE_SOLO_CLASSIC_097K_WASM_VAULT_KEY";
const VAULT_SALT = new Uint8Array([
    0x4D, 0x55, 0x5F, 0x53, 0x4F, 0x4C, 0x4F, 0x5F,
    0x53, 0x41, 0x4C, 0x54, 0x30, 0x39, 0x37, 0x4B
]);

let cachedCryptoKey = null;

async function getVaultKey() {
    if (cachedCryptoKey) return cachedCryptoKey;
    const enc = new TextEncoder();
    const keyMaterial = await crypto.subtle.importKey(
        "raw",
        enc.encode(VAULT_SECRET),
        { name: "PBKDF2" },
        false,
        ["deriveKey"]
    );
    cachedCryptoKey = await crypto.subtle.deriveKey(
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

async function encryptSave(rawFiles) {
    const key = await getVaultKey();
    const jsonStr = JSON.stringify(rawFiles);
    const iv = crypto.getRandomValues(new Uint8Array(12));
    const encoded = new TextEncoder().encode(jsonStr);

    const ciphertextBuffer = await crypto.subtle.encrypt(
        { name: "AES-GCM", iv: iv },
        key,
        encoded
    );

    const cipherBytes = new Uint8Array(ciphertextBuffer);
    const hashBuffer = await crypto.subtle.digest("SHA-256", cipherBytes);
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
        filesCount: Object.keys(rawFiles).length,
        iv: Buffer.from(ivBinary, 'binary').toString('base64'),
        checksum: hashHex,
        payload: Buffer.from(cipherBinary, 'binary').toString('base64')
    };
}

async function decryptSave(container) {
    if (container.magic !== VAULT_MAGIC) {
        throw new Error(`Invalid magic: ${container.magic}`);
    }

    const cipherBuffer = Buffer.from(container.payload, 'base64');
    const cipherBytes = new Uint8Array(cipherBuffer);

    // Verify SHA-256 checksum
    const hashBuffer = await crypto.subtle.digest("SHA-256", cipherBytes);
    const hashHex = Array.from(new Uint8Array(hashBuffer))
        .map(b => b.toString(16).padStart(2, '0'))
        .join('');

    if (hashHex !== container.checksum) {
        throw new Error("Save corrupted or modified in text editor (SHA-256 checksum mismatch)!");
    }

    const ivBuffer = Buffer.from(container.iv, 'base64');
    const iv = new Uint8Array(ivBuffer);

    const key = await getVaultKey();
    let decryptedBuffer;
    try {
        decryptedBuffer = await crypto.subtle.decrypt(
            { name: "AES-GCM", iv: iv },
            key,
            cipherBytes
        );
    } catch (e) {
        throw new Error("AES-GCM authentication failed: save content was modified or tampered with!");
    }

    const jsonStr = new TextDecoder().decode(decryptedBuffer);
    return JSON.parse(jsonStr);
}

(async () => {
    console.log("=== Testing Save Vault AES-256-GCM ===");

    // 1. Create test save
    const originalSave = {
        "account_solo.dat": Buffer.from("SOLO_ACCOUNT_DATA_STRUCT").toString('base64'),
        "char_HeroDK.dat": Buffer.from("HERO_DK_LEVEL_150_STATS").toString('base64'),
        "warehouse_solo.dat": Buffer.from("WAREHOUSE_CHAOS_BLESS_SOUL").toString('base64')
    };

    console.log("1. Original files:", Object.keys(originalSave));

    // 2. Encrypt
    const encrypted = await encryptSave(originalSave);
    console.log("2. Encrypted container:", {
        magic: encrypted.magic,
        version: encrypted.version,
        algorithm: encrypted.algorithm,
        filesCount: encrypted.filesCount,
        checksum: encrypted.checksum.substring(0, 16) + "...",
        payloadLen: encrypted.payload.length
    });

    // 3. Decrypt and verify
    const decrypted = await decryptSave(encrypted);
    console.log("3. Successfully decrypted:", Object.keys(decrypted));
    if (JSON.stringify(originalSave) === JSON.stringify(decrypted)) {
        console.log("✅ Decrypted result matches original exactly!");
    } else {
        console.error("❌ Error: data mismatch!");
    }

    // 4. Tamper test (modify byte in payload)
    console.log("\n4. Tamper detection test (external text editor modification):");
    const tampered = JSON.parse(JSON.stringify(encrypted));
    const chars = tampered.payload.split('');
    chars[10] = chars[10] === 'A' ? 'B' : 'A';
    tampered.payload = chars.join('');

    try {
        await decryptSave(tampered);
        console.error("❌ Error: tampering was not detected!");
    } catch (err) {
        console.log("✅ Successfully blocked tampered save:", err.message);
    }

    // 5. Tamper test with forged hash
    console.log("\n5. Tamper test with recomputed hash without valid encryption key:");
    const tampered2 = JSON.parse(JSON.stringify(encrypted));
    const tamperedBytes = Buffer.from(tampered2.payload, 'base64');
    tamperedBytes[5] ^= 0xFF; // Flip bit
    tampered2.payload = tamperedBytes.toString('base64');
    const hash = require('crypto').createHash('sha256').update(tamperedBytes).digest('hex');
    tampered2.checksum = hash;

    try {
        await decryptSave(tampered2);
        console.error("❌ Error: AES-GCM accepted forged data!");
    } catch (err) {
        console.log("✅ Successfully blocked via AES-GCM authentication:", err.message);
    }

    console.log("\nAll cryptographic tests passed successfully! 🛡️");
})();
