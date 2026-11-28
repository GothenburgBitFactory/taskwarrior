#!/bin/sh
# Build a fully static x86_64 Linux `task` on Alpine (musl) and package it as an
# install tree (bin/ + share/doc/task/rc/) in a tarball.
#
# Run from the repository root, with submodules checked out, inside a rust:alpine
# container. This is exactly what the static-linux-binary workflow does, so it can
# be reproduced locally with:
#
#   docker run --rm -v "$PWD":/src -w /src rust:alpine sh .github/scripts/build-static-linux.sh
#
# Output: dist/task-<version>-linux-x86_64-static.tar.gz and a matching .sha256 file.
#
# Why Alpine/musl and not `-static` on glibc: a static glibc binary still needs the
# matching shared glibc at runtime for getaddrinfo (sync) and getpwnam/getpwuid
# (~ expansion), so it is not actually portable.
#
# Why an install tree and not a bare binary: Taskwarrior parses its built-in
# defaults on every start, and those include `default.theme`, which is looked up
# under the compile-time TASK_RCDIR (<prefix>/share/doc/task/rc). The tarball is
# therefore meant to be extracted at the prefix it was built for (/usr/local).
set -eu

PREFIX=/usr/local
BUILD=build-static-linux
UUID_A=/usr/lib/libuuid.a

apk add --no-cache build-base cmake git ninja util-linux-dev util-linux-static

# The checkout is owned by a different uid than the container's root.
git config --global --add safe.directory '*'

if [ ! -f "$UUID_A" ]; then
  echo "error: $UUID_A not found (is util-linux-static installed?)" >&2
  exit 1
fi

# find_library() would otherwise pick libuuid.so, which cannot be linked statically.
cmake -S . -B "$BUILD" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_EXE_LINKER_FLAGS=-static \
  -DUUID_LIBRARY="$UUID_A"
cmake --build "$BUILD" --target task_executable

TASK="$BUILD/src/task"
strip "$TASK"

# Fail the build if the result is not actually static (a dynamic executable has an
# INTERP program header naming its loader).
if readelf -lW "$TASK" | grep -q INTERP; then
  echo "error: $TASK is dynamically linked" >&2
  readelf -dW "$TASK" | grep NEEDED >&2 || true
  exit 1
fi

VERSION=$("$TASK" --version)
NAME="task-${VERSION}-linux-x86_64-static"

rm -rf dist
mkdir -p "dist/$NAME/bin" "dist/$NAME/share/doc/task/rc"
cp "$TASK" "dist/$NAME/bin/task"
cp doc/rc/*.theme "dist/$NAME/share/doc/task/rc/"
cp LICENSE README.md "dist/$NAME/"
cat > "dist/$NAME/INSTALL-STATIC.txt" <<EOF
Taskwarrior ${VERSION}, static x86_64 Linux build (musl, no shared library dependencies).

Install, as root:

    tar -C ${PREFIX} --strip-components=1 -xzf ${NAME}.tar.gz \\
        ${NAME}/bin ${NAME}/share

This puts 'task' in ${PREFIX}/bin and the color themes in ${PREFIX}/share/doc/task/rc.
The themes must be at that location: task looks for default.theme there on every start.
EOF

tar -C dist -czf "dist/$NAME.tar.gz" "$NAME"
(cd dist && sha256sum "$NAME.tar.gz" > "$NAME.tar.gz.sha256")
rm -rf "dist/$NAME"

# Hand ownership back to the host user so the runner can clean up.
if [ -n "${HOST_UID:-}" ] && [ -n "${HOST_GID:-}" ]; then
  chown -R "$HOST_UID:$HOST_GID" .
fi

echo "built dist/$NAME.tar.gz"
