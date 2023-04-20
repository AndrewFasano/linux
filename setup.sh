#!/bin/sh
set -eux

# valid: arm, mipsel, mipseb
ARCH=$1

SHORT_ARCH=mips
ABI=
TARGETS=vmlinux

if [ "$ARCH" = "arm" ]; then
TARGETS="vmlinux zImage" # only for arm
ABI=eabi # only for arm
SHORT_ARCH=$ARCH
fi

# Firmadyne cross compilers from https://zenodo.org/record/4922202
# works for kernel 4.10
CROSS_CC=/opt/cross/fd/${ARCH}-linux-musl${ABI}/bin/${ARCH}-linux-musl${ABI}-


PANDA=~/git/panda
OUTDIR=~/git/HyDE/fws/
DWARF2JSON=~/git/dwarf2json/dwarf2json

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
  cp build/${ARCH}/arch/${SHORT_ARCH}/boot/zImage  ${OUTDIR}/zImage4.${ARCH}
fi

cp build/${ARCH}/vmlinux ${OUTDIR}/vmlinux4.${ARCH}

echo "[${ARCH}]" > ${OUTDIR}/${ARCH}_profile4.conf 
cat panda_profile.${ARCH} >> ${OUTDIR}/${ARCH}_profile4.conf 

echo 'Building volatility profile'
#dwarf2json wants a newer go than ships with Ubuntu 20.04, so I did this:
#mkdir -p ~/go-1.14.2
#wget -c https://dl.google.com/go/go1.14.2.linux-amd64.tar.gz -O - | tar -xz -C ~/go-1.14.2
#~/go-1.14.2/go/bin/go
DWARF2JSON=~/git/dwarf2json/dwarf2json
${DWARF2JSON} linux --elf build/${ARCH}/vmlinux | xz - > ./vmlinux.${ARCH}.json.xz
