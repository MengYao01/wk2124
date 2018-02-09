/*------------------------------------------------------------------------------
 | Copyright (c) 2014,�Ͼ�����ͨ�Źɷ����޹�˾.
 | All rights reserved.
 |------------------------------------------------------------------------------
 | �ļ����ƣ� Silion labs-sim3u146 �ӿڰ���������
 | �ļ����ţ� �������ƻ���
 | ��ժҪ�� 
 |------------------------------------------------------------------------------
 | ��ǰ�汾�� 1.1
 | ��   �ߣ� Mingliang.Lu
 | ���ʱ�䣺2014-07-28 
 |------------------------------------------------------------------------------
 | ԭ�汾 �� 1.0
 | ��  �ߣ�  MingLiang.lu
 | ���ʱ�䣺2013-05-20
 |------------------------------------------------------------------------------
 | �������⣬����ϵ: linux.lucian@gmail.com
 |------------------------------------------------------------------------------
 */
#include "myService.h" 
#include "crc8.h"
#include "stm32f10x.h"
#include <string.h>
#include <stdio.h>
#include "special_buff.h"
#include "wk2124_uart0.h"
#include "wk2124_usart1.h"
#include "wk2124_pca1.h"
#include "wk2124_pca0.h"
#include "wk2124_epca2.h"
#include "wk2124_epca1.h"
#include "wk2124_epca0.h"
#include "wk2124_usart0.h"
#include "wk2124s.h"
#include "wk2124s2.h"

extern uint8_t uCom_send_dataBase[8][COM_TX_SIZE];
extern uint8_t poll_410_cmd[8];
extern uint8_t frame_is_right[7];
extern uint8_t frame_is_wrong[7];

/*-----------------------��ĳ������ֹͣ����-------------------------------------
 * API ���ƣ�mySerial_disable_tx_work
 * ע    ��: ��ĳ�����ڵķ��Ͳ�����
 * �޸�ʱ�䣺2016-08-02 
 * ˵    ��: ��ĳ������ֹͣ����
 *------------------------------------------------------------------------------
 */
void mySerial_disable_tx(uint8_t uart_nr)
{
   switch(uart_nr){
   case 8://UART0
			EXHW_WK2412S_Disable_Tx(8);	 
      break;
   case 7://USART1
			EXHW_WK2412S_Disable_Tx(7);	 
      break;
   case 6://PCA1	
			EXHW_WK2412S_Disable_Tx(6);	 
   break;
   case 5://PCA0	
			EXHW_WK2412S_Disable_Tx(5);	 
      break;
   case 4://epca0  
			EXHW_WK2412S2_Disable_Tx(4);	 
      break;
   case 3://epca0  
			EXHW_WK2412S2_Disable_Tx(3);	 
      break;
   case 2://epca0  
			EXHW_WK2412S2_Disable_Tx(2);	 
      break;   
   case 1://usart0
			EXHW_WK2412S2_Disable_Tx(1);	 
      break;
   }
}

/*-----------------------��ĳ�����ڿ�ʼ����-------------------------------------
 * API ���ƣ�mySerial_disable_tx_work
 * ע    ��: ��ĳ�����ڵķ��Ͳ�����
 * �޸�ʱ�䣺2016-08-02 
 * ˵    ��: ��ĳ������ֹͣ����
 *------------------------------------------------------------------------------
 */
void mySerial_enable_tx(uint8_t uart_nr)
{
	switch(uart_nr){
	case 8://UART0
			EXHW_WK2412S_Enable_Tx(8);
      break;
	case 7://USART1
			EXHW_WK2412S_Enable_Tx(7);
      break;
	case 6://PCA1		 
			EXHW_WK2412S_Enable_Tx(6);
      break;
	case 5://PCA0	
			EXHW_WK2412S_Enable_Tx(5);		
		break;
	case 4://epca0 
			EXHW_WK2412S2_Enable_Tx(4);		
      break;
	case 3://epca1   
			EXHW_WK2412S2_Enable_Tx(3);		
      break;
	case 2://epca2  
			EXHW_WK2412S2_Enable_Tx(2);		
      break;   
	case 1://usart0  
			EXHW_WK2412S2_Enable_Tx(1);	
		break;
	}  
}

/*-----------------------��ĳ������ֹͣ����-------------------------------------
 * API ���ƣ�mySerial_disable_tx_work
 * ע    ��: ��ĳ�����ڵĽ��ղ�����
 * �޸�ʱ�䣺2016-08-02 
 * ˵    ��: ��ĳ������ֹͣ����
 *------------------------------------------------------------------------------
 */
void mySerial_disable_rx(uint8_t uart_nr)
{
   switch(uart_nr){
   case 8://UART0
				EXHW_WK2412S_Disable_Rx(8);	 
      break;
   case 7://USART1
				EXHW_WK2412S_Disable_Rx(7);		 
      break;
   case 6://PCA1	
				EXHW_WK2412S_Disable_Rx(6);		 
   break;
   case 5://PCA0	
				EXHW_WK2412S_Disable_Rx(5);		 
      break;
   case 4://epca2  
				EXHW_WK2412S2_Disable_Rx(4);		 
      break;
   case 3://epca1  
				EXHW_WK2412S2_Disable_Rx(3);		 
      break; 
   case 2://epca0   
				EXHW_WK2412S2_Disable_Rx(2);		 
      break;   
   case 1://usart0
				EXHW_WK2412S2_Disable_Rx(1);		 
      break;
   }
}


/*-----------------------��ĳ������ֹͣ����-------------------------------------
 * API ���ƣ�mySerial_disable_tx_work
 * ע    ��: ��ĳ�����ڵĽ��ղ�����
 * �޸�ʱ�䣺2016-08-02 
 * ˵    ��: ��ĳ������ֹͣ����
 *------------------------------------------------------------------------------
 */
void mySerial_enable_rx(uint8_t uart_nr)
{
	switch(uart_nr){
	case 8://UART0
				EXHW_WK2412S_Enable_Rx(8);
      break;
	case 7://USART1
				EXHW_WK2412S_Enable_Rx(6);	
      break;
	case 6://PCA1	
				EXHW_WK2412S_Enable_Rx(5);	
      break;
	case 5://PCA0	
				EXHW_WK2412S_Enable_Rx(5);	
		break;
	case 4://epca2  
				EXHW_WK2412S2_Enable_Rx(4);	
      break;
	case 3://epca1  
				EXHW_WK2412S2_Enable_Rx(3);	
      break;
	case 2://epca0  
				EXHW_WK2412S2_Enable_Rx(2);	
      break;   
	case 1://usart0  
				EXHW_WK2412S2_Enable_Rx(1);	
		break;
	}
   
}

void reset_mcu_enter_default_mode(void)
{

}

/*-----------------------��ĳ�����ڿ�ʼ����-------------------------------------
 * API ���ƣ�mySerial_enable_send
 * ע    ��: ��ĳ�����ڿ�ʼ����
 * �޸�ʱ�䣺2014-07-28 
 * ˵    ��: ��ĳ�����ڷ������ݣ��β�Ϊ��Ҫ���͵Ĵ��ںź�Ҫ���͵�����
 *------------------------------------------------------------------------------
 */
void mySerial_enable_send(uint8_t uart_nr,uint8_t *source,uint16_t len)
{
//	if(*(source + 0) == 0x00){//ֱ�Ӹ�λ
//	   reset_mcu_enter_default_mode();
//		 SI32_RSTSRC_A_generate_software_reset(SI32_RSTSRC_0);  //ǿ�������λ  // ��ת���û�����
//	}
	switch(uart_nr){
		case 8://UART0
			WK2124_uart0_Send_String(source,len);
		break;
		case 7://USART1
			WK2124_usart1_Send_String(source,len);
		break;
		case 6://PCA1
			WK2124_pca1_Send_String(source,len);  
		break;
		case 5://PCA0
			WK2124_pca0_Send_String(source,len);
		break;
		case 4://EPCA2
		WK2124_epca2_Send_String(source,len);
		break;
		case 3://EPCA1
			WK2124_epca1_Send_String(source,len);
		break;      
		case 2://EPCA0
			WK2124_epca0_Send_String(source,len);
		break;      
		case 1://usart0
			WK2124_usart0_Send_String(source,len); 		
		break;
		default:
			break;
	}
}

//���ڷ������ݻ���
void mySerial_send_string(uint8_t uart_nr,uint8_t *src)
{ 
	uint16_t len = 0;
	
	if(src == NULL)return ;//����׳�Կ��ǣ����ܶԿ�ָ�����κβ���
	
	len = (*(src + 1) << 8) | (*(src + 2)); 
	len += 2;
	if(len > 600)return ;
	
	if(*(src + 3) == 0x20){
		mySerial_enable_send(uart_nr,&poll_410_cmd[0],8);        
	}else if(*(src + 3) == 0xa6){      
		mySerial_enable_send(uart_nr,&frame_is_right[0],7);
	}else if(*(src + 3) == 0xf6){
		mySerial_enable_send(uart_nr,&frame_is_wrong[0],7);
	}else if(*(src + 3) != 0x0){
		mySerial_enable_send(uart_nr,&uCom_send_dataBase[uart_nr-1][0],len); 
	}

}
//--------------------------------------------------eof-------------------------------------------------------------------------
