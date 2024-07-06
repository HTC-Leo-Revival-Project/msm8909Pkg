
#define USB_ULPI_VIEWPORT    (MSM_USB_BASE + 0x0170)

/* ULPI bit map */
#define ULPI_WAKEUP           (1 << 31)
#define ULPI_RUN              (1 << 30)
#define ULPI_WRITE            (1 << 29)
#define ULPI_READ             (0 << 29)
#define ULPI_STATE_NORMAL     (1 << 27)
#define ULPI_ADDR(n)          (((n) & 255) << 16)
#define ULPI_DATA(n)          ((n) & 255)
#define ULPI_DATA_READ(n)     (((n) >> 8) & 255)