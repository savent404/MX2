#include "flashParam.h"

// single instance define
MX_FlashParam* MX_FlashParam::instance = nullptr;

/**
 * @brief write data to flash
 * @retval if flash is not enough or write error, return false
 */
__MX_WEAK bool MX_FlashParam_Write(const void* data, size_t size)
{
    return false;
}

/**
 * @brief read data from flash
 * @retvl if flash is not enough or read error, return false
 */
__MX_WEAK bool MX_FlashParam_Read(void* data, size_t size)
{
    return false;
}
