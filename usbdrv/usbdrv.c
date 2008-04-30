/* Name: usbdrv.c
 * Project: AVR USB driver
 * Author: Christian Starkjohann
 * Creation Date: 2004-12-29
 * Tabsize: 4
 * Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
 * This Revision: $Id$
 */

#include "iarcompat.h"
#ifndef __IAR_SYSTEMS_ICC__
#   include <avr/io.h>
#   include <avr/pgmspace.h>
#endif
#include "usbdrv.h"
#include "oddebug.h"

/*
General Description:
This module implements the C-part of the USB driver. See usbdrv.h for a
documentation of the entire driver.
*/

/* ------------------------------------------------------------------------- */

/* raw USB registers / interface to assembler code: */
uchar usbRxBuf[2*USB_BUFSIZE];  /* raw RX buffer: PID, 8 bytes data, 2 bytes CRC */
uchar       usbInputBufOffset;  /* offset in usbRxBuf used for low level receiving */
uchar       usbDeviceAddr;      /* assigned during enumeration, defaults to 0 */
uchar       usbNewDeviceAddr;   /* device ID which should be set after status phase */
uchar       usbConfiguration;   /* currently selected configuration. Administered by driver, but not used */
volatile schar usbRxLen;        /* = 0; number of bytes in usbRxBuf; 0 means free, -1 for flow control */
uchar       usbCurrentTok;      /* last token received or endpoint number for last OUT token if != 0 */
uchar       usbRxToken;         /* token for data we received; or endpont number for last OUT */
uchar       usbMsgLen = 0xff;   /* remaining number of bytes, no msg to send if -1 (see usbMsgPtr) */
volatile uchar usbTxLen = USBPID_NAK;   /* number of bytes to transmit with next IN token or handshake token */
uchar       usbTxBuf[USB_BUFSIZE];/* data to transmit with next IN, free if usbTxLen contains handshake token */
#if USB_COUNT_SOF
volatile uchar  usbSofCount;    /* incremented by assembler module every SOF */
#endif
#if USB_CFG_HAVE_INTRIN_ENDPOINT
volatile uchar usbTxLen1 = USBPID_NAK;  /* TX count for endpoint 1 */
uchar       usbTxBuf1[USB_BUFSIZE];     /* TX data for endpoint 1 */
#   if USB_CFG_HAVE_INTRIN_ENDPOINT3
volatile uchar usbTxLen3 = USBPID_NAK;  /* TX count for endpoint 3 */
uchar       usbTxBuf3[USB_BUFSIZE];     /* TX data for endpoint 3 */
#   endif
#endif

/* USB status registers / not shared with asm code */
uchar           *usbMsgPtr;     /* data to transmit next -- ROM or RAM address */
static uchar    usbMsgFlags;    /* flag values see below */

#define USB_FLG_TX_PACKET       (1<<0)
/* Leave free 6 bits after TX_PACKET. This way we can increment usbMsgFlags to toggle TX_PACKET */
#define USB_FLG_MSGPTR_IS_ROM   (1<<6)
#define USB_FLG_USE_DEFAULT_RW  (1<<7)

/*
optimizing hints:
- do not post/pre inc/dec integer values in operations
- assign value of PRG_RDB() to register variables and don't use side effects in arg
- use narrow scope for variables which should be in X/Y/Z register
- assign char sized expressions to variables to force 8 bit arithmetics
*/

/* ------------------------------------------------------------------------- */

#if USB_CFG_DESCR_PROPS_STRINGS == 0

#if USB_CFG_DESCR_PROPS_STRING_0 == 0
#undef USB_CFG_DESCR_PROPS_STRING_0
#define USB_CFG_DESCR_PROPS_STRING_0    sizeof(usbDescriptorString0)
PROGMEM char usbDescriptorString0[] = { /* language descriptor */
    4,          /* sizeof(usbDescriptorString0): length of descriptor in bytes */
    3,          /* descriptor type */
    0x09, 0x04, /* language index (0x0409 = US-English) */
};
#endif

#if USB_CFG_DESCR_PROPS_STRING_VENDOR == 0 && USB_CFG_VENDOR_NAME_LEN
#undef USB_CFG_DESCR_PROPS_STRING_VENDOR
#define USB_CFG_DESCR_PROPS_STRING_VENDOR   sizeof(usbDescriptorStringVendor)
PROGMEM int  usbDescriptorStringVendor[] = {
    USB_STRING_DESCRIPTOR_HEADER(USB_CFG_VENDOR_NAME_LEN),
    USB_CFG_VENDOR_NAME
};
#endif

#if USB_CFG_DESCR_PROPS_STRING_PRODUCT == 0 && USB_CFG_DEVICE_NAME_LEN
#undef USB_CFG_DESCR_PROPS_STRING_PRODUCT
#define USB_CFG_DESCR_PROPS_STRING_PRODUCT   sizeof(usbDescriptorStringDevice)
PROGMEM int  usbDescriptorStringDevice[] = {
    USB_STRING_DESCRIPTOR_HEADER(USB_CFG_DEVICE_NAME_LEN),
    USB_CFG_DEVICE_NAME
};
#endif

#if USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER == 0 && USB_CFG_SERIAL_NUMBER_LEN
#undef USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER
#define USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER    sizeof(usbDescriptorStringSerialNumber)
PROGMEM int usbDescriptorStringSerialNumber[] = {
    USB_STRING_DESCRIPTOR_HEADER(USB_CFG_SERIAL_NUMBER_LEN),
    USB_CFG_SERIAL_NUMBER
};
#endif

#endif  /* USB_CFG_DESCR_PROPS_STRINGS == 0 */

#if USB_CFG_DESCR_PROPS_DEVICE == 0
#undef USB_CFG_DESCR_PROPS_DEVICE
#define USB_CFG_DESCR_PROPS_DEVICE  sizeof(usbDescriptorDevice)
PROGMEM char usbDescriptorDevice[] = {    /* USB device descriptor */
    18,         /* sizeof(usbDescriptorDevice): length of descriptor in bytes */
    USBDESCR_DEVICE,        /* descriptor type */
    0x10, 0x01,             /* USB version supported */
    USB_CFG_DEVICE_CLASS,
    USB_CFG_DEVICE_SUBCLASS,
    0,                      /* protocol */
    8,                      /* max packet size */
    /* the following two casts affect the first byte of the constant only, but
     * that's sufficient to avoid a warning with the default values.
     */
    (char)USB_CFG_VENDOR_ID,/* 2 bytes */
    (char)USB_CFG_DEVICE_ID,/* 2 bytes */
    USB_CFG_DEVICE_VERSION, /* 2 bytes */
    USB_CFG_DESCR_PROPS_STRING_VENDOR != 0 ? 1 : 0,         /* manufacturer string index */
    USB_CFG_DESCR_PROPS_STRING_PRODUCT != 0 ? 2 : 0,        /* product string index */
    USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER != 0 ? 3 : 0,  /* serial number string index */
    1,          /* number of configurations */
};
#endif

#if USB_CFG_DESCR_PROPS_HID_REPORT != 0 && USB_CFG_DESCR_PROPS_HID == 0
#undef USB_CFG_DESCR_PROPS_HID
#define USB_CFG_DESCR_PROPS_HID     9   /* length of HID descriptor in config descriptor below */
#endif

#if USB_CFG_DESCR_PROPS_CONFIGURATION == 0
#undef USB_CFG_DESCR_PROPS_CONFIGURATION
#define USB_CFG_DESCR_PROPS_CONFIGURATION   sizeof(usbDescriptorConfiguration)
PROGMEM char usbDescriptorConfiguration[] = {    /* USB configuration descriptor */
    9,          /* sizeof(usbDescriptorConfiguration): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    18 + 7 * USB_CFG_HAVE_INTRIN_ENDPOINT + (USB_CFG_DESCR_PROPS_HID & 0xff), 0,
                /* total length of data returned (including inlined descriptors) */
    1,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    USBATTR_SELFPOWER,      /* attributes */
#else
    (char)USBATTR_BUSPOWER, /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,            /* max USB current in 2mA units */
/* interface descriptor follows inline: */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    USB_CFG_HAVE_INTRIN_ENDPOINT,   /* endpoints excl 0: number of endpoint descriptors to follow */
    USB_CFG_INTERFACE_CLASS,
    USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,          /* string index for interface */
#if (USB_CFG_DESCR_PROPS_HID & 0xff)    /* HID descriptor */
    9,          /* sizeof(usbDescrHID): length of descriptor in bytes */
    USBDESCR_HID,   /* descriptor type: HID */
    0x01, 0x01, /* BCD representation of HID version */
    0x00,       /* target country code */
    0x01,       /* number of HID Report (or other HID class) Descriptor infos to follow */
    0x22,       /* descriptor type: report */
    USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH, 0,  /* total length of report descriptor */
#endif
#if USB_CFG_HAVE_INTRIN_ENDPOINT    /* endpoint descriptor for endpoint 1 */
    7,          /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    (char)0x81, /* IN endpoint number 1 */
    0x03,       /* attrib: Interrupt endpoint */
    8, 0,       /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL, /* in ms */
#endif
};
#endif

/* We don't use prog_int or prog_int16_t for compatibility with various libc
 * versions. Here's an other compatibility hack:
 */
#ifndef PRG_RDB
#define PRG_RDB(addr)   pgm_read_byte(addr)
#endif

typedef union{
    unsigned    word;
    uchar       *ptr;
    uchar       bytes[2];
}converter_t;
/* We use this union to do type conversions. This is better optimized than
 * type casts in gcc 3.4.3 and much better than using bit shifts to build
 * ints from chars. Byte ordering is not a problem on an 8 bit platform.
 */

/* ------------------------------------------------------------------------- */

static inline void  usbResetDataToggling(void)
{
#if USB_CFG_HAVE_INTRIN_ENDPOINT
    USB_SET_DATATOKEN1(USB_INITIAL_DATATOKEN);  /* reset data toggling for interrupt endpoint */
#   if USB_CFG_HAVE_INTRIN_ENDPOINT3
    USB_SET_DATATOKEN3(USB_INITIAL_DATATOKEN);  /* reset data toggling for interrupt endpoint */
#   endif
#endif
}

static inline void  usbResetStall(void)
{
#if USB_CFG_IMPLEMENT_HALT && USB_CFG_HAVE_INTRIN_ENDPOINT
        usbTxLen1 = USBPID_NAK;
#if USB_CFG_HAVE_INTRIN_ENDPOINT3
        usbTxLen3 = USBPID_NAK;
#endif
#endif
}

/* ------------------------------------------------------------------------- */

#if USB_CFG_HAVE_INTRIN_ENDPOINT
USB_PUBLIC void usbSetInterrupt(uchar *data, uchar len)
{
uchar       *p, i;

#if USB_CFG_IMPLEMENT_HALT
    if(usbTxLen1 == USBPID_STALL)
        return;
#endif
#if 0   /* No runtime checks! Caller is responsible for valid data! */
    if(len > 8) /* interrupt transfers are limited to 8 bytes */
        len = 8;
#endif
    if(usbTxLen1 & 0x10){   /* packet buffer was empty */
        usbTxBuf1[0] ^= USBPID_DATA0 ^ USBPID_DATA1;    /* toggle token */
    }else{
        usbTxLen1 = USBPID_NAK; /* avoid sending outdated (overwritten) interrupt data */
    }
    p = usbTxBuf1 + 1;
    for(i=len;i--;)
        *p++ = *data++;
    usbCrc16Append(&usbTxBuf1[1], len);
    usbTxLen1 = len + 4;    /* len must be given including sync byte */
    DBG2(0x21, usbTxBuf1, len + 3);
}
#endif

#if USB_CFG_HAVE_INTRIN_ENDPOINT3
USB_PUBLIC void usbSetInterrupt3(uchar *data, uchar len)
{
uchar       *p, i;

    if(usbTxLen3 & 0x10){   /* packet buffer was empty */
        usbTxBuf3[0] ^= USBPID_DATA0 ^ USBPID_DATA1;    /* toggle token */
    }else{
        usbTxLen3 = USBPID_NAK; /* avoid sending outdated (overwritten) interrupt data */
    }
    p = usbTxBuf3 + 1;
    for(i=len;i--;)
        *p++ = *data++;
    usbCrc16Append(&usbTxBuf3[1], len);
    usbTxLen3 = len + 4;    /* len must be given including sync byte */
    DBG2(0x23, usbTxBuf3, len + 3);
}
#endif


static uchar usbRead(uchar *data, uchar len)
{
#if USB_CFG_IMPLEMENT_FN_READ
    if(!(usbMsgFlags & USB_FLG_USE_DEFAULT_RW)){
        if(len != 0)    /* don't bother app with 0 sized reads */
            len = usbFunctionRead(data, len);
    }else
#endif
    {
        uchar i = len, *r = usbMsgPtr;
        if(usbMsgFlags & USB_FLG_MSGPTR_IS_ROM){    /* ROM data */
            while(i--){
                uchar c = PRG_RDB(r);    /* assign to char size variable to enforce byte ops */
                *data++ = c;
                r++;
            }
        }else{                  /* RAM data */
            while(i--)
                *data++ = *r++;
        }
        usbMsgPtr = r;
    }
    return len;
}


#define GET_DESCRIPTOR(cfgProp, staticName)         \
    if(cfgProp){                                    \
        if((cfgProp) & USB_PROP_IS_RAM)             \
            flags &= ~USB_FLG_MSGPTR_IS_ROM;        \
        if((cfgProp) & USB_PROP_IS_DYNAMIC){        \
            replyLen = usbFunctionDescriptor(rq);   \
        }else{                                      \
            replyData = (uchar *)(staticName);      \
            SET_REPLY_LEN((cfgProp) & 0xff);        \
        }                                           \
    }
/* We use if() instead of #if in the macro above because #if can't be used
 * in macros and the compiler optimizes constant conditions anyway.
 */


/* Don't make this function static to avoid inlining.
 * The entire function would become too large and exceed the range of
 * relative jumps.
 * 2006-02-25: Either gcc 3.4.3 is better than the gcc used when the comment
 * above was written, or other parts of the code have changed. We now get
 * better results with an inlined function. Test condition: PowerSwitch code.
 */
static void usbProcessRx(uchar *data, uchar len)
{
usbRequest_t    *rq = (void *)data;
uchar           replyLen = 0, flags = USB_FLG_USE_DEFAULT_RW;
/* We use if() cascades because the compare is done byte-wise while switch()
 * is int-based. The if() cascades are therefore more efficient.
 */
/* usbRxToken can be:
 * 0x2d 00101101 (USBPID_SETUP for endpoint 0)
 * 0xe1 11100001 (USBPID_OUT for endpoint 0)
 * 0xff 11111111 (USBPID_OUT for endpoint 1)
 */
    DBG2(0x10 + ((usbRxToken >> 1) & 3), data, len);    /* SETUP0=12; OUT0=10; OUT1=13 */
#ifdef USB_RX_USER_HOOK
    USB_RX_USER_HOOK(data, len)
#endif
#if USB_CFG_IMPLEMENT_FN_WRITEOUT
    if(usbRxToken < 0x10){  /* endpoint number in usbRxToken */
        usbFunctionWriteOut(data, len);
        return; /* no reply expected, hence no usbMsgPtr, usbMsgFlags, usbMsgLen set */
    }
#endif
    if(usbRxToken == (uchar)USBPID_SETUP){
        if(len != 8)    /* Setup size must be always 8 bytes. Ignore otherwise. */
            return;
        usbTxLen = USBPID_NAK;  /* abort pending transmit */
        uchar type = rq->bmRequestType & USBRQ_TYPE_MASK;
        if(type == USBRQ_TYPE_STANDARD){
            #define SET_REPLY_LEN(len)  replyLen = (len); usbMsgPtr = replyData
            /* This macro ensures that replyLen and usbMsgPtr are always set in the same way.
             * That allows optimization of common code in if() branches */
            uchar *replyData = usbTxBuf + 9; /* there is 3 bytes free space at the end of the buffer */
            replyData[0] = 0;   /* common to USBRQ_GET_STATUS and USBRQ_GET_INTERFACE */
            if(rq->bRequest == USBRQ_GET_STATUS){           /* 0 */
                uchar __attribute__((__unused__)) recipient = rq->bmRequestType & USBRQ_RCPT_MASK;  /* assign arith ops to variables to enforce byte size */
#if USB_CFG_IS_SELF_POWERED
                if(recipient == USBRQ_RCPT_DEVICE)
                    replyData[0] =  USB_CFG_IS_SELF_POWERED;
#endif
#if USB_CFG_HAVE_INTRIN_ENDPOINT && USB_CFG_IMPLEMENT_HALT
                if(recipient == USBRQ_RCPT_ENDPOINT && rq->wIndex.bytes[0] == 0x81)   /* request status for endpoint 1 */
                    replyData[0] = usbTxLen1 == USBPID_STALL;
#endif
                replyData[1] = 0;
                SET_REPLY_LEN(2);
            }else if(rq->bRequest == USBRQ_SET_ADDRESS){    /* 5 */
                usbNewDeviceAddr = rq->wValue.bytes[0];
#ifdef USB_SET_ADDRESS_HOOK
                USB_SET_ADDRESS_HOOK();
#endif
            }else if(rq->bRequest == USBRQ_GET_DESCRIPTOR){ /* 6 */
                flags = USB_FLG_MSGPTR_IS_ROM | USB_FLG_USE_DEFAULT_RW;
                if(rq->wValue.bytes[1] == USBDESCR_DEVICE){ /* 1 */
                    GET_DESCRIPTOR(USB_CFG_DESCR_PROPS_DEVICE, usbDescriptorDevice)
                }else if(rq->wValue.bytes[1] == USBDESCR_CONFIG){   /* 2 */
                    GET_DESCRIPTOR(USB_CFG_DESCR_PROPS_CONFIGURATION, usbDescriptorConfiguration)
                }else if(rq->wValue.bytes[1] == USBDESCR_STRING){   /* 3 */
#if USB_CFG_DESCR_PROPS_STRINGS & USB_PROP_IS_DYNAMIC
                    if(USB_CFG_DESCR_PROPS_STRINGS & USB_PROP_IS_RAM)
                        flags &= ~USB_FLG_MSGPTR_IS_ROM;
                    replyLen = usbFunctionDescriptor(rq);
#else   /* USB_CFG_DESCR_PROPS_STRINGS & USB_PROP_IS_DYNAMIC */
                    if(rq->wValue.bytes[0] == 0){   /* descriptor index */
                        GET_DESCRIPTOR(USB_CFG_DESCR_PROPS_STRING_0, usbDescriptorString0)
                    }else if(rq->wValue.bytes[0] == 1){
                        GET_DESCRIPTOR(USB_CFG_DESCR_PROPS_STRING_VENDOR, usbDescriptorStringVendor)
                    }else if(rq->wValue.bytes[0] == 2){
                        GET_DESCRIPTOR(USB_CFG_DESCR_PROPS_STRING_PRODUCT, usbDescriptorStringDevice)
                    }else if(rq->wValue.bytes[0] == 3){
                        GET_DESCRIPTOR(USB_CFG_DESCR_PROPS_STRING_SERIAL_NUMBER, usbDescriptorStringSerialNumber)
                    }else if(USB_CFG_DESCR_PROPS_UNKNOWN & USB_PROP_IS_DYNAMIC){
                        replyLen = usbFunctionDescriptor(rq);
                    }
#endif  /* USB_CFG_DESCR_PROPS_STRINGS & USB_PROP_IS_DYNAMIC */
#if USB_CFG_DESCR_PROPS_HID_REPORT  /* only support HID descriptors if enabled */
                }else if(rq->wValue.bytes[1] == USBDESCR_HID){          /* 0x21 */
                    GET_DESCRIPTOR(USB_CFG_DESCR_PROPS_HID, usbDescriptorConfiguration + 18)
                }else if(rq->wValue.bytes[1] == USBDESCR_HID_REPORT){   /* 0x22 */
                    GET_DESCRIPTOR(USB_CFG_DESCR_PROPS_HID_REPORT, usbDescriptorHidReport)
#endif  /* USB_CFG_DESCR_PROPS_HID_REPORT */
                }else if(USB_CFG_DESCR_PROPS_UNKNOWN & USB_PROP_IS_DYNAMIC){
                    replyLen = usbFunctionDescriptor(rq);
                }
            }else if(rq->bRequest == USBRQ_GET_CONFIGURATION){  /* 8 */
                replyData = &usbConfiguration;  /* send current configuration value */
                SET_REPLY_LEN(1);
            }else if(rq->bRequest == USBRQ_SET_CONFIGURATION){  /* 9 */
                usbConfiguration = rq->wValue.bytes[0];
                usbResetStall();
            }else if(rq->bRequest == USBRQ_GET_INTERFACE){      /* 10 */
                SET_REPLY_LEN(1);
#if USB_CFG_HAVE_INTRIN_ENDPOINT
            }else if(rq->bRequest == USBRQ_SET_INTERFACE){      /* 11 */
                usbResetDataToggling();
                usbResetStall();
#   if USB_CFG_IMPLEMENT_HALT
            }else if(rq->bRequest == USBRQ_CLEAR_FEATURE || rq->bRequest == USBRQ_SET_FEATURE){   /* 1|3 */
                if(rq->wValue.bytes[0] == 0 && rq->wIndex.bytes[0] == 0x81){   /* feature 0 == HALT for endpoint == 1 */
                    usbTxLen1 = rq->bRequest == USBRQ_CLEAR_FEATURE ? USBPID_NAK : USBPID_STALL;
                    usbResetDataToggling();
                }
#   endif
#endif
            }else{
                /* the following requests can be ignored, send default reply */
                /* 1: CLEAR_FEATURE, 3: SET_FEATURE, 7: SET_DESCRIPTOR */
                /* 12: SYNCH_FRAME */
            }
            #undef SET_REPLY_LEN
        }else{  /* not a standard request -- must be vendor or class request */
            replyLen = usbFunctionSetup(data);
        }
#if USB_CFG_IMPLEMENT_FN_READ || USB_CFG_IMPLEMENT_FN_WRITE
        if(replyLen == 0xff){   /* use user-supplied read/write function */
            if((rq->bmRequestType & USBRQ_DIR_MASK) == USBRQ_DIR_DEVICE_TO_HOST){
                replyLen = rq->wLength.bytes[0];    /* IN transfers only */
            }
            flags &= ~USB_FLG_USE_DEFAULT_RW;  /* we have no valid msg, use user supplied read/write functions */
        }else   /* The 'else' prevents that we limit a replyLen of 0xff to the maximum transfer len. */
#endif
        if(!rq->wLength.bytes[1] && replyLen > rq->wLength.bytes[0])  /* limit length to max */
            replyLen = rq->wLength.bytes[0];
        /* make sure that data packets which are sent as ACK to an OUT transfer are always zero sized */
    }else{  /* DATA packet from out request */
#if USB_CFG_IMPLEMENT_FN_WRITE
        if(!(usbMsgFlags & USB_FLG_USE_DEFAULT_RW)){
            uchar rval = usbFunctionWrite(data, len);
            replyLen = 0xff;
            if(rval == 0xff){       /* an error occurred */
                usbMsgLen = 0xff;   /* cancel potentially pending data packet for ACK */
                usbTxLen = USBPID_STALL;
            }else if(rval != 0){    /* This was the final package */
                replyLen = 0;       /* answer with a zero-sized data packet */
            }
            flags = 0;    /* start with a DATA1 package, stay with user supplied write() function */
        }
#endif
    }
    usbMsgFlags = flags;
    usbMsgLen = replyLen;
}

/* ------------------------------------------------------------------------- */

static void usbBuildTxBlock(void)
{
uchar   wantLen, len, txLen, token;

    wantLen = usbMsgLen;
    if(wantLen > 8)
        wantLen = 8;
    usbMsgLen -= wantLen;
    token = USBPID_DATA1;
    if(usbMsgFlags & USB_FLG_TX_PACKET)
        token = USBPID_DATA0;
    usbMsgFlags++;
    len = usbRead(usbTxBuf + 1, wantLen);
    if(len <= 8){           /* valid data packet */
        usbCrc16Append(&usbTxBuf[1], len);
        txLen = len + 4;    /* length including sync byte */
        if(len < 8)         /* a partial package identifies end of message */
            usbMsgLen = 0xff;
    }else{
        txLen = USBPID_STALL;   /* stall the endpoint */
        usbMsgLen = 0xff;
    }
    usbTxBuf[0] = token;
    usbTxLen = txLen;
    DBG2(0x20, usbTxBuf, txLen-1);
}

/* ------------------------------------------------------------------------- */

static inline uchar isNotSE0(void)
{
uchar   rval;
/* We want to do
 *     return (USBIN & USBMASK);
 * here, but the compiler does int-expansion acrobatics.
 * We can avoid this by assigning to a char-sized variable.
 */
    rval = USBIN & USBMASK;
    return rval;
}

static inline void usbHandleResetHook(uchar notResetState)
{
#ifdef USB_RESET_HOOK
static uchar    wasReset;
uchar           isReset = !notResetState;

    if(wasReset != isReset){
        USB_RESET_HOOK(isReset);
        wasReset = isReset;
    }
#endif
}

/* ------------------------------------------------------------------------- */

USB_PUBLIC void usbPoll(void)
{
schar   len;
uchar   i;

    if((len = usbRxLen) > 0){
/* We could check CRC16 here -- but ACK has already been sent anyway. If you
 * need data integrity checks with this driver, check the CRC in your app
 * code and report errors back to the host. Since the ACK was already sent,
 * retries must be handled on application level.
 * unsigned crc = usbCrc16(buffer + 1, usbRxLen - 3);
 */
        usbProcessRx(usbRxBuf + USB_BUFSIZE + 1 - usbInputBufOffset, len - 3);
#if USB_CFG_HAVE_FLOWCONTROL
        if(usbRxLen > 0)    /* only mark as available if not inactivated */
            usbRxLen = 0;
#else
        usbRxLen = 0;       /* mark rx buffer as available */
#endif
    }
    if(usbTxLen & 0x10){ /* transmit system idle */
        if(usbMsgLen != 0xff){  /* transmit data pending? */
            usbBuildTxBlock();
        }
    }
    for(i = 10; i > 0; i--){
        if(isNotSE0())
            break;
    }
    if(i == 0){ /* RESET condition, called multiple times during reset */
        usbNewDeviceAddr = 0;
        usbDeviceAddr = 0;
        usbResetStall();
        DBG1(0xff, 0, 0);
    }
    usbHandleResetHook(i);
}

/* ------------------------------------------------------------------------- */

USB_PUBLIC void usbInit(void)
{
#if USB_INTR_CFG_SET != 0
    USB_INTR_CFG |= USB_INTR_CFG_SET;
#endif
#if USB_INTR_CFG_CLR != 0
    USB_INTR_CFG &= ~(USB_INTR_CFG_CLR);
#endif
    USB_INTR_ENABLE |= (1 << USB_INTR_ENABLE_BIT);
    usbResetDataToggling();
}

/* ------------------------------------------------------------------------- */
