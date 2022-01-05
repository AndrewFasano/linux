#!/bin/sh
set -eux

# valid: arm, mipsel, mipseb
ARCH=$1
VERS=4.10

SHORT_ARCH=mips
ABI=
TARGETS=vmlinux

if [ "$ARCH" = "arm" ]; then
TARGETS="vmlinux zImage" # only for arm
ABI=eabi # only for arm
SHORT_ARCH=$ARCH
fi

CROSS_CC=/opt/cross/${ARCH}-linux-musl${ABI}/bin/${ARCH}-linux-musl${ABI}-


PANDA=~/git/panda
OUTDIR=~/git/HyDE/fws/

# MIPS: make malta_kvm_defconfig, then enable vsockets and debug_info

echo "Configuring kernel"
mkdir -p build/${ARCH}
cp config.${ARCH} build/${ARCH}/.config
make ARCH=$SHORT_ARCH CROSS_COMPILE=${CROSS_CC} O=build/${ARCH} olddefconfig

echo "Building kernel"
make ARCH=${SHORT_ARCH} CROSS_COMPILE=${CROSS_CC} O=build/${ARCH} $TARGETS -j$(nproc)

echo 'Updating PANDA info'
${PANDA}/panda/plugins/osi_linux/utils/kernelinfo_gdb/run.sh ./build/${ARCH}/vmlinux ./panda_profile.${ARCH}

if [ -e build/${ARCH}/arch/${SHORT_ARCH}/boot/zImage ]; then
  cp build/${ARCH}/arch/${SHORT_ARCH}/boot/zImage  ${OUTDIR}/zImage$VERS.${ARCH}
fi

cp build/${ARCH}/vmlinux ${OUTDIR}/vmlinux$VERS.${ARCH}

echo "[${ARCH}]" > ${OUTDIR}/${ARCH}_profile$VERS.conf 
cat panda_profile.${ARCH} >> ${OUTDIR}/${ARCH}_profile$VERS.conf 
