/*
 *WK2124S ֧�� SPI0ģʽ  ��ǰ
 *WK2124S IRQ�ж��źŵ�����͵�ƽ��Ч
 *WK2124S RSTӲ����λ���ŵ͵�ƽ��Ч
 *
 *WK2124S �Ĵ�����6λ��ַ��� 000000 --- 111111
 *
 *  case 8: --- UART0
 *  case 7: --- USART1
 *  case 6: --- PCA1	
 *  case 5: --- PCA0	
 
 *  case 4: --- epca2  
 *  case 3: --- epca1  
 *  case 2: --- epca0  
 *  case 1: --- usart0
 */

#include "wk2124s.h"
#include "wk2124s2.h"
#include "bsp_spi.h"
#include "wk2124_uart0.h"
#include "wk2124_usart1.h"
#include "wk2124_pca1.h"
#include "wk2124_pca0.h"
#include "wk2124_epca2.h"
#include "wk2124_epca1.h"
#include "wk2124_epca0.h"
#include "wk2124_usart0.h"


/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void EXHW_WK2412S2_Write_Reg(uint8_t reg, uint8_t dat)
{
	SPI2_WK2412S2_CS_LOW();
	SPI2_WK2412S2_Read_Write(reg);
	SPI2_WK2412S2_Read_Write(dat);
	SPI2_WK2412S2_CS_HIGH();
}

/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
uint8_t EXHW_WK2412S2_Read_Reg(uint8_t reg)
{
	uint8_t rec_data = 0; 
	SPI2_WK2412S2_CS_LOW();
	SPI2_WK2412S2_Read_Write(0x40 + reg);
	rec_data = SPI2_WK2412S2_Read_Write(Dummy_Byte);
	SPI2_WK2412S2_CS_HIGH();
	return rec_data;
}

/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void EXHW_WK2412S2_Write_FIFO(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint16_t cnt = 0;
	
	SPI2_WK2412S2_CS_LOW();
	SPI2_WK2412S2_Read_Write(0x80 + reg);
	for(cnt = 0;cnt < len; cnt++){
		SPI2_WK2412S2_Read_Write(*(buf + cnt));
	} 
	SPI2_WK2412S2_CS_HIGH();
}

/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void EXHW_WK2412S2_Read_FIFO(uint8_t reg, uint8_t *buf, uint16_t len)
{
	uint16_t cnt = 0;
	
	SPI2_WK2412S2_CS_LOW();
	SPI2_WK2412S2_Read_Write(0xC0 + reg);
	for(cnt = 0;cnt < len; cnt++){
		*(buf + cnt) = SPI2_WK2412S2_Read_Write(Dummy_Byte);
	} 
	SPI2_WK2412S2_CS_HIGH();
}


/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void EXHW_WK2412S2_Init(void)
{
	/*ʹ���Ӵ���1,2,3,4��ʱ��*/
	EXHW_WK2412S2_Write_Reg(GENA,0x0F);
	
	/*��λ�Ӵ���1,2,3,4*/
	EXHW_WK2412S2_Write_Reg(GRST,0x0F);
	
	/*ʹ���Ӵ���1,2,3,4��ȫ���ж� */
	EXHW_WK2412S2_Write_Reg(GIER,0x0F);
	
}

/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
uint8_t Wk2xxx2Test(void)
{
	uint8_t rec_data = 0,rv = 0;
	//���ӿ�ΪSPI	
	rec_data = EXHW_WK2412S2_Read_Reg(GENA);
	if(rec_data == 0x30){
		rv = 0;
	}
	else{
		rv = 1;
	}
	return rv;
}

/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void EXHW_WK2412S2_Disable_Tx(uint8_t port)
{
	uint8_t scr = 0;
	
	switch(port){
		case 4:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(0)); 
		   scr &= ~(1 << 1);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(0),scr);
			break;
		
		case 3:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(1)); 
		   scr &= ~(1 << 1);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(1),scr);
			break;
		
		case 2:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(2)); 
		   scr &= ~(1 << 1);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(2),scr);
			break;
		
		case 1:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(3)); 
		   scr &= ~(1 << 1);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(3),scr);
			break;
		
		default:
			break;
	}
}

void EXHW_WK2412S2_Enable_Tx(uint8_t port)
{
	uint8_t scr = 0;
	switch(port){
		case 4:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(0)); 
		   scr |= (1 << 1);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(0),scr);
			break;
		
		case 3:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(1)); 
		   scr |= (1 << 1);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(1),scr);
			break;
		
		case 2:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(2)); 
		   scr |= (1 << 1);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(2),scr);
			break;
		
		case 1:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(3)); 
		   scr |= (1 << 1);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(3),scr);
			break;
		
		default:
			break;
	}
}

void EXHW_WK2412S2_Disable_Rx(uint8_t port)
{
	uint8_t scr = 0;
	switch(port){
		case 4:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(0)); 
		   scr &= ~(1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(0),scr);
			break;
		
		case 3:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(1)); 
		   scr &= ~(1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(1),scr);
			break;
		
		case 2:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(2)); 
		   scr &= ~(1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(2),scr);
			break;
		
		case 1:
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(3)); 
		   scr &= ~(1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(3),scr);
			break;
		
		default:
			break;
	}
}

void EXHW_WK2412S2_Enable_Rx(uint8_t port)
{
	uint8_t scr = 0,fcr = 0;
	switch(port){
		case 4:
			 //��λ n ���ڵ�FIFO
			 fcr = EXHW_WK2412S2_Read_Reg(SPAGE0_FCR(0)); 
		   fcr |= (1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_FCR(0),fcr);
			 //ʹ�ܴ��� n  �Ľ���
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(0)); 
		   scr |= (1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(0),scr);
			break;
		
		case 3:
			 //��λ n ���ڵ�FIFO
			 fcr = EXHW_WK2412S2_Read_Reg(SPAGE0_FCR(1)); 
		   fcr |= (1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_FCR(1),fcr);
			 //ʹ�ܴ��� n  �Ľ���
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(1)); 
		   scr |= (1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(1),scr);
			break;
		
		case 2:
			 //��λ n ���ڵ�FIFO
			 fcr = EXHW_WK2412S2_Read_Reg(SPAGE0_FCR(2)); 
		   fcr |= (1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_FCR(2),fcr);
			 //ʹ�ܴ��� n  �Ľ���
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(2)); 
		   scr |= (1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(2),scr);
			break;
		
		case 1:
			 //��λ n ���ڵ�FIFO
			 fcr = EXHW_WK2412S2_Read_Reg(SPAGE0_FCR(3)); 
		   fcr |= (1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_FCR(3),fcr);
			 //ʹ�ܴ��� n  �Ľ���
			 scr = EXHW_WK2412S2_Read_Reg(SPAGE0_SCR(3)); 
		   scr |= (1 << 0);
		   EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(3),scr);
			break;
		
		default:
			break;
	}
}

/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
	volatile uint8_t g_irq_stat = 0; 
	
	if(EXTI_GetITStatus(EXTI_Line10) != RESET){
		EXTI_ClearITPendingBit(EXTI_Line10);
		
		/*ʹ���Ӵ���1,2,3,4��ʱ��*/
		g_irq_stat = EXHW_WK2412S2_Read_Reg(GIFR);
		
		if(g_irq_stat & (1 << 0)){//�Ӵ��� 1 ���ж�
			WK2124_epca2_IRQHandler();
		}
		if(g_irq_stat & (1 << 1)){//�Ӵ��� 2 ���ж�
			WK2124_epca1_IRQHandler();
		}
		if(g_irq_stat & (1 << 2)){//�Ӵ��� 3 ���ж�
			WK2124_epca0_IRQHandler();
		}
		if(g_irq_stat & (1 << 3)){//�Ӵ��� 4 ���ж�
			WK2124_usart0_IRQHandler();
		}
	}
	
//	printf("this is wk2412_2 IRQ \r\n");
}



