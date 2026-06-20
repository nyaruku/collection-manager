# collection-manager
Manage osu! Stable and Lazer collections side by side on Linux.

## Installation (Arch Linux)

```bash
yay -S collection-manager-git
```

## Dependencies

- CMake 3.20+
- GCC 12+ or Clang 15+
- zlib (`zlib` on Arch)
- libuv (`libuv` on Arch)
- brotli (`brotli` on Arch)
- zstd (`zstd` on Arch)

## Building

```bash
git clone --recurse-submodules https://github.com/your-username/collection-manager.git
cd collection-manager
cmake --preset x86-debug-linux
cmake --build --target collection-manager --preset x86-debug-linux
```

If you forgot `--recurse-submodules`:
```bash
git submodule update --init --recursive
```

Use `x86-release-linux` for release build.

## Running

```bash
./out/build/x86-debug-linux/collection-manager
```

Opens at [http://127.0.0.1:11727](http://127.0.0.1:11727) (default host:port).

## Vendored libraries

| Library | Version |
|---|---|
| [Crow](https://github.com/CrowCpp/Crow) | v1.3.2 |
| [nlohmann/json](https://github.com/nlohmann/json) | v3.12.0 |
| [libsass](https://github.com/sass/libsass) | 3.6.6 |
| [realm-cpp](https://github.com/realm/realm-cpp) | local |
