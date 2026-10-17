const fs = require('fs');
const pngPath = process.argv[2];
const metaPath = process.argv[3];
const meta = JSON.parse(fs.readFileSync(metaPath, 'utf8'));
meta.pngBase64 = fs.readFileSync(pngPath).toString('base64');
fetch('http://127.0.0.1:8770/api/shots', {
    method: 'POST',
    headers: { 'content-type': 'application/json' },
    body: JSON.stringify(meta),
}).then(async (res) => {
    const text = await res.text();
    if (!res.ok) {
        console.error(res.status, text);
        process.exit(1);
    }
    const saved = JSON.parse(text);
    console.log(saved.id, saved.scene, saved.sceneFlag, saved.build);
}).catch((err) => {
    console.error(err);
    process.exit(1);
});
