// Joint_Render.cpp
// FUN_0046c3e0 @ 0x0046c3e0
//
// Joint_RenderAll — renders all active joints as textured triangle strips.
//
// Iterates the joint render pool starting at DAT_07c608b4 (stride 0x2f0
// bytes = 0xbc int-words).  For each active slot (piVar2[-3] != 0):
//   - Selects blend mode based on joint type and blink flag (+0x1be):
//       type < 3 and !blink → GL_SetBlendAdditive (additive)
//       otherwise           → GL_SetBlendSrcAlpha (src-alpha)
//   - Binds the texture via GL_BindTextureSlot(type + 0x48d)
//   - Draws (segCount - 1) quad strips using GL_TRIANGLE_STRIP (glBegin 6):
//       Each segment has 4 vertices (top/bottom of current and next node)
//       with UV coords interpolated along the strip.
//       Colour fades from 1.0 → 0.0 along the strip when !blink.
//
// Pool slot layout (relative to piVar2, which is &slot[3]):
//   piVar2[-3] (byte) — active flag
//   piVar2[-2] (int)  — joint sub-type (0–5)
//   piVar2[-1]        — unused / padding
//   piVar2[0]  (int*) — pointer to model/entity base (for blink flag)
//   piVar2[1]  (int)  — segment count
//   piVar2[2]  (int)  — colour R (float reinterpreted as int)
//   piVar2[3]  (int)  — colour G
//   piVar2[4]  (int)  — colour B
//   piVar2[0x5f..]    — vertex positions (stride 3 floats = 12 bytes)
//                       relative to piVar2+0x5f, each vertex 3 floats
//
// Sub-functions:
//   GL_SetBlendAdditive — GL_SetBlendAdditive (blend type 3)
//   GL_SetBlendSrcAlpha — GL_SetBlendSrcAlpha (blend type 4)
//   GL_BindTextureSlot — GL_BindTextureSlot (texture slot)
//
// Globals:
//   DAT_07c608b4 — joint render pool base (ends at 0x7c72e74)
//   _DAT_0055256c — float 1.0

#include "stdafx.h"
#ifdef __EMSCRIPTEN__
#include "mu_mesh_draw.h"
#endif



/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

// IDA: Trail_RenderAll (0x0046C3E0)
// Pool g_RenderPool_07c608a8 (100 slots × 0x2f0, en globals.cpp); DAT_07c608b4
// es el ancla +12 dentro del slot[0]. Se itera por cuenta explícita.
void Trail_RenderAll(void)

{
  float fVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  int local_8;

  piVar2 = (int*)&DAT_07c608b4;
  for (int slotIdx = 0; slotIdx < 100; ++slotIdx, piVar2 += 0xbc) {
    if ((char)piVar2[-3] != '\0') {
      iVar3 = piVar2[-2];
      if ((*(short *)(*piVar2 + 0x1be) == 0) && (iVar3 < 3)) {
        GL_SetBlendAdditive();
      }
      else {
        GL_SetBlendSrcAlpha();
      }
      if (2 < iVar3) {
        iVar3 = iVar3 + -3;
      }
      if (1 < piVar2[1]) {
        GL_BindTextureSlot(iVar3 + 0x48d);
        local_8 = 0;
        if (piVar2[1] != 1 && -1 < piVar2[1] + -1) {
          piVar4 = piVar2 + 0x5f;
          do {
#ifdef __EMSCRIPTEN__
            {
              float fade0 = _DAT_0055256c;
              if (*(short *)(*piVar2 + 0x1be) == 0)
                fade0 = (float)(piVar2[1] - local_8) / (float)piVar2[1];
              float fade1 = _DAT_0055256c;
              if (*(short *)(*piVar2 + 0x1be) == 0)
                fade1 = (float)((piVar2[1] - local_8) + -1) / (float)piVar2[1];
              int segs = piVar2[1];
              float cr = *(float*)&piVar2[2], cg = *(float*)&piVar2[3], cb = *(float*)&piVar2[4];
              const float* p0 = (const float*)(piVar4 + -0x5a);
              const float* p1 = (const float*)piVar4;
              const float* p2 = (const float*)(piVar4 + 3);
              const float* p3 = (const float*)(piVar4 + -0x57);
              float uv[8] = {
                (float)local_8 / (float)segs, 1.0f,
                (float)local_8 / (float)segs, 0.0f,
                (float)(local_8 + 1) / (float)segs, 0.0f,
                (float)(local_8 + 1) / (float)segs, 1.0f
              };
              float xyz[12] = {
                p0[0], p0[1], p0[2], p1[0], p1[1], p1[2],
                p2[0], p2[1], p2[2], p3[0], p3[1], p3[2]
              };
              float rgba[16] = {
                fade0 * cr, fade0 * cg, fade0 * cb, 1.0f,
                fade0 * cr, fade0 * cg, fade0 * cb, 1.0f,
                fade1 * cr, fade1 * cg, fade1 * cb, 1.0f,
                fade1 * cr, fade1 * cg, fade1 * cb, 1.0f
              };
              if (mu_mesh_emit4(MU_MESH_UV_COLOR, 1, uv, rgba, xyz)) {
                local_8 = local_8 + 1;
                piVar4 = piVar4 + 3;
                continue;
              }
            }
#endif
            glBegin(6);
            fVar1 = _DAT_0055256c;
            if (*(short *)(*piVar2 + 0x1be) == 0) {
              fVar1 = (float)(piVar2[1] - local_8) / (float)piVar2[1];
            }
            // IDA Trail_RenderAll lee el color como FLOAT (`*((float*)v0+2)`):
            // reinterpretar los bits, no convertir int→float.
            glColor3f(fVar1 * *(float*)&piVar2[2],fVar1 * *(float*)&piVar2[3],fVar1 * *(float*)&piVar2[4]);
            iVar3 = piVar2[1];
            glTexCoord2f((float)local_8 / (float)iVar3,1.0f);
            glVertex3fv((const GLfloat*)(piVar4 + -0x5a));
            glTexCoord2f((float)local_8 / (float)iVar3,0);
            glVertex3fv((const GLfloat*)piVar4);
            fVar1 = _DAT_0055256c;
            if (*(short *)(*piVar2 + 0x1be) == 0) {
              fVar1 = (float)((piVar2[1] - local_8) + -1) / (float)piVar2[1];
            }
            // IDA Trail_RenderAll lee el color como FLOAT (`*((float*)v0+2)`):
            // reinterpretar los bits, no convertir int→float.
            glColor3f(fVar1 * *(float*)&piVar2[2],fVar1 * *(float*)&piVar2[3],fVar1 * *(float*)&piVar2[4]);
            local_8 = local_8 + 1;
            iVar3 = piVar2[1];
            glTexCoord2f((float)local_8 / (float)iVar3,0);
            glVertex3fv((const GLfloat*)(piVar4 + 3));
            glTexCoord2f((float)local_8 / (float)iVar3,1.0f);
            glVertex3fv((const GLfloat*)(piVar4 + -0x57));
            glEnd();
            piVar4 = piVar4 + 3;
          } while (local_8 < piVar2[1] + -1);
        }
      }
    }
  }  // end of explicit count loop (advance happens in for-statement)
  return;
}
