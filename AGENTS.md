# TP1 Algoritmos — Battle Zone

## Build & run

```sh
make          # gcc -Wall -Werror -std=c99 -pedantic -g $(sdl2-config --cflags --libs) -lm
make clean    # rm -f *.o battlezone
./battlezone  # requires modelos.stl in CWD
```

- **`-Werror` is on** — any warning = build failure. Fix warnings before building.
- No test framework, no linter, no typecheck.
- Single binary, flat directory, no header-only units.
- `EJ1.c` `EJ2.c` `EJ3.c` are standalone exercises **not compiled by `make`**.

## Architecture

| File | Role |
|------|------|
| `main.c` | SDL init, game loop, render (camera, HUD, crosshair, compass). All game logic delegated to `juego.c`. |
| `juego.c/h` | **TDA juego**: game state (tanks, obstacles, models, stack), init, events, update (physics, collisions, enemy AI, respawn, score). Exposes accessors for rendering. |
| `modelo.c/h` | Opaque 3D wireframe model type (copies arrays on create) |
| `matriz.c/h` | Opaque 4×4 matrix algebra (row-major, homogenous coords) |
| `tanque.c/h` | Tank state & movement mechanics (position, heading, turret, missile). Movement uses clamped dt for precision. |
| `obstaculo.c/h` | Static obstacle (position + model pointer, not owned) |
| `stl.c/h` | Binary STL reader (cátedra format, not standard STL) |
| `pila.c/h` | Generic `void*` stack (used for transform matrix stack) |
| `lista.c/h` | Generic linked list (used for model catalog in `juego.c`) |
| `cola.c/h` | Generic queue (used during model loading in `juego.c`) |
| `modelos.stl` | Binary file with all 3D models (TANQUE, TORRETA, RADAR, MISIL, HORIZONTE, MONTANA, LUNA, CUBO1-3, PIRAMIDE1-3, letters A-Z, digits 0-9, `*`, `#`, RESTO1, RESTO2) |

See `guia.md` for detailed architecture walkthrough.

## Gotchas

- All types are opaque — use accessors, never dereference structs.
- `modelo_crear` copies input arrays; caller may `free` immediately.
- `matriz_crear_mc` ≡ `matriz_crear_mz` (rotation in Z/XY plane).
- `_matriz_crear` is internal allocator (prefix `_` signals internal use).
- STL model named `"MONTANA"` (no S), not `"MONTANAS"`.
- `crear_tanque_enemigo` returns `NULL` if spawn overlaps an obstacle (< 5 units).
- Pila cleanup: must `matriz_destruir(pila_desapilar(stack))` *before* `pila_destruir(stack)` to free the identity matrix at bottom.
- M_PI defined locally as 3.14 in `main.c`.
- Game world: x,y ∈ [-150, 150], player at (0,0) heading π/2 (+Y), 4 lives.
- FPS target: 24. Prints `"Perdiendo cuadros"` if frame overruns.
- Tank collision radius: 5 units (check `dx² + dy² < 36`). Missile collision: 3 units.
- Enemy respawns at exactly 50 units from player.
- `juego_modelo(j, name)` accepts model names as string literals; for single-char names pass `(char[]){ch, '\0'}`.
