/* Name: iarcompat.h
 * Project: AVR USB driver
 * Author: Christian Starkjohann
 * Creation Date: 2006-03-01
 * Tabsize: 4
 * Copyright: (c) 2006 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
 * This Revision: $Id$
 */

/*
General Description:
This header is included when we compile with the IAR C-compiler and assembler.
It defines macros for cross compatibility between gcc and IAR-cc.

Thanks to Oleg Semyonov for his help with the IAR tools port!
*/

#ifndef __iarcompat_h_INCLUDED__
#define __iarcompat_h_INCLUDED__

#if defined __IAR_SYSTEMS_ICC__ || defined __IAR_SYSTEMS_ASM__

/* Enable bit definitions */
#ifndef ENABLE_BIT_DEFINITIONS
#   define ENABLE_BIT_DEFINITIONS	1
#endif

/* Include IAR headers */
#include <ioavr.h>
#ifndef __IAR_SYSTEMS_ASM__
#   include <inavr.h>
#endif

#define __attribute__(arg)

#ifdef __IAR_SYSTEMS_ASM__
#   define __ASSEMBLER__
#endif

#ifdef __HAS_ELPM__
#   define PROGMEM __farflash
#else
#   define PROGMEM __flash
#endif

#define PRG_RDB(addr)   (*(PROGMEM char *)(addr))

/* The following definitions are not needed by the driver, but may be of some
 * help if you port a gcc based project to IAR.
 */
#define cli()       __disable_interrupt()
#define sei()       __enable_interrupt()
#define wdt_reset() __watchdog_reset()

/* Depending on the device you use, you may get problems with the way usbdrv.h
 * handles the differences between devices. Since IAR does not use #defines
 * for MCU registers, we can't check for the existence of a particular
 * register with an #ifdef. If the autodetection mechanism fails, include
 * definitions for the required USB_INTR_* macros in your usbconfig.h. See
 * usbconfig-prototype.h and usbdrv.h for details.
 */

#endif  /* defined __IAR_SYSTEMS_ICC__ || defined __IAR_SYSTEMS_ASM__ */
#endif  /* __iarcompat_h_INCLUDED__ */
