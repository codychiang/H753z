#include "usbd_custom_if.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_custom.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint8_t UserRxBuffer[512];

static uint8_t Custom_Init(void);
static uint8_t Custom_DeInit(void);
static uint8_t Custom_Receive(uint8_t* buf, uint32_t *len);
static uint8_t Custom_TxCplt(uint8_t epnum);

// Struct with callback function pointers
USBD_CUSTOM_InterfaceTypeDef USBD_Custom_fops_FS =
{
    .Init = Custom_Init,
    .DeInit = Custom_DeInit,
    .Receive = Custom_Receive,
    .TransmitCplt = Custom_TxCplt
};

static uint8_t Custom_Init(void)
{
    // Initialization code, if any
    return USBD_OK;
}

static uint8_t Custom_DeInit(void)
{
    // Deinitialization code, if any
    return USBD_OK;
}

static uint8_t Custom_Receive(uint8_t* buf, uint32_t *len)
{
    // Example: echo back received data
    memcpy(UserRxBuffer, buf, *len);
    USBD_CUSTOM_SendData(UserRxBuffer, *len);
    return USBD_OK;
}

static uint8_t Custom_TxCplt(uint8_t epnum)
{
    // Transmission complete callback
    return USBD_OK;
}



