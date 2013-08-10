/* $Id: //WIFI_SOC/release/SDK_4_1_0_0/source/linux-2.6.21.x/include/asm-m68k/sun3-head.h#1 $ */
#ifndef __SUN3_HEAD_H
#define __SUN3_HEAD_H

#define KERNBASE        0xE000000  /* First address the kernel will eventually be */
#define LOAD_ADDR       0x4000      /* prom jumps to us here unless this is elf /boot */
#define FC_CONTROL  3
#define FC_SUPERD    5
#define FC_CPU      7

#endif /* __SUN3_HEAD_H */
