#include "pico/bootrom.h"
#include "SensifyBoard.h"

#define REBOOT_SENSIFY_TO_BOOTSEL() reset_usb_boot(0 | (1<<SENSIFY_STATUS_LED_2_ORANGE),2)