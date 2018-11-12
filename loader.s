.set MagicNumber,   0x1badb002
.set Bootflags,     0x00000007
.set Checksum,      -(MagicNumber + Bootflags)
.set Ignore,        0x0
.set GraphicsMode,  0x1
.set DisplayWidth,  80
.set DisplayHeight, 25
.set DisplayDepth,  0x0

.section .multiboot
  .long MagicNumber
  .long Bootflags
  .long Checksum
  .long Ignore
  .long Ignore
  .long Ignore
  .long Ignore
  .long Ignore
  .long GraphicsMode
  .long DisplayWidth
  .long DisplayHeight
  .long DisplayDepth

.section .text
.extern kernel

.global loader
loader:
  mov $stack, %esp
  call kernel
  
  cli
  hlt

.section bss
.space 512*1024
stack:

.section gdt
.global globalDescriptorTable
.space (1<<20)
globalDescriptorTable:
