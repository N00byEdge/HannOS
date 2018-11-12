ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS {
	. = 0x00001000;

	.text :{
		*(.multiboot)
		*(.text*)
	}

	.rodata :{
		*(.rodata)
	}

	.data :{
		*(.data);
	}

	.bss :{
		*(.bss)
	}

	.gdt : {
		*(.gdt)
	}

	/DISCARD/ : {
		*(.fini_array)
		*(.comment)
	}
}
