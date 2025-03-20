## Install dependencies on Arch
yay -S base-devel git qemu qemu-system-aarch64 dtc flex bison openssl libelf ncurses aarch64-linux-gnu-gcc arm-none-eabi-gcc rtl8812au-dkms-git

## Build u-boot

git clone https://github.com/u-boot/u-boot/
cd u-boot

make qemu_arm64_defconfig
make -j$(nproc) CROSS_COMPILE=aarch64-linux-gnu-
## This should make a u-boot.bin file in u-boot directory

cp u-boot.bin ../
cd ..

## Test u-boot

timeout 5 qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -bios u-boot.bin

## Build linux kernel

git clone --depth=1 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
cd linux

make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- defconfig
make -j$(nproc) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image 
## This should make the Image file in arch/arm64/boot/Image

cp arch/arm64/boot/Image ../
cd ..

## Test kernel
timeout 5 qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -kernel Image

## Run both u-boot with kernel
timeout 10 qemu-system-aarch64 -M virt -cpu cortex-a57 -nographic -bios u-boot.bin -kernel Image
