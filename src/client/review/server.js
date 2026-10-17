// Local board for checkpoint screenshots.
// Each shot is shots/<id>.png plus shots/<id>.json.
// Bind is localhost only. The json is what a later pass should read.

const http = require('http');
const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

const PORT = Number(process.env.REVIEW_PORT || 8770);
const ROOT = __dirname;
const SHOTS = path.join(ROOT, 'shots');
const PAGE = path.join(ROOT, 'public', 'index.html');
const SCENES = ['login', 'connecting', 'account', 'char-select', 'char-create', 'loading', 'world'];

fs.mkdirSync(SHOTS, { recursive: true });

function gitHead() {
    try {
        return execSync('git rev-parse --short HEAD', {
            cwd: path.resolve(ROOT, '..', '..'),
            encoding: 'utf8',
        }).trim();
    } catch (err) {
        return '';
    }
}

const GIT = gitHead();

function send(res, status, body, type) {
    res.writeHead(status, {
        'content-type': type || 'application/json; charset=utf-8',
        'cache-control': 'no-store',
        'access-control-allow-origin': '*',
        'access-control-allow-headers': 'content-type',
    });
    res.end(body);
}

function readBody(req) {
    return new Promise((resolve, reject) => {
        const chunks = [];
        let size = 0;
        req.on('data', (chunk) => {
            size += chunk.length;
            if (size > 12 * 1024 * 1024) {
                reject(new Error('too large'));
                req.destroy();
                return;
            }
            chunks.push(chunk);
        });
        req.on('end', () => resolve(Buffer.concat(chunks)));
        req.on('error', reject);
    });
}

function safeId(value) {
    const id = String(value || '').trim().toLowerCase();
    if (!/^[a-z0-9][a-z0-9-]{0,40}$/.test(id)) return '';
    return id;
}

function shotPath(id, ext) {
    const file = path.join(SHOTS, id + ext);
    if (path.dirname(file) !== SHOTS) return '';
    return file;
}

function blankMeta(id) {
    return {
        id: id,
        title: '',
        capturedAt: new Date().toISOString(),
        build: '',
        git: GIT,
        page: '',
        scene: 'login',
        sceneFlag: null,
        step: '',
        character: '',
        map: null,
        tile: '',
        canvas: { w: 640, h: 480 },
        note: '',
        updatedAt: '',
    };
}

function loadMeta(id) {
    const file = shotPath(id, '.json');
    if (!file || !fs.existsSync(file)) return blankMeta(id);
    try {
        const parsed = JSON.parse(fs.readFileSync(file, 'utf8'));
        return Object.assign(blankMeta(id), parsed, { id: id });
    } catch (err) {
        return blankMeta(id);
    }
}

function writeMeta(meta) {
    const file = shotPath(meta.id, '.json');
    if (!file) throw new Error('bad id');
    fs.writeFileSync(file, JSON.stringify(meta, null, 2) + '\n');
}

function listShots() {
    const names = fs.readdirSync(SHOTS);
    const ids = new Set();
    for (const name of names) {
        const m = name.match(/^([a-z0-9][a-z0-9-]{0,40})\.(png|json)$/);
        if (m) ids.add(m[1]);
    }
    return Array.from(ids).map((id) => {
        const meta = loadMeta(id);
        meta.hasImage = fs.existsSync(shotPath(id, '.png'));
        return meta;
    }).sort((a, b) => String(a.capturedAt).localeCompare(String(b.capturedAt)));
}

function cleanMeta(input, id) {
    const meta = blankMeta(id);
    const src = input && typeof input === 'object' ? input : {};
    meta.title = String(src.title || '').slice(0, 80);
    meta.capturedAt = String(src.capturedAt || meta.capturedAt).slice(0, 40);
    meta.build = String(src.build || '').slice(0, 40);
    meta.git = String(src.git || GIT).slice(0, 40);
    meta.page = String(src.page || '').slice(0, 200);
    meta.scene = SCENES.indexOf(src.scene) >= 0 ? src.scene : 'login';
    const flag = Number(src.sceneFlag);
    meta.sceneFlag = Number.isFinite(flag) ? flag : null;
    meta.step = String(src.step || '').slice(0, 240);
    meta.character = String(src.character || '').slice(0, 20);
    const map = Number(src.map);
    meta.map = Number.isFinite(map) ? map : null;
    meta.tile = String(src.tile || '').slice(0, 20);
    const w = Number(src.canvas && src.canvas.w);
    const h = Number(src.canvas && src.canvas.h);
    meta.canvas = {
        w: Number.isFinite(w) && w > 0 ? w : 640,
        h: Number.isFinite(h) && h > 0 ? h : 480,
    };
    meta.note = String(src.note || '').slice(0, 4000);
    meta.updatedAt = new Date().toISOString();
    return meta;
}

function decodePng(dataBase64) {
    const raw = String(dataBase64 || '').replace(/^data:image\/png;base64,/, '');
    const buf = Buffer.from(raw, 'base64');
    if (buf.length < 24 || buf[0] !== 0x89 || buf[1] !== 0x50) return null;
    return buf;
}

const server = http.createServer(async (req, res) => {
    const url = new URL(req.url, 'http://127.0.0.1');
    if (req.method === 'OPTIONS') {
        send(res, 204, '');
        return;
    }
    try {
        if (req.method === 'GET' && url.pathname === '/') {
            send(res, 200, fs.readFileSync(PAGE), 'text/html; charset=utf-8');
            return;
        }
        if (req.method === 'GET' && url.pathname === '/api/shots') {
            send(res, 200, JSON.stringify({ git: GIT, scenes: SCENES, shots: listShots() }));
            return;
        }
        const img = url.pathname.match(/^\/shots\/([a-z0-9][a-z0-9-]{0,40})\.png$/);
        if (req.method === 'GET' && img) {
            const file = shotPath(img[1], '.png');
            if (!file || !fs.existsSync(file)) {
                send(res, 404, '{"error":"missing"}');
                return;
            }
            send(res, 200, fs.readFileSync(file), 'image/png');
            return;
        }
        if (req.method === 'POST' && url.pathname === '/api/shots') {
            const body = JSON.parse((await readBody(req)).toString('utf8') || '{}');
            const id = safeId(body.id);
            const png = decodePng(body.pngBase64);
            if (!id || !png) {
                send(res, 400, '{"error":"need id and a png"}');
                return;
            }
            const prev = fs.existsSync(shotPath(id, '.json')) ? loadMeta(id) : null;
            fs.writeFileSync(shotPath(id, '.png'), png);
            const meta = cleanMeta(body, id);
            if (prev && (body.note === undefined || body.note === null || body.note === '')) {
                meta.note = prev.note;
            }
            writeMeta(meta);
            send(res, 200, JSON.stringify(loadMeta(id)));
            return;
        }
        const note = url.pathname.match(/^\/api\/shots\/([a-z0-9][a-z0-9-]{0,40})\/note$/);
        if (req.method === 'POST' && note) {
            const id = note[1];
            if (!fs.existsSync(shotPath(id, '.json')) && !fs.existsSync(shotPath(id, '.png'))) {
                send(res, 404, '{"error":"missing"}');
                return;
            }
            const body = JSON.parse((await readBody(req)).toString('utf8') || '{}');
            const meta = loadMeta(id);
            meta.note = String(body.note || '').slice(0, 4000);
            meta.updatedAt = new Date().toISOString();
            writeMeta(meta);
            send(res, 200, JSON.stringify(meta));
            return;
        }
        send(res, 404, '{"error":"not found"}');
    } catch (err) {
        send(res, 500, JSON.stringify({ error: String(err && err.message || err) }));
    }
});

server.listen(PORT, '127.0.0.1', () => {
    console.log('review http://127.0.0.1:' + PORT + ' git=' + (GIT || 'unknown'));
});
