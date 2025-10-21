#!/usr/bin/env bash

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

  if dpkg --compare-versions "${taskchampion_msrv}" gt "${taskchampion_lib_msrv}"; then
    echo "Those MSRVs should be the same, or taskchampion-lib should be greater"
    exit 1
  else
    echo "✓ MSRVs are compatible."
  fi
}

check_msrv
