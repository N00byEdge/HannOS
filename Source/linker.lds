ENTRY(loader)
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(loader)

SECTIONS {
  . = 0x5000;

  .text BLOCK(4K) : ALIGN(4K) {
    *(.multiboot)
    *(.loadertext)
    *(.text)
  }

  .rodata BLOCK(4K) : ALIGN(4K) {
    constructorsStart = .;
    KEEP(*( .init_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    constructorsEnd = .;

    destructorsStart = .;
    KEEP(*( .fini_array ));
    KEEP(*(SORT_BY_INIT_PRIORITY( .fini_array.* )));
    destructorsEnd = .;

    *(.rodata)
  }

  .data BLOCK(4K) : ALIGN(4K) {
    *(.data);

    /* Calculate TSS GDT descriptor */
    tssSize = tssEnd - tssBegin - 1;
    tssAddr = ABSOLUTE(tssBegin);

    tssSizeLow = tssSize & 0xffff;
    tssSizeHigh = (tssSize >> 16) & 0xf;

    tssAddrLow = tssAddr & 0xffff;
    tssAddrMid = (tssAddr >> 16) & 0xff;
    tssAddrHigh = (tssAddr >> 24) & 0xff;
    tssAddrHighest = (tssAddr >> 32) - tssAddr;

    /* https://wiki.osdev.org/images/f/f3/GDT_Entry.png */
    gdtTssData = (
         tssSizeLow         /* Limit  0:15 */
      | (tssAddrLow  << 16) /* Base   0:15 */
      | (tssAddrMid  << 32) /* Base  16:23 */
      | (0x89        << 40) /* Access Byte */
      | (tssSizeHigh << 48) /* Limit 16:19 */
      | (0x4         << 52) /* Flags       */
      | (tssAddrHigh << 56) /* Base  24:31 */
    ) - tssAddr;

    multibootDataSpace = .;
    . += 16K;
    multibootDataEnd = .;
  }

  .bss BLOCK(4K) : ALIGN(4K) {
    *(.bss)
  }

  . = ALIGN(4K);
  kernelEnd = .;

  /DISCARD/ : {
    *(.comment)
  }
}
