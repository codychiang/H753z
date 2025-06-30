#include "usbd_custom.h"
#include "usbd_ctlreq.h"

static uint8_t* USBD_Custom_GetCfgDesc(uint16_t *length);
static uint8_t USBD_Custom_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_Custom_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_Custom_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_Custom_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_Custom_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_Custom_EP0_RxReady(USBD_HandleTypeDef *pdev);

USBD_ClassTypeDef USBD_CustomClass = {
    USBD_Custom_Init,
    USBD_Custom_DeInit,
    USBD_Custom_Setup,
    NULL,
    USBD_Custom_EP0_RxReady,
    USBD_Custom_DataIn,
    USBD_Custom_DataOut,
    
    NULL,
    NULL,
    NULL,
    NULL,
    USBD_Custom_GetCfgDesc,
    NULL,
    NULL,
};

__ALIGN_BEGIN uint8_t USBD_Custom_CfgDesc[] __ALIGN_END =
{
  0x09, USB_DESC_TYPE_CONFIGURATION, 0x20, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32,
  0x09, USB_DESC_TYPE_INTERFACE, 0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x00,
  0x07, USB_DESC_TYPE_ENDPOINT, 0x01, 0x02, 0x00, 0x02, 0x00,
  0x07, USB_DESC_TYPE_ENDPOINT, 0x81, 0x02, 0x00, 0x02, 0x00
};

static uint8_t* USBD_Custom_GetCfgDesc(uint16_t *length) {
    *length = sizeof(USBD_Custom_CfgDesc);
    return USBD_Custom_CfgDesc;
}

static uint8_t USBD_Custom_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
    USBD_LL_OpenEP(pdev, 0x81, USBD_EP_TYPE_BULK, 512);
    USBD_LL_OpenEP(pdev, 0x01, USBD_EP_TYPE_BULK, 512);
    pdev->pClassData = NULL;
    USBD_LL_PrepareReceive(pdev, 0x01, NULL, 512);
    return USBD_OK;
}

static uint8_t USBD_Custom_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
    USBD_LL_CloseEP(pdev, 0x81);
    USBD_LL_CloseEP(pdev, 0x01);
    return USBD_OK;
}

static uint8_t USBD_Custom_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
    USBD_CtlError(pdev, req);
    return USBD_FAIL;
}

static uint8_t USBD_Custom_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
    return USBD_OK;
}

static uint8_t USBD_Custom_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
    USBD_LL_PrepareReceive(pdev, 0x01, NULL, 512);
    return USBD_OK;
}

static uint8_t USBD_Custom_EP0_RxReady(USBD_HandleTypeDef *pdev) {
    return USBD_OK;
}
