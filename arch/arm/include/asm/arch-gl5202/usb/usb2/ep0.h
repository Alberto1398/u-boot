#ifndef	__ADFU_EP0_H
#define	__ADFU_EP0_H

extern void UsbOtgEp0Write(UCHAR bDataLength, UCHAR * pDataSource);
extern unsigned char UsbOtgEp0Read(void);
extern unsigned char UsbOtgEp0Read(void);
extern void UsbOtgEp0Stall(UCHAR bStall);
extern void UsbOtgEp0Stall(UCHAR bStall);
extern void UsbOtgReset(void);
extern void UsbOtgSpeedChange(void);
extern void UsbOtgGetStatus(void);
extern void UsbOtgSetAddress(void);
extern void UsbOtgGetDescriptor(void);
extern void UsbOtgSetConfiguration(void);
extern void UsbOtgSetInterface(void);
extern void UsbOtgSetUpDealWith(void);
void UsbOtgEp0AckStatus(void);

#endif/*__ADFU_EP0_H*/