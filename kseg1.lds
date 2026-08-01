OUTPUT_ARCH(mips)

MEMORY
{
	IRAM      (xrw)  : ORIGIN = 0xa0000000,   LENGTH = 256M
}

SECTIONS
{
	.text : ALIGN(4)
	{
		__start = .;
		KEEP(*(.init.text*))
		*(.text*)
	} > IRAM

	.data : ALIGN(4)
	{
		*(.rodata*)
		*(.data*)
	} > IRAM

	.bss :
	{
		. = ALIGN(4);
		bssbegin = .;
		*(.sbss*)
		*(.bss*)
		*(COMMON)
		*(.scommon*)
		. = ALIGN(4);
		bssend = .;
	} > IRAM

	.discard(NOLOAD):
	{
		*(.MIPS.abiflags)
	}
}
