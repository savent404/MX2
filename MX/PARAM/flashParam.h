#ifndef BSPH1_FLASHPARAM_H
#define BSPH1_FLASHPARAM_H

#include "FreeRTOS.h"
#include "MX_def.h"
#include "PARAM_def.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief write data to flash
 * @retval if flash is not enough or write error, return false
 */
MX_PORT_API bool MX_FlashParam_Write(const void* data, size_t size);

/**
 * @brief read data from flash
 * @retvl if flash is not enough or read error, return false
 */
MX_PORT_API bool MX_FlashParam_Read(void* data, size_t size);

#ifdef __cplusplus

#    define malloc pvPortMalloc
#    define free vPortFree

class MX_FlashParam {
private:
    static MX_FlashParam* instance;
    static const int      flashFlag = -2019;
    struct storageData {
        int initedFlag;
        int bankNum;
        int bankIndex;
        int colorNum;
        int colorIndex[ 0 ];
    };

public:
    MX_FlashParam()
    {
    }

    bool save(const colorMatrix_t* ptr) const
    {
        storageData* pStorage = nullptr;
        size_t       neededSize;
        neededSize = sizeof(*pStorage) + ptr->bankNum * sizeof(int);
        // malloc to package the param
        pStorage = static_cast<struct storageData*>(malloc(neededSize));
        // fill package
        pStorage->initedFlag = flashFlag;
        pStorage->bankNum    = ptr->bankNum;
        pStorage->colorNum   = ptr->num;
        pStorage->bankIndex  = ptr->bankIndex;
        memcpy(pStorage->colorIndex, ptr->colorIndex, sizeof(int) * ptr->bankNum);

        auto res = MX_FlashParam_Write(pStorage, neededSize);
        free(pStorage);

        return res;
    }
    bool load(colorMatrix_t* ptr)
    {
        storageData* pStorage = nullptr;
        size_t       neededSize;
        bool         res;

        neededSize = sizeof(*pStorage);
        pStorage   = static_cast<struct storageData*>(malloc(neededSize));
        res        = MX_FlashParam_Read(pStorage, neededSize);
        if (!res || pStorage->initedFlag != flashFlag || pStorage->bankNum != ptr->bankNum || pStorage->colorNum != ptr->num) {
            goto Failed;
        }
        free(pStorage);

        neededSize += sizeof(int) * ptr->bankNum;
        pStorage = static_cast<storageData*>(malloc(neededSize));
        res      = MX_FlashParam_Read(pStorage, neededSize);
        if (!res)
            goto Failed;

        // check index is valid?
        for (int i = 0; i < ptr->bankNum; i++) {
            if (pStorage->colorIndex[ i ] < 0 || pStorage->colorIndex[ i ] >= pStorage->colorNum)
                goto Failed;
        }
        if (pStorage->bankIndex >= ptr->bankNum)
            goto Failed;
            
        ptr->bankIndex = pStorage->bankIndex;
        memcpy(ptr->colorIndex, pStorage->colorIndex, sizeof(int) * ptr->num);
        free(pStorage);
        return true;
    Failed:
        free(pStorage);
        return false;
    }

    static MX_FlashParam* GetInstance()
    {
        if (!instance)
            instance = new MX_FlashParam;
        return instance;
    }
};
#endif

#endif //BSPH1_FLASHPARAM_H
