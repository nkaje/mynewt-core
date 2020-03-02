/*
* Common header to store the defines
* that are used by C and Python code
*/

#include <errno.h>

enum {
    ERR_OTP_BASE_ERR = INT_MIN,
    ERR_INIT_UART,              /* UART init error */
    ERR_INVALID_RESP,           /* Invalid response */
    ERR_CMD_NOT_FOUND,          /* Command not found */
    ERR_SEG_NOT_FOUND,          /* key segment not found */
    ERR_SLOT_NOT_FOUND,         /* incorrect otp slot */
    ERR_CRC,                    /* CRC check failed */
    ERR_KEY_NOT_EMPTY,          /* Previously programmed key slot */
    ERR_OTP_INV_ALIGNMENT,      /* Invalid alignment */
    ERR_OTP_INV_ADDRESS,        /* Invalid address */
    ERR_OTP_VERF_FAILED,        /* verification failed */
    ERR_FLASH_INV,              /* HAL Flash Invalid device */
    ERR_FLASH_EACCES,           /* HAL Flash Access error */
    ERR_FLASH_EIO,              /* HAL Flash IO Error */
    ERR_OTP_END,                /* Marker for end of base errors */
} OTP_PRGM_ERRS;


