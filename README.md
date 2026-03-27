# OS-Development — Toolchain Setup

This script fetches the latest GCC and Binutils from the GNU FTP, builds them from source, and installs NASM, QEMU, GDB, and Make alongside them. When it's done, you have everything you need to write, assemble, link, and emulate an x86 OS.

---

## What It Installs

| Tool | Purpose |
|---|---|
| `i386-elf-gcc` | Freestanding C/C++ cross-compiler |
| `i386-elf-ld` | Cross-linker |
| `nasm` | x86 assembler |
| `qemu-system-i386` | Emulator to run your OS |
| `gdb` | Debugger |
| `make` | Build system |

All binaries go to `/usr/local`. Source files build in `~/cross`.

---

## Requirements

- Ubuntu or Debian-based Linux
- `sudo` access
- Internet connection (downloads from ftp.gnu.org)

---

## Usage

```bash
chmod +x setup.sh
./setup.sh
```

The script will:
1. Auto-detect the latest GCC and Binutils versions
2. Show you the config and ask for confirmation
3. Install build dependencies via `apt`
4. Download, build, and install Binutils, then GCC
5. Verify all tools are working
6. Offer to clean up the build workspace

GCC takes a while to compile. That's expected — go outside.

---

## Reinstall / Upgrade

Re-running the script on a machine that already has the toolchain will detect the installed version and offer to:
- **Reinstall** (same version, reuses cached sources)
- **Upgrade** (newer version available, downloads fresh)

---

## Verify Manually

After install, run:

```bash
i386-elf-gcc --version
i386-elf-ld --version
nasm --version
qemu-system-i386 --version
```

---

## Cleanup

The script asks at the end whether to delete `~/cross`. You can always do it manually:

```bash
rm -rf ~/cross
```

The installed binaries in `/usr/local` are unaffected.