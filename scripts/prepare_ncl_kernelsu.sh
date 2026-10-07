#!/bin/sh
set -eu

KSU_COMMIT=bf3aeeda4a0358712d2d04e9e476ec136c54a80e
KSU_REPOSITORY=https://github.com/NCLnclNCL/KernelSU.git

if [ "$#" -gt 1 ]; then
    echo "Usage: $0 [kernel-source-root]" >&2
    exit 2
fi

kernel_root=$(CDPATH= cd -- "${1:-.}" && pwd)
cd "$kernel_root"

if [ ! -d KernelSU/.git ]; then
    if [ -e KernelSU ]; then
        echo "Refusing to replace non-git path: $kernel_root/KernelSU" >&2
        exit 1
    fi
    git clone --branch main "$KSU_REPOSITORY" KernelSU
fi

current_commit=$(cd KernelSU && git rev-parse HEAD)
if [ "$current_commit" != "$KSU_COMMIT" ]; then
    if [ -n "$(cd KernelSU && git status --porcelain)" ]; then
        echo "Refusing to change dirty KernelSU checkout at $kernel_root/KernelSU" >&2
        exit 1
    fi
    (cd KernelSU && git fetch origin main)
    (cd KernelSU && git checkout --detach "$KSU_COMMIT")
else
    (cd KernelSU && git checkout --detach "$KSU_COMMIT")
fi

current_commit=$(cd KernelSU && git rev-parse HEAD)
if [ "$current_commit" != "$KSU_COMMIT" ]; then
    echo "KernelSU checkout is $current_commit; expected $KSU_COMMIT" >&2
    exit 1
fi

if ! grep -Fq 'obj-$(CONFIG_KSU) += kernelsu/' drivers/Makefile; then
    printf '\nobj-$(CONFIG_KSU) += kernelsu/\n' >> drivers/Makefile
fi
if ! grep -Fq 'source "drivers/kernelsu/Kconfig"' drivers/Kconfig; then
    if ! grep -Fq 'endmenu' drivers/Kconfig; then
        echo "Cannot find insertion point in $kernel_root/drivers/Kconfig" >&2
        exit 1
    fi
    sed -i '/^endmenu/i source "drivers/kernelsu/Kconfig"' drivers/Kconfig
fi

ksu_link=drivers/kernelsu
if [ -L "$ksu_link" ]; then
    if [ "$(readlink "$ksu_link")" != ../KernelSU/kernel ]; then
        echo "Existing KernelSU link points elsewhere: $kernel_root/$ksu_link" >&2
        exit 1
    fi
elif [ -e "$ksu_link" ]; then
    echo "Refusing to replace non-symlink KernelSU path: $kernel_root/$ksu_link" >&2
    exit 1
else
    ln -s ../KernelSU/kernel "$ksu_link"
fi

sh scripts/apply_susfs_patch.sh ncl-kernelsu "$kernel_root/KernelSU"
