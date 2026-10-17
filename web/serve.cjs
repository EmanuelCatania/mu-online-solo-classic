const http = require('http');
const fs = require('fs');
const path = require('path');
const os = require('os');

const PORT = 8088;
const WEB_DIR = __dirname;

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.wasm': 'application/wasm',
    '.data': 'application/octet-stream',
    '.css': 'text/css; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.musav': 'application/json; charset=utf-8',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.jpeg': 'image/jpeg',
    '.mp3': 'audio/mpeg',
    '.wav': 'audio/wav'
};

const server = http.createServer((req, res) => {
    // Strip query string
    const cleanUrl = req.url.split('?')[0];
    if (cleanUrl === '/favicon.ico') {
        res.writeHead(204);
        res.end();
        return;
    }

    let filePath = path.join(WEB_DIR, cleanUrl === '/' ? 'index.html' : cleanUrl);
    const isHead = req.method === 'HEAD';

    // Cross-Origin Isolation headers for WebAssembly & SharedArrayBuffer
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
    res.setHeader('Access-Control-Allow-Origin', '*');

    fs.stat(filePath, (err, stats) => {
        if (err) {
            console.warn('[HTTP 404]', req.url);
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('404 Not Found');
            return;
        }

        // Handle directory index
        if (stats.isDirectory()) {
            filePath = path.join(filePath, 'index.html');
            return fs.stat(filePath, (subErr, subStats) => {
                if (subErr) {
                    res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
                    res.end('404 Not Found (Directory has no index.html)');
                    return;
                }
                serveFileStream(filePath, subStats, res, isHead);
            });
        }

        serveFileStream(filePath, stats, res, isHead);
    });
});

function serveFileStream(filePath, stats, res, isHead) {
    const ext = path.extname(filePath).toLowerCase();
    const contentType = MIME_TYPES[ext] || 'application/octet-stream';

    res.writeHead(200, {
        'Content-Type': contentType,
        'Content-Length': stats.size,
        'Cache-Control': 'no-cache'
    });

    if (isHead) {
        res.end();
        return;
    }

    const stream = fs.createReadStream(filePath);
    stream.on('error', (err) => {
        console.error(`Stream error for ${filePath}:`, err.message);
        if (!res.headersSent) {
            res.writeHead(500, { 'Content-Type': 'text/plain' });
        }
        res.end();
    });
    stream.pipe(res);
}

server.listen(PORT, '0.0.0.0', () => {
    console.log(`MU Online Solo Classic live at:`);
    console.log(`  > Local:   http://localhost:${PORT}/`);
    const nets = os.networkInterfaces();
    for (const name of Object.keys(nets)) {
        for (const net of nets[name]) {
            if (net.family === 'IPv4' && !net.internal) {
                console.log(`  > Network: http://${net.address}:${PORT}/ (${name})`);
            }
        }
    }
});

