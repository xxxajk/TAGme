/*
 * File:   sketch_settings.h
 * Author: xxxajk
 *
 * Created on January 26, 2014, 5:13 PM
 */

#ifndef usb_settings_h
#define usb_settings_h
#include <stdint.h>
#include <stddef.h>

#define MAX_USB_IO_SIZE 64

#define EP_TYPE_CTRL 0x00
#define EP_TYPE_BULK 0x02
#define EP_TYPE_INTR 0x03

#define ENDPOINT_UNUSED                 0x00
#define ENDPOINT_TRANSIMIT_ONLY         0x15
#define ENDPOINT_RECEIVE_ONLY           0x19
#define ENDPOINT_TRANSMIT_AND_RECEIVE   0x1D


#define _LSB_(n) ((n) & 0xff)
#define _MSB_(n) (((n) >> 8) & 0xff)


#define TXEP (JOYSTICK_ENDPOINT+1)
#define RXEP (JOYSTICK_ENDPOINT+2)

//#define VENDOR_ID 0x16C0
#define VENDOR_ID 0x16C0
// 0x03E8 - 03F1
#define PRODUCT_ID 0x03E8

#define MANUFACTURER_NAME {'T','e','e','n','s','y','d','u','i','n','o'}
#define PRODUCT_NAME {'T','e','e','n','s','y','d','u','i','n','o',' ','J','T','A','G','I','F'}

#define NUM_USB_BUFFERS 18
#define NUM_INTERFACE 1

#define USB_CUSTOM_INTERFACE 0

// Define the interface, and endpoints
#define USB_CUSTOM_INTERFACE_EPS 2
#define USB_CUSTOM_INTERFACE_bInterfaceClass 0xFF
#define USB_CUSTOM_INTERFACE_bInterfaceSubClass 0x00
#define USB_CUSTOM_INTERFACE_bInterfaceProtocol 0x00
#define USB_CUSTOM_INTERFACE_interface_descriptor_bLength 9
#define USB_CUSTOM_INTERFACE_additional_blengths 0
#define RX_INTERVAL 0
#define TX_INTERVAL 0

// Make the interface_descriptor
#define USB_CUSTOM_INTERFACE_interface_descriptor \
        USB_CUSTOM_INTERFACE_interface_descriptor_bLength, \
        4, \
        USB_CUSTOM_INTERFACE, 0, USB_CUSTOM_INTERFACE_EPS, \
        USB_CUSTOM_INTERFACE_bInterfaceClass, \
        USB_CUSTOM_INTERFACE_bInterfaceSubClass, \
        USB_CUSTOM_INTERFACE_bInterfaceProtocol, \
        USB_CUSTOM_INTERFACE

// Make the endpoints
#define USB_CUSTOM_INTERFACE_endpoint_descriptors \
        7, 5, (TXEP|0x80), EP_TYPE_BULK, _LSB_(MAX_USB_IO_SIZE), _MSB_(MAX_USB_IO_SIZE),  TX_INTERVAL, \
        7, 5, RXEP, EP_TYPE_BULK, _LSB_(MAX_USB_IO_SIZE), _MSB_(MAX_USB_IO_SIZE),  RX_INTERVAL



// REQUIRED Descriptor stuff

// The size of each endpoint
#define USB_CUSTOM_INTERFACE_EPSZ MAX_USB_IO_SIZE, MAX_USB_IO_SIZE

// The direction of each endpoint
#define USB_CUSTOM_INTERFACE_EPCFG ENDPOINT_TRANSIMIT_ONLY, ENDPOINT_RECEIVE_ONLY

// The length of the whole description, assumes endpoints are 7 bytes long
#define USB_CUSTOM_INTERFACE_LEN ( \
        USB_CUSTOM_INTERFACE_interface_descriptor_bLength + \
        USB_CUSTOM_INTERFACE_additional_blengths + \
        (7*USB_CUSTOM_INTERFACE_EPS))

// The description
#define USB_CUSTOM_DESCRIPTORS \
        USB_CUSTOM_INTERFACE_interface_descriptor, \
        USB_CUSTOM_INTERFACE_endpoint_descriptors

//#undef _LSB_
//#undef _MSB_

#include "usb_help.h"

#endif

