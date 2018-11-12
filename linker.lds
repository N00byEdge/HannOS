ENTRY(loader)
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

SECTIONS {
	. = 4K;

	.boot :{
		*(.multiboot)
		*(.text*)
	}

	. = 2M;

	.text :{
		
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
