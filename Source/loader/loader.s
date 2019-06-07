.intel_syntax noprefix

.code64
.section .data
gdt:     .quad 0x000100000000ffff
gdtcode: .quad 0x00af9a0000000000
gdtdata: .quad 0x0000920000000000
gdtpointer:
.word .-gdt-1
.quad gdt

.section .bss
.space 512*1024
.global stack
stack:
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

  # Jump into 64 bit mode
  jmp 0x08:go64
.code64
go64:
  # Set stack for C runtime
  mov rsp, offset stack

  # Call global constructors
.extern doConstructors
  call  doConstructors

  # Load multiboot info
.extern loadMultibootInfo
  call  loadMultibootInfo

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
