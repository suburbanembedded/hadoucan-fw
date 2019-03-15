/*
*****************************************************************************
**

**  File        : stm32_flash.ld
**
**  Abstract    : Linker script for STM32H750VB Device with
**                128KByte FLASH, 1056KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Environment : Atollic TrueSTUDIO(R)
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
**  (c)Copyright Atollic AB.
**  You may use this file as-is or modify it according to the needs of your
**  project. This file may only be built (assembled or compiled and linked)
**  using the Atollic TrueSTUDIO(R) product. The use of this file together
**  with other tools than Atollic TrueSTUDIO(R) is not permitted.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = 0x20020000;    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size  = 0x0800; /* required amount of heap  */
_Min_Stack_Size = 0x0800; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  ITCMRAM (xrw)   : ORIGIN = 0x00000000, LENGTH = 64K

  FLASH   (rx)    : ORIGIN = 0x08000000, LENGTH = 128K

  DTCMRAM (rw)   : ORIGIN = 0x20000000, LENGTH = 128K
  
  AXI_D1_SRAM (xrw)  : ORIGIN = 0x24000000, LENGTH = 512K

  /* CPU Inst */
  AHB_D2_SRAM1 (xrw)  : ORIGIN = 0x30000000, LENGTH = 128K
  /* CPU Data */
  AHB_D2_SRAM2 (rw)  : ORIGIN = 0x30020000, LENGTH = 128K
  /* Peripheral Buffers */
  AHB_D2_SRAM3 (rw)  : ORIGIN = 0x30040000, LENGTH = 32K

  AHB_D3_SRAM4 (rw)  : ORIGIN = 0x38000000, LENGTH = 64K
  
  BBRAM  (rw)    : ORIGIN = 0x38800000, LENGTH = 4K

  QUADSPI (rx)    : ORIGIN = 0x90000000, LENGTH = 16M
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(512);
    start_isr_vector = .;
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
    end_isr_vector = .;
  } >AXI_D1_SRAM

  /* The OS goes in ITCM */
  /*
  .ram_itcm : ALIGN(4)
  {
    start_ram_itcm = .;
    *(.ram_itcm)
    *libfreertos_v9.a:*
    *libfreertos_v9_cmsis.a:*
    *libfreertos_v9_heap4.a:*
    . = ALIGN(4);
    end_ram_itcm = .;
  } >ITCMRAM AT> AXI_D1_SRAM
  start_ram_itcm_laddr = LOADADDR(.ram_itcm);
  */

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >AXI_D1_SRAM

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >AXI_D1_SRAM

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >AXI_D1_SRAM
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >AXI_D1_SRAM

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >AXI_D1_SRAM
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >AXI_D1_SRAM
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >AXI_D1_SRAM

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >DTCMRAM AT> AXI_D1_SRAM
  
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >DTCMRAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >DTCMRAM

  /* RAM sections */
  .ram_dtcm : ALIGN(4)
  {
    start_ram_dtcm = .;
    *(.ram_dtcm)
    . = ALIGN(4);
    end_ram_dtcm = .;
  } >DTCMRAM AT> AXI_D1_SRAM
  start_ram_dtcm_laddr = LOADADDR(.ram_dtcm);
  
  .ram_dtcm_noload (NOLOAD) : ALIGN(4)
  {
    start_ram_dtcm_noload = .;
    *(.ram_dtcm_noload)
    . = ALIGN(4);
    end_ram_dtcm_noload = .;
  } >DTCMRAM

  .ram_d1_s0 : ALIGN(4)
  {
    start_ram_d1_s0 = .;
    *(.ram_d1_s0)
    . = ALIGN(4);
    end_ram_d1_s0 = .;
  } >AXI_D1_SRAM AT> AXI_D1_SRAM
  start_ram_d1_s0_laddr = LOADADDR(.ram_d1_s0);

  .ram_d1_s0_noload (NOLOAD) : ALIGN(4)
  {
    start_ram_d1_s0_noload = .;
    *(.ram_d1_s0_noload)
    . = ALIGN(4);
    end_ram_d1_s0_noload = .;
  } >AXI_D1_SRAM

  .ram_d2_s1 : ALIGN(4)
  {
    start_ram_d2_s1 = .;
    *(.ram_d2_s1)
    . = ALIGN(4);
    end_ram_d2_s1 = .;
  } >AHB_D2_SRAM1 AT> AXI_D1_SRAM
  start_ram_d2_s1_laddr = LOADADDR(.ram_d2_s1);
  
  .ram_d2_s1_noload (NOLOAD) : ALIGN(4)
  {
    start_ram_d2_s1_noload = .;
    *(.ram_d2_s1_noload)
    . = ALIGN(4);
    end_ram_d2_s1_noload = .;
  } >AHB_D2_SRAM1

  .ram_d2_s2 : ALIGN(4)
  {
    start_ram_d2_s2 = .;
    *(.ram_d2_s2)
    . = ALIGN(4);
    end_ram_d2_s2 = .;
  } >AHB_D2_SRAM2 AT> AXI_D1_SRAM
  start_ram_d2_s2_laddr = LOADADDR(.ram_d2_s2);

  .ram_d2_s2_noload (NOLOAD) : ALIGN(4)
  {
    start_ram_d2_s2_noload = .;
    *(.ram_d2_s2_noload)
    . = ALIGN(4);
    end_ram_d2_s2_noload = .;
  } >AHB_D2_SRAM2

  .ram_d2_s3 : ALIGN(4)
  {
    start_ram_d2_s3 = .;
    *(.ram_d2_s3)
    . = ALIGN(4);
    end_ram_d2_s3 = .;
  } >AHB_D2_SRAM3 AT> AXI_D1_SRAM
  start_ram_d2_s3_laddr = LOADADDR(.ram_d2_s3);
  
  .ram_d2_s3_noload (NOLOAD) : ALIGN(4)
  {
    start_ram_d2_s3_noload = .;
    *(.ram_d2_s3_noload)
    . = ALIGN(4);
    end_ram_d2_s3_noload = .;
  } >AHB_D2_SRAM3

  .ram_d3_s4 : ALIGN(4)
  {
    start_ram_d3_s4 = .;
    *(.ram_d3_s4)
    . = ALIGN(4);
    end_ram_d3_s4 = .;
  } >AHB_D3_SRAM4 AT> AXI_D1_SRAM
  start_ram_d3_s4_laddr = LOADADDR(.ram_d3_s4);
  
  .ram_d3_s4_noload (NOLOAD) : ALIGN(4)
  {
    start_ram_d3_s4_noload = .;
    *(.ram_d3_s4_noload)
    . = ALIGN(4);
    end_ram_d3_s4_noload = .;
  } >AHB_D3_SRAM4

  .bbram : ALIGN(4)
  {
    start_bbram = .;
    *(.bbram)
    . = ALIGN(4);
    end_bbram = .;
  } >BBRAM AT> AXI_D1_SRAM
  start_bbram_laddr = LOADADDR(.bbram);
  
  .bbram_noload (NOLOAD) : ALIGN(4)
  {
    start_bbram_noload = .;
    *(.bbram_noload)
    . = ALIGN(4);
    end_bbram_noload = .;
  } >BBRAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}


