#!/bin/sh

set -eux

rm -rf kernels
mkdir kernels

./setup.sh arm
./setup.sh mipsel
./setup.sh mipseb

cp build/arm/arch/arm/boot/zImage kernels/zImage.armel
cp build/arm/vmlinux kernels/vmlinux.armel

cp build/mipseb/vmlinux kernels/vmlinux.mipseb
cp build/mipsel/vmlinux kernels/vmlinux.mipsel

#PROF=kernels/firmadyne_profiles.conf
#echo "[armel]" > $PROF
#cat panda_profile.arm >> $PROF
#echo "[mipseb]" >> $PROF
#cat panda_profile.mipseb >> $PROF
#echo "[mipsel]" >> $PROF
#cat panda_profile.mipsel >> $PROF
#cp console.bins/* kernels/
echo "Built by $(whoami) on $(date) at version $(git describe HEAD)" > kernels/README.txt
