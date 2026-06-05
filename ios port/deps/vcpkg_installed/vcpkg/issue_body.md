Package: vcpkg-tool-meson:arm64-osx@1.9.0#9

**Host Environment**

- Host: arm64-osx
- Compiler: AppleClang 17.0.0.17000604
- CMake Version: 4.3.2
-    vcpkg-tool version: 2026-05-27-d5b6777d666efc1a7f491babfcdab37794c1ae3e
    vcpkg-scripts version: 059d7604 2026-06-03 (10 hours ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
-- Installing: /Users/tobiasfriedly/Documents/super tux ios/v1 tux ios/ios port/deps/vcpkg/packages/vcpkg-tool-meson_arm64-osx/share/vcpkg-tool-meson/copyright
Downloading https://github.com/mesonbuild/meson/archive/1.9.0.tar.gz -> meson-1.9.0.tar.gz
error: curl operation failed with error code 6 (Couldn't resolve host name).
error: Not a transient network error, won't retry download from https://github.com/mesonbuild/meson/archive/1.9.0.tar.gz
note: If you are using a proxy, please ensure your proxy settings are correct.
Possible causes are:
1. You are actually using an HTTP proxy, but setting HTTPS_PROXY variable to `https://address:port`.
This is not correct, because `https://` prefix claims the proxy is an HTTPS proxy, while your proxy (v2ray, shadowsocksr, etc...) is an HTTP proxy.
Try setting `http://address:port` to both HTTP_PROXY and HTTPS_PROXY instead.
2. If you are using Windows, vcpkg will automatically use your Windows IE Proxy Settings set by your proxy software. See: https://github.com/microsoft/vcpkg-tool/pull/77
The value set by your proxy might be wrong, or have same `https://` prefix issue.
3. Your proxy's remote server is out of service.
If you believe this is not a temporary download server failure and vcpkg needs to be changed to download this file from a different location, please submit an issue to https://github.com/Microsoft/vcpkg/issues
CMake Error at scripts/cmake/vcpkg_download_distfile.cmake:136 (message):
  Download failed, halting portfile.
Call Stack (most recent call first):
  packages/vcpkg-tool-meson_arm64-osx/share/vcpkg-tool-meson/vcpkg-port-config.cmake:22 (vcpkg_download_distfile)
  ports/vcpkg-tool-meson/portfile.cmake:44 (include)
  scripts/ports.cmake:206 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg.schema.json",
  "dependencies": [
    "fmt",
    "libogg",
    "libvorbis",
    "freetype",
    "physfs",
    "glm",
    "libpng",
    "zlib"
  ],
  "features": {
    "nonwasm": {
      "description": "Add dependencies that aren't needed for WebAssembly",
      "dependencies": [
        "sdl2",
        "sdl2-image",
        "curl",
        "openal-soft",
        "libraqm"
      ],
      "supports": "!emscripten"
    },
    "glew": {
      "description": "Use glew for OpenGL support",
      "dependencies": [
        "glew"
      ],
      "supports": "!(android | emscripten)"
    },
    "libepoxy": {
      "description": "Use libepoxy for OpenGL support",
      "dependencies": [
        "libepoxy"
      ],
      "supports": "!(android | emscripten)"
    },
    "bidi": {
      "description": "Add proper support for bidirectional text (kind of a hack)",
      "dependencies": [
        "libraqm"
      ],
      "supports": "!emscripten"
    }
  },
  "default-features": [
    "nonwasm",
    "glew",
    "bidi"
  ]
}

```
</details>
