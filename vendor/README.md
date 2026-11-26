# Vendored ring compatibility patch

`ring/` contains the published ring 0.17.14 crate with a local compatibility
patch for the MSYS2 `x86_64-pc-cygwin` Rust target. The exact changes are in
[ring-cygwin.patch](ring-cygwin.patch):

- `src/rand.rs` enables the existing getrandom-backed `SystemRandom` on Cygwin.
- `build.rs` selects the published Win64 NASM objects for x86-64 Cygwin and
  excludes the incompatible System V assembly sources.
- `include/ring-core/target.h` uses the Windows ABI configuration for Cygwin.
- `src/ec/curve25519/{ops,x25519}.rs` uses the same non-ADX implementation
  selected on Windows, matching the C implementation and available objects.

The patch uses zero context so whitespace hooks preserve it; apply to a fresh
crate with `git apply --unidiff-zero` if reconstructing the patched source.

No cryptographic algorithms or random-source implementations are replaced.
No custom RNG or less-safe fallback feature is enabled.

## Provenance and dependency selection

- Crate: https://crates.io/api/v1/crates/ring/0.17.14/download
- Archive SHA-256: `a4689e6c2294d81e88dc6261c768b63bc4fcdb852be6d1352498b114f61383b7`
- Formatting hooks exclude `vendor/ring/` to preserve the published sources
  and generated files. The local README and patch remain subject to hooks.
- All 391 published files, including licenses, are retained. Exactly the five
  files listed above differ from the published archive.
- The workspace Cargo.lock pins getrandom 0.2.17, which supports Cygwin via
  the operating system's getrandom function.
- The workspace `[patch.crates-io]` selects this copy for transitive consumers.
  No Cargo cache edits or sibling checkout are required, and no dependency
  versions change. Cargo.lock drops the registry source/checksum for ring
  because it is now a path dependency; the original checksum is recorded above.
- The published crate metadata is preserved. Its recorded source revision is
  `2723abbca9e83347d82b056d5b239c6604f786df` with `dirty: true`; the published
  crate remains the authoritative basis of this patch.

Only x86-64 MSYS/Cygwin has been validated here. This is a local compatibility
patch, not an upstream release or upstream endorsement. Review it when
upgrading ring, and retire the override when the selected upstream release
provides and validates the required support.

## Validation

On October 24, 2026, MSYS GCC 15.3.0 and Rust/Cargo 1.99.0 passed all 174 ring
unit/integration tests across 15 executables, with zero failures or ignored
tests. This includes operating-system random fill at boundary sizes, ECDSA,
RSA signing/verification, X25519 key agreement, and authenticated encryption.
The compiler reports existing Rust lifetime-style warnings and an ignored C
visibility attribute. This testing is not a cryptographic security audit.

The published crate omits test vectors required by most tests. Validation used
an ignored copy under `build/msys-baseline/ring-validation`, retaining the
patched library source and adding only missing test data from the recorded
upstream source revision. Its isolated manifest adds an empty workspace table.
The test lockfile was seeded with Taskwarrior's lockfile; Cargo resolved
additional test-only dependencies, retaining the application runtime versions.

To reproduce in fresh directories from an MSYS shell:

```sh
mkdir -p build/ring-validation build/ring-vectors
cp -R vendor/ring/. build/ring-validation/
cp Cargo.lock build/ring-validation/Cargo.lock
printf '\n[workspace]\n' >> build/ring-validation/Cargo.toml
curl -fL -o build/ring-vectors.tar.gz \
  https://codeload.github.com/briansmith/ring/tar.gz/2723abbca9e83347d82b056d5b239c6604f786df
printf '%s  %s\n' \
  53b06a7b7af16bcf4111715a60ea7af4d37c158fbfb90b60309215b864cee7de \
  build/ring-vectors.tar.gz | sha256sum -c -
tar -xzf build/ring-vectors.tar.gz -C build/ring-vectors --strip-components=1
cp -Rn build/ring-vectors/src build/ring-vectors/tests \
  build/ring-vectors/crypto build/ring-validation/
cargo test --manifest-path build/ring-validation/Cargo.toml --lib --tests
```

The no-clobber copy preserves patched source. Subsequent test runs can add
`--locked`. Logs from the initial validation are in
`build/msys-baseline/ring-tests.log`.

Build Taskwarrior using its normal CMake commands; the override is selected
automatically from the workspace manifest.

## Taskwarrior consumer result

With local edits on top of the committed date/time fix `2d217e131`, the MSYS
Release `task_executable` target builds and `task --version` reports 3.5.0.
All three selected Python tests pass: `color.cmd.test.py`,
`color.rules.test.py`, and `diag_color.test.py`. For this uninstalled Release
build, existing `doc/rc/*.theme` files were copied into the ignored CTest
working directory `build/msys-baseline/test`.

The combined build's `test_runner` target fails separately because strict
C++17 hides `unsetenv` and `fileno` declarations in the existing test harness.
No harness source change is included here, and the full Taskwarrior suite has
not passed. Logs are `build/msys-baseline/build-ring.log` and
`build/msys-baseline/color-tests-ring.log`. Submodules are unchanged.
