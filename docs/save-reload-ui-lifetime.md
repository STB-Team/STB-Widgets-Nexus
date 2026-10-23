# Save reload / UI ownership fix

Gameplay validation is pending. This corrects observable unsafe code paths; it
does not establish that every reported Scaleform crash has the same cause.

## Problem

`kPostLoadGame` called `CheckInI()` synchronously. Loading-menu close also started
`PollHudConfig()` via the game-thread task queue, and PlayerCharacter::Update had
a third configuration path. All could invoke `setPreset`/`gotoAndPlay` on widgets
while their movies were being advanced elsewhere. A check that LoadingMenu is
closed is not a UI-thread dispatch or proof that every widget has advanced.
Equipment/actor-value hooks and the input callback also mutated those movies.
The hook problem was independently reported in upstream issue #2.

## Change

- Producers submit settings revisions, visibility intent and scalar alpha
  requests. They never retain movie pointers, GFxValues or task closures.
- Each widget consumes its own changes from its own `AdvanceMovie` override,
  after the base movie advance has returned. No widget waits for the equipment
  movie; delayed/recreated menus configure independently.
- Save-load and LoadingMenu gates are independent. Either event ordering works.
  A failed load does not enable game-data updates. Entering a load invalidates
  transient requests and increases the configuration revision.
- The UI frame checks the generation again after the engine advance. A load
  begun during that call cannot apply the previously captured configuration.
- Alpha requests are bounded to one latest request per widget. The mutex only
  protects plain data; no engine calls run under it. No recursive task polling.
- Visibility uses current blocking-menu state, including nested menus and the
  loading/fader screens. Widgets stay open; there is no hide/show teardown loop.
- Update throttles are menu-local instead of counters shared between the game
  and UI threads. Missing player skills are guarded in the level widget.
- Timestamped reload/configuration logs provide evidence for gameplay testing.

Settings names/defaults, SWFs, serialization IDs/records, and save contents are
unchanged. This is not a general audit of every shared setting or every engine
data lookup in STB, and is not a universal guarantee against other HUD mods.

## Sanitized diagnostic evidence

A Skyrim SE 1.5.97 / SKSE 2.0.20 session crashed while reloading a save after death.
The installed official package was STB Widgets 3.0 (internal DLL version 1.0.0).
The exception was at `SkyrimSE+0xFFFCF3`, `inc qword ptr [rax+0x20]`, with
`rax=0x20`, attempting to write `0x40`.

The dump unwind included `STB_Widgets+0x611AD` beneath the engine Scaleform path.
Disassembly of the containing STB routine sets the movie's `CurrentTime` and
calls GFxMovieView virtual slot 0x25 (`Advance`); `+0x611AD` is the return site.
The faulting thread differs from the process main thread. Exact private symbols
were unavailable, so the nearest export label is **not** treated as a function
identity. This proves the active UI-advance path, not the origin of the invalid
object. It does not prove that another plugin could not have contributed.

No raw dump, save, personal path, character name, or unrelated mod list is included.

## Offline validation

```
cmake -S tests -B build-tests
cmake --build build-tests --config Release
ctest --test-dir build-tests -C Release --output-on-failure
python tests/check_ui_ownership.py
```

The native suite compiles the **production HudRuntime.cpp** against a small
recording engine double. It checks UI thread affinity and advance/configure
ordering, both reload event orders, failed-load recovery, missing UI/player/movie,
all eight delayed widgets, nested blocking menus, camera visibility, paused
configuration, conditional fades, coalescing, generation changes inside an engine
callback, and 160,000 requests from eight concurrent producers across 100 reloads.
It does not simulate Scaleform's allocator or replace a real gameplay test.

## Gameplay checklist (pending)

1. Start with an existing save; confirm all enabled widgets retain positions,
   scales, presets, colours, opacity, and XP-number preference.
2. Check the widget hotkey, menu transitions, combat and change-only fades, and
   settings preview. Include Tween -> Map and dialogue/follower commands.
3. Quickload repeatedly, then test loading after death and fast travel. Confirm
   one configuration entry per widget/revision after each successful load, with
   no persistent missing widgets or crashes.
4. Quit and load again. Keep the STB log and any new crash report for comparison.

No save cleaning, mod removal, or new game is required. To roll back, close the
game and restore the previous DLL and matching PDB from the installation backup.
