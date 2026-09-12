# Bootloader

*To compile use*

```bash
nasm -f bin boot.asm -o boot.bin
qemu-system-i386 -drive format=raw,file=boot.bin
```

## 1. Boot Process Basics

- Power on → CPU starts in **16-bit real mode**.
- BIOS runs POST (power-on self test), initializes basic devices.
- BIOS loads **first 512-byte sector** of a bootable device into memory at address **`0x7C00`**.
- BIOS checks the last two bytes of that sector for the **magic number `0xAA55`** (stored little-endian as bytes `55 AA`) to confirm it's a valid boot sector.
- BIOS jumps to `0x7C00` and CPU starts executing your code.
- No filesystem, no libraries, no protection — you are the only "software" running besides BIOS's own ISRs.

## 2. Boot Sector Rules

| Requirement | Detail |
|---|---|
| Size | Exactly 512 bytes |
| Load address | `0x7C00` (real mode) |
| Last 2 bytes | `0x55, 0xAA` (magic number) |
| Padding | `times 510-($-$$) db 0` before magic number |
| Assemble | `nasm -f bin boot.asm -o boot.bin` |
| Run (QEMU) | `qemu-system-i386 -drive format=raw,file=boot.bin` |
| Run (Bochs) | needs `bochsrc` config file pointing to image |
| Inspect bytes | `od -t x1 -A n boot.bin` |

```asm
; minimal valid boot sector
loop:
    jmp loop
times 510-($-$$) db 0
dw 0xaa55
```

## 3. CPU Registers (Real Mode, 16-bit)

- General purpose: `ax, bx, cx, dx` (16-bit), split into high/low bytes: `ah/al, bh/bl, ch/cl, dh/dl`
- Segment registers: `cs` (code), `ds` (data), `ss` (stack), `es` (extra)
- `bp` / `sp`: stack base pointer / stack pointer
- `ip`: instruction pointer (not directly accessible)
- Flags register: holds results of `cmp`, carry flag (CF) for BIOS error signaling

### Segment:Offset Addressing (Real Mode)
```
physical_address = segment * 16 + offset
```
- Max reachable: `0xFFFF * 16 + 0xFFFF` ≈ 1MB+64KB
- Segment registers can't be set directly with literals — must go through a general-purpose register:
```asm
mov bx, 0x7c0
mov ds, bx      ; now ds points to where boot sector was loaded
```
- Use `[org 0x7c00]` directive so the assembler auto-adjusts label addresses to match BIOS's load address.

## 4. Interrupts (BIOS Services)

Interrupts let you call BIOS-provided routines instead of writing your own drivers.

| Interrupt | Purpose | Key setup |
|---|---|---|
| `int 0x10` | Video services | `ah=0x0E` teletype print char in `al` |
| `int 0x13` | Disk services | `ah=0x02` read sectors (CHS addressing) |
| `int 0x16` | Keyboard services | read keypress |
| `int 0x15, ax=0xE820` | Memory map detection | used before setting up paging |

### Print a character (teletype)
```asm
mov ah, 0x0e
mov al, 'H'
int 0x10
```

### Read disk sectors (`int 0x13`, `ah=0x02`)
Params:
- `dl` = drive number (0 = floppy)
- `ch` = cylinder, `dh` = head, `cl` = sector (1-based!)
- `al` = number of sectors to read
- `es:bx` = destination buffer address
- Returns: `CF` set on error; `al` = actual sectors read

```asm
disk_load:
    push dx
    mov ah, 0x02
    mov al, dh        ; sectors to read
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02      ; start at 2nd sector (after boot sector)
    int 0x13
    jc disk_error
    pop dx
    cmp dh, al
    jne disk_error
    ret
```

## 5. Strings, Stack, Control Flow

- Strings = raw bytes in memory, conventionally **null-terminated**:
  ```asm
  msg: db 'Hello', 0
  ```
- Stack grows **downward**. `push`/`pop` operate in 16-bit chunks in real mode.
  ```asm
  mov bp, 0x8000
  mov sp, bp
  ```
- Function calls: `call` pushes return address, `ret` pops and jumps back.
  - Preserve registers with `pusha` / `popa` inside functions.
- Conditionals: `cmp a, b` sets flags, then jump:
  `je / jne / jl / jle / jg / jge`

## 6. Memory Map (After Boot Sector Loads)

```
0x00000 - 0x003FF   Interrupt Vector Table (IVT)
0x00400 - 0x004FF   BIOS Data Area
0x00500 - 0x07BFF   free (usable) low memory
0x07C00 - 0x07DFF   Boot sector (loaded by BIOS, 512 bytes)
0x07E00 - 0x9FFFF   free (usable) — good place for stage2/kernel/stack
0xA0000 - 0xBFFFF   Video memory (VGA text mode buffer at 0xB8000)
0xC0000 - 0xFFFFF   ROM / BIOS area
```

- VGA text mode buffer: `0xB8000`, 80x25 chars, 2 bytes/cell (ASCII + attribute byte).
  - Address of `(row, col)`: `0xB8000 + 2*(row*80 + col)`

## 7. Switching to 32-bit Protected Mode

### Why
- Access full 32-bit registers (`eax`, `ebx`, ...) and 4GB address space.
- Enables memory protection (rings/privilege levels) and paging (virtual memory).
- **BIOS calls stop working** once in protected mode — you need your own drivers (e.g. direct VGA memory writes).

### Steps
1. Disable interrupts: `cli`
2. Define and load the **GDT**: `lgdt [gdt_descriptor]`
3. Set PE bit in `cr0`:
   ```asm
   mov eax, cr0
   or eax, 0x1
   mov cr0, eax
   ```
4. **Far jump** to flush the CPU pipeline and reload `cs`:
   ```asm
   jmp CODE_SEG:init_pm
   ```
5. In 32-bit code, reload all segment registers to `DATA_SEG` and reset the stack.

## 8. Global Descriptor Table (GDT)

Each entry = **8-byte Segment Descriptor** defining a memory segment.

Required entries for the "flat model":
1. **Null descriptor** — 8 zero bytes (mandatory, index 0)
2. **Code segment** — base 0x0, limit 0xFFFFF (× 4K granularity = 4GB)
3. **Data segment** — same base/limit, different type flags

### GDT Descriptor (what you actually load with `lgdt`)
```
struct gdt_descriptor {
    uint16_t size;      // size of GDT - 1
    uint32_t address;   // start address of GDT
}
```

### Assembly Layout
```asm
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff       ; limit bits 0-15
    dw 0x0          ; base bits 0-15
    db 0x0          ; base bits 16-23
    db 10011010b    ; access byte: present,priv00,desc_type1 | code,readable
    db 11001111b    ; granularity,32bit,limit bits 16-19
    db 0x0          ; base bits 24-31

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b    ; same as code but writable data, not executable
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start   ; = 0x08
DATA_SEG equ gdt_data - gdt_start   ; = 0x10
```

### Access Byte Bit Meaning (code segment `10011010b`)
| Bit(s) | Field | Value | Meaning |
|---|---|---|---|
| 7 | Present | 1 | segment is in memory |
| 6-5 | Privilege (DPL) | 00 | ring 0 (kernel) |
| 4 | Descriptor type | 1 | code/data (not system) |
| 3 | Executable | 1 | code segment |
| 2 | Conforming | 0 | not conforming |
| 1 | Readable | 1 | readable (allows reading constants) |
| 0 | Accessed | 0 | set by CPU when accessed |

### Flags Nibble (`1100`)
| Bit | Field | Meaning |
|---|---|---|
| Granularity | 1 | limit scaled ×4K → 0xFFFFF becomes ~4GB |
| Size (D/B) | 1 | 32-bit segment |
| 64-bit (L) | 0 | not long mode |
| AVL | 0 | available for OS use |

## 9. Full Switch Routine (Reusable Skeleton)

```asm
[bits 16]
switch_to_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp
    call BEGIN_PM        ; jump into your 32-bit kernel/entry code
```

## 10. Quick Reference Table: Real Mode vs Protected Mode

| Feature | Real Mode (16-bit) | Protected Mode (32-bit) |
|---|---|---|
| Register width | 16-bit (`ax`) | 32-bit (`eax`) |
| Addressable memory | ~1MB (segment:offset) | 4GB |
| Segmentation | `seg*16 + offset` | GDT-based descriptors |
| BIOS interrupts | Available | Not usable |
| Memory protection | None | Yes (privilege rings) |
| Extra segment regs | `cs, ds, ss, es` | + `fs, gs` |
| Video output | `int 0x10` | Direct write to `0xB8000` |

## 11. Next Steps After This Stage

1. Load a **stage-2 bootloader** from disk (bypass 512-byte limit).
2. Set up **paging** for virtual memory (needed for a real kernel).
3. Switch to **long mode (64-bit)** if targeting x86_64.
4. Build a cross-compiler (`i686-elf-gcc`) and jump into C code (OSDev "Bare Bones" tutorial).
5. Set up **IDT** (Interrupt Descriptor Table) for your own interrupt handlers (keyboard, timer, etc.) since BIOS's IVT is gone.
6. Write drivers: VGA text/graphics, keyboard, disk (ATA/IDE), timer (PIT).
7. Add memory management, a scheduler, and a simple filesystem.

## Key References
- OSDev Wiki: https://wiki.osdev.org
- Ralf Brown's Interrupt List: https://www.cs.cmu.edu/~ralf/files.html
- "Writing a Simple Operating System from Scratch" – Nick Blundell (source of most examples above)
- "Writing an OS in Rust": https://os.phil-opp.com/