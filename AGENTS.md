# AGENTS.md — TP1_algoritmos

"Battle Zone" tank game (SDL2, C), FIUBA Algorithms assignment.

## Build & run

```sh
gcc -o tp1 *.c -lSDL2 -lm
./tp1   # modelos.stl must be in CWD
```

No Makefile, test/lint/CI tooling.

## The one hard rule: student sections only

`main.c` has four `BEGIN código del alumno` / `END código del alumno` blocks. **Only edit inside them.** The framework owns everything else.

| Section | Lines | Purpose |
|---------|-------|---------|
| Init    | 27–64 | STL loading → `lista_modelos` → create tanks/obstacles |
| Event   | 72–96 | Input handling (keyboard) |
| Draw    | 105–108 | Per-frame update + rendering |
| Cleanup | 123–133 | Free all dynamic memory |

Line numbers are stable (verified against current `main.c`).

## Event loop quirk

```
SDL_PollEvent → event handler → continue (skips render this iteration)
```

Events and per-frame updates **never run in the same iteration**. The `continue` at line 98 is outside your control. Put physics, AI, spawning, and missile updates in the Draw section.

## Local-only data

`struct nodo_m` (singly linked list: `modelo_t *modelo` + `sig`) and `lista_modelos` are local variables inside `main()` init section. No other module sees them.

## Architecture

| Module | Role |
|--------|------|
| `stl`   | Custom binary STL parser (not standard STL) — reads `modelos.stl` |
| `modelo` | Opaque `modelo_t` storing vertices (`float coords[3*N]`) + line topology (`size_t lineas[2*N]`) |
| `obstaculo` | Position + rotation + pointer to a `modelo_t` (does not own it) |
| `tanque` | Full tank state (pos, hull rotation, turret angle, lives, missile w/ cooldown) |
| `main.c` | Orchestrator: SDL2 window (1024×768), 24 FPS loop, owns `lista_modelos` |

**Data flow:** `stl` parses → `modelo` stores → `main.c` builds linked list → student code creates `tanque_t`/`obstaculo_t` which reference `modelo_t` instances → main loop polls events → updates state → draws.

## Key gotchas

- All types are opaque — always use pointers and accessor functions.
- Coordinates are `float`, angles in radians, line indices `size_t`.
- `tanque.c:4` defines `M_PI 3.14` locally (not the real π, used consistently).
- `modelo_crear` deep-copies coords/lineas arrays; the caller's copies can be freed immediately (see main.c:47-48).
- The student-facing guide at `guia.md` has detailed per-section instructions.
