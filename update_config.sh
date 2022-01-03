
#!/bin/sh
set -eux

# valid: arm, mipsel, mipseb
ARCH=$1

SHORT_ARCH=mips
ABI=
TARGETS=vmlinux

if [ "$ARCH" = "arm" ]; then
  echo "ARMY"
  TARGETS="vmlinux zImage" # only for arm
  ABI=eabi # only for arm
  SHORT_ARCH=$ARCH
fi

CROSS_CC=/opt/cross/${ARCH}-linux-musl${ABI}/bin/${ARCH}-linux-musl${ABI}-

echo "CROSS: ${CROSS_CC}"

echo "Loading old config"
mkdir -p build/${ARCH}
cp config.${ARCH} build/${ARCH}/.config
make ARCH=$SHORT_ARCH CROSS_COMPILE=${CROSS_CC} O=build/${ARCH} olddefconfig

echo "Generating savedconfig"
make ARCH=$SHORT_ARCH CROSS_COMPILE=${CROSS_CC} O=build/${ARCH} savedefconfig

cp build/${ARCH}/defconfig config.${ARCH} 
