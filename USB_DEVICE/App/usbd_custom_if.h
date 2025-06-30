#ifndef __USBD_CUSTOM_IF_H__
#define __USBD_CUSTOM_IF_H__

#include "usbd_custom.h"

uint8_t USBD_CUSTOM_SendData(uint8_t* buf, uint16_t len);
void USBD_CUSTOM_DataReceived(uint8_t* buf, uint16_t len);

#endif /* __USBD_CUSTOM_IF_H__ */