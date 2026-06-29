# AGENTS.md — TP1_algoritmos

Battle Zone (FIUBA Algoritmos) — clon mejorado del clásico Atari con SDL2 y motor 3D propio.

## Build & run

```sh
make          # produce ./battlezone
./battlezone  # modelos.stl must be in CWD
```

Must compile **without warnings, without memory leaks** (`valgrind --suppressions=suppressions.supp --leak-check=full ./battlezone`).

## Student sections only

`main.c` has four `BEGIN código del alumno` / `END código del alumno` blocks. Everything else is framework. Current line ranges:

| Section | Lines | Purpose |
|---------|-------|---------|
| Init    | 32–131 | STL → `lista_modelos` → create game objects |
| Event   | 142–162 | Keyboard input |
| Draw    | 170–339 | Per-frame update + 3D render |
| Cleanup | 354–368 | Free all dynamic memory |

## World coordinates

**Game world is 2D:** x,y in `[-150, 150]`. Screen is a separate projection step.

- Player starts at `(0, 0)` facing `π/2` (toward +Y).
- 50 obstacles at random `x,y ∈ [-150, 150]`, random rotation.
- Enemy spawns 50 units from player, not on any obstacle.

## 3D rendering via matrix stack

**Critical:** the spec mandates transformation **matrices** and a **stack**. You must port these from previous course exercises:

| Module | Source | Notes |
|--------|--------|-------|
| `matriz` | EJ1   | `matriz_crear_mper()`, `matriz_crear_mt()`, `matriz_crear_mz()`, `matriz_crear_my()`, `matriz_multiplicar()` |
| `aplicar` | EJ3  | Updated: returns 3-component vector (x,y,z). Third component is depth; discard if `< 1` (behind camera). |
| `pila` | Cátedra | Opaque stack for transformation matrices. |
| `lista` | Cátedra | Must hold the loaded STL models (already partially implemented as `struct nodo_m` in Init). |

Camera matrix (composed via sequential multiplication, then pushed as single matrix):

```
cam = I × MPER × Mz(π/2 + angz) × My(π/2 - angx) × Mz(-phi) × Mt(-x, -y, -3)
```

Where `(x, y, phi)` = player position/rotation, `angx`/`angz` = head-bob animation (random `±0.00001` during movement).

After camera push, for each object: push `Mt(x,y,z) × Mz(rot)`, apply with `matriz_aplicar`, pop. The camera stays on the stack during the whole render frame.

## Model naming

From `modelos.stl` — classify by `modelo_nombre`:

| Category | Models | Usage |
|----------|--------|-------|
| Obstacles | `CUBO1`–`CUBO3`, `PIRAMIDE1`–`PIRAMIDE3` | 50 random obstacles |
| Tank hull | `TANQUE` | One per tank |
| Turret | `TORRETA` | On tank, rotates independently |
| Radar | `RADAR` | On turret |
| Missile | `MISIL` | Projectile |
| Background | `HORIZONTE`, `MONTANAS`, `LUNA` | 3D skybox (drawn at origin with no camera offset) |
| HUD chars | `A`–`Z`, `0`–`9`, ` ` (space) | Score, messages |
| HUD icons | `*` (lives), `-` / `+` (crosshair), `#` (destruction) | 2D overlay |
| Debris | `RESTO1`, `RESTO2` | Enemy destruction animation |

## Game rules summary

- **Lives:** player has 3 extra + current = 4 total.
- **Score:** +1000 per enemy kill.
- **Enemy:** dies → animation → new enemy 50 units from player.
- **Player:** dies → lose life → if 0 → game over.
- **Missile:** speed 24 m/s, lifetime 2 s, radius 3 m for collision.
- **Movement collision:** radius 5 m — can't move into obstacles or other tank.
- **Firing cooldown:** 2 s per tank.
- **Player turret is fixed** (no A/D rotation for player).
- **Enemy turret** tracks player within 1 rad FOV, resets to 0 otherwise, fires if within 0.1 rad.

## Event timing

Player movement is **timed** (0.5 s). Holding a key re-triggers the timer. Speed: 7 m/s forward, 0.36 rad/s rotation.

## Key map

| Key | Action |
|-----|--------|
| Up / Down | Move forward / backward (7 m/s) |
| Left / Right | Rotate hull (0.36 rad/s) |
| Space | Fire |

## HUD

- Lives: `*` character drawn 2D
- Score: numeric text drawn 2D
- Crosshair: `-` normally, `+` when enemy within 0.15 rad
- Direction indicator: text when enemy outside 1 rad FOV ("izquierda", "derecha", "detrás")

## Animations

- **Player hit:** `#` model (27 lines), draw one line per frame.
- **Enemy kill:** 6 debris pieces (`TORRETA`, `RADAR`, 2×`RESTO1`, 2×`RESTO2`) on parabolic trajectory (`vx=5, vz=10, g=-9.81`), each rotated 60° apart in Z, plus random self-rotation.

## Gotchas

- `tanque.c:5` defines `#define M_PI 3.14` (redefines system macro). Clamping uses this value.
- `modelo_crear` deep-copies arrays; caller can `free(coords)`/`free(lineas)` immediately.
- All types are opaque — use accessor functions.
- `guia.md` has per-section instructions but its line numbers are stale — use table above.
- Screen-to-world: `(x_screen, y_screen)` → `(x_world = x_screen - W/2, y_world = -(y_screen - H/2))` scaled by projection.
- `matriz_aplicar` returns 3 components: (x, y, depth). Column 2 (index 2) stores the w' (4th homogeneous coordinate). Points with depth < 1 are behind camera.
- `RENDER_MODELO` macro in Draw pushes `cam × Mt(x,y,z) × Mz(rot)`, applies it to vertex points, renders lines, then pops everything.
