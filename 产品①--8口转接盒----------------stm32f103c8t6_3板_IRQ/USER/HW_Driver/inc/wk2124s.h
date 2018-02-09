#ifndef __WK2124S_H_
#define __WK2124S_H_

#include "stm32f10x.h"
#include "bsp_printf.h"

//ȫ�ּĴ����б� ��4��
#define GENA	0x00   													/*ȫ�ֿ��ƼĴ��� 											R/W*/
#define GRST  0x01   													/*ȫ���Ӵ��ڸ�λ�Ĵ��� 								R/W*/
#define GIER  0x10   													/*ȫ���жϼĴ��� 											R/W*/
#define GIFR  0x11   													/*ȫ���жϱ�־�Ĵ��� 									R*/

//�Ӵ��ڼĴ���   ��18�� (x:00 --- 11)
#define	SPAGE(x)     					((x << 4)|0x03)	/*�Ӵ���ҳ���ƼĴ��� 									R/W*/

#define	SPAGE0_SCR(x)     		((x << 4)|0x04) /*�Ӵ���ʹ�ܼĴ��� 										R/W*/
#define	SPAGE0_LCR(x)     		((x << 4)|0x05) /*�Ӵ������üĴ��� 										R/W*/
#define	SPAGE0_FCR(x)     		((x << 4)|0x06) /*�Ӵ���FIFO���ƼĴ��� 								R/W*/
#define	SPAGE0_SIER(x)     		((x << 4)|0x07) /*�Ӵ����ж�ʹ�ܼĴ��� 								R/W*/
#define	SPAGE0_SIFR(x)     		((x << 4)|0x08) /*�Ӵ����жϱ�־�Ĵ��� 								R/W*/
#define	SPAGE0_TFCNT(x)     	((x << 4)|0x09) /*�Ӵ��ڷ���FIFO�����Ĵ��� 						R*/
#define	SPAGE0_RFCNT(x)     	((x << 4)|0x0A) /*�Ӵ��ڽ���FIFO�����Ĵ��� 						R*/
#define	SPAGE0_FSR(x)     		((x << 4)|0x0B) /*�Ӵ���FIFO״̬�Ĵ��� 								R*/
#define	SPAGE0_LSR(x)     		((x << 4)|0x0C) /*�Ӵ��ڽ���״̬�Ĵ��� 								R*/
#define	SPAGE0_FDAT(x)     		((x << 4)|0x0D) /*�Ӵ���FIFO���ݼĴ��� 								R/W*/

#define	SPAGE1_BAUD1(x)     	((x << 4)|0x04) /*�Ӵ��ڲ��������üĴ������ֽ� 				R/W*/
#define	SPAGE1_BAUD0(x)     	((x << 4)|0x05) /*�Ӵ��㲨����Ʋֻ�Ĵ������ֽ� 				R/W*/
#define	SPAGE1_PRES(x)     		((x << 4)|0x06) /*�Ӵ��㲨�������üĴ���С������ 			R/W*/
#define	SPAGE1_RFTL(x)     		((x << 4)|0x07) /*�Ӵ��ڽ���FIFO�жϴ��������üĴ��� 	R/W*/
#define	SPAGE1_TFTL(x)     		((x << 4)|0x08) /*�Ӵ��ڷ���FIFO�жϴ��������üĴ��� 	R/W*/



void EXHW_WK2412S_Init(void);

void EXHW_WK2412S1_Write_Reg(uint8_t reg, uint8_t dat);
uint8_t EXHW_WK2412S1_Read_Reg(uint8_t reg);
void EXHW_WK2412S1_Write_FIFO(uint8_t reg, uint8_t *buf, uint16_t len);
void EXHW_WK2412S1_Read_FIFO(uint8_t reg, uint8_t *buf, uint16_t len);

uint8_t Wk2xxxTest(void);


void EXHW_WK2412S_Disable_Tx(uint8_t port);
void EXHW_WK2412S_Enable_Tx(uint8_t port);
void EXHW_WK2412S_Disable_Rx(uint8_t port);
void EXHW_WK2412S_Enable_Rx(uint8_t port);

#endif
