# AGENTS.md — TP1_algoritmos

Battle Zone (FIUBA Algoritmos) — clon del clásico Atari con SDL2 y motor 3D propio.

## Build & run

```sh
make          # produce ./battlezone
./battlezone  # modelos.stl must be in CWD
```

Compile **without warnings**. On macOS: `valgrind --leak-check=full ./battlezone` (no `suppressions.supp` in repo).

## Student sections only

`main.c` has four `BEGIN código del alumno` / `END código del alumno` blocks. Everything else is framework. Current line ranges:

| Section | Lines | Purpose |
|---------|-------|---------|
| Init    | 84–183 | STL → `lista_modelos` → create game objects |
| Event   | 194–215 | Keyboard input |
| Draw    | 222–344 | Per-frame update + 3D render |
| Cleanup | 358–373 | Free all dynamic memory |

Rendering is done via `renderizar_modelo` (static helper at lines 19–59, outside student sections).

## Implementation state

**Already implemented:** STL loading, model lookup, obstacle/tank creation, player movement (timed 0.5s), missile launch & lifetime, enemy tank creation, camera matrix, full 3D wireframe rendering, crosshair, background (HORIZONTE/MONTANAS/LUNA).

**Still TODO** (per `guia.md`):
- Collisions: movement radius 5, missile radius 3
- Enemy AI: turret tracking, firing, random movement
- Animations: `#` model for player hit, `RESTO1`/`RESTO2` debris for enemy destruction
- HUD: lives (`*`), score, direction indicator text
- Score, game over state

## World coordinates

**Game world is 2D:** x,y ∈ `[-150, 150]`. Player starts at `(0, 0)` facing `π/2` (+Y).
- 50 obstacles at random x,y, random rotation
- Enemy spawns 50 units from player, not overlapping any obstacle (distance ≥ 5)

## Model naming

From `modelos.stl` — classify by `modelo_nombre`:

| Category | Models | Usage |
|----------|--------|-------|
| Obstacles | `CUBO1`–`CUBO3`, `PIRAMIDE1`–`PIRAMIDE3` | 50 random obstacles |
| Tank hull | `TANQUE` | One per tank |
| Turret | `TORRETA` | On tank, rotates independently |
| Radar | `RADAR` | On turret |
| Missile | `MISIL` | Projectile |
| Background | `HORIZONTE`, `MONTANAS`, `LUNA` | 3D skybox (drawn at origin) |
| HUD chars | `A`–`Z`, `0`–`9`, ` ` (space) | Score, messages |
| HUD icons | `*` (lives), `-` / `+` (crosshair), `#` (destruction) | 2D overlay |
| Debris | `RESTO1`, `RESTO2` | Enemy destruction animation |

## Key map

| Key | Action |
|-----|--------|
| Up / Down | Move forward / backward (7 m/s, 0.5s timer) |
| Left / Right | Rotate hull (0.36 rad/s, 0.5s timer) |
| Space | Fire (2s cooldown) |

**Player turret is fixed** — no A/D rotation. Holding a key re-triggers the 0.5s timer.

## 3D rendering pipeline

Camera matrix (built per-frame in Draw, single push):

```
cam = I × MPER × Mz(π/2 + angz) × My(π/2 - angx) × Mz(-phi) × Mt(-x, -y, -3)
```

Where `(x, y, phi)` = player position/rotation, `angx`/`angz` = head-bob (±0.00001 random during movement).

`renderizar_modelo` function (framework, outside student sections): pushes `cam × Mt(x,y,z) × Mz(rot)`, applies via `matriz_aplicar` (returns 3‑component `(x', y', depth)`), renders lines with depth ≥ 1, pops everything.

Modules already in the repo: `matriz` (EJ1), `matriz_aplicar` (returns 3 cols — depth in column 2 = w'), `pila` (Cátedra stack), `lista` (Cátedra list — unused in main.c, models kept in linked list via `struct nodo_m`).

## Gotchas

- `tanque.c:5–7`: `#ifndef M_PI #define M_PI 3.14159265358979323846` (standard value, guarded).
- `modelo_crear` deep-copies arrays; caller can `free(coords)`/`free(lineas)` immediately.
- All types are opaque — use accessor functions.
- `guia.md` has per-section instructions but line numbers are stale — use table above.
- `_matriz_crear` (underscore prefix) is the internal allocator, used directly in `renderizar_modelo`.
- Screen projection in `renderizar_modelo`: `sx = (int)(x' * H/2 + W/2)`, `sy = (int)(H/2 - y' * H/2)`.
