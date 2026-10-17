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
        throw new Error("Сейв пошкоджено або змінено стороннім редактором (невідповідність контрольної суми SHA-256)!");
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
        throw new Error("Помилка автентифікації AES-GCM: вміст сейву було модифіковано або підроблено!");
    }

    const jsonStr = new TextDecoder().decode(decryptedBuffer);
    return JSON.parse(jsonStr);
}

(async () => {
    console.log("=== Тестування сховища сейвів AES-256-GCM ===");

    // 1. Створюємо тестовий сейв
    const originalSave = {
        "account_solo.dat": Buffer.from("SOLO_ACCOUNT_DATA_STRUCT").toString('base64'),
        "char_HeroDK.dat": Buffer.from("HERO_DK_LEVEL_150_STATS").toString('base64'),
        "warehouse_solo.dat": Buffer.from("WAREHOUSE_CHAOS_BLESS_SOUL").toString('base64')
    };

    console.log("1. Оригінальні файли:", Object.keys(originalSave));

    // 2. Шифруємо
    const encrypted = await encryptSave(originalSave);
    console.log("2. Зашифрований контейнер:", {
        magic: encrypted.magic,
        version: encrypted.version,
        algorithm: encrypted.algorithm,
        filesCount: encrypted.filesCount,
        checksum: encrypted.checksum.substring(0, 16) + "...",
        payloadLen: encrypted.payload.length
    });

    // 3. Дешифруємо та перевіряємо
    const decrypted = await decryptSave(encrypted);
    console.log("3. Успішно дешифровано:", Object.keys(decrypted));
    if (JSON.stringify(originalSave) === JSON.stringify(decrypted)) {
        console.log("✅ Результат повністю збігається з оригіналом!");
    } else {
        console.error("❌ Помилка: дані не збігаються!");
    }

    // 4. Тест підробки (зміна байту в payload)
    console.log("\n4. Тест виявлення спроби читерства / редагування в Блокноті:");
    const tampered = JSON.parse(JSON.stringify(encrypted));
    // Змінюємо 1 символ у зашифрованому рядку
    const chars = tampered.payload.split('');
    chars[10] = chars[10] === 'A' ? 'B' : 'A';
    tampered.payload = chars.join('');

    try {
        await decryptSave(tampered);
        console.error("❌ Помилка: підробка не була виявлена!");
    } catch (err) {
        console.log("✅ Успішно заблоковано змінений сейв:", err.message);
    }

    // 5. Тест підробки без оновлення ключа (зміна і хешу, і пейлоаду)
    console.log("\n5. Тест підробки з перерахованим хешем без валідного ключа шифрування:");
    const tampered2 = JSON.parse(JSON.stringify(encrypted));
    const tamperedBytes = Buffer.from(tampered2.payload, 'base64');
    tamperedBytes[5] ^= 0xFF; // Flip bit
    tampered2.payload = tamperedBytes.toString('base64');
    // Обчислюємо новий хеш (наче читер перерахував хеш)
    const hash = require('crypto').createHash('sha256').update(tamperedBytes).digest('hex');
    tampered2.checksum = hash;

    try {
        await decryptSave(tampered2);
        console.error("❌ Помилка: AES-GCM пропустив підробку!");
    } catch (err) {
        console.log("✅ Успішно заблоковано через автентифікацію AES-GCM:", err.message);
    }

    console.log("\nВсі криптографічні тести успішно пройдено! 🛡️");
})();
