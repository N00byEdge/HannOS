.intel_syntax noprefix

.set MagicNumber,   0xe85250d6
.set Arch,          0x00000000
.set mblen,         mbend - mbbegin;
.set Checksum,      (-(MagicNumber + Arch + mblen)&0xffffffff)

.section .multiboot
mbbegin:
  .long MagicNumber
  .long Arch
  .long mblen
  .long Checksum
  .word 0
  .word 0
  .long 8
mbend:
