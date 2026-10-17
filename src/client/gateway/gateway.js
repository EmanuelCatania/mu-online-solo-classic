// Byte copy between one WebSocket and one TCP socket.
// Replay mode sends a captured byte string. It does not read C1/C2/C3/C4.

const http = require('http');
const crypto = require('crypto');
const net = require('net');
const fs = require('fs');

const MAGIC = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11';
// The page asks for the hostname baked into the client. The TCP side is the
// server listening on this machine (ConnectServer 44405, GameServer 55901).
const LOCAL_HOST = '127.0.0.1';
const ALLOW_PORT = new Set([44405, 55901]);

function arg(name) {
    const i = process.argv.indexOf(name);
    return i >= 0 ? process.argv[i + 1] : '';
}

function loadCapture(path) {
    const steps = [];
    if (!path) return steps;
    const text = fs.readFileSync(path, 'utf8');
    for (const line of text.split(/\r?\n/)) {
        const s = line.trim();
        if (!s || s.startsWith('#')) continue;
        const parts = s.split(/\s+/);
        if (parts.length < 3) continue;
        steps.push({
            port: Number(parts[0]),
            when: parts[1],
            bytes: Buffer.from(parts[2], 'hex'),
        });
    }
    return steps;
}

const capture = loadCapture(arg('--replay'));

function sendFrame(socket, opcode, payload) {
    const len = payload.length;
    let hdr;
    if (len < 126) {
        hdr = Buffer.from([0x80 | opcode, len]);
    } else if (len <= 0xffff) {
        hdr = Buffer.alloc(4);
        hdr[0] = 0x80 | opcode;
        hdr[1] = 126;
        hdr.writeUInt16BE(len, 2);
    } else {
        hdr = Buffer.alloc(10);
        hdr[0] = 0x80 | opcode;
        hdr[1] = 127;
        hdr.writeUInt32BE(0, 2);
        hdr.writeUInt32BE(len, 6);
    }
    if (!socket.destroyed) socket.write(Buffer.concat([hdr, payload]));
}

function readFrames(state, chunk, onBinary) {
    state.buf = Buffer.concat([state.buf, chunk]);
    while (state.buf.length >= 2) {
        const op = state.buf[0] & 0x0f;
        const masked = (state.buf[1] & 0x80) !== 0;
        let len = state.buf[1] & 0x7f;
        let off = 2;
        if (len === 126) {
            if (state.buf.length < 4) return;
            len = state.buf.readUInt16BE(2);
            off = 4;
        } else if (len === 127) {
            state.socket.destroy();
            return;
        }
        const maskLen = masked ? 4 : 0;
        if (state.buf.length < off + maskLen + len) return;
        let payload = state.buf.subarray(off + maskLen, off + maskLen + len);
        if (masked) {
            const mask = state.buf.subarray(off, off + 4);
            const raw = Buffer.from(payload);
            for (let i = 0; i < raw.length; i++) raw[i] ^= mask[i & 3];
            payload = raw;
        }
        state.buf = state.buf.subarray(off + maskLen + len);
        if (op === 8) {
            state.socket.end();
            return;
        }
        if (op === 9) {
            sendFrame(state.socket, 0x0a, payload);
            continue;
        }
        if (op === 1) {
            state.socket.end();
            return;
        }
        if (op === 2) onBinary(payload);
    }
}

function startReplay(port, socket) {
    const steps = capture.filter((s) => s.port === port);
    let i = 0;
    let clientDone = false;
    function kick(reason) {
        while (i < steps.length) {
            const step = steps[i];
            if (step.when === reason) {
                i++;
                sendFrame(socket, 2, step.bytes);
                console.log('replay port', port, reason, step.bytes.length);
                continue;
            }
            if (step.when[0] === '+' && (reason === 'client' || reason === 'timer' || reason === 'open')) {
                const ms = Number(step.when.slice(1));
                const bytes = step.bytes;
                i++;
                setTimeout(() => {
                    if (!socket.destroyed) {
                        sendFrame(socket, 2, bytes);
                        console.log('replay port', port, 'delay', bytes.length);
                        kick('timer');
                    }
                }, ms);
                return;
            }
            return;
        }
    }
    return {
        open() { kick('open'); },
        client(payload) {
            if (clientDone) return;
            if (!payload.length || payload.length > 0x2001) return;
            clientDone = true;
            kick('client');
        },
    };
}

function startLive(host, port, socket) {
    const tcp = net.connect({ host, port });
    tcp.on('data', (d) => {
        const step = 16384;
        for (let off = 0; off < d.length; off += step)
            sendFrame(socket, 2, d.subarray(off, Math.min(off + step, d.length)));
    });
    tcp.on('error', (err) => {
        console.log('tcp error', err.message);
        socket.destroy();
    });
    tcp.on('close', () => socket.end());
    socket.on('close', () => tcp.destroy());
    return {
        open() {},
        client(payload) {
            if (!payload.length || payload.length > 0x2001) return;
            if (!tcp.destroyed) tcp.write(payload);
        },
    };
}

const server = http.createServer((req, res) => {
    res.writeHead(200, { 'content-type': 'text/plain' });
    res.end('mu gateway\n');
});

server.on('upgrade', (req, socket) => {
    const key = req.headers['sec-websocket-key'];
    if (!key) {
        socket.destroy();
        return;
    }
    const accept = crypto.createHash('sha1').update(key + MAGIC).digest('base64');
    socket.write(
        'HTTP/1.1 101 Switching Protocols\r\n' +
        'Upgrade: websocket\r\n' +
        'Connection: Upgrade\r\n' +
        'Sec-WebSocket-Accept: ' + accept + '\r\n\r\n'
    );
    socket.on('error', (err) => {
        console.log('ws error', err.message);
        socket.destroy();
    });
    const url = new URL(req.url, 'http://127.0.0.1');
    const host = url.searchParams.get('host') || '';
    const port = Number(url.searchParams.get('port') || '0');
    console.log('ws', host, port, '->', LOCAL_HOST);
    let session;
    if (capture.length) {
        session = startReplay(port, socket);
    } else if (host && ALLOW_PORT.has(port)) {
        session = startLive(LOCAL_HOST, port, socket);
    } else {
        console.log('refused', host, port);
        socket.end();
        return;
    }
    const state = { buf: Buffer.alloc(0), socket };
    socket.on('data', (chunk) => {
        readFrames(state, chunk, (payload) => session.client(payload));
    });
    session.open();
});

server.listen(8091, '0.0.0.0', () => {
    console.log(capture.length ? 'GATEWAY replay 0.0.0.0:8091' : 'GATEWAY live 0.0.0.0:8091');
});
