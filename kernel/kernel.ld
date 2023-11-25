OUTPUT_ARCH( "riscv" )
ENTRY( _entry_nk)

SECTIONS
{
  /*
   * ensure that entry.S / _entry is at 0x80000000,
   * where qemu's -kernel jumps.
   */
  . = 0x80000000;

  .text : {
  kernel/NK*.o(.text)
  kernel/NK*.o(.text.*)
   . = ALIGN(0x1000);
   _trampoline_nk = .;
    *(trampsec_nk)
    . = ALIGN(0x1000);
    ASSERT(. - _trampoline_nk == 0x1000, "error: trampoline larger than one page");
  }

 PROVIDE(etext_nk = .);
 .rodata : {
    . = ALIGN(16);
    kernel/NK*(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    kernel/NK*(.rodata .rodata.*)
  }

  .data : {
    . = ALIGN(16);
    kernel/NK*(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    kernel/NK*(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    kernel/NK*(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    kernel/NK*(.bss .bss.*)
  }

  PROVIDE(end_nk = .);


  . = 0x81000000;


  .text1 : {
    *(.text .text.*)
    . = ALIGN(0x1000);
    _trampoline = .;
    *(trampsec)
    . = ALIGN(0x1000);
    ASSERT(. - _trampoline == 0x1000, "error: trampoline larger than one page");
    PROVIDE(etext = .);
  }

  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
  }

  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    *(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
  }

  PROVIDE(end = .);
}
