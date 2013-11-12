/* Name: main.c
 * Project: hid-custom-rq example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-07
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
This example should run on most AVRs with only little changes. No special
hardware resources except INT0 are used. You may have to change usbconfig.h for
different I/O pins for USB. Please note that USB D+ must be the INT0 pin, or
at least be connected to INT0 as well.
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */

#if TUNE_OSCCAL
uchar   lastTimer0Value;
#endif

#if CALIBRATE_OSCCAL
#include "osccal.c"
#endif


/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uchar    dataBuffer[64];
static uchar    writeIndex;

uchar usbFunctionWrite(uchar *data, uchar len)
{

    if(writeIndex + len <= sizeof(dataBuffer)){
        uchar i;
        for(i = 0; i < len; i++){
            dataBuffer[writeIndex++] = *data++;
        }
    }
    return writeIndex >= sizeof(dataBuffer);
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
    if(rq->bRequest == CUSTOM_RQ_SET_DATA){
        writeIndex = 0;
        return USB_NO_MSG;
    }else if(rq->bRequest == CUSTOM_RQ_GET_DATA){
        usbMsgPtr = dataBuffer;     /* tell the driver which data to return */
        return sizeof(dataBuffer);  /* tell the driver how many bytes to send */
    }else if(rq->bRequest == CUSTOM_RQ_SET_OSCCAL){
        OSCCAL = rq->wValue.bytes[0];
    }else if(rq->bRequest == CUSTOM_RQ_GET_OSCCAL){
        usbMsgPtr = (uchar *)&OSCCAL;
        return 1;
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void)
{
uchar   i;

    wdt_enable(WDTO_1S);
    /* If you don't use the watchdog, replace the call above with a wdt_disable().
     * On newer devices, the status of the watchdog (on/off, period) is PRESERVED
     * OVER RESET!
     */
    odDebugInit();
    DBG1(0x00, 0, 0);       /* debug output: main starts */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    TCCR2 = 9 | (1 << COM20);
    OCR2 = 3;               /* should give F_CPU/8 clock */

    DDRB = (1 << 2) | (1 << 3);
    TCCR0 = 3;              /* 1/64 prescaler */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    DBG1(0x01, 0, 0);       /* debug output: main loop starts */
    for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
        cli();              /* disable interrupts for some cycles, use other cli as nop */
        cli();
        cli();
        cli();
        cli();
        cli();
        cli();
        cli();
        cli();
        cli();
        cli();
        cli();
        cli();
        cli();
        sei();
    }
}

/* ------------------------------------------------------------------------- */
