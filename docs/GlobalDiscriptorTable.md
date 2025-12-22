# Complete GDT Guide - Easy Notes for Future Me

## What is GDT? Why Do We Need It?

### The Old Days - Real Mode Problems

Long time ago, Intel made 8086 processor. It was 16-bit processor but Intel wanted more than 64KB memory. So they made trick:

```
Physical Address = Segment × 16 + Offset
```

This gave them 1MB memory (20-bit addressing). But later, 1MB was not enough!

### The Big Problem
- Applications getting bigger
- People wanted multitasking (run many programs together)
- RAM becoming cheaper, so people had more RAM
- But CPU still stuck at 1MB maximum

### Intel's Smart Solution
Intel engineers thought: "What if we make segment register point to a table instead of direct memory?"

**Old way:** Segment register → Direct memory
**New way:** Segment register → Table entry → Memory

This table is called **GDT (Global Descriptor Table)**

## How Did They Think of This Idea?

### The Thinking Process

**Step 1:** "We need more memory but can't make registers bigger (breaks old software)"

**Step 2:** "What if register holds a reference instead of actual value?"

**Step 3:** "Like array index! Index points to bigger information!"

**Step 4:** "Table can have big addresses and protection information!"

### The Breakthrough Moment
Someone probably drew this on whiteboard:
```
Instead of: [Segment Register] → Direct Memory
Do this:    [Segment Register] → [Table Entry] → Memory
```

This simple idea solved:
- More memory (4GB instead of 1MB)
- Memory protection
- Privilege levels
- Backward compatibility

## GDT Entry Structure - Byte by Byte

Each GDT entry is 8 bytes (64 bits). Here's what goes where:

```
Byte 0: Limit bits 0-7
Byte 1: Limit bits 8-15
Byte 2: Base address bits 0-7
Byte 3: Base address bits 8-15
Byte 4: Base address bits 16-23
Byte 5: Access byte (protection info)
Byte 6: Flags + Limit bits 16-19
Byte 7: Base address bits 24-31
```

### Why This Weird Layout?

**The Evolution:**
- **80286 (1982)**: Used 24-bit base (bytes 2-4), byte 7 was unused
- **80386 (1985)**: Added 32-bit addressing, used byte 7 for extra base bits
- **Couldn't change layout**: Would break existing software!

### Base Address (32 bits total)
- **Purpose**: Where does this segment start in memory?
- **Split location**: Bytes 2-4 (24 bits) + Byte 7 (8 bits)
- **Why split?**: Backward compatibility with 80286

### Limit (20 bits total)
- **Purpose**: How big is this segment?
- **Split location**: Bytes 0-1 (16 bits) + Byte 6 bits 0-3 (4 bits)
- **Max value**: 0xFFFFF
- **With granularity**: 0xFFFFF × 4KB = 4GB

### Access Byte (Byte 5)
```
Bit 7: P (Present) - Is segment in memory?
Bit 6-5: DPL (Descriptor Privilege Level) - Ring 0-3
Bit 4: S (Descriptor Type) - System or Code/Data
Bit 3: E (Executable) - Can run code here?
Bit 2: DC (Direction/Conforming) - Grows up/down
Bit 1: RW (Read/Write) - Can write to it?
Bit 0: A (Accessed) - CPU touched this?
```

### Flags (Byte 6 upper 4 bits)
```
Bit 7: G (Granularity) - Byte or 4KB units?
Bit 6: D (Default/Big) - 16-bit or 32-bit?
Bit 5: L (Long) - 64-bit mode?
Bit 4: AVL (Available) - For system use
```

## Setting Up Basic GDT

### The Minimum Required Entries

```assembly
gdt_start:
    ; Entry 0: Null Descriptor (MANDATORY!)
    dw 0x0000, 0x0000
    db 0x00, 0x00, 0x00, 0x00
    
    ; Entry 1: Kernel Code Segment
    dw 0xFFFF, 0x0000      ; Limit=65535, Base=0
    db 0x00, 0x9A, 0xCF, 0x00  ; Ring 0, executable, 4GB
    
    ; Entry 2: Kernel Data Segment  
    dw 0xFFFF, 0x0000      ; Limit=65535, Base=0
    db 0x00, 0x92, 0xCF, 0x00  ; Ring 0, writable, 4GB
    
    ; Entry 3: User Code Segment
    dw 0xFFFF, 0x0000      ; Limit=65535, Base=0
    db 0x00, 0xFA, 0xCF, 0x00  ; Ring 3, executable, 4GB
    
    ; Entry 4: User Data Segment
    dw 0xFFFF, 0x0000      ; Limit=65535, Base=0
    db 0x00, 0xF2, 0xCF, 0x00  ; Ring 3, writable, 4GB
gdt_end:

; GDT Descriptor (tells CPU where to find GDT)
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size minus 1 (Intel requirement)
    dd gdt_start                ; Address of GDT
```

### Understanding the Selectors

Selectors are what you put in segment registers:
- **Entry 0**: Selector = 0x00 (null - causes fault if used)
- **Entry 1**: Selector = 0x08 (kernel code)
- **Entry 2**: Selector = 0x10 (kernel data)
- **Entry 3**: Selector = 0x18 (user code) 
- **Entry 4**: Selector = 0x20 (user data)

**Why these numbers?** Each entry is 8 bytes: 0×8=0, 1×8=8, 2×8=16(0x10), etc.

## Assembly Data Types - Important!

```assembly
db    ; define byte (1 byte)     - Use for: access byte, flags
dw    ; define word (2 bytes)    - Use for: limit, base low
dd    ; define doubleword (4 bytes) - Use for: GDT descriptor address
```

### Common Mistakes:
```assembly
dw 0x00000000  ; ❌ WRONG! 4 bytes in 2-byte instruction
dd 0x0000      ; ❌ Wasteful! 2 bytes in 4-byte instruction
dw 0x0000      ; ✅ CORRECT! 2 bytes in 2-byte instruction
```

## Switching to Protected Mode - Step by Step

### Complete Code Example:

```assembly
[org 0x7c00]
[bits 16]           ; Start in 16-bit real mode

start:
    ; 1. Set up segments in real mode
    mov ax, 0
    mov ds, ax
    mov ss, ax
    mov sp, 0x7c00
    
    ; 2. Disable interrupts
    cli
    
    ; 3. Load GDT
    lgdt [gdt_descriptor]
    
    ; 4. Enable protected mode
    mov eax, cr0        ; Use EAX even in 16-bit mode!
    or eax, 1           ; Set PE bit
    mov cr0, eax
    
    ; 5. Far jump to flush pipeline
    jmp 0x08:protected_mode_start
    
[bits 32]           ; Now we're in 32-bit mode
protected_mode_start:
    ; 6. Set up segment registers
    mov ax, 0x10        ; Kernel data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Set up stack
    
    ; SUCCESS! Now in 32-bit protected mode
    ; Your kernel code goes here...
    
; Your GDT here (same as above)
```

### Important Notes:

1. **Use EAX in 16-bit mode**: The assembler adds override prefixes automatically
2. **Far jump is crucial**: Flushes CPU pipeline and loads new CS
3. **Reload all segments**: They still contain real mode values
4. **Set up stack**: ESP should point to safe memory

## Memory Protection - How It Really Works

### The Big Confusion
Many people think: "If all segments have same base (0) and limit (4GB), where is protection?"

**Answer**: Protection comes from **privilege rings**, not different memory ranges!

### Privilege Levels (Rings)
```
Ring 0: Kernel (highest privilege)
Ring 1: Device drivers (rarely used)
Ring 2: System services (rarely used)  
Ring 3: User programs (lowest privilege)
```

### CPL vs DPL - The Magic Check

**CPL (Current Privilege Level)**: What ring is CPU running in right now?
- Stored in lower 2 bits of CS register
- Changes when you jump to different code segments

**DPL (Descriptor Privilege Level)**: What ring is required to access this segment?  
- Stored in bits 5-6 of access byte in GDT entry
- Set when you create the GDT entry

### The Automatic Hardware Check

**Rule**: CPL ≤ DPL (current privilege ≤ required privilege)

Every time you do these instructions, CPU automatically checks:
```assembly
mov ds, ax      ; Check: Can current ring access this segment?
jmp far_address ; Check: Can current ring jump to this code?
mov ss, ax      ; Check: Can current ring use this stack?
```

### Example Checks:

```assembly
; Scenario 1: Kernel accessing user data
; CPU running Ring 0 code (CPL = 0)
mov ax, 0x20        ; User data selector (DPL = 3)
mov ds, ax          ; Check: CPL(0) ≤ DPL(3)? YES! ✅

; Scenario 2: User accessing kernel data
; CPU running Ring 3 code (CPL = 3)  
mov ax, 0x10        ; Kernel data selector (DPL = 0)
mov ds, ax          ; Check: CPL(3) ≤ DPL(0)? NO! ❌ #GP Fault
```

### What Ring 0 Can Do That Ring 3 Cannot:
```assembly
; Ring 0 (kernel) privileges:
mov eax, cr0        ; ✅ Access control registers
lgdt [gdt_ptr]      ; ✅ Load GDT
cli                 ; ✅ Disable interrupts  
in al, 0x60         ; ✅ Access hardware ports
halt                ; ✅ Stop CPU

; Ring 3 (user) restrictions:
mov eax, cr0        ; ❌ General Protection Fault!
lgdt [gdt_ptr]      ; ❌ General Protection Fault!
cli                 ; ❌ General Protection Fault!
in al, 0x60         ; ❌ General Protection Fault!
```

## Real Mode vs Protected Mode Differences

### What 32-bit Registers in Real Mode Give You:
- Bigger calculations (32-bit instead of 16-bit)
- More data handling capability
- **BUT**: Still limited to 1MB memory!

### What You CANNOT Do in Real Mode:
- Access more than 1MB memory
- Memory protection between programs
- Privilege levels (everything is Ring 0)
- Flat memory model
- Prevent programs from interfering with each other

### Why Protected Mode is Needed:
```assembly
; What you WANT to do:
mov eax, [0x12345678]   ; Access 4GB memory
call user_program       ; Run with limited privileges
; Handle memory faults safely

; What real mode CANNOT do:
; - Access beyond 1MB
; - User/kernel separation
; - Memory protection
```

## Common Mistakes and Solutions

### 1. Using Wrong Register Size with CR0
```assembly
; ❌ WRONG - corrupts upper 16 bits of CR0
mov ax, cr0
or ax, 1  
mov cr0, ax

; ✅ CORRECT - preserves full 32-bit register
mov eax, cr0
or eax, 1
mov cr0, eax
```

### 2. Forgetting Null Descriptor
```assembly
; ❌ WRONG - first entry must be null
gdt_start:
    dw 0xFFFF, 0x0000   ; Starting with real entry
    
; ✅ CORRECT - first entry is always null  
gdt_start:
    dw 0x0000, 0x0000   ; Null descriptor
    db 0x00, 0x00, 0x00, 0x00
```

### 3. Wrong Data Types
```assembly
; ❌ WRONG - trying to fit 4 bytes in 2-byte instruction
dw 0x00000000

; ✅ CORRECT - use right size
dw 0x0000       ; For 2-byte values
dd 0x00000000   ; For 4-byte values
```

### 4. Forgetting Far Jump
```assembly
; ❌ WRONG - doesn't flush pipeline or load CS
mov eax, cr0
or eax, 1
mov cr0, eax
; Continue here - CS still has real mode value!

; ✅ CORRECT - far jump loads new CS and flushes pipeline
mov eax, cr0  
or eax, 1
mov cr0, eax
jmp 0x08:protected_mode_start   ; Essential!
```

## Access Byte Values - Quick Reference

### Common Access Byte Values:
```
0x9A = Kernel Code: Present, Ring 0, Code, Executable, Readable
0x92 = Kernel Data: Present, Ring 0, Data, Writable  
0xFA = User Code:   Present, Ring 3, Code, Executable, Readable
0xF2 = User Data:   Present, Ring 3, Data, Writable
```

### Breaking Down 0x9A (Kernel Code):
```
Binary: 1001 1010
Bit 7: 1 = Present
Bit 6-5: 00 = DPL Ring 0  
Bit 4: 1 = Code/Data segment
Bit 3: 1 = Executable
Bit 2: 0 = Non-conforming
Bit 1: 1 = Readable
Bit 0: 0 = Not accessed
```

## Memory Layout After Setup

```
Physical Memory (4GB):
0x00000000 ┌─────────────────┐
           │                 │
           │  All segments   │ ← Ring 0 and Ring 3 both access
           │  point to       │   same physical memory
           │  same space     │
           │                 │   Protection comes from:
           │                 │   - Ring levels  
           │                 │   - Instruction restrictions
           │                 │   - Later: Paging
           │                 │
0xFFFFFFFF └─────────────────┘
```

## Summary - Key Points to Remember

1. **GDT solves memory limitation**: From 1MB to 4GB addressing
2. **Each entry is 8 bytes**: With weird split layout for compatibility  
3. **Null descriptor is mandatory**: Entry 0 must be all zeros
4. **Protection via rings**: CPL ≤ DPL checking happens automatically
5. **Far jump is essential**: To flush pipeline and load CS register
6. **Use correct data types**: dw for 2 bytes, dd for 4 bytes
7. **EAX works in 16-bit mode**: Assembler adds override prefixes
8. **Real mode ≠ Protected mode**: Different CPU behavior by design

## What Happens Next?

After setting up GDT and entering protected mode:
1. **Set up IDT (Interrupt Descriptor Table)**: Handle exceptions and interrupts
2. **Enable paging**: Real memory protection and virtual memory
3. **Set up multitasking**: Task switching between processes
4. **System calls**: Controlled way for user programs to access kernel

The GDT is just the first step in building a real operating system!

---
*These notes cover everything we discussed about GDT. Keep this for future reference when building your kernel!*
