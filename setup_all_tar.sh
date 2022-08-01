#!/bin/sh
set -eux

# Build all 3 architectures + profiles and tar up
today=$(date +%y_%m_%d)

rm -rf kernels || true
mkdir kernels

echo "Building arm..."
./setup.sh arm
cp build/arm/arch/arm/boot/zImage kernels/zImage.arm
cp build/arm/vmlinux kernels/vmlinux.arm
cp build/arm/.config kernels/config.arm

echo "Building mips (big)..."
./setup.sh mipseb
cp build/mipseb/vmlinux vmlinux.mipseb
cp build/mipseb/.config kernels/config.mipseb

echo "Building mips (little)..."
./setup.sh mipsel
cp build/mipsel/vmlinux vmlinux.mipsel
cp build/mipsel/.config kernels/config.mipsel
cat *_profile.conf > kernels/profiles.conf

tar cvfz kernels.${today}.tar.gz kernels/
