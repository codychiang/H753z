#ifndef __USBD_CUSTOM_H
#define __USBD_CUSTOM_H

#include "usbd_ioreq.h"
#include "usbd_def.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_ClassTypeDef USBD_CustomClass;
#define USBD_CUSTOM_CLASS &USBD_CustomClass

typedef struct {
    uint8_t (*Init)(void);
    uint8_t (*DeInit)(void);
    uint8_t (*Receive)(uint8_t* buf, uint32_t *len);
    uint8_t (*TransmitCplt)(uint8_t epnum);
} USBD_CUSTOM_InterfaceTypeDef;

extern USBD_CUSTOM_InterfaceTypeDef USBD_Custom_fops_FS;

uint8_t USBD_CUSTOM_SendData(uint8_t* buf, uint16_t len);
uint8_t USBD_CUSTOM_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CUSTOM_InterfaceTypeDef *fops);

#endif /* __USBD_CUSTOM_H */