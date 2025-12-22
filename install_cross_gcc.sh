#!/bin/bash

set -e

TARGET=i386-elf
PREFIX=/usr/local
BINUTILS_VERSION=2.41
GCC_VERSION=13.2.0
CPUS=$(nproc)

echo "Installing dependencies..."
sudo apt update
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev \
    libmpfr-dev texinfo wget

mkdir -p $HOME/cross
cd $HOME/cross

echo "Downloading Binutils..."
wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz
tar -xf binutils-$BINUTILS_VERSION.tar.xz

echo "Building Binutils..."
mkdir -p build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure \
    --target=$TARGET \
    --prefix=$PREFIX \
    --disable-nls \
    --disable-werror \
    MAKEINFO=true

make -j$CPUS
sudo make install
cd ..

echo "Downloading GCC..."
wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz
tar -xf gcc-$GCC_VERSION.tar.xz

echo "Building GCC..."
mkdir -p build-gcc
cd build-gcc

../gcc-$GCC_VERSION/configure \
    --target=$TARGET \
    --prefix=$PREFIX \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers

make all-gcc -j$CPUS
make all-target-libgcc -j$CPUS

sudo make install-gcc
sudo make install-target-libgcc

echo "DONE!"
echo "Testing compiler:"
$TARGET-gcc --version
