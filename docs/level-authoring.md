# Data-driven level authoring

Each playable stage is opened through a `.level` manifest. The manifest is
stored beside its terrain `.txt` file:

```text
assets/maps/1.1/1-1.level
assets/maps/1.1/1-1.txt
```

The terrain file remains an ASCII grid. `@` is the player start marker and is
metadata only. All enemy/item/platform/block/portal/camera positions in the
manifest use tile coordinates; movement speeds and exit velocities use pixels
per second.

## Manifest records

- `[stage]`: stage identity, terrain/background paths, time limit, and next stage.
- `[rules]`: kill plane and horizontal boundaries in tile units.
- `[entities]`: enemy symbol, area, position, direction, and optional speed.
- `[items]`: initial item symbol, area, and position.
- `[platforms]`: position, width, bounds, speed, and motion mode.
- `[blocks]`: block position and content such as `PowerupByForm`, `I3`, or `I5`.
- `[anchors]`: named portal destinations and exit velocity.
- `[portals]`: source area, activation direction, trigger rectangle, and target.
- `[camera_zones]`: camera bounds, follow behavior, center Y, and background mode.

Entity aliases are defined in `assets/config/entities.catalog`. The same
mapping is documented in `docs/symbol.json`. For example, `E1` means
`Goomba`, and `I5` means `StarItem`.

Example:

```text
[stage]
version=1
id=world-2-1
name=WORLD 2-1
terrain=2-1.txt
background=background.txt
next_stage=2.2/2-2.level
initial_area=overworld
time_limit=400
tile_size=16

[entities]
entity id=g01 symbol=E1 area=overworld tile=24,12 direction=-1

[items]
item id=coin01 symbol=I1 area=overworld tile=30,8

[platforms]
platform id=p01 area=overworld tile=64,6 width=3 bounds=6,15 speed=50 motion=vertical

[blocks]
block area=overworld tile=40,9 content=I5

[anchors]
anchor id=secret_entry area=secret tile=2,30 velocity=0,0

[portals]
portal id=secret_pipe area=overworld activation=down trigger=50,10,2,2 target=secret_entry

[camera_zones]
camera id=main area=overworld bounds=0,0,200,16 follow_x=true follow_y=false center_y=8 dark=false
```

Unknown symbols, duplicate IDs, missing files, malformed coordinates, and
out-of-bounds records fail validation with the manifest path and line number.
