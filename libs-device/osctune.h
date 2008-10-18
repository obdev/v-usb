/* Name: osctune.h
 * Author: Christian Starkjohann
 * Creation Date: 2008-10-18
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
 * This Revision: $Id$
 */

/*
General Description:
This file is declared as C-header file although it is mostly documentation
how the RC oscillator can be kept in sync to the USB frame rate. The code
shown here must be added to usbconfig.h or this header file is included from
there. This code works only if D- is wired to the interrupt, not D+!!!

You may want to store a good calibration value in EEPROM. You know that the
calibration value is good when the first USB message is received. Do not store
the value on every received message because the EEPROM has a limited endurance.
*/

#define TIMER0_PRESCALING           8 /* must match the configuration for TIMER0 in main */
#define TOLERATED_DEVIATION_PPT     5 /* max clock deviation before we tune in 1/10 % */
/* derived constants: */
#define EXPECTED_TIMER0_INCREMENT   ((F_CPU / TIMER0_PRESCALING) & 0xff)
#define TOLERATED_DEVIATION         (TOLERATED_DEVIATION_PPT * F_CPU / (1000000 * TIMER0_PRESCALING))

#ifdef __ASSEMBLER__
macro tuneOsccal
    in      YL, TCNT0
    lds     YH, lastTimer0Value
    sts     lastTimer0Value, YL
    sub     YL, YH      ; time passed since last frame
    subi    YL, EXPECTED_TIMER0_INCREMENT
    in      YH, OSCCAL
    cpi     YL, (TOLERATED_DEVIATION) + 1
    brlt    notTooHigh
    dec     YH          ; clock rate was too high
    rjmp    osctuneDone
notTooHigh:
    cpi     YL, -TOLERATED_DEVIATION
    brge    osctuneDone ; not too low
    inc     YH          ; clock rate was too low
osctuneDone:
    out     OSCCAL, YH  ; store tuned value
    endm
#endif

#define USB_SOF_HOOK        tuneOsccal
