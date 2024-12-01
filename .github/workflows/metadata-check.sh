#! /bin/bash

# Check the 'cargo metadata' for various requirements

set -e

META=$(mktemp)
trap 'rm -rf -- "${META}"' EXIT

cargo metadata --locked --format-version 1 > "${META}"

get_version() {
  local package="${1}"
  jq -r '.packages[] | select(.name == "'"${package}"'") | .version' "${META}"
}

get_msrv() {
  local package="${1}"
  jq -r '.packages[] | select(.name == "'"${package}"'") | .rust_version' "${META}"
}

# check that the cxx packages all have the same version
check_cxx_versions() {
  local cxx_version=$(get_version "cxx")
  local cxx_build_version=$(get_version "cxx-build")
  local cxxbridge_cmd_version=$(get_version "cxx-build")
  local cxxbridge_flags_version=$(get_version "cxxbridge-flags")
  local cxxbridge_macro_version=$(get_version "cxxbridge-macro")

  ok=true
  echo "Found cxx version ${cxx_version}"
  if [ "${cxx_version}" != "${cxx_build_version}" ]; then
    echo "Found differing cxx-build version ${cxx_build_version}"
    ok = false
  fi
  if [ "${cxx_version}" != "${cxxbridge_cmd_version}" ]; then
    echo "Found differing cxxbridge-cmd version ${cxxbridge_cmd_version}"
    ok = false
  fi
  if [ "${cxx_version}" != "${cxxbridge_flags_version}" ]; then
    echo "Found differing cxxbridge-flags version ${cxxbridge_flags_version}"
    ok = false
  fi
  if [ "${cxx_version}" != "${cxxbridge_macro_version}" ]; then
    echo "Found differing cxxbridge-macro version ${cxxbridge_macro_version}"
    ok = false
  fi

  if ! $ok; then
    echo "All cxx packages must be at the same version. Fix this in src/taskchampion-cpp/Cargo.toml."
    exit 1
  else
    echo "✓ All cxx packages are at the same version."
  fi
}

check_msrv() {
  local taskchampion_msrv=$(get_msrv taskchampion)
  local taskchampion_lib_msrv=$(get_msrv taskchampion-lib)

  echo "Found taskchampion MSRV ${taskchampion_msrv}"
  echo "Found taskchampion-lib MSRV ${taskchampion_lib_msrv}"

  if [ "${taskchampion_msrv}" != "${taskchampion_lib_msrv}" ]; then
    echo "Those MSRVs should be the same (or taskchampion-lib should be greater, in which case adjust this script)"
    exit 1
  else
    echo "✓ MSRVs are at the same version."
  fi
}

check_cxx_versions
check_msrv
