#!/bin/sh
set -eu

usage() {
    echo "Usage: $0 <ncl-kernelsu|susfs-cli> <repository-directory>" >&2
    exit 2
}

[ "$#" -eq 2 ] || usage

case "$1" in
    ncl-kernelsu)
        expected_commit=bf3aeeda4a0358712d2d04e9e476ec136c54a80e
        patch_name=0001-ncl-kernelsu-susfs.patch
        ;;
    susfs-cli)
        expected_commit=41ba0b533b8524a0ba95a5952506a75f17355450
        patch_name=0002-susfs-cli-ncl-supercall.patch
        ;;
    *)
        usage
        ;;
esac

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/.." && pwd)
repo_dir=$(CDPATH= cd -- "$2" && pwd)
patch_file="$repo_root/kernel_patches/$patch_name"
actual_commit=$(cd "$repo_dir" && git rev-parse HEAD)

if [ "$actual_commit" != "$expected_commit" ]; then
    echo "Refusing to patch $repo_dir at $actual_commit; expected $expected_commit" >&2
    exit 1
fi

if (cd "$repo_dir" && git apply --unidiff-zero --reverse --check "$patch_file") >/dev/null 2>&1; then
    echo "$patch_name is already applied"
    exit 0
fi

(cd "$repo_dir" && git apply --unidiff-zero --check "$patch_file")
(cd "$repo_dir" && git apply --unidiff-zero "$patch_file")
