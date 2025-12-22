# Function Definitions for kmain.c

## Core Kernel Functions

### 1. `void kmain(void)`
**Purpose:** Main kernel entry point, called by bootloader  
**When Called:** Immediately after bootloader switches to 32-bit protected mode  
**What It Does:**
- Orchestrates the entire kernel initialization process
- Calls early init, prints welcome message, calls late init
- Enters the main kernel loop that runs indefinitely
- Acts as the "main()" function for the kernel

**Parameters:** None  
**Returns:** Never returns (infinite loop)  
**Dependencies:** All other kernel subsystems

---

### 2. `void kernel_early_init(void)`
**Purpose:** Initialize critical systems needed for basic operation  
**When Called:** First thing in kmain(), before any output or complex operations  
**What It Does:**
- Initializes terminal/display for debugging output
- Sets up minimal memory management if needed
- Optionally reinitializes GDT (if replacing bootloader's)
- Only initializes systems that other systems depend on

**Parameters:** None  
**Returns:** void  
**Critical:** Must complete successfully or kernel cannot continue

---

### 3. `void kernel_late_init(void)`
**Purpose:** Initialize additional systems after basic output is working  
**When Called:** After early init and welcome message display  
**What It Does:**
- Sets up Interrupt Descriptor Table (IDT)
- Enables hardware interrupts (sti instruction)
- Initializes device drivers (keyboard, timer, etc.)
- Sets up memory management, scheduling, file systems
- Prepares system for normal operation

**Parameters:** None  
**Returns:** void  
**Note:** Can use terminal output for debugging since early_init completed

---

### 4. `void kernel_main_loop(void)`
**Purpose:** Main execution loop where kernel spends most of its time  
**When Called:** After all initialization is complete  
**What It Does:**
- Enters infinite loop that never exits
- Currently uses `hlt` instruction to wait for interrupts
- In future: will run scheduler, handle system calls, manage processes
- Responds to hardware interrupts and system events

**Parameters:** None  
**Returns:** Never returns  
**CPU Usage:** Minimal (halts CPU between interrupts)

---

## Utility Functions

### 5. `void print_kernel_info(void)`
**Purpose:** Display kernel version, system information, and welcome banner  
**When Called:** Early in kmain(), after terminal is initialized  
**What It Does:**
- Clears the screen for clean display
- Shows kernel name, version, architecture
- Displays system capabilities and status
- Provides visual confirmation that kernel loaded successfully

**Parameters:** None  
**Returns:** void  
**Output:** Multi-line formatted text to terminal

---

### 6. `void kernel_panic(const char* message)`
**Purpose:** Handle critical unrecoverable errors  
**When Called:** When a fatal error occurs that prevents normal operation  
**What It Does:**
- Disables all interrupts (cli instruction)
- Changes screen to red background (panic screen)
- Displays error message prominently
- Halts the system completely (infinite hlt loop)
- Prevents system corruption or unpredictable behavior

**Parameters:**
- `message` - String describing the error that occurred

**Returns:** Never returns (system halted)  
**Critical:** Last resort when kernel cannot continue safely

---

## Function Call Flow

```
Bootloader → kmain()
             │
             ├── kernel_early_init()
             │   └── terminal_initialize()
             │
             ├── print_kernel_info()
             │   └── [Display welcome banner]
             │
             ├── kernel_late_init()
             │   ├── idt_init()
             │   └── asm("sti") [enable interrupts]
             │
             └── kernel_main_loop()
                 └── while(1) { hlt; } [wait for interrupts]

[If error occurs anywhere]
             └── kernel_panic(message)
                 └── [Red screen, halt system]
```

## Error Handling Strategy

**Early Init Failures:**
- If terminal_initialize() fails → System unusable (no output)
- If GDT setup fails → Protection violations likely

**Late Init Failures:**
- If IDT setup fails → Call kernel_panic()
- If driver init fails → May continue with limited functionality

**Runtime Errors:**
- Hardware faults → Interrupt handlers → May call kernel_panic()
- Memory corruption → kernel_panic()
- Invalid operations → kernel_panic()

## Future Expansions

These functions are designed to be extended:

**kernel_late_init() will grow to include:**
- `keyboard_init()`
- `timer_init()`
- `memory_manager_init()`
- `filesystem_init()`
- `process_scheduler_init()`

**kernel_main_loop() will evolve to:**
- Run process scheduler
- Handle system calls
- Manage memory allocation
- Coordinate device drivers
- Balance system resources