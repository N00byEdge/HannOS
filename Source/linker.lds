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
		*(.fini_array)
		*(.comment)
	}
}
