#include "usbd_custom_if.h"
#include "usbd_def.h"
#include "usbd_core.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t tx_busy = 0;

uint8_t USBD_CUSTOM_SendData(uint8_t* buf, uint16_t len)
{
    if (tx_busy) return USBD_BUSY;
    tx_busy = 1;
    USBD_LL_Transmit(&hUsbDeviceFS, 0x81, buf, len);
    return USBD_OK;
}

// This function should be called from DataOut callback
void USBD_CUSTOM_DataReceived(uint8_t* buf, uint16_t len)
{
    // Process received data here
    // Example: echo back
    USBD_CUSTOM_SendData(buf, len);
}

// You can reset tx_busy in DataIn callback
uint8_t USBD_Custom_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    tx_busy = 0;
    return USBD_OK;
}