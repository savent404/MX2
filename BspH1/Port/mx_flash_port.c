#include "HW_CONFIG.h"
#include <stdbool.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_flash.h"
#include <string.h>



#define POLYNOMIAL          0x1021  

#define INITIAL_REMAINDER   0xFFFF  

#define FINAL_XOR_VALUE     0x0000   

#define WIDTH (8 * sizeof(uint16_t))  

#define TOPBIT (1 << (WIDTH - 1))  

extern void FLASH_PageErase(uint32_t PageAddress);
static unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen);
static void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf);
static void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf);



typedef struct _configHeader{
    uint16_t magiccode;                 //magic code match means configuration valid
    uint16_t readflag;                   //when readflag not equate 0xFF, means the configuration
    uint16_t recoveryflag;                //when recoveryflag not equate 0xFF, means
    uint16_t crc16;                     //the payload crc check value
    uint32_t payloadsize;               //the payload size
    uint8_t* dataptr;                   //the data point
}configHeader, *pconfigHeader;

/**
 * @brief write data to flash
 * @retval if flash is not enough or write error, return false
 */
bool MX_FlashParam_Write(const void* data, size_t size)
{
    uint8_t i;
    pconfigHeader configptr;
    uint32_t WrAddr;
    uint8_t *pdata;
    uint32_t temp=0xFFFFFFFF;

    pdata = (uint8_t *)data;

    //check if there is block need to recovery
    for(i=0;i<CFG_STORAGE_SECTORNUM;i++) {
        configptr = (pconfigHeader)(CFG_STORAGE_BASEADDR +  i * CFG_STORAGE_SECTORSIZE);

        if(CFG_STORAGE_VALIDCODE == configptr->magiccode && 0x0000 == configptr->readflag) {
            if(0xFFFF == configptr->recoveryflag) {
                HAL_FLASH_Unlock();
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&configptr->recoveryflag, (uint64_t)(0x0000));
                HAL_FLASH_Lock();
            }
        }

        if(0xFFFF == configptr->magiccode && 0xFFFF == configptr->readflag && 0xFFFF == configptr->recoveryflag) {
            break;
        }
    }

    HAL_FLASH_Unlock();

    if(i==CFG_STORAGE_SECTORNUM) {
        for(i=0;i<CFG_STORAGE_SECTORNUM;i++) {
            configptr = (pconfigHeader)(CFG_STORAGE_BASEADDR +  i * CFG_STORAGE_SECTORSIZE);
            FLASH_PageErase((uint32_t)configptr);
        }
        configptr = (pconfigHeader)(CFG_STORAGE_BASEADDR);
    }

    WrAddr = (uint32_t)&configptr->dataptr;
    
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&configptr->payloadsize, (uint64_t)size);

    while(size > 4) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, WrAddr, (uint64_t)(*(uint32_t *)pdata));
        WrAddr += 4;
        pdata += 4;
        size -= 4;
    }
    for(i=0;i<4;i++) {
        if(size>0) {
            temp = temp >> 8;
            temp &= 0x00FFFFFF;
            temp += (*pdata << 24);
            pdata++;
            size--;
        }
        else {
            break;
        }
    }
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, WrAddr, (uint64_t)(temp));
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&configptr->crc16, (uint64_t)(CRC16_MODBUS((uint8_t *)&configptr->dataptr, configptr->payloadsize)));
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&configptr->magiccode, (uint64_t)CFG_STORAGE_VALIDCODE);
    
    HAL_FLASH_Lock();
    
    return true;
}

/**
 * @brief read data from flash
 * @retvl if flash is not enough or read error, return false
 */
bool MX_FlashParam_Read(void* data, size_t size)
{
    static uint8_t i=0;
    bool status=false;
    pconfigHeader configptr;
    static bool validflag = false;
    
    if(true == validflag) {
        configptr = (pconfigHeader)(CFG_STORAGE_BASEADDR +  i * CFG_STORAGE_SECTORSIZE);
        memcpy(data, &configptr->dataptr, size);
        status=true;
    }
    else {
        for(i=0;i<CFG_STORAGE_SECTORNUM;i++) {     
            configptr = (pconfigHeader)(CFG_STORAGE_BASEADDR +  i * CFG_STORAGE_SECTORSIZE);
            //if magic code is valid and payload size match required size
            if(CFG_STORAGE_VALIDCODE  == configptr->magiccode && \
               0xFFFF == configptr->readflag && \
               0xFFFF == configptr->recoveryflag && \
               configptr->crc16 == CRC16_MODBUS((uint8_t *)&configptr->dataptr, configptr->payloadsize > CFG_STORAGE_SECTORSIZE ? CFG_STORAGE_SECTORSIZE : configptr->payloadsize)) {
                status=true;
                memcpy(data, &configptr->dataptr, size);
                HAL_FLASH_Unlock();
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)&configptr->readflag, (uint64_t)(0x0000));
                HAL_FLASH_Lock();
                validflag = true;
                break;
            }
            else {
            }
        }
        //all configuration broken
        if(i == CFG_STORAGE_SECTORNUM) {
            HAL_FLASH_Unlock();

            if(i==CFG_STORAGE_SECTORNUM) {
                for(i=0;i<CFG_STORAGE_SECTORNUM;i++) {
                    configptr = (pconfigHeader)(CFG_STORAGE_BASEADDR +  i * CFG_STORAGE_SECTORSIZE);
                    FLASH_PageErase((uint32_t)configptr);
                }
                configptr = (pconfigHeader)(CFG_STORAGE_BASEADDR);
            }
        }
    }
    
    return status;
}


static unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen)  
{  
  unsigned short wCRCin = 0xFFFF;  
  unsigned short wCPoly = 0x8005;  
  unsigned char wChar = 0;  
    
  while (usDataLen--)     
  {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar,&wChar);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
  }  
  InvertUint16(&wCRCin,&wCRCin);  
  return (wCRCin) ;  
}

static void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf)  
{  
    int i;  
    unsigned char tmp[4];  
    tmp[0] = 0;  
    for(i=0;i< 8;i++)  
    {  
      if(srcBuf[0]& (1 << i))  
        tmp[0]|=1<<(7-i);  
    }  
    dBuf[0] = tmp[0];  
      
}  
static void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf)  
{  
    int i;  
    unsigned short tmp[4];  
    tmp[0] = 0;  
    for(i=0;i< 16;i++)  
    {  
      if(srcBuf[0]& (1 << i))  
        tmp[0]|=1<<(15 - i);  
    }  
    dBuf[0] = tmp[0];  
}


