/* Name: osctune.h
 * Author: Christian Starkjohann
 * Creation Date: 2008-10-18
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
General Description:
This file is declared as C-header file although it is mostly documentation
how the RC oscillator can be kept in sync to the USB frame rate. The code
shown here must be added to usbconfig.h or this header file is included from
there. This code works only if D- is wired to the interrupt, not D+!!!

This is an alternative to the osccal routine in osccal.c. It has the advantage
that the synchronization is done continuously and that it has more compact
code size. The disadvantages are slow synchronization (it may take a while
until the driver works), that messages immediately after the SOF pulse may be
lost (and need to be retried by the host) and that the interrupt is on D-
contrary to most examples.

You may want to store a good calibration value in EEPROM for the next startup.
You know that the calibration value is good when the first USB message is
received. Do not store the value on every received message because the EEPROM
has a limited endurance.

Notes:
(*) You must declare the global character variable "lastTimer0Value" in your
main code.

(*) Timer 0 must be free running (not written by your code) and the prescaling
must be consistent with the TIMER0_PRESCALING define.

(*) Good values for Timer 0 prescaling depend on how precise the clock must
be tuned and how far away from the default clock rate the target clock is.
For precise tuning, choose a low prescaler factor, for a broad range of tuning
choose a high one. A prescaler factor of 64 is good for the entire OSCCAL
range and allows a precision of better than +/-1%. A prescaler factor of 8
allows tuning to slightly more than +/-6% of the default frequency and is
more precise than one step of OSCCAL. It is therefore not suitable to tune an
8 MHz oscillator to 12.5 MHz.

Thanks to Henrik Haftmann for the idea to this routine!
*/

#ifdef __ASSEMBLER__

// F_CPU isn't always usable in assembler, because with some AVR toolchains it
// ends in 'L' or 'UL' (e.g. 12000000UL)
// This clever routine creates a new macro, "F_CPU_NUMERIC" that doens't end in
// 'UL'.
// Idea from https://www.avrfreaks.net/comment/334055#comment-334055
.set F_CPU_NUMERIC,0  // init to 0
.irpc param,F_CPU     // go through all 'characters' in F_CPU
.ifnc \param,U        // if not a 'U'
.ifnc \param,L        // and not an 'L'
.set F_CPU_NUMERIC,F_CPU_NUMERIC*10+\param // multiply by 10, then add new digit
.endif
.endif
.endr 

#define TIMER0_PRESCALING           64 /* must match the configuration for TIMER0 in main */
#define TOLERATED_DEVIATION_PPT     5  /* max clock deviation before we tune in 1/10 (e.g. 5 = 0.5%) */

/* derived constants: */

/* Timer ticks in 1ms - the expected number of ticks between USB SOFs ('Start Of Frames') */
#define EXPECTED_TIMER0_INCREMENT   ((F_CPU_NUMERIC / (1000 * TIMER0_PRESCALING)) & 0xff) 

#define TOLERATED_DEVIATION         (TOLERATED_DEVIATION_PPT * F_CPU_NUMERIC / (1000000 * TIMER0_PRESCALING))

macro tuneOsccal
    push    YH                              ;[0]  Save HY. We are allowed to clobber only YL.
    in      YL, TCNT0                       ;[2]  YL = TCNT
    lds     YH, lastTimer0Value             ;[3]  YH = lastTimer0Value
    sts     lastTimer0Value, YL             ;[5]  lastTimer0Value = YL (store new timer value for next time)
    sub     YL, YH                          ;[7]  YL = YL - YH (YL = time passed since last frame)
    subi    YL, EXPECTED_TIMER0_INCREMENT   ;[8]  YL = YL - EXPECTED_TIMER0_INCREMENT
#if OSCCAL > 0x3f   /* outside I/O addressable range */
    lds     YH, OSCCAL                      ;[6]  HY = OSCCAL (timer calibration byte)
#else
    in      YH, OSCCAL                      ;[6]  assembler modle uses __SFR_OFFSET == 0
#endif
    cpi     YL, TOLERATED_DEVIATION + 1     ;[10] Compare YL with TOLERATED_DEVIATION + 1 
    brmi    notTooHigh                      ;[11] Branch to notTooHigh if the result is <0
    subi    YH, 1                           ;[12] Clock rate was too high - subtract 1 from YH (the OSCCal we loaded above).
;   brcs    tuningOverflow                  ; optionally check for overflow
    rjmp    osctuneDone                     ;[13] goto osctuneDone.
notTooHigh:
    cpi     YL, -TOLERATED_DEVIATION        ;[13] Compare YL with -TOLERATED_DEVIATION.
    brpl    osctuneDone                     ;[14] If it compares higher, we are in spec. No need to do anything.
    inc     YH                              ;[15] clock rate was too low - add 1 to YH (the OSCCal we loaded above).
;   breq    tuningOverflow                  ; optionally check for overflow
osctuneDone:
#if OSCCAL > 0x3f   /* outside I/O addressable range */
    sts     OSCCAL, YH                      ;[12-13] store tuned value
#else
    out     OSCCAL, YH                      ;[12-13] store tuned value
#endif
tuningOverflow:
    pop     YH                              ;[17] Restore YH.
    endm                                    ;[19] max number of cycles
#endif

#define USB_SOF_HOOK        tuneOsccal
