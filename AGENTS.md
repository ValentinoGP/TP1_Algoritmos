# AGENTS.md — TP1_algoritmos

"Battle Zone" tank game, FIUBA Algorithms course assignment. SDL2, C.

## Build & run

```sh
gcc -o tp1 *.c -lSDL2 -lm
./tp1   # modelos.stl must be in CWD
```

No Makefile, no test/lint/CI tooling.

## Constraint: student code sections only

**Only change code between `BEGIN código del alumno` / `END código del alumno` markers** in `main.c`. The framework owns everything else. There are three student sections plus a cleanup block:

| Section | Lines | Purpose |
|---------|-------|---------|
| Init | 27–64 | STL file loading, model list, initial state |
| Event | 72–96 | Input handling (keyboard, etc.) |
| Draw | 105–108 | Per-frame rendering logic |
| Cleanup | 123–133 | Free `lista_modelos` before exit |

## Event loop semantics

```
SDL_PollEvent → student event handler → continue (skips render for this iteration)
```

Events and per-frame updates **never happen in the same loop iteration**. The `continue` at line 98 is outside student control. Per-frame logic (physics, updates, spawning) must go in the Draw section.

## Local variables

`struct nodo_m` (singly linked list, `modelo_t *modelo` + `sig`) and `lista_modelos` head are **local to `main()`** in the init section. Not accessible from other modules.

## Architecture

| Module | Role |
|--------|------|
| `stl` | Binary STL parser (custom format, not standard STL) — reads `modelos.stl` |
| `modelo` | Opaque `modelo_t`: vertices (`float coords[3*N]`) + lines (`size_t lineas[2*N]`) |
| `obstaculo` | Position + rotation + model reference |
| `tanque` | Tank: pos, rotation, turret, lives, missile with cooldown |
| `main.c` | SDL2 window (1024×768), 24 FPS event loop, (de)allocates the model list |

### Data flow

1. `stl.c` parses `modelos.stl` → models (name, vertex coords, line topology)
2. `modelo.c` stores each as opaque `modelo_t`
3. `main.c` loads all `modelo_t` into its local `lista_modelos`
4. `obstaculo_t` / `tanque_t` reference `modelo_t` or compute their own geometry
5. `main.c` polls events, updates state, draws

### Type conventions

- `modelo_t`, `obstaculo_t`, `tanque_t` are opaque — always use pointers
- Coordinates are `float`, angles in radians, line indices `size_t`
- `unidades_t` enum: `MM, CM, M, IN, FT, MILS`
- `tanque.c:4` defines `M_PI 3.14` locally (not the real π, but used consistently)
