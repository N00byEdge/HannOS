.intel_syntax noprefix

.code64
.section .data
.global tssBegin
        tssBegin:
.long 0  # 0x00 reserved
.quad interruptStack # 0x04 RSP 0
.quad 0  # 0x0C RSP 1
.quad 0  # 0x14 RSP 2
.quad 0  # 0x1C reserved
.quad 0  # 0x24 IST1
.quad 0  # 0x2C IST2
.quad 0  # 0x34 IST3
.quad 0  # 0x3C IST4
.quad 0  # 0x44 IST5
.quad 0  # 0x4C IST6
.quad 0  # 0x54 IST7
.quad 0  # 0x5c reserved
.word 0  # 0x64 reserved
.word tssSize # 0x66 iopb offset
.global tssEnd
        tssEnd:

.set tssSize, tssEnd - tssBegin

.extern gdtTssData [data, size=8]

gdt:     .quad 0x000100000000ffff
gdtcode: .quad 0x00af9a0000000000
gdtdata: .quad 0x0000920000000000
gdttss:  .quad gdtTssData # Calculated in linker script
         .long tssAddrHighest
         .long 0 # reserved
gdtpointer:
.word .-gdt-1
.quad gdt

.set codeDescriptor, gdtcode - gdt
.set dataDescriptor, gdtdata - gdt
.global tssDescriptor
   .set tssDescriptor, gdttss - gdt

.section .bss
.space 4096
.global earlyStack
        earlyStack:
.space 4096
interruptStack:
.space 8
.global multibootInfoLoc
multibootInfoLoc:
.quad # 64 bit ptr

.code32
.section .loadertext
.global loader
        loader:
  # Store multiboot info struct location
  mov dword ptr[multibootInfoLoc], ebx

  # Clear memory for page tables
  cld
  xor eax, eax
  mov edi, 0x1000
  mov ecx, 0x1000
  rep stosd

  # 0x0003 mask says present and readwrite
  mov edi, 0x1000                      # PML4T at 0x1000
  mov dword ptr [edi], 0x2003          # PDPT at 0x2000
  mov dword ptr [edi + 0x1000], 0x3003 # PDT at 0x3000
  mov dword ptr [edi + 0x2000], 0x4003 # PT at 0x4000

  # Fill page table
  mov edi, 0x4000
  mov ebx, 0x00000003 # 0x00000003 mask again for present and writable
  mov ecx, 512        # Identity map first 2M = 4K(page size) * 512 pages, one full PT
pageEntry:
  mov dword ptr [edi], ebx # Set page pointer and flags
  add ebx, 0x1000          # Move pointer to next page
  add edi, 8               # Move pointer to next page table entry
  loop pageEntry

  # Enable PAE
  mov eax, cr4
  or  eax, 1 << 5
  mov cr4, eax

  # Point to PML4T
  mov edi, 0x1000
  mov cr3, edi

  # Enable long mode
  mov ecx, 0xC0000080 # 0xC0000080 = EFER MSR
  rdmsr
  or eax, 1 << 8 # Set LM-bit (8)
  wrmsr

  # Enable paging
  mov eax, cr0
  or eax, 1 << 31
  mov cr0, eax

  # Load 64 bit global descriptor table for long jump
  lgdt [gdtpointer]

  mov ax, dataDescriptor
  mov ss, ax
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  # Jump into 64 bit mode
  jmp codeDescriptor:go64
.code64
noSSE:
  hlt

go64:
  # Load TSS
  # mov ax, tssDescriptor
  # ltr ax

  mov eax, 0x1
  cpuid
  test edx, 1<<25
  jz noSSE

  # Enable SSE
  mov rax, cr0
  and ax, 0xFFFB
  or ax, 0x2
  mov cr0, rax
  mov rax, cr4
  or ax, 3 << 9
  mov cr4, rax
 
  # Set stack for C runtime
  mov rsp, offset earlyStack
  mov rbp, rsp

  # Call global constructors
.extern doConstructors
  call  doConstructors

  # Enable interrupts
.extern enableInterrupts
  call  enableInterrupts

  # Load info from grub, including
  # memory maps
.extern loadMultibootInfo
  call  loadMultibootInfo

  # Get a larger stack now that
  # we've initialized more memory
.extern aquireLorgeStack
  call  aquireLorgeStack
  mov rsp, rax
  mov rbp, rsp

  # Go 64 bit kernel
.extern kernel
  call  kernel

  # Call global destructors
.extern doDestructors
  call  doDestructors

  # Halt
  cli
haltForReals:
  hlt
  jmp haltForReals
