#! /bin/bash

# Check the 'cargo metadata' for various requirements

set -e

META=$(mktemp)
trap 'rm -rf -- "${META}"' EXIT

cargo metadata --locked --format-version 1 > "${META}"

get_msrv() {
  local package="${1}"
  jq -r '.packages[] | select(.name == "'"${package}"'") | .rust_version' "${META}"
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

check_msrv
