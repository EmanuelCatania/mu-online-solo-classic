# Full port — MU Online 0.97k in the browser

This continues `browser-porting-plan.md`. M0–M5 are done. They proved the pipes: one mesh, the canvas loop, one map, a byte-copy gateway, and five scripted actions. They are not a finished client. M6 puts ArmorClass01 on Lorencia through the real drawer, and M7 walks that body across the grass from a ground click. The inventory panel has no interface art.

100% means the browser runs this client. Same scenes, same parsers, same protocol, same `Data/` files. The Win32 `main.exe` keeps building. The browser target stays in `emscripten_port/`.

## What 100% is

A person can open the page and do what `bin/Client/main.exe` does on this source tree:

- Type an account, pick a server, pass HWID, and land on the character list.
- Create, delete, and enter a character. The four classes this client has (Dark Wizard, Dark Knight, Elf, Magic Gladiator) all enter the world.
- Stand on the map the server sent, in the real body, with the equipped parts, and walk there by clicking the ground.
- Hit a monster, see the damage, die, and respawn. Use the skills that class already has.
- Open inventory, move and equip items, buy and sell, store in the warehouse, trade, mix in the chaos machine, party, guild, and quest.
- Chat, including a line the server accepts and a line that comes back.
- Cross a gate into every map this client already loads.
- Hear the map music and the effect that belongs to the action.
- Change the options the client already saves, close the page, and find them again.

Those screens are the client's own draw, from the same `Data/` files `main.exe` opens. Login, server select, loading, character create, the character list, and inventory match the original layout: the same pictures, the same buttons, the same slots. A DOM field only supplies keys to the buffer the client already reads.

The exit test for the whole plan is one recorded session that does that list on the public server (`mu.server-pups.space`, ConnectServer 44405, GameServer 55901), with those screens visible, and a Win32 configure that still succeeds.

## What 100% is not

A shader rewrite of `BMD_DrawMesh`, a Rust client, a JavaScript protocol, Asyncify around `Sleep`, new gameplay, or a new server. GameGuard stays until a capture shows the server never reads those packets. Immediate mode stays until a populated map is slower than the game can use. M5 measured 10–16 ms on the shell scene, which is not that map.

## Rules that stay

Do not edit `BMD_DrawMesh.cpp` to get a screenshot. Do not stub a game system to make a link succeed. Platform stand-ins stay behind `#ifdef __EMSCRIPTEN__` or in `emscripten_port/browser/`. The gateway copies bytes and does not parse C1/C2/C3/C4. Do not invent an opcode. `CustomerName`, `ServerSerial`, and `ClientVersion` stay. Each milestone has a browser check (the action, not only a link) and a Win32 `cmake -A Win32` afterwards.

## Where the work sits

| Area | Files | What M5 actually did |
|---|---:|---|
| Render | 78 | Axe through `BMD__RenderMesh`. ArmorClass01 did not. Terrain and one UI panel did. |
| UI | 32 | ASCII glyphs. Interface art is not loaded. `GlobalText` is empty, so the client strings do not appear. |
| Net | 32 | Adapter, HWID, replayed login, four live frames (move, attack, chat). |
| Item | 29 | Inventory flag and `RenderInventoryWindow`. No item icons, no equip. |
| Scene | 20 | Not the scene sequence. `WinMain` is not linked. |
| Game | 12 | A shell called `Combat_SendMovePathPacket` and `Action`. |
| Combat | 10 | The attack packet. No monster, no skill. |
| Entity | 11 | A hand-filled pool. |
| Terrain | 9 | World1 tiles, with the light floor raised so the map is visible. |
| Model | 7 | ArmorClass01 opened. Bones for the real drawer are not filled. |
| Sound | 5 | One mp3 and one wav through miniaudio. |
| Input | 5 | Keys and a click reach the shell, not the world tick. |
| Config | 3 | Not read. The registry path has no browser store. |

## Schedule

Estimates match the old plan: one coding agent plus a reviewer, focused work, cumulative tokens.

| Milestone | Exit test | Time | Tokens |
|---|---|---|---|
| M6 Real body | ArmorClass01 on Lorencia through unchanged `BMD__RenderMesh`, textured, one idle frame | 1–2 weeks | 4–8 million |
| M7 Walk | Click the ground, the body plays the walk, the path packet matches the tiles | 1–2 weeks | 4–8 million |
| M8 Enter world | Character list to a server spawn, no shell-placed hero | 2–3 weeks | 6–12 million |
| M9 Client frame | Login, server select, loading, character create, list, and world, with the original pictures | 2–4 weeks | 8–15 million |
| M10 Look of one map | Equipped parts, one monster, one NPC, objects, effects, inventory window with icons, `Text.bmd` | 3–5 weeks | 10–18 million |
| M11 Fight and items | A real hit, loot, equip from the inventory grid, one skill, on Lorencia | 3–5 weeks | 10–18 million |
| M12 The other classes | Dark Knight, Elf, and Magic Gladiator enter and use their own skills | 3–6 weeks | 10–20 million |
| M13 Town | Shop, warehouse, trade, party, guild, chaos mix, quest, chat box | 4–8 weeks | 15–25 million |
| M14 Every map | A gate crossing into each world this client loads | 2–4 weeks | 6–12 million |
| M15 The rest of the client | Options, saved config, every music track, positional effects, message boxes | 2–4 weeks | 6–12 million |
| M16 Measure | Frame time on a populated Lorencia decides whether immediate mode stays | 1–2 weeks | 3–6 million |
| **M6–M16** | | **about 6–12 months** | **about 80–150 million** |

M6 is the gate. If the real drawer still emits nothing after the bone and texture state is filled, stop. That result means `Render/` is a rewrite, and the totals above do not apply. The old plan already priced that rewrite at about 4–9 months and 80–150 million tokens on top.

### M6 — real body

Find why `BMD__RenderMesh` emits no fragments for ArmorClass01. The axe path already draws. The likely gap is the bone buffer the drawer reads, the texture bind, or a GL state the shell leaves off (cull, depth, alpha test). Fill that state the way the client does for a player object. Draw one idle frame on World1.

Done when the white shell triangles are gone and the browser shows the model. Record the fragment count. Do not change `BMD_DrawMesh.cpp`.

Result: passed. `emscripten_port/out/shell_m6.html` is World1. ArmorClass01 is slot 1: 1 mesh, 1 bone, 56 actions, texture `skin_barbarian_01.jpg` in bitmap 301 from `Player/skin_barbarian_01.OZJ`. `BMD_Animation` (action 0, frame 0) and `Skeleton_Transform` write the pose into `g_BoneVertexBuf`. The shell then shifts the body origin so the bbox sits on tile 124,6: (12368.1, 587.6, 165.0)–(12431.9, 612.4, 204.1). `BMD__RenderMesh` is the unchanged drawer, flag 2 (`RENDER_TEXTURE`), alpha 1, blend mesh -1. It submits 150 triangles. Emscripten's immediate-mode renderer never disables the attributes it enabled for the previous batch, so the terrain's texture and normal attributes stay bound to a 128-byte buffer and the 450-vertex draw returns `GL_INVALID_OPERATION` (1282). The shell drops any enabled attribute whose buffer cannot cover the draw. After that the body changes **17641** pixels against the terrain-only frame. The canvas shows the black armor, skin, and gauntlets on the Lorencia grass. Cull stays off the same way the axe proof does (`DAT_083a411c=1`), because `GL_ResetState` only enables cull when that cache is clear. Depth stays off; `GL_ResetState` does not turn it on. `BMD_DrawMesh.cpp` is unchanged. This is the chest piece, not a dressed standing hero. `cmake -A Win32` still configures.

### M7 — walk

A click on the terrain calls `Path_FindRoute` and the client's move. `MoveCharacter` advances the animation. The camera is the one the world tick already updates. The move packet's tiles are the tiles the body crossed.

Done when the body walks across Lorencia grass and the log shows one path packet per walk, not a hand-built single step.

Result: passed. `emscripten_port/out/shell_m7.html` is World1 with ArmorClass01 on tile 124,6. A click on the grass is unprojected with the 640×480 matrices (`glGetFloatv` into `gluUnProject`); the client's own ray is used only when the hit is within 40 tiles, because `Camera_BuildMouseRay` was pointing away from the hero. `Path_FindRoute` wrote 124,6 124,7 124,8 124,9. `Combat_SendMovePathPacket` ran once: 8 bytes, and it set the route byte at entity+748. `CharacterMoveSpeed` returns the ushort at +762 for this body, and that word was 0, so `MoveCharacterPosition` was adding nothing. The shell sets it to 12, the rate the same function returns for a walking player. `Entity_AdvancePath` then `MoveCharacterPosition` carry the body from (12450, 650) to (12450, 950). The log is one packet and `M7 arrived crossed=124,6 124,7 124,8 124,9`. `MoveCharacter` runs every frame. The action stays 0 and the frame stays 0.00: `SetAction` reads the bone count (1) as the action count, so the walk action is rejected. The chest translates; the pose does not cycle. Those animation functions are unchanged. `MoveMainCamera` is the camera. It sits 1000 units south of the body at pitch -48.5 and follows it up the plateau. A `glColor(0,0,0)` left outside `glBegin` had turned the current color black, and these quads do not enable a per-vertex color, so the grass was multiplied to black after the first step. The shell sets white before `RenderTerrain` and the body tint (1, 0.75, 0.55) before the mesh. The canvas shows the chest farther up the Lorencia grass, with the cliff behind it. `BMD_DrawMesh.cpp` is unchanged. `cmake -A Win32` still configures.

### M8 — enter world

The character list from M4 is the start. Choosing DarkWizard sends the enter packet the client already builds. The spawn handler places the hero. Delete the shell's `place_entity`.

Done when the name and the coordinates on screen come from the server packet, and a second login puts the body on that spot without a hardcoded tile.

### M9 — client frame

Stop driving the world from `shell_m5.cpp`. `emscripten_set_main_loop` calls `Game_MainLoop` with the scene flag the client uses for login, server select, character create, character select, loading, and the world. Load `Text.bmd` into `GlobalText` the way `WinMain` does. Load `Enc1.dat` and `Dec2.dat` before the first C3, which the shell already has to do because `WinMain` is not linked. Preload the interface images those scenes already bind.

The page needs a place to type the account, the password, the new character name, and chat. A DOM field writes the same buffer the client reads. Korean IME composition has to survive that field. The pictures, buttons, and layout stay the client's draw.

Done when one session shows, in order, the original login picture, the server list with its panels and names, the loading picture and its progress, the character list, and the world. Creating a character shows the class buttons, the name field, and the 3D preview of that class. The shell does not draw the hero, the HUD text, or a substitute screen.

Result so far: `emscripten_port/out/shell_login.html` is the client loop. `emscripten_set_main_loop` calls `Scene_Dispatch` at 25 fps, the same switch `WinMain` uses. `Scene_Intro` draws the Webzen logo, returns after `SwapBuffers`, and finishes `Scene_LoadGameAssets` on a later frame, so the logo stays up while the files load. The next frames are `Game_MainLoop` with `SceneFlag` 2. `Game_SceneUpdate` runs `Scene_LoadAccountResources`, spawns the ships, and calls `MoveCamera`. The canvas shows the sunset sky, the two login ships, the deck, the ropes, and the gold MU mark. MEMFS is case-sensitive and `fopen` asks for `0Account_new.OZT` / `sky0.OZJ` while the staged names are `0account_new.OZT` / `Sky0.OZJ`. `FS.open` also passes `noent_okay`, so a case miss returns an empty node instead of throwing. The shell's `FS.lookupPath` now walks the real directory names and opens that file. `ozj_open_pixels` feeds libjpeg the main RGB frame (Sky0 and Sky5 are 512×256, Logo and Logo2 are 256×128), not the Photoshop thumbnail. Desktop `glTexImage2D` internal format 4 is rewritten to `GL_RGBA`; format 3 stays a literal 3 so the ship JPGs keep uploading. A click on the MuServer row is the client's hit test (`CServerSelWin_UpdateWhileActive`): the group at (150, 220), then the channel, then `CreateSocket` and sub-state 1. The shell does not set that state. The connecting picture is the client's: "Conectando al servidor." and "Por favor, espere un momento." over the gray message box and the progress quads. `PlayBuffer` plays the button click (`M5 sfx`, 22050 Hz). The copyright and version lines are the glyph atlas at the height `CFont::Create` picks. The connect timer `DAT_083a7ac8` was a `char`, so `GetTickCount` was stored as one byte and the same frame treated the connect as a 15 s timeout. It is a `DWORD` now, and sub-state 1 stays up. The socket opens `ws://127.0.0.1:8091/?host=connect.muonline.co.kr&port=55901`. A 12-byte reply moves the client to sub-state 2. The account plaque is `Logo/0account_new.OZT` (250×216): the stone frame, the input slots, OK, and Exit. The three figures on the deck are still black silhouettes, and the cursor quad is a white 24×24 block. `CSimpleModulus_LoadEncryptionKey` / `LoadDecryptionKey` run from the shell on `Data\Enc1.dat` and `Data\Dec2.dat` before the first C3, so the HWID send no longer divides by a zero modulus. The local wait code 0x13 stays on the account panel until the server answers. A login on the local JoinServer returned F1/01 code 0x01, the client set `SceneFlag` 4, and the canvas is the character-select forest. The account `andrii` reaches that scene with its three characters. The first frame used to abort with a wasm signature mismatch: the cloth-node array helper calls `SpringNode_Ctor` as `void*(void*)` while the function was `void(void*)`. On the browser build the constructor returns its pointer, and the scene stays up. Character-select meshes were sampling texel (0,0): a previous batch left its attributes enabled, and GL_FFP_ONLY does not point the current batch while an array buffer is bound. The draw now restores this batch's position, color, and texcoord, and drops attributes this batch does not use. The name plate uses the 16px atlas line. The canvas shows the forest, the metal frame, the three dressed characters, the NEW CHARACTER plaque, and the name lines. The content catalog arrives in the same burst, ahead of the login result. The browser socket reads 8 KB per event, and a 64 KB queue was dropping the tail, so the panel stayed on the wait state. The queue now keeps the tail and drains it before the next frame. The page is `shell_login.html?t=login56`. A click on a character, then Enter, is the client's own select path: the panel slides, `SceneFlag` becomes 3 for the loading picture, then 5. `Game_CharSelectTick` sends the enter packet, and `Recv_JoinMapServer` loads that character's map. Account `andrii` enters DarkWizard on map 6 at (48, 56): the arena sand, the fences, the banners, the HUD, the monster bodies, and "Welcome 'DarkWizard'!". The chat listbox calls that return `int` are invoked with that signature, so the world frame no longer traps. A terrain JPEG with a bad marker returns from the decoder instead of exiting the page. The whole `Data` tree is in the preload, including NPC, Item, Skill, every world and object folder, and the music and sound packs. The canvas hides the system cursor because the client draws its own, and the right button is delivered as `WM_RBUTTONDOWN` without the browser menu. `OpenMonsterModel` loads those BMDs when a viewport packet spawns a monster, so the arena shows the bodies that were missing while that folder was absent. Character create is the same loop once the client opens that panel. `cmake -A Win32` still configures.

### M10 — look of one map

Preload the files Lorencia already asks for: player part models, the equipped item models, one monster, one NPC, the objects on the tiles around the spawn, the effect textures those spawn, and the interface images `RenderInventoryWindow` binds (the blue panel is those missing images). That includes the item icons and the tooltip art. Raise terrain light only if the real lightmap is still black after the texture format fix from M3. Prefer the file's light.

Done when the browser shows a dressed character, a monster, an NPC, map objects, one effect, and a client string from `GlobalText`. The inventory is the client's window: the original panel, the slot grid, the item icons, and a tooltip.

### M11 — fight and items

Click a real monster. `Action` sends the attack because the target is that monster, in range. The damage packet comes back through `Recv_Combat` and a number is drawn. The monster can die. Walk onto a drop, pick it up, and see its icon in the inventory grid. Move it between slots and equip it from that window. The body changes. Cast one skill the Dark Wizard already has. The skill packet is the one `Combat_DispatchHeroSkillAttack` already builds.

Done when that sequence is visible on Lorencia, the inventory grid shows the icon in the slot it occupies, and the gateway log is only byte counts.

### M12 — the other classes

Repeat M11's enter, walk, hit, and one class skill for Dark Knight, Elf, and Magic Gladiator. Use each class's own body and weapon animation. Do not add skills the 0.97 client does not have.

Done when each class has been entered from the list and has landed its own basic attack and one skill.

### M13 — town

NPC shop, warehouse, player trade, party, guild, chaos mix, and the quest window. Each one already has a recv handler and a panel. Drive the panel from the client's click path. Chat is the box and the input field, a line out and a line back, not a shell `SendChat("hello")`.

Done when each panel opens on Lorencia, one real action succeeds (buy, deposit, trade one item, invite, mix one jewel, accept one quest), and the chat line is echoed.

### M14 — every map

Take a gate the client already uses. Arrive on the next map with that map's terrain, tiles, music cue, and objects. Repeat until every world directory under `Data/` that `OpenTerrainMapping` can load has been entered.

Done when the session log names each world and the canvas shows that world's ground.

### M15 — the rest of the client

`Config.ini` is read and written in MEMFS, and the values that must survive a reload (account, resolution, sound levels) are copied to `localStorage`. The registry read becomes that store. Options change music and effect volume. Every music file `PlayTrack` can name plays on its map. Effect slots that the walk, hit, and UI already call are audible, including the 3D position update. `MessageBoxA` becomes a page dialog that does not freeze the browser. `CreateFileA` keeps the slash rewrite so `Data\...` paths open.

Done when a changed option is still set after a reload, a second map has its own track, and a connect error shows a dialog instead of `PostQuitMessage` killing the page.

### M16 — measure

Stand in Lorencia with the dressed hero, several monsters, effects, and the HUD. Record frame time for a minute of walking and fighting. If the frame stays inside the budget the client already uses (the 40 ms logic step), immediate mode stays and the port is complete. If it does not, the next plan is a buffer path for the meshes that dominate the time. That plan is not part of this one.

## Order

M6 blocks everything visual. M7 blocks "it walks." M8 and M9 block "it is the client." M10–M15 can overlap only when they do not share a half-filled hero. Do not start M11 on the white shell.

## Done

All of these are true:

- The session in "What 100% is" has been done in the browser against the public server, and login, server select, loading, character create, and inventory were the client's own pictures.
- No shell file draws the hero, the HUD, or a stand-in packet.
- `BMD_DrawMesh.cpp` is unchanged.
- The gateway still does not parse frames.
- `cmake -A Win32` configures.
- M16 has a recorded frame time, and immediate mode is either kept or explicitly handed to a later plan.
