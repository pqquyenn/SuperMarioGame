# PvP Map Authoring

PvP arenas use the same data-driven `.level` manifests and terrain `.txt`
files as solo stages. Store them below `assets/maps`; the current arenas live
in `assets/maps/pvp`.

## Required arena data

Every terrain file needs exactly one `@` start marker because it is validated
by the shared level loader. PvP player positions come from these manifest
anchors instead:

```ini
[anchors]
anchor id=p1_spawn area=overworld tile=3,12 velocity=0,0
anchor id=p2_spawn area=overworld tile=21,12 velocity=0,0
```

The player spawn coordinates describe the top-left position of a Small
character. Super mode keeps the same feet position while changing the body to
Super height.

Super arenas may declare any number of Fire Flower locations. Their IDs must
start with `fire_spawn_`:

```ini
anchor id=fire_spawn_left area=overworld tile=5,12 velocity=0,0
anchor id=fire_spawn_center area=overworld tile=14,12 velocity=0,0
```

The anchor cell must be empty (`-`) in the terrain file. Put the anchor one
tile above the surface supporting the flower; do not place it inside `<>` or
`[]` pipe cells.

Use ordinary `[entities]`, `[platforms]`, `[rules]`, and `[camera_zones]`
sections for arena hazards. Enemy symbols are defined in
`assets/config/entities.catalog`.

Piranha Plants may configure their repeating visible/hidden cycle and their
first appearance independently. This makes it possible to stagger multiple
plants in one pipe:

```ini
entity id=plant_left symbol=E7 area=overworld tile=12,11 direction=0 visible_time=1.5 hidden_time=2.3 initial_delay=0
entity id=plant_right symbol=E7 area=overworld tile=13,11 direction=0 visible_time=1.5 hidden_time=2.3 initial_delay=2.3
```

## Loading another arena

`PvPState` accepts the ruleset and map independently:

```cpp
std::make_unique<PvPState>(
    PvPMatchType::Super,
    "pvp/super-arena-2.level"
);
```

If the map path is omitted, Small and Super use `small-arena.level` and
`super-arena.level` respectively. Rematches retain the selected map path.

To expose multiple arenas in the UI, add an arena-selection menu entry and
pass its `.level` path to this constructor. No PvP combat code needs to be
duplicated.
