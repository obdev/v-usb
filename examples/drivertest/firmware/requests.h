/* Name: requests.h
 * Project: custom-class, a basic USB example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-09
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/* This header is shared between the firmware and the host software. It
 * defines the USB request numbers (and optionally data types) used to
 * communicate between the host and the device.
 */

#ifndef __REQUESTS_H_INCLUDED__
#define __REQUESTS_H_INCLUDED__

#define CUSTOM_RQ_SET_DATA      1
/* Send data to device. Control-OUT with 64 bytes data.
 */

#define CUSTOM_RQ_GET_DATA      2
/* Get data from device. Control-IN with 64 bytes data.
 */
#define CUSTOM_RQ_SET_OSCCAL    3
#define CUSTOM_RQ_GET_OSCCAL    4

#endif /* __REQUESTS_H_INCLUDED__ */
