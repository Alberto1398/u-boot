/*
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (lennart@augustsson.net) at
 * Carlstedt Research & Technology.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Modified by Synopsys, Inc, 12/12/2007 */
/* modified by yujing actions*/

#include "asm/arch/usb/usb2/data_type_define.h"



#ifndef _USB_H_
#define _USB_H_


/*
 * The USB records contain some unaligned little-endian word
 * components.  The U[SG]ETW macros take care of both the alignment
 * and endian problem and should always be used to access non-byte
 * values.
 */

//typedef struct {
//	uByte		bmRequestType;
//	uByte		bRequest;
//	uWord		wValue;
//	uWord		wIndex;
//	uWord		wLength;
//} usb_device_request_t;
//
//typedef struct {
//	uByte		bLength;
//	uByte		bDescriptorType;
//	uByte		bDescriptorSubtype;
//}  usb_descriptor_t;
//
//typedef struct {
//	uByte		bLength;
//	uByte		bDescriptorType;
//}  usb_descriptor_header_t;
//
//typedef struct {
//	uByte		bLength;
//	uByte		bDescriptorType;
//	uWord		bcdUSB;
//#define UD_USB_2_0		0x0200
//#define UD_USB_3_0		0x0300
//#define UD_USB_1_1		0x0110
//#define UD_IS_USB2(d) (UGETW((d)->bcdUSB) >= UD_USB_2_0)
//	uByte		bDeviceClass;
//	uByte		bDeviceSubClass;
//	uByte		bDeviceProtocol;
//	uByte		bMaxPacketSize;
//	/* The fields below are not part of the initial descriptor. */
//	uWord		idVendor;
//	uWord		idProduct;
//	uWord		bcdDevice;
//	uByte		iManufacturer;
//	uByte		iProduct;
//	uByte		iSerialNumber;
//	uByte		bNumConfigurations;
//}  usb_device_descriptor_t;
//#define USB_DEVICE_DESCRIPTOR_SIZE 18
//
//typedef struct {
//	uByte		bLength;
//	uByte		bDescriptorType;
//	uWord		wTotalLength;
//	uByte		bNumInterface;
//	uByte		bConfigurationValue;
//	uByte		iConfiguration;
//	uByte		bmAttributes;
//	uByte		bMaxPower; /* max current in 2 mA units */
//
//}  usb_config_descriptor_t;
//#define USB_CONFIG_DESCRIPTOR_SIZE 9
//
//typedef struct {
//	uByte		bLength;
//	uByte		bDescriptorType;
//	uByte		bInterfaceNumber;
//	uByte		bAlternateSetting;
//	uByte		bNumEndpoints;
//	uByte		bInterfaceClass;
//	uByte		bInterfaceSubClass;
//	uByte		bInterfaceProtocol;
//	uByte		iInterface;
//}  usb_interface_descriptor_t;
//#define USB_INTERFACE_DESCRIPTOR_SIZE 9
//
//typedef struct {
//	uByte		bLength;
//	uByte		bDescriptorType;
//	uByte		bEndpointAddress;
//	uByte		bmAttributes;
//	uWord		wMaxPacketSize;
//	uByte		bInterval;
//}  usb_endpoint_descriptor_t;
//#define USB_ENDPOINT_DESCRIPTOR_SIZE 7
//
//
//typedef struct {
//	uByte		bLength;
//	uByte		bDescriptorType;
//	uWord		bString[127];
//}  usb_string_descriptor_t;
//#define USB_MAX_STRING_LEN 128
//#define USB_LANGUAGE_TABLE 0	/* # of the string language id table */
//
//
//
//typedef struct {
//	uByte		bLength;
//	uByte		bDescriptorType;
//	uWord		bcdUSB;
//	uByte		bDeviceClass;
//	uByte		bDeviceSubClass;
//	uByte		bDeviceProtocol;
//	uByte		bMaxPacketSize0;
//	uByte		bNumConfigurations;
//	uByte		bReserved;
//}  usb_device_qualifier_t;
//#define USB_DEVICE_QUALIFIER_SIZE 10
//
//
//typedef struct {
//	usb_config_descriptor_t config;
//	usb_interface_descriptor_t interface;
//	usb_endpoint_descriptor_t endpointout1;
//	usb_endpoint_descriptor_t endpointin2;
//
//} usb_adfu_configall_t;
//#define USB_ADFU_CONFIGALL_SIZE 32


/* Device class codes */
#define UDCLASS_IN_INTERFACE	0x00
#define UDCLASS_COMM		0x02
#define UDCLASS_HUB		0x09
#define  UDSUBCLASS_HUB		0x00
#define  UDPROTO_FSHUB		0x00
#define  UDPROTO_HSHUBSTT	0x01
#define  UDPROTO_HSHUBMTT	0x02
#define UDCLASS_DIAGNOSTIC	0xdc
#define UDCLASS_WIRELESS	0xe0
#define  UDSUBCLASS_RF		0x01
#define   UDPROTO_BLUETOOTH	0x01
#define UDCLASS_VENDOR		0xff

/*******************************************************************************/
#define  USB_DT_DEVICE		        0x01
#define  USB_DT_CONFIG		        0x02
#define  USB_DT_STRING		        0x03
#define  USB_DT_INTERFACE	        0x04
#define  USB_DT_ENDPOINT		    0x05
#define  USB_DT_DEVICE_QUALIFIER	0x06
#define  USB_DT_OTHER_SPEED_CONFIGURATION 0x07
#define  USB_DT_INTERFACE_POWER	    0x08
#define  USB_DT_OTG		            0x09
#define  USB_DT_BOS         		0x0f
#define USB_DT_DEVICE_CAPABILITY    0x10
#define  USB_DT_SS_USB_COMPANION	0x30

#define MAXPACKETSIZE_FS 64
#define MAXPACKETSIZE_HS 512
#define MAXPACKETSIZE_SS 1024
#define EP0_MAXPACKETSIZE_FS 8
#define EP0_MAXPACKETSIZE_HS 64
#define EP0_MAXPACKETSIZE_SS 9      //means 512 bytes


#define INTERFACE_CLASS_CODE_MSC     0X08
#define INTERFACE_SUBCLASS_CODE_SCSI 0x06
#define INTERFACE_PROTOCOL_BULK      0x50
#define INTERFACE_CLASS_CODE_VendorSpecific    0xff
#define INTERFACE_SUBCLASS_CODE_VendorSpecific 0xff
#define INTERFACE_PROTOCOL_VendorSpecific      0xff


#define USB_DIR_OUT 0x00
#define USB_DIR_IN  0x80





#endif /* _USB_H_ */
