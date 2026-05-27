#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper #deep clean - clean kerbel build tree, .config file
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig #config "vert" arm dev board
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all #build kernel image for QEMU
    # make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules #build any kernel module **skipped**
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs # build device tree
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable//arch/arm64/boot/Image ${OUTDIR}/Image
echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir ${OUTDIR}/rootfs && cd "${OUTDIR}/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone https://git.busybox.net/busybox/
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make distclean
    make defconfig
    sed -i 's/^CONFIG_TC=y/CONFIG_TC=n/' .config #  use ubuntu24.04, need to disable tc
else
    cd busybox
fi

# TODO: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install


echo "Library dependencies"
# Output from readelf: 
# [Requesting program interpreter: /lib/ld-linux-aarch64.so.1]
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | \
sed -n 's/.*program interpreter: \(.*\)\]/\1/p' | \
xargs basename | \
xargs -I {} find $(${CROSS_COMPILE}gcc -print-sysroot) -name "{}" | \
xargs -I {} sh -c 'cp "{}" "'${OUTDIR}'/rootfs/lib/$(basename "{}")"'


# Output from readelf:
# 0x000000000000001 (NEEDED) Shared library: [libm.so.6]
# 0x000000000000001 (NEEDED) Shared library: [libresolv.so.2]
# 0x0000000000000001 (NEEDED) Shared library: [libc.so.6]
${CROSS_COMPILE}readelf -d ${OUTDIR}/rootfs/bin/busybox | \
sed -n 's/.*Shared library: \[\(.*\)\]/\1/p' | \
xargs -I {} find $(${CROSS_COMPILE}gcc -print-sysroot) -name "{}" | \
xargs -I {} sh -c 'cp "{}" "'${OUTDIR}'/rootfs/lib64/$(basename "{}")"'


# TODO: Add library dependencies to rootfs

# TODO: Make device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1

# TODO: Clean and build the writer utility
cd ${FINDER_APP_DIR}
make clean
make

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp writer ${OUTDIR}/rootfs/home/writer
cp finder.sh ${OUTDIR}/rootfs/home/finder.sh

mkdir ${OUTDIR}/rootfs/home/conf
cp conf/username.txt ${OUTDIR}/rootfs/home/conf/username.txt
cp conf/assignment.txt ${OUTDIR}/rootfs/home/conf/assignment.txt
sed 's|../conf/assignment.txt|conf/assignment.txt|g' finder-test.sh > ${OUTDIR}/rootfs/home/finder-test.sh
chmod +x ${OUTDIR}/rootfs/home/finder-test.sh
cp autorun-qemu.sh ${OUTDIR}/rootfs/home/autorun-qemu.sh
cp writer.sh ${OUTDIR}/rootfs/home/writer.sh

# TODO: Chown the root directory
sudo chown root:root ${OUTDIR}/rootfs

# TODO: Create initramfs.cpio.gz
cd ${OUTDIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f ${OUTDIR}/initramfs.cpio #compress to .gz