# AGENTS.md — TP1_algoritmos

"Battle Zone" tank game, FIUBA Algorithms course assignment. SDL2, C.

## Build & run

```sh
gcc -o tp1 *.c -lSDL2 -lm
./tp1   # modelos.stl must be in CWD (fopen relative path)
```

No Makefile, no test/lint/CI tooling. `tanque.c:4` redefines `M_PI` as `3.14` — keep or fix, either is fine.

## Constraints

- **Only change code between `BEGIN código del alumno` / `END código del alumno` markers** in `main.c`. Everything else is framework.
- `struct nodo_m` (singly linked list, `modelo_t *modelo` + `sig` pointer) and `lista_modelos` head are **local variables inside `main()`** (defined in the student section).
- Framework event loop: `SDL_PollEvent` → student event handler → `continue` skips the update frame. Student code must not depend on interleaving event handling and per-frame logic.

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

- `modelo_t`, `obstaculo_t`, `tanque_t` are opaque — always pointer
- Coordinates are `float`, angles in radians, line indices `size_t`
- `unidades_t` enum: `MM, CM, M, IN, FT, MILS`
