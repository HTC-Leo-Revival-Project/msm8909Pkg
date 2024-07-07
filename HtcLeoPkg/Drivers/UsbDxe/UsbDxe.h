
#define USB_ULPI_VIEWPORT    (MSM_USB_BASE + 0x0170)
#define USB_OTGSC            (MSM_USB_BASE + 0x01A4)

#define B_SESSION_VALID   (1 << 11)

/* ULPI bit map */
#define ULPI_WAKEUP           (1 << 31)
#define ULPI_RUN              (1 << 30)
#define ULPI_WRITE            (1 << 29)
#define ULPI_READ             (0 << 29)
#define ULPI_STATE_NORMAL     (1 << 27)
#define ULPI_ADDR(n)          (((n) & 255) << 16)
#define ULPI_DATA(n)          ((n) & 255)
#define ULPI_DATA_READ(n)     (((n) >> 8) & 255)

/*
// Endpoint Indexes
#define ISP1761_EP0SETUP                    0x20
#define ISP1761_EP0RX                       0x00
#define ISP1761_EP0TX                       0x01
#define ISP1761_EP1RX                       0x02
#define ISP1761_EP1TX                       0x03
*/
#define CHIPIDEA_EP0RX                       0x00
#define CHIPIDEA_EP0TX                       0x01

#define ULPI_DATA_READ(n)     (((n) >> 8) & 255)