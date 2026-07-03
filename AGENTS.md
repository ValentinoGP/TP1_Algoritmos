# TP1 Algoritmos — Battle Zone

## Build & run

```sh
make          # gcc -g -Wall -Wextra -pedantic -lSDL2 -lm
make clean    # rm -f battlezone
./battlezone  # requires modelos.stl in CWD
```

- No test framework, no linter, no typecheck.
- Single binary, flat directory, no header-only units.
- Code between `BEGIN código del alumno` / `END código del alumno` markers.

## Architecture

| File | Role |
|------|------|
| `main.c` | Game loop: init, events, physics, render, cleanup |
| `modelo.c/h` | Opaque 3D wireframe model type (copies arrays on create) |
| `matriz.c/h` | Opaque 4×4 matrix algebra (row-major, homogenous coords) |
| `tanque.c/h` | Tank logic: position, heading, turret, missile, AI |
| `obstaculo.c/h` | Static obstacle (position + model pointer, not owned) |
| `stl.c/h` | Binary STL reader (cátedra format, not standard STL) |
| `pila.c/h` | Generic `void*` stack (used for transform matrix stack) |
| `lista.c/h` | Generic linked list (not used in `main.c`) |
| `modelos.stl` | Binary file with all wireframe models |

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
