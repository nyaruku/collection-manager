#!/bin/sh
set -eu

version="${1:-1.0.0}"
root="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
build_dir="$root/out/build/release-archive-linux"
dist_dir="$root/out/dist"
stage="$dist_dir/collection-manager-$version-linux-x86_64"
archive="$dist_dir/collection-manager-$version-linux-x86_64.tar.zst"

cmake -S "$root" -B "$build_dir" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DREALM_DISABLE_ALIGNED_STORAGE=ON \
  -DREALM_CPP_NO_TESTS=ON \
  -DBUILD_TESTING=OFF
cmake --build "$build_dir" --target collection-manager

rm -rf "$stage"
mkdir -p "$stage"

cp "$build_dir/collection-manager" "$stage/collection-manager"
cp -r "$build_dir/www" "$stage/www"
cp "$root/README.md" "$stage/README.md"

if command -v sassc >/dev/null 2>&1; then
  sassc -t compressed \
    "$stage/www/static/index.scss" \
    "$stage/www/static/index.css"
fi

mkdir -p "$dist_dir"
tar -C "$dist_dir" -I zstd -cf "$archive" "$(basename "$stage")"
sha256sum "$archive"

