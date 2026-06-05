# SuperTux iOS Port

This folder is the self-contained iOS port. The port scripts, signing settings,
assets, and Xcode generation live at the top level so this folder is the only
folder you need for iPhone and simulator builds.

For normal Xcode use, open the checked-in project at the workspace root:

```text
../SuperTux-iOS.xcodeproj
```

Select the `SuperTux iOS` target, then set your Team and Bundle Identifier in
Signing & Capabilities. That project forwards the Xcode signing settings into
the generated SuperTux build.

The upstream SuperTux source tree is included under `source/supertux/`.
Third-party source dependencies are in `vendor/`, vcpkg and compiled iOS
libraries live under `deps/`, generated Xcode projects live under `build/`, and
bundled game data is emitted to `assets/data.zip`.

Compiled dependency installs are separated by vcpkg triplet under `deps/vcpkg_installed/<triplet>/`, so simulator and device projects can coexist.

## Build

```sh
./scripts/build-simulator.sh Debug
```

The generated Xcode project is:

```text
build/iphonesimulator/SUPERTUX.xcodeproj
```

The generated project is disposable. Delete `build/` at any time and rerun a
build script, or build from `../SuperTux-iOS.xcodeproj`, to regenerate it from
this self-contained port folder.

For an iPhone device build:

```sh
./scripts/build-device.sh Release
```

Device builds use Xcode automatic signing. If the project has already been
opened in Xcode and a team was selected, the configure script preserves that
team. You can also pass it explicitly:

```sh
DEVELOPMENT_TEAM=YOURTEAMID ./scripts/build-device.sh Release
```

If Xcode cannot create a profile for the default bundle identifier, pass a
unique identifier under your own domain:

```sh
BUNDLE_IDENTIFIER=com.example.supertux2 DEVELOPMENT_TEAM=YOURTEAMID ./scripts/build-device.sh Release
```

To build, install, and launch on a connected iPhone:

```sh
DEVICE_ID=YOUR_DEVICE_IDENTIFIER BUNDLE_IDENTIFIER=com.example.supertux2 DEVELOPMENT_TEAM=YOURTEAMID ./scripts/install-device.sh Release
```

The device identifier appears in Xcode run error metadata and can also be found
in Xcode's Devices and Simulators window.

Free Apple developer accounts can only keep a small number of developer-signed
apps installed on one device at the same time. If install fails with that limit,
delete one of the other developer apps from the iPhone or uninstall it from the
command line:

```sh
xcrun devicectl device uninstall app --device YOUR_DEVICE_IDENTIFIER com.example.bundleid
```

## Run In Simulator

```sh
./scripts/run-simulator.sh Debug
```

The CMake project disables OpenGL for iOS, uses the SDL renderer, enables
mobile controls, enables curl-backed add-on downloads, and bundles `data.zip`
into the app resources to match the Android asset model.

The main menu exposes the built-in SuperTux level editor on iOS with touch
support for level placement, camera movement, zooming, layer/object menus, and
the tile/object palette.
