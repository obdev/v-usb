/* Name: hiddata.c
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-11
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

#include <stdio.h>
#include "hiddata.h"

/* ######################################################################## */
#if defined(WIN32) /* ##################################################### */
/* ######################################################################## */

#include <windows.h>
#include <setupapi.h>
#include "hidsdi.h"
#include <ddk/hidpi.h>

#ifdef DEBUG
#define DEBUG_PRINT(arg)    printf arg
#else
#define DEBUG_PRINT(arg)
#endif

/* ------------------------------------------------------------------------ */

static void convertUniToAscii(char *buffer)
{
unsigned short  *uni = (void *)buffer;
char            *ascii = buffer;

    while(*uni != 0){
        if(*uni >= 256){
            *ascii++ = '?';
        }else{
            *ascii++ = *uni++;
        }
    }
    *ascii++ = 0;
}

int usbhidOpenDevice(usbDevice_t **device, int vendor, char *vendorName, int product, char *productName, int usesReportIDs)
{
GUID                                hidGuid;        /* GUID for HID driver */
HDEVINFO                            deviceInfoList;
SP_DEVICE_INTERFACE_DATA            deviceInfo;
SP_DEVICE_INTERFACE_DETAIL_DATA     *deviceDetails = NULL;
DWORD                               size;
int                                 i, openFlag = 0;  /* may be FILE_FLAG_OVERLAPPED */
int                                 errorCode = USBOPEN_ERR_NOTFOUND;
HANDLE                              handle = INVALID_HANDLE_VALUE;
HIDD_ATTRIBUTES                     deviceAttributes;
				
    HidD_GetHidGuid(&hidGuid);
    deviceInfoList = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    deviceInfo.cbSize = sizeof(deviceInfo);
    for(i=0;;i++){
        if(handle != INVALID_HANDLE_VALUE){
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
        }
        if(!SetupDiEnumDeviceInterfaces(deviceInfoList, 0, &hidGuid, i, &deviceInfo))
            break;  /* no more entries */
        /* first do a dummy call just to determine the actual size required */
        SetupDiGetDeviceInterfaceDetail(deviceInfoList, &deviceInfo, NULL, 0, &size, NULL);
        if(deviceDetails != NULL)
            free(deviceDetails);
        deviceDetails = malloc(size);
        deviceDetails->cbSize = sizeof(*deviceDetails);
        /* this call is for real: */
        SetupDiGetDeviceInterfaceDetail(deviceInfoList, &deviceInfo, deviceDetails, size, &size, NULL);
        DEBUG_PRINT(("checking HID path \"%s\"\n", deviceDetails->DevicePath));
#if 0
        /* If we want to access a mouse our keyboard, we can only use feature
         * requests as the device is locked by Windows. It must be opened
         * with ACCESS_TYPE_NONE.
         */
        handle = CreateFile(deviceDetails->DevicePath, ACCESS_TYPE_NONE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, openFlag, NULL);
#endif
        /* attempt opening for R/W -- we don't care about devices which can't be accessed */
        handle = CreateFile(deviceDetails->DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, openFlag, NULL);
        if(handle == INVALID_HANDLE_VALUE){
            DEBUG_PRINT(("opening failed: %d\n", (int)GetLastError()));
            /* errorCode = USBOPEN_ERR_ACCESS; opening will always fail for mouse -- ignore */
            continue;
        }
        deviceAttributes.Size = sizeof(deviceAttributes);
        HidD_GetAttributes(handle, &deviceAttributes);
        DEBUG_PRINT(("device attributes: vid=%d pid=%d\n", deviceAttributes.VendorID, deviceAttributes.ProductID));
        if(deviceAttributes.VendorID != vendor || deviceAttributes.ProductID != product)
            continue;   /* ignore this device */
        errorCode = USBOPEN_ERR_NOTFOUND;
        if(vendorName != NULL && productName != NULL){
            char    buffer[512];
            if(!HidD_GetManufacturerString(handle, buffer, sizeof(buffer))){
                DEBUG_PRINT(("error obtaining vendor name\n"));
                errorCode = USBOPEN_ERR_IO;
                continue;
            }
            convertUniToAscii(buffer);
            DEBUG_PRINT(("vendorName = \"%s\"\n", buffer));
            if(strcmp(vendorName, buffer) != 0)
                continue;
            if(!HidD_GetProductString(handle, buffer, sizeof(buffer))){
                DEBUG_PRINT(("error obtaining product name\n"));
                errorCode = USBOPEN_ERR_IO;
                continue;
            }
            convertUniToAscii(buffer);
            DEBUG_PRINT(("productName = \"%s\"\n", buffer));
            if(strcmp(productName, buffer) != 0)
                continue;
        }
        break;  /* we have found the device we are looking for! */
    }
    SetupDiDestroyDeviceInfoList(deviceInfoList);
    if(deviceDetails != NULL)
        free(deviceDetails);
    if(handle != INVALID_HANDLE_VALUE){
        *device = (usbDevice_t *)handle;
        errorCode = 0;
    }
    return errorCode;
}

/* ------------------------------------------------------------------------ */

void    usbhidCloseDevice(usbDevice_t *device)
{
    CloseHandle((HANDLE)device);
}

/* ------------------------------------------------------------------------ */

int usbhidSetReport(usbDevice_t *device, char *buffer, int len)
{
BOOLEAN rval;

    rval = HidD_SetFeature((HANDLE)device, buffer, len);
    return rval == 0 ? USBOPEN_ERR_IO : 0;
}

/* ------------------------------------------------------------------------ */

int usbhidGetReport(usbDevice_t *device, int reportNumber, char *buffer, int *len)
{
BOOLEAN rval = 0;

    buffer[0] = reportNumber;
    rval = HidD_GetFeature((HANDLE)device, buffer, *len);
    return rval == 0 ? USBOPEN_ERR_IO : 0;
}

/* ------------------------------------------------------------------------ */

/* ######################################################################## */
#else /* defined WIN32 #################################################### */
/* ######################################################################## */

#include <string.h>
#include <libusb.h>

#define usbDevice   libusb_device_handle  /* use libusb's device structure */

/* ------------------------------------------------------------------------- */

#define USBRQ_HID_GET_REPORT    0x01
#define USBRQ_HID_SET_REPORT    0x09

#define USB_HID_REPORT_TYPE_FEATURE 3


static int  usesReportIDs;

/* ------------------------------------------------------------------------- */

int usbhidOpenDevice(usbDevice_t **device, int vendor, char *vendorName, int product, char *productName, int _usesReportIDs)
{

    libusb_device       *dev = NULL;
    libusb_device_handle *handle = NULL;
    struct libusb_device_descriptor desc;
    int                 errorCode = USBOPEN_ERR_NOTFOUND;
    static int          didUsbInit = 0;
    char    string[256];
    int                 r;

    if(!didUsbInit){
        r = libusb_init(NULL);
        if (0 != r) {
            fprintf(stderr, "Warning: cannot initialize libusb: %s\n", libusb_strerror(r));
            return errorCode;
        }
        didUsbInit = 1;
    }

    handle = libusb_open_device_with_vid_pid(NULL, vendor, product);
    if(handle == NULL) {
        errorCode = USBOPEN_ERR_ACCESS;
        fprintf(stderr, "Warning: cannot open USB device\n");
        return errorCode;
    }

    dev = libusb_get_device(handle);
    r = libusb_get_device_descriptor(dev, &desc);
    if(r < 0) {
        errorCode = USBOPEN_ERR_IO;
        fprintf(stderr, "Warning: cannot query descriptor for device: %s\n", libusb_strerror(r));
        return errorCode;
    }

    if(vendorName != NULL){
        r = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char *)string, sizeof(string));
        if(r < 0) {
            errorCode = USBOPEN_ERR_IO;
            fprintf(stderr, "Warning: cannot query manufacturer for device: %s\n", libusb_strerror(r));
            return errorCode;
        }
        if(strcmp(string, vendorName) != 0){
            return errorCode;
        }
        /* fprintf(stderr, "seen device from vendor ->%s<-\n", string); */
    }

    if(productName != NULL){
        r = libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char *)string, sizeof(string));
        if(r < 0) {
            errorCode = USBOPEN_ERR_IO;
            fprintf(stderr, "Warning: cannot query product for device: %s\n", libusb_strerror(r));
            return errorCode;
        }
        if(strcmp(string, productName) != 0){
            return errorCode;
        }
        /* fprintf(stderr, "seen product ->%s<-\n", string); */
    }

    if(handle != NULL){
        errorCode = 0;
        *device = (void *)handle;
        usesReportIDs = _usesReportIDs;
    }
    return errorCode;
}

/* ------------------------------------------------------------------------- */

void    usbhidCloseDevice(usbDevice_t *device)
{
    if(device != NULL)
        libusb_close((void *)device);
}

/* ------------------------------------------------------------------------- */

int usbhidSetReport(usbDevice_t *device, char *buffer, int len)
{
int bytesSent, reportId = buffer[0];

    if(!usesReportIDs){
        buffer++;   /* skip dummy report ID */
        len--;
    }
    bytesSent = libusb_control_transfer((void *)device, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, USBRQ_HID_SET_REPORT, USB_HID_REPORT_TYPE_FEATURE << 8 | (reportId & 0xff), 0, (unsigned char *)buffer, len, 5000);
    if(bytesSent != len){
        if(bytesSent < 0)
            fprintf(stderr, "Error sending message: %s\n", libusb_strerror(bytesSent));
        return USBOPEN_ERR_IO;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int usbhidGetReport(usbDevice_t *device, int reportNumber, char *buffer, int *len)
{
int bytesReceived, maxLen = *len;

    if(!usesReportIDs){
        buffer++;   /* make room for dummy report ID */
        maxLen--;
    }
    bytesReceived = libusb_control_transfer((void *)device, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, USBRQ_HID_GET_REPORT, USB_HID_REPORT_TYPE_FEATURE << 8 | reportNumber, 0, (unsigned char *)buffer, maxLen, 5000);
    if(bytesReceived < 0){
        fprintf(stderr, "Error sending message: %s\n", libusb_strerror(bytesReceived));
        return USBOPEN_ERR_IO;
    }
    *len = bytesReceived;
    if(!usesReportIDs){
        buffer[-1] = reportNumber;  /* add dummy report ID */
        (*len)++;
    }
    return 0;
}

/* ######################################################################## */
#endif /* defined WIN32 ################################################### */
/* ######################################################################## */
