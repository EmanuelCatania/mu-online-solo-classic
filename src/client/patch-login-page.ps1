param([string]$Token = "login23")
$ErrorActionPreference = "Stop"
$js = "c:\Work\_Projects\mu_online_game\emscripten_port\out\shell_login.js"
$html = "c:\Work\_Projects\mu_online_game\emscripten_port\out\shell_login.html"

function Replace-Once([string]$path, [string]$old, [string]$new) {
    $text = [IO.File]::ReadAllText($path).Replace("`r`n", "`n").Replace("`r", "`n")
    $old = $old.Replace("`r`n", "`n").Replace("`r", "`n")
    $new = $new.Replace("`r`n", "`n").Replace("`r", "`n")
    $n = ([regex]::Matches($text, [regex]::Escape($old))).Count
    if ($n -ne 1) { throw "expected 1 match in $path, found $n`n----`n$old" }
    $utf8 = New-Object System.Text.UTF8Encoding $false
    [IO.File]::WriteAllText($path, $text.Replace($old, $new), $utf8)
}

function Replace-Count([string]$path, [string]$old, [string]$new, [int]$expect) {
    $text = [IO.File]::ReadAllText($path).Replace("`r`n", "`n").Replace("`r", "`n")
    $old = $old.Replace("`r`n", "`n").Replace("`r", "`n")
    $new = $new.Replace("`r`n", "`n").Replace("`r", "`n")
    $n = ([regex]::Matches($text, [regex]::Escape($old))).Count
    if ($n -ne $expect) { throw "expected $expect matches in $path, found $n`n----`n$old" }
    $utf8 = New-Object System.Text.UTF8Encoding $false
    [IO.File]::WriteAllText($path, $text.Replace($old, $new), $utf8)
}

Replace-Once $js @"
  var _emscripten_glTexImage2D = (target, level, internalFormat, width, height, border, format, type, pixels) => {
      if (true) {
"@ @"
  var muGlFormat = (fmt) => {
      if (fmt == 4) return 0x1908;
      return fmt;
    };
  var _emscripten_glTexImage2D = (target, level, internalFormat, width, height, border, format, type, pixels) => {
      internalFormat = muGlFormat(internalFormat);
      format = muGlFormat(format);
      if (true) {
"@

Replace-Once $js "      GLImmediate.mode = mode;`n      GLImmediate.vertexCounter = 0;" @"
      if (Module._muBatchOpen && !Module._muBatchDrawing && Module._mu_mesh_flush) {
        Module._muBatchDrawing = 1;
        Module._mu_mesh_flush();
        Module._muBatchDrawing = 0;
      }
      GLImmediate.mode = mode;
      GLImmediate.vertexCounter = 0;
      GLImmediate.muColorArmed = false;
      GLImmediate.muColorFirst = false;
      GLImmediate.muColorThisVertex = false;
"@

Replace-Count $js "      GLImmediate.matricesModified = true;" @"
      if (Module._muBatchOpen && !Module._muBatchDrawing && Module._mu_mesh_flush) {
        Module._muBatchDrawing = 1;
        Module._mu_mesh_flush();
        Module._muBatchDrawing = 0;
      }
      GLImmediate.matricesModified = true;
"@ 7

Replace-Once $js @"
        GLImmediate.vertexCounter++;
        GLImmediate.addRendererComponent(GLImmediate.COLOR, 4, GLctx.UNSIGNED_BYTE);
"@ @"
        GLImmediate.vertexCounter++;
        if (!GLImmediate.rendererComponents[GLImmediate.COLOR]) {
          GLImmediate.muColorFirst = GLImmediate.rendererComponentPointer === 0;
        }
        GLImmediate.clientColor[0] = r;
        GLImmediate.clientColor[1] = g;
        GLImmediate.clientColor[2] = b;
        GLImmediate.clientColor[3] = a;
        GLImmediate.muColorArmed = true;
        GLImmediate.muColorThisVertex = true;
        GLImmediate.addRendererComponent(GLImmediate.COLOR, 4, GLctx.UNSIGNED_BYTE);
"@

Replace-Once $js @"
            if (GLImmediate.enabledClientAttributes[GLImmediate.COLOR]) {
              var colorAttr = clientAttributes[GLImmediate.COLOR];
              if (!GLctx.currentArrayBufferBinding) {
                GLctx.vertexAttribPointer(GLImmediate.COLOR, colorAttr.size, colorAttr.type, true, GLImmediate.stride, colorAttr.offset);
              }
            }
"@ @"
            if (GLImmediate.enabledClientAttributes[GLImmediate.COLOR]) {
              var colorAttr = clientAttributes[GLImmediate.COLOR];
              if (!GLctx.currentArrayBufferBinding) {
                GLctx.vertexAttribPointer(GLImmediate.COLOR, colorAttr.size, colorAttr.type, true, GLImmediate.stride, colorAttr.offset);
              }
            } else if (GLImmediate.clientColor) {
              GLctx.disableVertexAttribArray(GLImmediate.COLOR);
              GLctx.vertexAttrib4fv(GLImmediate.COLOR, GLImmediate.clientColor);
            }
"@

Replace-Once $js @"
          } else if (pname == 0xBC2) { // GL_ALPHA_TEST_REF
            HEAPF32[((params)>>2)] = GLEmulation.alphaTestRef;
          } else {
            orig_glGetFloatv(pname, params);
          }
"@ @"
          } else if (pname == 0xBC2) { // GL_ALPHA_TEST_REF
            HEAPF32[((params)>>2)] = GLEmulation.alphaTestRef;
          } else if (pname == 0xB00) { // GL_CURRENT_COLOR
            var c = GLImmediate.clientColor;
            HEAPF32[((params)>>2)] = c[0];
            HEAPF32[(((params)+(4))>>2)] = c[1];
            HEAPF32[(((params)+(8))>>2)] = c[2];
            HEAPF32[(((params)+(12))>>2)] = c[3];
          } else {
            orig_glGetFloatv(pname, params);
          }
"@

Replace-Once $js @"
  var _emscripten_glTexCoord2i = (u, v) => {
      assert(GLImmediate.mode >= 0); // must be in begin/end
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = u;
"@ @"
  var muEmitRepeatColor = () => {
      if (!GLImmediate.muColorArmed || !GLImmediate.muColorFirst || GLImmediate.muColorThisVertex) return;
      var start = GLImmediate.vertexCounter << 2;
      var c = GLImmediate.clientColor;
      GLImmediate.vertexDataU8[start + 0] = c[0] * 255;
      GLImmediate.vertexDataU8[start + 1] = c[1] * 255;
      GLImmediate.vertexDataU8[start + 2] = c[2] * 255;
      GLImmediate.vertexDataU8[start + 3] = c[3] * 255;
      GLImmediate.vertexCounter++;
      GLImmediate.muColorThisVertex = true;
    };
  var _emscripten_glTexCoord2i = (u, v) => {
      assert(GLImmediate.mode >= 0); // must be in begin/end
      muEmitRepeatColor();
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = u;
"@

Replace-Once $js @"
  var _emscripten_glVertex2f = (x, y) => {
      assert(GLImmediate.mode >= 0); // must be in begin/end
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
"@ @"
  var _emscripten_glVertex2f = (x, y) => {
      assert(GLImmediate.mode >= 0); // must be in begin/end
      muEmitRepeatColor();
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
"@

Replace-Once $js @"
      GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, GLctx.FLOAT);
    };
  var _glVertex2f = _emscripten_glVertex2f;
"@ @"
      GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, GLctx.FLOAT);
      GLImmediate.muColorThisVertex = false;
    };
  var _glVertex2f = _emscripten_glVertex2f;
"@

Replace-Once $js @"
  var _emscripten_glVertex3f = (x, y, z) => {
      assert(GLImmediate.mode >= 0); // must be in begin/end
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
"@ @"
  var _emscripten_glVertex3f = (x, y, z) => {
      assert(GLImmediate.mode >= 0); // must be in begin/end
      muEmitRepeatColor();
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
"@

Replace-Once $js @"
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = z;
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = 1;
      assert(GLImmediate.vertexCounter << 2 < GL.MAX_TEMP_BUFFER_SIZE);
      GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, GLctx.FLOAT);
    };
  var _glVertex3f = _emscripten_glVertex3f;
"@ @"
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = z;
      GLImmediate.vertexData[GLImmediate.vertexCounter++] = 1;
      assert(GLImmediate.vertexCounter << 2 < GL.MAX_TEMP_BUFFER_SIZE);
      GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, GLctx.FLOAT);
      GLImmediate.muColorThisVertex = false;
    };
  var _glVertex3f = _emscripten_glVertex3f;
"@

Replace-Once $js @"
              case GL_ALPHA_SCALE:
                if (env.alphaScale != param) {
                  env.invalidateKey();
                  env.alphaScale = param;
                }
                break;
  
              default:
                err('WARNING: Unhandled ``pname`` in call to ``glTexEnvf``.');
"@ @"
              case GL_ALPHA_SCALE:
                if (env.alphaScale != param) {
                  env.invalidateKey();
                  env.alphaScale = param;
                }
                break;
              case GL_TEXTURE_ENV_MODE:
                if (env.mode != param) {
                  env.invalidateKey();
                  env.mode = param;
                }
                break;
  
              default:
                err('WARNING: Unhandled ``pname`` in call to ``glTexEnvf``.');
"@

Replace-Once $js "GLImmediate.MAX_TEXTURES = Math.min(maxTextureUnits, 28);" "GLImmediate.MAX_TEXTURES = Math.min(maxTextureUnits, 4);"

Replace-Once $js @"
                  var attribLoc = GLImmediate.TEXTURE0+i;
                  var texAttr = clientAttributes[attribLoc];
                  if (texAttr.size) {
                    GLctx.vertexAttribPointer(attribLoc, texAttr.size, texAttr.type, false, GLImmediate.stride, texAttr.offset);
                  } else {
                    // These two might be dangerous, but let's try them.
                    GLctx.vertexAttrib4f(attribLoc, 0, 0, 0, 1);
                  }
"@ @"
                  var attribLoc = GLImmediate.TEXTURE0+i;
                  if (!GLImmediate._muMaxAttrib) GLImmediate._muMaxAttrib = GLctx.getParameter(GLctx.MAX_VERTEX_ATTRIBS);
                  if (attribLoc < GLImmediate._muMaxAttrib) {
                  var texAttr = clientAttributes[attribLoc];
                  if (texAttr.size) {
                    GLctx.vertexAttribPointer(attribLoc, texAttr.size, texAttr.type, false, GLImmediate.stride, texAttr.offset);
                  } else {
                    // These two might be dangerous, but let's try them.
                    GLctx.vertexAttrib4f(attribLoc, 0, 0, 0, 1);
                  }
                  }
"@

Replace-Once $js "  var _emscripten_glTexParameteri = (x0, x1, x2) => GLctx.texParameteri(x0, x1, x2);" @"
  var _emscripten_glTexParameteri = (x0, x1, x2) => {
      if (x1 == 0x1004) return;
      if (x2 == 0x2900 || x2 == 0x812D) x2 = 0x812F;
      GLctx.texParameteri(x0, x1, x2);
    };
"@

Replace-Once $js @"
  var _emscripten_glTexParameterfv = (target, pname, params) => {
      var param = HEAPF32[((params)>>2)];
      GLctx.texParameterf(target, pname, param);
    };
"@ @"
  var _emscripten_glTexParameterfv = (target, pname, params) => {
      if (pname == 0x1004) return;
      var param = HEAPF32[((params)>>2)];
      if (param == 0x2900 || param == 0x812D) param = 0x812F;
      GLctx.texParameterf(target, pname, param);
    };
"@

Replace-Once $html "      canvas { background: #000; display: block; margin-top: 8px; }" @"
      html, body { margin: 0; width: 100%; height: 100%; background: #000; overflow: hidden; cursor: default; }
      #status, #log { display: none; }
      canvas { background: #000; position: fixed; left: 0; top: 0; z-index: 2; outline: none; border: 0; cursor: none; }
      canvas:focus, canvas:focus-visible { outline: none !important; }
"@

Replace-Once $html "      var Module = {" @"
      window.onerror = function (msg, src, line, col, err) {
        logElement.textContent += 'ONERR ' + msg + '\n' + (err && err.stack ? err.stack : '') + '\n';
      };
      var Module = {
"@

Replace-Once $html "        preRun: [function () {" @"
        locateFile: function (path) { return path + '?t=$Token'; },
        preRun: [function () {
"@

Replace-Once $html '    <script async type="text/javascript" src="shell_login.js"></script>' "    <script async type=`"text/javascript`" src=`"shell_login.js?t=$Token`"></script>"

Write-Output "PATCH_OK $Token"
