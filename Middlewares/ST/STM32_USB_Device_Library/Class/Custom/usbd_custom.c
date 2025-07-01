#include "usbd_custom.h"
#include "usbd_ctlreq.h"
#include "usbd_custom_if.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

#define CUSTOM_IN_EP           0x81  // EP1 IN  (IN = device → host)
#define CUSTOM_OUT_EP          0x01  // EP1 OUT (OUT = host → device)

#define EP_DATA_SIZE  64

#define CUSTOM_DATA_IN_PACKET_SIZE    64
#define CUSTOM_DATA_OUT_PACKET_SIZE   64

typedef struct {
  uint8_t RxBuffer[CUSTOM_DATA_IN_PACKET_SIZE];    // 收到資料的暫存區
  uint8_t TxBuffer[CUSTOM_DATA_OUT_PACKET_SIZE];    // 要傳送的資料（可選）
  uint32_t RxLength;        // 實際收到的資料長度
  uint32_t TxLength;        // 實際要傳送的資料長度
} USBD_CUSTOM_HandleTypeDef;

__attribute__((aligned(4))) USBD_CUSTOM_HandleTypeDef ghcustom;

static USBD_CUSTOM_InterfaceTypeDef *pIf = NULL;
static uint8_t tx_busy = 0;
static uint8_t zlp_needed = 0;

static uint8_t USBD_Custom_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_Custom_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_Custom_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
uint8_t USBD_Custom_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_Custom_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_Custom_EP0_RxReady(USBD_HandleTypeDef *pdev);
uint8_t* USBD_Custom_GetCfgDesc(uint16_t *length);

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
    NULL
};

__ALIGN_BEGIN uint8_t USBD_Custom_CfgDesc[] __ALIGN_END =
{
  // ===== Configuration Descriptor =====
  0x09,                         // bLength: Configuration Descriptor size (9 bytes)
  USB_DESC_TYPE_CONFIGURATION,  // bDescriptorType: Configuration
  0x20, 0x00,                   // wTotalLength: Total length of data = 32 bytes (9+9+7+7)
  0x01,                         // bNumInterfaces: 1 interface
  0x01,                         // bConfigurationValue: Configuration index = 1
  0x00,                         // iConfiguration: No string descriptor
  0x80,                         // bmAttributes: Bus powered, no remote wakeup (0x80)
  0x32,                         // bMaxPower: 100 mA (0x32 = 50 * 2mA)

  // ===== Interface Descriptor =====
  0x09,                         // bLength: Interface Descriptor size (9 bytes)
  USB_DESC_TYPE_INTERFACE,     // bDescriptorType: Interface
  0x00,                         // bInterfaceNumber: Interface 0
  0x00,                         // bAlternateSetting: Alternate setting = 0
  0x02,                         // bNumEndpoints: 2 endpoints (IN and OUT)
  0xFF,                         // bInterfaceClass: Vendor Specific (0xFF)
  0x00,                         // bInterfaceSubClass
  0x00,                         // bInterfaceProtocol
  0x00,                         // iInterface: No string

  // ===== Endpoint Descriptor (OUT, EP 1) =====
  0x07,                         // bLength: Endpoint Descriptor size (7 bytes)
  USB_DESC_TYPE_ENDPOINT,       // bDescriptorType: Endpoint
  CUSTOM_OUT_EP,                // bEndpointAddress: OUT endpoint 1 (0x01)
  0x02,                         // bmAttributes: Bulk transfer
  0x40, 0x00,                   // wMaxPacketSize: **64 bytes**
  0x00,                         // bInterval: N/A for Bulk

  // ===== Endpoint Descriptor (IN, EP 1) =====
  0x07,                         // bLength: Endpoint Descriptor size (7 bytes)
  USB_DESC_TYPE_ENDPOINT,       // bDescriptorType: Endpoint
  CUSTOM_IN_EP,                 // bEndpointAddress: IN endpoint 1 (0x81 = EP1 | IN)
  0x02,                         // bmAttributes: Bulk transfer
  0x40, 0x00,                   // wMaxPacketSize: **64 bytes**
  0x00                          // bInterval: N/A for Bulk
};

uint8_t* USBD_Custom_GetCfgDesc(uint16_t *length) {
    *length = sizeof(USBD_Custom_CfgDesc);
    return USBD_Custom_CfgDesc;
}

static uint8_t USBD_Custom_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {

    USBD_CUSTOM_HandleTypeDef *hcustom = &ghcustom;

//    hcustom = USBD_malloc(sizeof(USBD_CUSTOM_HandleTypeDef));
//    if (hcustom == NULL)
//      return USBD_FAIL;
  
    memset(hcustom, 0, sizeof(USBD_CUSTOM_HandleTypeDef));
    pdev->pClassData = hcustom;
  
    USBD_LL_OpenEP(pdev, CUSTOM_IN_EP, USBD_EP_TYPE_BULK, EP_DATA_SIZE);
    USBD_LL_OpenEP(pdev, CUSTOM_OUT_EP, USBD_EP_TYPE_BULK, CUSTOM_DATA_IN_PACKET_SIZE);

    USBD_LL_PrepareReceive(pdev, CUSTOM_OUT_EP, hcustom->RxBuffer, CUSTOM_DATA_IN_PACKET_SIZE);
    return USBD_OK;
}

static uint8_t USBD_Custom_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {

    USBD_LL_CloseEP(pdev, 0x81);
    USBD_LL_CloseEP(pdev, 0x01);

    if (pdev->pClassData != NULL) {
      USBD_free(pdev->pClassData);
      pdev->pClassData = NULL;
    }

    return USBD_OK;
}

static uint8_t USBD_Custom_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req) {
	switch (req->bmRequest & USB_REQ_TYPE_MASK) {
	    case USB_REQ_TYPE_CLASS:
	        // 處理 class-specific request（如果你有）
	        break;

	    case USB_REQ_TYPE_STANDARD:
	        switch (req->bRequest) {
	        case USB_REQ_GET_INTERFACE:
	            USBD_CtlSendData(pdev, (uint8_t *)&(uint8_t){0}, 1);
	            break;

	        case USB_REQ_SET_INTERFACE:
	            // 不用回傳資料，但不能 STALL
	            break;

	        default:
	            USBD_CtlError(pdev, req);
	            return USBD_FAIL;
	        }
	        break;

	    default:
	        USBD_CtlError(pdev, req);
	        return USBD_FAIL;
	    }

	    return USBD_OK;
}

static uint8_t USBD_Custom_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
    USBD_CUSTOM_HandleTypeDef *hcustom = (USBD_CUSTOM_HandleTypeDef *)pdev->pClassData;

    if (hcustom == NULL)
        return USBD_FAIL;

    // ⬇️ 把實際接收到的資料長度取出
    hcustom->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

    // ✅ 呼叫你的上層 callback（你可以在 usb_custom_if.c 中實作）
    USBD_CUSTOM_DataReceived(pdev);

    // ✅ 準備下一筆資料接收（必須）
    USBD_LL_PrepareReceive(pdev, CUSTOM_OUT_EP, hcustom->RxBuffer, CUSTOM_DATA_IN_PACKET_SIZE);

    return USBD_OK;
}

static uint8_t USBD_Custom_EP0_RxReady(USBD_HandleTypeDef *pdev) {
    return USBD_OK;
}

uint8_t USBD_CUSTOM_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CUSTOM_InterfaceTypeDef *fops)
{
    if (fops == NULL) return USBD_FAIL;
    pIf = fops;
    return USBD_OK;
}

uint8_t USBD_CUSTOM_SendData(uint8_t* buf, uint16_t len)
{
    if (tx_busy) return USBD_BUSY;
    
    tx_busy = 1;
    zlp_needed = ((len % 64) == 0);
    return USBD_LL_Transmit(&hUsbDeviceFS, 0x81, buf, len);
}

uint8_t USBD_Custom_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    if (zlp_needed)
    {
        zlp_needed = 0;
        USBD_LL_Transmit(pdev, 0x81, NULL, 0);
    }
    else
    {
        tx_busy = 0;
        if (pIf && pIf->TransmitCplt)
            pIf->TransmitCplt(epnum);
    }
    return USBD_OK;
}

void USBD_CUSTOM_DataReceived(uint8_t* buf, uint16_t len)
{
    if (pIf && pIf->Receive)
    {
        uint32_t l = len;
        pIf->Receive(buf, &l);
    }
}

