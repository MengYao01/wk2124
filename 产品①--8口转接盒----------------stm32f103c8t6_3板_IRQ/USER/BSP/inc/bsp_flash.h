#ifndef __BSP_FLASH_H_
#define __BSP_FLASH_H_

#include "stm32f10x.h"


#define  FLASH_TEST_PAGE_ADDR  (0x1FC00) //31K
#define	FLASH_TEST_PAGE_ADDR_PTR (( unsigned short int *)FLASH_TEST_PAGE_ADDR)

#define  RAM_ADDR  0x20000000     
#define	RAM_ADDR_PTR (( unsigned int *)RAM_ADDR)

struct myflash
{
	unsigned char resource;			  //��¼�����Ƿ񱻲ɼ���	
	unsigned char Tary_Num[8];      //ʵ�ʽ�������̺�
	unsigned char Updata[2];        //������־λ
};

void myFLASHCTRL0_run_erase_page_mode(int FLASH_START_ADDR);
void myFLASHCTRL0_run_write_flash_mode(int FLASH_START_ADDR,unsigned char *src,int srcLength);
void ReadFlashData(unsigned short int *FlashAddress,unsigned char *RamAddress,int length);

#endif
