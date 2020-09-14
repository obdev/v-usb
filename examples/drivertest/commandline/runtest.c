/* Name: runtest.c
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-10
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*
General Description:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libusb.h>        /* this is libusb */
#include "opendevice.h" /* common code moved to separate module */

#include "../firmware/requests.h"   /* custom request numbers */
#include "../firmware/usbconfig.h"  /* device's VID/PID and names */

#define uchar   unsigned char

static void hexdump(char *_buffer, int len, FILE *fp)
{
int i;
uchar *buffer = (uchar *)_buffer;

    for(i = 0; i < len; i++){
        if(i != 0){
            if(i % 16 == 0){
                fprintf(fp, "\n");
            }else{
                fprintf(fp, " ");
            }
        }
        fprintf(fp, "%02x", buffer[i]);
    }
    if(i != 0)
        fprintf(fp, "\n");
}

static void fillBuffer(char *buffer, int len)
{
static int  type = 0;

    if(type == 0){          /* all 0 */
        bzero(buffer, len);
    }else if(type == 1){    /* all 0xff */
        memset(buffer, 0xff, len);
    }else{                  /* random */
        int i;
        for(i = 0; i < len; i++){
            buffer[i] = random() & 0xff;
        }
    }
    if(++type >= 1000)
        type = 0;
}

static int compareBuffers(char *txBuffer, char *rxBuffer, int len)
{
int i, rval = 0;

    for(i = 0; i < len; i++){
        if(rxBuffer[i] != txBuffer[i]){
            fprintf(stderr, "compare error at index %d: byte is 0x%x instead of 0x%x\n", i, rxBuffer[i], txBuffer[i]);
            rval = 1;
        }
    }
    if(rval){
        fprintf(stderr, "txBuffer was:\n");
        hexdump(txBuffer, len, stderr);
        fprintf(stderr, "rxBuffer is:\n");
        hexdump(rxBuffer, len, stderr);
    }
    return rval;
}

int main(int argc, char **argv)
{
libusb_device_handle  *handle = NULL;
const uchar     rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
char            vendor[] = {USB_CFG_VENDOR_NAME, 0}, product[] = {USB_CFG_DEVICE_NAME, 0};
char            txBuffer[64], rxBuffer[64];
int             cnt, vid, pid, i, j, r;

    r = libusb_init(NULL);
    if (0 != r) {
        fprintf(stderr, "Warning: cannot initialize libusb: %s\n", libusb_strerror(r));
        exit(1);
    }
    /* compute VID/PID from usbconfig.h so that there is a central source of information */
    vid = rawVid[1] * 256 + rawVid[0];
    pid = rawPid[1] * 256 + rawPid[0];
    /* The following function is in opendevice.c: */
    if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
        exit(1);
    }
    if(argc > 1 && strcasecmp(argv[1], "osccal") == 0){
        if(argc > 2){   /* set osccal */
            int osccal = atoi(argv[2]);
            printf("setting osccal to %d\n", osccal);
            cnt = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, CUSTOM_RQ_SET_OSCCAL, osccal, 0, (unsigned char *)txBuffer, 0, 5000);
            if(cnt < 0){
                fprintf(stderr, "\nUSB error setting osccal: %s\n", libusb_strerror(cnt));
            }
        }else{
            cnt = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, CUSTOM_RQ_GET_OSCCAL, 0, 0, (unsigned char *)rxBuffer, 1, 5000);
            if(cnt < 0){
                fprintf(stderr, "\nUSB error getting osccal: %s\n", libusb_strerror(cnt));
            }else{
                printf("osccal = %d\n", (unsigned char)rxBuffer[0]);
            }
        }
    }else{
#ifdef __linux__
        srandom(time(NULL));
#else
        srandomdev();
#endif
        for(i = 0; i <= 100000; i++){
            fillBuffer(txBuffer, sizeof(txBuffer));
            cnt = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, CUSTOM_RQ_SET_DATA, 0, 0, (unsigned char *)txBuffer, sizeof(txBuffer), 5000);
            if(cnt < 0){
                fprintf(stderr, "\nUSB tx error in iteration %d: %s\n", i, libusb_strerror(cnt));
                break;
            }else if(cnt != sizeof(txBuffer)){
                fprintf(stderr, "\nerror in iteration %d: %d bytes sent instead of %d\n", i, cnt, (int)sizeof(txBuffer));
                break;
            }
            for(j = 0; j < sizeof(rxBuffer); j++){
                rxBuffer[j] = ~txBuffer[j];
            }
            cnt = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, CUSTOM_RQ_GET_DATA, 0, 0, (unsigned char *)rxBuffer, sizeof(rxBuffer), 5000);
            if(cnt < 0){
                fprintf(stderr, "\nUSB rx error in iteration %d: %s\n", i, libusb_strerror(cnt));
                break;
            }else if(cnt != sizeof(txBuffer)){
                fprintf(stderr, "\nerror in iteration %d: %d bytes received instead of %d\n", i, cnt, (int)sizeof(rxBuffer));
                break;
            }
            if(compareBuffers(txBuffer, rxBuffer, sizeof(rxBuffer))){
                fprintf(stderr, "\ncompare error in iteration %d.\n", i);
                break;
            }
            if(i != 0 && i % 100 == 0){
                printf(".");
                fflush(stdout);
                if(i % 5000 == 0)
                    printf(" %6d\n", i);
            }
        }
        fprintf(stderr, "\nTest completed.\n");
    }
    libusb_close(handle);
    return 0;
}
