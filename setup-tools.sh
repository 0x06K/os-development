#!/bin/bash

# ============================================================
#   OS FROM SCRATCH — TOOLCHAIN SETUP
#   Installs a compiler. Uses a compiler to compile a compiler.
#   Installs NASM and some other stuff. Till then Go Touch Grass.
# ============================================================

set -euo pipefail

# ------------------------------------------------------------
# COLORS
# ------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

# ------------------------------------------------------------
# HELPERS
# ------------------------------------------------------------
print_step()  { echo -e "\n${CYAN}${BOLD}>> $1${RESET}"; }
print_done()  { echo -e "${GREEN}✓ $1${RESET}"; }
print_warn()  { echo -e "${YELLOW}! WARNING: $1${RESET}"; }
print_error() { echo -e "${RED}✗ ERROR: $1${RESET}"; }
print_info()  { echo -e "  $1"; }

die() {
    print_error "$1"
    exit 1
}

# Show human-readable size of a file or directory
print_size() {
    local label=$1
    local target=$2
    if [[ -e "$target" ]]; then
        local size
        size=$(du -sh "$target" 2>/dev/null | cut -f1)
        print_info "${label}: ${CYAN}${size}${RESET}"
    fi
}

# Fetch remote file size via HTTP Content-Length before downloading
print_remote_size() {
    local url=$1
    local bytes
    bytes=$(wget --spider --server-response -q "$url" 2>&1 \
        | grep -i 'Content-Length' | tail -n1 | awk '{print $2}' | tr -d '\r') || true
    if [[ -n "$bytes" && "$bytes" -gt 0 ]]; then
        local human
        human=$(numfmt --to=iec-i --suffix=B "$bytes" 2>/dev/null || echo "${bytes} bytes")
        print_info "Remote size: ${CYAN}${human}${RESET}"
    else
        print_info "Remote size: ${CYAN}unknown${RESET}"
    fi
}

# ------------------------------------------------------------
# BANNER
# ------------------------------------------------------------
echo -e "${BOLD}${CYAN}"
echo "   ██████╗ ██╗  ██╗ ██████╗  ██████╗ ██╗  ██╗"
echo "  ██╔═████╗╚██╗██╔╝██╔═████╗██╔════╝ ██║ ██╔╝"
echo "  ██║██╔██║ ╚███╔╝ ██║██╔██║███████╗ █████╔╝ "
echo "  ████╔╝██║ ██╔██╗ ████╔╝██║██╔═══██╗██╔═██╗ "
echo "  ╚██████╔╝██╔╝ ██╗╚██████╔╝╚██████╔╝██║  ██╗"
echo "   ╚═════╝ ╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝"
echo -e "${RESET}"
echo -e "${GREEN}"
echo "┌─────────────────────────────────────────────────────────────┐"
echo "│  0x06k                                    github.com/0x06k  │"
echo "├─────────────────────────────────────────────────────────────┤"
echo "│ Installs a compiler. Uses a compiler to compile a compiler. │"
echo "│       NASM, GDB, MAKE, QEMU. Till then Go Touch Grass       │"
echo "└─────────────────────────────────────────────────────────────┘"
echo -e "${RESET}"

# ------------------------------------------------------------
# CONFIGURATION
# ------------------------------------------------------------
TARGET=i386-elf
PREFIX=/usr/local
CPUS=$(nproc)
WORKSPACE=$HOME/cross

# Track whether we are doing a reinstall so downstream sections
# can skip their workspace-clean prompts and reuse cached files.
IS_REINSTALL=false

# ------------------------------------------------------------
# FETCH LATEST COMPATIBLE VERSIONS DYNAMICALLY
# GCC and Binutils are always fetched together as latest.
# Mixing versions manually can cause build failures.
# ------------------------------------------------------------

print_step "Fetching latest GCC version"
GCC_VERSION=$(wget -qO- https://ftp.gnu.org/gnu/gcc/ \
    | grep -oP 'gcc-\K[0-9]+\.[0-9]+\.[0-9]+' \
    | sort -V | tail -n1) || die "Could not fetch latest GCC version. Check your internet connection."
[[ -z "$GCC_VERSION" ]] && die "Failed to parse GCC version from GNU FTP."
print_done "Latest GCC: ${GCC_VERSION}"

print_step "Fetching latest Binutils version"
BINUTILS_VERSION=$(wget -qO- https://ftp.gnu.org/gnu/binutils/ \
    | grep -oP 'binutils-\K[0-9]+\.[0-9]+(?=\.tar)' \
    | sort -V | tail -n1) || die "Could not fetch latest Binutils version. Check your internet connection."
[[ -z "$BINUTILS_VERSION" ]] && die "Failed to parse Binutils version from GNU FTP."
print_done "Latest Binutils: ${BINUTILS_VERSION}"

echo ""
echo -e "  Target:    ${CYAN}${TARGET}${RESET}"
echo -e "  GCC:       ${CYAN}${GCC_VERSION}${RESET}"
echo -e "  Binutils:  ${CYAN}${BINUTILS_VERSION}${RESET}"
echo -e "  Prefix:    ${CYAN}${PREFIX}${RESET}"
echo -e "  CPU cores: ${CYAN}${CPUS}${RESET}"
echo -e "  Workspace: ${CYAN}${WORKSPACE}${RESET}"
echo ""

read -rp "Do you want to continue with the current configurations? (y/n): " CONFIRM
[[ "$CONFIRM" =~ ^[Yy]$ ]] || { echo "Aborted."; exit 0; }

# ------------------------------------------------------------
# CHECK IF ALREADY INSTALLED — VERSION-AWARE
# ------------------------------------------------------------
print_step "Checking for existing installation"

if command -v "${TARGET}-gcc" &>/dev/null; then
    # Extract just the version number, e.g. "13.2.0" from the --version output
    INSTALLED_GCC_VERSION=$("${TARGET}-gcc" --version | head -n1 | grep -oP '\d+\.\d+\.\d+' | head -n1)

    if [[ "$INSTALLED_GCC_VERSION" == "$GCC_VERSION" ]]; then
        # Already on the latest version
        print_warn "${TARGET}-gcc ${INSTALLED_GCC_VERSION} is already installed and up to date."
        read -rp "  Reinstall anyway? (y/n): " REINSTALL
        if [[ "$REINSTALL" =~ ^[Yy]$ ]]; then
            IS_REINSTALL=true
            print_info "Reinstall confirmed. Cached sources in the workspace will be reused."
        else
            echo "Nothing to do. Exiting."
            exit 0
        fi
    else
        # Outdated — offer an upgrade
        echo ""
        echo -e "  ${YELLOW}Installed:${RESET} ${TARGET}-gcc ${CYAN}${INSTALLED_GCC_VERSION}${RESET}"
        echo -e "  ${GREEN}Available:${RESET} ${TARGET}-gcc ${CYAN}${GCC_VERSION}${RESET}"
        echo ""
        read -rp "  Upgrade from ${INSTALLED_GCC_VERSION} → ${GCC_VERSION}? (y/n): " UPGRADE
        if [[ "$UPGRADE" =~ ^[Yy]$ ]]; then
            IS_REINSTALL=false   # treat as fresh build; old source dirs won't match new version
            print_info "Upgrade confirmed. New sources will be downloaded if not already cached."
        else
            echo "Keeping existing installation. Exiting."
            exit 0
        fi
    fi
else
    print_done "No existing installation found. Fresh install."
fi

# ------------------------------------------------------------
# INSTALL DEPENDENCIES
# ------------------------------------------------------------
print_step "Installing dependencies"

sudo apt update --ignore-missing 2>/dev/null || print_warn "Some apt indexes failed. Continuing anyway."
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev \
    libmpfr-dev texinfo wget nasm qemu-system-x86 make gdb || die "Failed to install dependencies."

print_done "Dependencies installed"

# ------------------------------------------------------------
# SETUP WORKSPACE
# ------------------------------------------------------------
print_step "Setting up workspace at ${WORKSPACE}"

if [[ -d "$WORKSPACE" ]]; then
    if [[ "$IS_REINSTALL" == true ]]; then
        # On reinstall, keep everything — tarballs and source dirs are reused below.
        print_info "Reinstall mode: keeping existing workspace to reuse cached sources."
        print_size "Current workspace size" "$WORKSPACE"
    else
        print_warn "Workspace already exists at ${WORKSPACE}"
        print_size "Current workspace size" "$WORKSPACE"
        read -rp "  Clean it and start fresh? (y/n): " CLEAN
        if [[ "$CLEAN" =~ ^[Yy]$ ]]; then
            rm -rf "$WORKSPACE"
            print_done "Workspace cleaned"
        else
            print_info "Keeping existing workspace. Files may conflict."
        fi
    fi
fi

mkdir -p "$WORKSPACE"
cd "$WORKSPACE"
print_done "Workspace ready"

# ------------------------------------------------------------
# DOWNLOAD BINUTILS
# ------------------------------------------------------------
print_step "Downloading Binutils ${BINUTILS_VERSION}"

BINUTILS_TARBALL="binutils-${BINUTILS_VERSION}.tar.xz"
BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/${BINUTILS_TARBALL}"

if [[ -f "$BINUTILS_TARBALL" ]]; then
    print_warn "${BINUTILS_TARBALL} already exists. Skipping download."
    print_size "Cached tarball" "$BINUTILS_TARBALL"
else
    print_info "Fetching remote file size..."
    print_remote_size "$BINUTILS_URL"
    print_info "Downloading from: ${BINUTILS_URL}"
    wget --show-progress "$BINUTILS_URL" || die "Failed to download Binutils. Check version number or internet connection."
    print_size "Downloaded tarball" "$BINUTILS_TARBALL"
fi

if [[ ! -d "binutils-${BINUTILS_VERSION}" ]]; then
    print_info "Extracting ${BINUTILS_TARBALL}..."
    tar -xf "$BINUTILS_TARBALL" || die "Failed to extract Binutils tarball."
    print_size "Extracted source" "binutils-${BINUTILS_VERSION}"
else
    print_info "Source directory binutils-${BINUTILS_VERSION} already exists. Skipping extraction."
fi

print_done "Binutils ready"

# ------------------------------------------------------------
# BUILD BINUTILS
# ------------------------------------------------------------
print_step "Building Binutils"

if [[ -d "build-binutils" ]]; then
    print_warn "build-binutils directory already exists. Cleaning it."
    rm -rf build-binutils
fi

mkdir -p build-binutils && cd build-binutils

print_info "Running configure..."
../binutils-${BINUTILS_VERSION}/configure \
    --target=$TARGET \
    --prefix=$PREFIX \
    --disable-nls \
    --disable-werror \
    MAKEINFO=true || die "Binutils configure failed."

print_info "Compiling with ${CPUS} cores..."
make -j$CPUS || die "Binutils build failed."
print_size "Binutils build output" "."

print_info "Installing to ${PREFIX}..."
sudo make install || die "Binutils install failed."
cd ..

print_done "Binutils built and installed"
print_size "Binutils build dir (can be deleted)" "build-binutils"

# ------------------------------------------------------------
# DOWNLOAD GCC
# ------------------------------------------------------------
print_step "Downloading GCC ${GCC_VERSION}"

GCC_TARBALL="gcc-${GCC_VERSION}.tar.xz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/${GCC_TARBALL}"

if [[ -f "$GCC_TARBALL" ]]; then
    print_warn "${GCC_TARBALL} already exists. Skipping download."
    print_size "Cached tarball" "$GCC_TARBALL"
else
    print_info "Fetching remote file size..."
    print_remote_size "$GCC_URL"
    print_info "Downloading from: ${GCC_URL}"
    wget --show-progress "$GCC_URL" || die "Failed to download GCC. Check version number or internet connection."
    print_size "Downloaded tarball" "$GCC_TARBALL"
fi

if [[ ! -d "gcc-${GCC_VERSION}" ]]; then
    print_info "Extracting ${GCC_TARBALL}..."
    tar -xf "$GCC_TARBALL" || die "Failed to extract GCC tarball."
    print_size "Extracted source" "gcc-${GCC_VERSION}"
else
    print_info "Source directory gcc-${GCC_VERSION} already exists. Skipping extraction."
fi

print_done "GCC ready"

# ------------------------------------------------------------
# BUILD GCC
# ------------------------------------------------------------
print_step "Building GCC — go touch some grass, this will take a while"

if [[ -d "build-gcc" ]]; then
    print_warn "build-gcc directory already exists. Cleaning it."
    rm -rf build-gcc
fi

mkdir -p build-gcc && cd build-gcc

print_info "Running configure..."
../gcc-${GCC_VERSION}/configure \
    --target=$TARGET \
    --prefix=$PREFIX \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers || die "GCC configure failed."

print_info "Compiling all-gcc with ${CPUS} cores..."
make all-gcc -j$CPUS || die "GCC build failed at all-gcc."

print_info "Compiling all-target-libgcc with ${CPUS} cores..."
make all-target-libgcc -j$CPUS || die "GCC build failed at all-target-libgcc."
print_size "GCC build output" "."

print_info "Installing GCC to ${PREFIX}..."
sudo make install-gcc || die "GCC install failed."

print_info "Installing libgcc to ${PREFIX}..."
sudo make install-target-libgcc || die "libgcc install failed."
cd ..

print_done "GCC built and installed"
print_size "GCC build dir (can be deleted)" "build-gcc"

# ------------------------------------------------------------
# VERIFY
# ------------------------------------------------------------
echo ""
echo -e "${GREEN}${BOLD}╔══════════════════════════════════════════════════════════════════════════╗${RESET}"
echo -e "${GREEN}${BOLD}║              Everything is ready. Let us build an OS.                    ║${RESET}"
echo -e "${GREEN}${BOLD}╚══════════════════════════════════════════════════════════════════════════╝${RESET}"
echo ""

print_step "Verifying tools"

verify_tool() {
    local name=$1
    local cmd=$2
    if command -v "$cmd" &>/dev/null; then
        local bin_path
        bin_path=$(command -v "$cmd")
        local bin_size
        bin_size=$(du -sh "$bin_path" 2>/dev/null | cut -f1)
        print_done "${name}: $($cmd --version 2>&1 | head -n1)  ${CYAN}(${bin_size} @ ${bin_path})${RESET}"
    else
        print_error "${name} not found. Something went wrong."
    fi
}

verify_tool "Freestanding GCC" "${TARGET}-gcc"
verify_tool "Linker"           "${TARGET}-ld"
verify_tool "NASM"             "nasm"
verify_tool "QEMU"             "qemu-system-i386"
verify_tool "GDB"              "gdb"
verify_tool "Make"             "make"

# Show total installed footprint under prefix for our target
echo ""
print_step "Installed footprint under ${PREFIX}"
du -sh \
    "${PREFIX}/bin/${TARGET}-"* \
    "${PREFIX}/lib/gcc/${TARGET}" \
    "${PREFIX}/${TARGET}" \
    2>/dev/null | sort -h | while read -r size path; do
        print_info "${CYAN}${size}${RESET}  ${path}"
    done

# ------------------------------------------------------------
# WORKSPACE CLEANUP
# ------------------------------------------------------------
print_step "Workspace cleanup"
print_info "All binaries are installed to ${PREFIX}. The workspace at"
print_info "${CYAN}${WORKSPACE}${RESET} is no longer needed."
echo ""
print_size "Total workspace size" "$WORKSPACE"
echo ""
read -rp "  Delete workspace ${WORKSPACE}? (y/n): " DELETE_WS
if [[ "$DELETE_WS" =~ ^[Yy]$ ]]; then
    print_info "Removing ${WORKSPACE}..."
    rm -rf "$WORKSPACE"
    print_done "Workspace deleted. Your disk thanks you."
else
    print_info "Keeping workspace at ${WORKSPACE}."
    print_info "You can delete it manually later with:  ${CYAN}rm -rf ${WORKSPACE}${RESET}"
fi

echo ""
echo -e "${YELLOW}  Now go write some assembly. :)${RESET}"
echo ""