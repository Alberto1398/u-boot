#ifndef	__ADFU_USB_IO_H
#define	__ADFU_USB_IO_H

//#include "asm/arch/usb/usb2/usb_ch9.h"
//#include "asm/arch/usb/usb2/os_compatible.h"

#define	ADFU_USB_ENDPOINTS		3/*ep0,bulk out1,bulk in2*/
#define	CTRL_PACKET_SIZE	    64
#define BULK_HS_PACKET_SIZE		512	
#define BULK_FS_PACKET_SIZE		64	
#define INT_FS_PACKET_SIZE		64

enum ep0_state{
    EP0_IDLE,
	  EP0_TX,
	  EP0_RX
};


struct usb_ep {
	//const char							*name;
	//const struct usb_endpoint_descriptor	*desc;
	unsigned short			      		maxpacket;
	unsigned char   	           		address;
	//u8					          		attributes;
	

};

enum usb_device_speed{
    USB_SPEED_FULL,
	  USB_SPEED_HIGH,
};

struct usb_dev {
	enum ep0_state		    		ep0state;	
	struct usb_ep					ep[ADFU_USB_ENDPOINTS];
	enum usb_device_speed			speed;
	
};

#define UPACKED __attribute__((__packed__))
typedef struct {
	unsigned char		bmRequestType;
	unsigned char		bRequest;
	unsigned short	wValue;
	unsigned short	wIndex;
	unsigned short	wLength;
}UPACKED usb_device_request_t;

//typedef struct {
//	usb_device_request_t setup;
//	unsigned short wRemaindLength;
//	unsigned char *pDataSourceAddress;
//}UPACKED usb_device_setup_t;

typedef struct {
	usb_device_request_t setup;
	unsigned short wRemaindLength;
	unsigned char *pDataSourceAddress;
} usb_device_setup_t;


extern struct usb_dev  *udev;  
extern void USB0_epout1_receivedata(unsigned int dLength, unsigned char * pBuffer);
extern void USB0_epin2_sendata(unsigned int dLength, unsigned char * pBuffer);


#endif/*__ADFU_USB_IO_H*/
