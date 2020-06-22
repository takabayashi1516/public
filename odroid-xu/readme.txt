https://wiki.odroid.com/old_product/odroid-xu

--- tool chain ---
http://dn.odroid.in/ODROID-XU/compiler/arm-eabi-4.6.tar.gz
$ export PATH=${PATH}:${HOME}/odroid/arm-eabi-4.6/bin
$ export CROSS_COMPILE=arm-eabi-

--- u-boot (cross) ---
$ export PATH=${PATH}:${HOME}/odroid/arm-eabi-4.6/bin
$ export CROSS_COMPILE=arm-eabi-

$ git clone https://github.com/hardkernel/u-boot.git -b odroid-v2012.07
$ cd u-boot
$ make smdk5410_config
$ make -j 4

--- kernel (self) ---
$ git clone --depth 0 https://github.com/hardkernel/linux.git -b odroidxu-3.4.y odroidxu-3.4.y
$ export ARCH=arm

$ make odroidxu_ubuntu_defconfig
$ make menuconfig or make xconfig  # Do changes if you need/want
$ make -j5 zImage modules

$ cp arch/arm/boot/zImage /media/boot
$ make modules_install
$ cp .config /boot/config-3.4.5
$ update-initramfs -c -k 3.4.5
$ mkimage -A arm -O linux -T ramdisk -C none -a 0 -e 0 -n uInitrd -d /boot/initrd.img-3.4.5 /boot/uInitrd-3.4.5
$ cp /boot/uInitrd-3.4.5 /media/boot/uInitrd
$ reboot: sync && reboot

--- ubuntu ---
https://odroid.in/
https://odroid.in/?directory=.%2FUbuntu_XU%2F

Partition Contents
Partition 1:
    Kernel Image (zImage)
    boot.scr
    uInitrd (if applicable)

Partition 2:
    rootfs (a.k.a. File System)




--- kernel (cross) (test) ---
$ git clone --depth 1 https://github.com/hardkernel/linux.git -b odroidxu-3.4.y odroidxu-3.4.y

$ export PATH=${PATH}:${HOME}/odroid/arm-eabi-4.6/bin
$ export CROSS_COMPILE=arm-eabi-
$ export ARCH=arm

$ make ARCH=arm CROSS_COMPILE=arm-eabi- odroidxu_ubuntu_defconfig
$ make ARCH=arm CROSS_COMPILE=arm-eabi- zImage modules -j 2

# $ sudo make ARCH=arm CROSS_COMPILE=arm-eabi- INSTALL_MOD_PATH=${mount_path_rootfs} modules_install

$ sudo cp arch/arm/boot/zImage /media/boot


