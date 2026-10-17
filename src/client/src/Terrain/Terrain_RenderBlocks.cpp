// Terrain_RenderBlocks.cpp
//
// Culling/render de bloques de terreno y la variante de luz dinámica clampeada.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#ifdef __EMSCRIPTEN__
#include "mu_mesh_draw.h"
#endif
// AddTerrainLightClip @ 0x004F7800 (~74 lines) — adds clamped light to terrain buffer
// Iterates a square region around (xf,yf). Per cell: falloff = (Range-dist)/Range.
// Adds Light * falloff to Buffer, clamps to [0.0, 1.0].
// AddTerrainLightClip (0x004F7800).
//
// No tiene callers en nuestro árbol y no es código muerto: en el binario la
// llama 0x4C0E59, dentro de una función todavía no portada; cuando se porte,
// debe llamar a esta y no a AddTerrainLight.
//
// Diferencia entre las dos: esta clampea a [0, 1]; 0x4F76C0 solo evita negativos
// y deja que la luz supere 1.0 (que es lo que produce el resplandor del fuego).
void __cdecl AddTerrainLightClip(float xf, float yf, float Light[3], int Range, float Buffer[3]) {
    // 0x004F7800 — Add clamped light to terrain light buffer.
    // Iterates a square region of radius Range around (xf,yf) in grid coords.
    // Per cell: falloff = (Range - dist) / Range. Adds Light * falloff to Buffer, clamps [0,1].
    // Buffer is a 256x256 grid of float[3] (RGB), stride 0xC per cell.
    // _DAT_00552594 = world-to-grid scale factor.

    float gridX = xf * _DAT_00552594;
    float gridY = yf * _DAT_00552594;
    int centerX = (int)gridX;
    int centerY = (int)gridY;

    int yStart = centerY - Range;
    int yEnd   = centerY + Range;

    if (yStart > yEnd) return;

    float fY = (float)yStart;
    int iy = yStart;
    do {
        int xStart = centerX - Range;
        int xEnd   = centerX + Range;
        float fX = (float)xStart;
        if (xStart <= xEnd) {
            int ix = xStart;
            do {
                float dx = gridX - fX;
                float dy = gridY - fY;
                float dist = (float)sqrt(dx * dx + dy * dy);
                float falloff = ((float)Range - dist) / (float)Range;

                if (_DAT_00552580 < falloff) {
                    // Index into 256x256 terrain light buffer
                    int cellIdx = ((unsigned int)(iy) & 0xFF) * 0x100 + ((unsigned int)(ix) & 0xFF);
                    float* pCell = Buffer + cellIdx * 3;

                    for (int ch = 0; ch < 3; ch++) {
                        float val = falloff * Light[ch] + pCell[ch];
                        pCell[ch] = val;
                        if (val < _DAT_00552580) {
                            pCell[ch] = 0.0f;
                        } else if (val > _DAT_0055256c) {
                            pCell[ch] = 1.0f;
                        }
                    }
                }

                ix++;
                fX += _DAT_0055256c;  // 1.0f
            } while (ix <= xEnd);
        }
        fY += _DAT_0055256c;  // 1.0f
        iy++;
    } while (iy <= yEnd);
}

#ifdef __EMSCRIPTEN__
// Grass only exists on a solid layer-1 tile. Everything else returns before
// drawing, so the height setup in RenderTerrainTile is wasted work.
static int mu_grass_skip(int x, int y) {
    if (DAT_0814b2dc) return 1;
    int map = (int)gMapManager.GetCurrentMap();
    if (map >= 11 && map <= 16) return 1;
    if ((unsigned)x > 255u || (unsigned)y > 255u) return 0;
    int i0 = y * 256 + x;
    int i3 = i0 + 256 + 1;
    if (i3 >= 256 * 256) return 0;
    if (TerrainMappingAlpha[i0] > 0.f || TerrainMappingAlpha[i0 + 1] > 0.f ||
        TerrainMappingAlpha[i0 + 256] > 0.f || TerrainMappingAlpha[i3] > 0.f)
        return 1;
    int slot = (int)TerrainMappingLayer1[i0] + 50;
    if (!Bitmaps[slot].Buffer) return 1;
    return 0;
}
#endif

// RenderTerrainBlock @ 0x004F9720 (~33 lines) — renders a 4x4 terrain tile block
void __cdecl RenderTerrainBlock(float xf, float yf, int xi, int yi, bool EditFlag) {
    // 0x004F9720 — renders a 4x4 terrain tile block
    // _DAT_00552504 = 0.5f (half-tile center), _DAT_0055256c = 1.0f (tile stride)
    // CameraTopViewEnable = CameraTopViewEnabled
    // RenderTerrainTile (declared with int params but actually takes floats via stack)
    // TestFrustrum2D = TestFrustrum2D
    float startX = xf;
    int row = 0;
    do {
        float centerY = yf + _DAT_00552504;  // 0.5f
        int col = 0;
        xf = startX;
        do {
            unsigned short visible = TestFrustrum2D(xf + _DAT_00552504, centerY, 0.0f);
            if (visible || CameraTopViewEnabled != 0) {
                // RenderTerrainTile(xf, yf, col+xi, row+yi, 1.0f, 1, EditFlag)
                // functions.h declares first 2 params as int; reinterpret float bits
#ifdef __EMSCRIPTEN__
                int skip = 0;
                if (!EditFlag && DAT_0838bc44 == 2 && mu_grass_skip(col + xi, row + yi))
                    skip = 1;
                int began = 0;
                if (!skip && !EditFlag && DAT_0838bc44 == 0)
                    began = mu_tile_begin(col + xi, row + yi);
                if (!skip && began != 1)
#endif
                RenderTerrainTile(*(int*)&xf, *(int*)&yf, col + xi, row + yi, 1.0f, 1, (int)EditFlag);
#ifdef __EMSCRIPTEN__
                if (began == 2)
                    mu_tile_end(col + xi, row + yi);
#endif
            }
            xf = xf + _DAT_0055256c;  // 1.0f
            col++;
        } while (col < 4);
        yf = yf + _DAT_0055256c;
        row++;
    } while (row < 4);
}

// RenderTerrainFrustrum @ 0x004F97E0 (~42 lines) — iterates visible terrain blocks
void __cdecl RenderTerrainFrustrum(bool EditFlag) {
    // 0x004F97E0 — iterates visible terrain blocks in frustum bounds
    // FrustrumBoundMinX_1 = FrustrumBoundMinX_1, FrustrumBoundMinY_1 = FrustrumBoundMinY_1
    // FrustrumBoundMaxX_1 = FrustrumBoundMaxX_1, FrustrumBoundMaxY_1 = FrustrumBoundMaxY_1
    // _DAT_0055264c = 2.0f (block center offset), _DAT_00552650 = 4.0f (block stride)
    // TestFrustrum2D = TestFrustrum2D, CameraTopViewEnable = CameraTopViewEnabled
    int yi = (int)FrustrumBoundMinY_1;  // FrustrumBoundMinY_1
    if (yi <= (int)FrustrumBoundMaxY_1) {  // FrustrumBoundMaxY_1
        float blockY = (float)yi;
        do {
            int maxX = (int)FrustrumBoundMaxX_1;  // FrustrumBoundMaxX_1
            if ((int)FrustrumBoundMinX_1 <= maxX) {
                float centerY = blockY + _DAT_0055264c;  // 2.0f
                int xi = (int)FrustrumBoundMinX_1;  // FrustrumBoundMinX_1
                float blockX = (float)xi;
                do {
                    unsigned short visible = TestFrustrum2D(blockX + _DAT_0055264c, centerY, -40.0f);
                    if (visible || CameraTopViewEnabled != 0) {
                        RenderTerrainBlock(blockX, blockY, xi, yi, EditFlag);
                    }
                    blockX = blockX + _DAT_00552650;  // 4.0f
                    xi += 4;
                    maxX = (int)FrustrumBoundMaxX_1;
                } while (xi <= maxX);
            }
            blockY = blockY + _DAT_00552650;  // 4.0f
            yi += 4;
        } while (yi <= (int)FrustrumBoundMaxY_1);
    }
#ifdef __EMSCRIPTEN__
    if (!EditFlag && DAT_0838bc44 == 0)
        mu_tile_commit();
#endif
}
