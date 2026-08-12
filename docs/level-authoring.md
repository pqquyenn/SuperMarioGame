# Data-driven level authoring

Each playable stage consists of a terrain grid (`.txt`) and a stage manifest
(`.level`) in the same directory. The menu discovers valid `.level` files
automatically. Adding a stage does not require editing `Level`, `PlayState`,
`CollisionManager`, or `MenuState`.

Run these checks before opening a PR:

```powershell
out/build/x64-Debug/bin/mario.exe --validate-levels
out/build/x64-Debug/bin/mario.exe --smoke-levels
```

## Terrain grid

The terrain grid remains ASCII so its shape is readable in Git. `@` is the
single player start and `!` is the single stage-completion trigger. They are
metadata cells: invisible and non-solid. Static tile symbols come from
`assets/config/tiles.catalog`, which is the source of truth for texture,
rectangle, collision, animation, and placement offset.

To add a static tile, add its texture to the asset manifest/manager and add a
`tile` or `grid` record to `tiles.catalog`. No `TileMap.cpp` change is needed.
Texture paths are declared in `assets/config/assets.catalog`; tile properties
are declared in `assets/config/tiles.catalog`.

## Stage manifest sections

- `[stage]`: stable ID, menu name, terrain/background paths, time and next stage.
- `[rules]`: kill plane and horizontal world boundaries, expressed in tiles.
- `[entities]`: enemy type, tile position, initial direction and optional speed.
- `[items]`: initially visible item type and tile position.
- `[platforms]`: position, width, movement bounds, speed and motion type.
- `[blocks]`: explicit contents for question blocks or item bricks.
- `[anchors]`: named portal destinations and exit velocities.
- `[portals]`: source area, activation direction, trigger rectangle and target.
- `[camera_zones]`: per-area camera following and background rules.
- `[checkpoints]`: trigger rectangles and map-based respawn positions.

All positions and rectangles use tile coordinates. Velocities and speeds use
pixels per second.

Example:

```text
[stage]
version=1
id=world-2-1
name=WORLD 2-1
terrain=2-1.txt
background=background.txt
next_stage=world-2-2
initial_area=overworld
time_limit=400
tile_size=16

[rules]
kill_plane_tile=20
left_boundary_tile=0
right_boundary_tile=-1
enemy_void_margin_tiles=4

[entities]
entity id=g01 type=Goomba tile=24,12 direction=-1

[items]
item id=coin01 type=Coin tile=30,8

[platforms]
platform id=p01 tile=64,6 width=3 bounds=6,15 speed=50 motion=vertical

[blocks]
block tile=21,9 content=PowerupByForm
block tile=40,9 content=StarItem

[anchors]
anchor id=secret_entry area=secret tile=2,30 velocity=0,0

[portals]
portal id=secret_pipe area=overworld activation=down trigger=50,10,2,2 target=secret_entry

[camera_zones]
camera id=main area=overworld bounds=0,0,200,16 follow_x=true follow_y=false center_y=8.96875 dark=false
camera id=secret area=secret bounds=0,28,40,10 follow_x=true follow_y=false center_y=33 dark=true

[checkpoints]
checkpoint id=midway area=overworld trigger=90,0,2,16 spawn=90,10
```

Supported platform motions are `vertical`, `horizontal`, `loop_down`, and
`loop_up`. A negative `right_boundary_tile` means the terrain width.

`PowerupByForm` resolves to Mushroom for a small character and Fire Flower for
a powered character. Other content names are passed to `EntityFactory`.

## Extending gameplay

A new enemy or item behavior still requires a C++ class and registration in
`EntityFactory`. A new conditional block-content rule requires registration in
`BlockContentResolver`. Once registered, every stage can use it without further
C++ changes.

Do not add stage IDs, filenames, or pixel-coordinate checks to gameplay code.
Stage-specific behavior belongs in its `.level` definition.
