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
