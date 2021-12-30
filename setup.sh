#!/bin/sh
set -eux

ARCH=mipseb
CROSS_CC=/opt/cross/${ARCH}-linux-musleabi/bin/${ARCH}-linux-musleabi-
PANDA=~/git/panda
OUTDIR=~/git/HyDE/fws/

echo "Configuring kernel"
mkdir -p build/${ARCH}
cp config.${ARCH} build/${ARCH}/.config
make ARCH=$ARCH CROSS_COMPILE=${CROSS_CC} O=build/${ARCH} olddefconfig

echo "Building kernel"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_CC} O=build/${ARCH} vmlinux zImage -j32

echo 'Updating PANDA info'
${PANDA}/panda/plugins/osi_linux/utils/kernelinfo_gdb/run.sh ./build/${ARCH}/vmlinux ./panda_profile.${ARCH}

cp build/${ARCH}/arch/${ARCH}/boot/zImage  ${OUTDIR}/zImage.${ARCH}
cp build/${ARCH}/vmlinux ${OUTDIR}/vmlinux.${ARCH}

echo "[${ARCH}]" > ${OUTDIR}/{$ARCH}_profile.conf 
cat panda_profile.${ARCH} >> ${OUTDIR}/${ARCH}_profile.conf 
