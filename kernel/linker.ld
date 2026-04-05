ENTRY(kernel_main)
SECTIONS {
    . = 0x1000;
    .text : {
		kernel/kernel.o(.text)
		*(.text) 
     }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
}
