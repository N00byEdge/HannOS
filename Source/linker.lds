ENTRY(loader)
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

SECTIONS {
	. = 0x5000;

	.text :{
		*(.multiboot)
		*(.loadertext)
		*(.text)
	}

	.rodata :{
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

	.data :{
		*(.data);
	}

	.bss :{
		*(.bss)
	}

	.gdt :{
		*(.gdt)
	}

	/DISCARD/ : {
		*(.comment)
	}
}
