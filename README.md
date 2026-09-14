# 6 player Revenant fixes

## Vibe Coding Warning

**I have no C++ development experience. All project-owned code was written by AI and has only been validated through my in-game testing, without a professional C++ code review. I cannot guarantee that the code is minimal or elegant, or that every native hook, multiplayer interaction and recovery path is correct and reliable.** Third-party code and assets retain their original authorship and licenses.

This native DLL primarily addresses Revenant summon handling in expanded co-op parties, and includes related party and networking behavior.

Works in the Nightreign Seamless environment.

## Included fixes

### Revenant summons

- Revenant players in slots 4–6 can now summon their buddies correctly.
- When Revenant's buddies are replaced with other entities, summon creation, recall, switching and health restoration are handled correctly.

### Related expanded-party handling

The current DLL **does more than modify summons**. It also includes:

- Players in slots 4–6 can correctly receive talisman rewards from sorcerer's towers and castles.
- Items dropped directly from the Dormant Power interface are visible to other players.
- World-map and minimap player markers for slots 4–6.
- Reconnect-related logic for the Seamless Co-op environment.

This describes the current implementation scope, not a promise that every six-player scenario has been tested.

## Relationship to other mods

This release does not distribute `regulation.bin`, `common.emevd` or map replacement files, so it does not itself require merging those files. This does not mean that conflicts with other native hooks or overlapping features are impossible.

## Requirements and tested scope

- Windows x64 version of Nightreign.
- [Nightreign Seamless Co-op](https://www.nexusmods.com/eldenringnightreign/mods/3), installed separately.
- [Mod Engine 3](https://github.com/garyttierney/me3) for the installation method below.

**Tested with up to five players; six-player testing has not yet been performed.**

## Installation

1. Close the game and back up your modded save and current configuration.
2. Confirm that your existing Seamless and ME3 setup works.
3. Place the release's `RevenantFix` folder beside your active `.me3` profile.
4. Add one native entry:

```toml
[[natives]]
path = "./RevenantFix/RevenantFix.dll"
load_after = [{id = "nrsc.dll", optional = false}]
```

Keep your existing `nrsc.dll` entry; do not add Seamless again. Adjust the relative path if using a different location. Launch with this profile; no additional key press is needed to activate the mod.

The DLL uses these files beside it:

```text
RevenantFix.dll              Native plugin
RevenantFix.ini              User configuration; generated if missing
RevenantFix.log              Runtime log
RevenantFixReconnect.ini     Local reconnect state, written as needed
```

The reconnect state file is not a configuration preset. **Do not distribute it or copy it between players.**

## Known limitations

- Game updates and other mods that modify the same functions may cause conflicts.
- Disconnects and reconnects, simultaneous player returns, host migration and data recovery are not guaranteed to be lossless in every situation.
- In some cases, players may not acquire aggro correctly after reconnecting. There have been too few reports to investigate reliably, so this still needs confirmation.

## Credits

Yui and Seamless Co-op; the Mod Engine 3 contributors; and Umgak/Sayu and Nightreign 6 Player Fixes as a reference for expanded-player-count modding.

## License

Project-owned code is licensed under the [MIT License](LICENSE). Use, modification, redistribution and commercial use are permitted under its terms; retain the copyright and license notice.

Third-party code and assets retain their respective licenses. This license does not grant rights to Seamless Co-op, other mods or game files.

## Implementation notes

The CMake target builds `RevenantFix.dll` from `core`, `buddy_fix`, `reward_fix`, `reconnect_fix`, `network_tools`, `log` and the DLL entry point. Native hooks and peer synchronization are part of the implementation; this is not an event-file patch package.

The current CMake setup uses a unity build. Retain that project configuration when reproducing a release. Function names and namespaces were renamed from BuddyHook, but the existing BHRC wire identifier was retained; the product rename itself does not create a new protocol compatibility guarantee.

`RevenantFix.ini` is configuration. `RevenantFixReconnect.ini` is local reconnect state, not a redistributable preset. Not all installed hooks are independently disabled through the configuration file.

## Building

Use Windows x64, CMake 3.21 or newer, an MSVC toolchain with C++20 support, and the Windows SDK. From an appropriate Visual Studio developer environment:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

With a Visual Studio multi-configuration build, the DLL is `build/Release/RevenantFix.dll`. Building proves compilation, not runtime correctness. The inspected CMake project does not register an automated test suite. Record release-specific game tests and preserve matching debugging symbols.
