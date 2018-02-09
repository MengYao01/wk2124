#include "bsp_usart2.h"
#include <string.h>
#include "bsp_gpio.h"
#include "bsp_timer2.h"
#include "os_struct.h"
#include "soft_timer.h"

OS_UART1 uart1;
//����ô��ڵ�֡���������
uint16_t uart1_frame_record;
extern struct soft_timer uart1_timer;
extern struct soft_timer led_timer;

void Bsp_Usart2_Init(void)
{
	USART_InitTypeDef	USART2_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure; 
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	
	  /* Enable USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_Init(&NVIC_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //TXD10 --- PA2 --- USART2_TX
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStructure);  //RXD10 --- PA3 --- USART2_RX
	
	USART2_InitStructure.USART_BaudRate = 115200;
	USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART2_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	USART2_InitStructure.USART_Parity =	USART_Parity_No;
	USART2_InitStructure.USART_StopBits =	USART_StopBits_1;
	USART2_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2,&USART2_InitStructure);
	


  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);               //������1�����жϡ�  
  USART_Cmd(USART2,ENABLE);                                  //��������  
  USART_ClearFlag(USART2,USART_FLAG_TC);                     //������ɱ�־λ 

}

void open_uart1_rx_function(void)
{  
   memset(&uart1.recvBuff[0], 0x0, sizeof(uart1.recvBuff));
   uart1.recvFlag = 0x0;
   uart1.count = 0x0;

   uart1.didex = 0x0;
   uart1.dsize = 0x0;
   uart1.txstat = 0x0;
   uart1.txbuf = NULL;
   
//   SI32_UART_A_enable_rx(SI32_UART_1);
//   SI32_UART_A_enable_tx(SI32_UART_1);
   
   start_timer(&led_timer);
}



//void myUART1_send_multi_bytes(uint8_t *buf , uint16_t len)
//{
//	uart1.txbuf = buf;
//	uart1.dsize = len;
//	uart1.didex = 0;
//	USART2->CR1 |= (1 << 7);//���ͻ��������ж�ʹ��
//	SP3485_IN_DE_SEND();
//	Delay_ms(30); // 30  --- 3ms 
//	
////	while(len--){
////		USART_SendData(USART2, buf[send_index]); 
////		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
////		send_index++;
////	}
//}



void usart2_rx_irq_enable(uint8_t enable)
{
	if(enable == 1){
//		USART2->SR &= ~(1 << 6);//�����������жϵ�״̬
		USART2->CR1 |= (1 << 2);//ʹ�ܽ���  RE
//		USART2->CR1 |= 	(1 << 5);//RXNEIE:���ջ������ǿ��ж�ʹ��  ��USART_SR�е�ORE����RXNEΪ��1��ʱ������USART�ж�  
	}else{
		USART2->CR1 &= ~(1 << 2);//�رս���  RE
//		USART2->CR1 &= ~(1 << 5);
	}
}


void usart2_tx_irq_enable(uint8_t enable)
{
	if(enable == 1){
		USART2->CR1 |= (1 << 3);//ʹ�ܷ���  TE  
//		USART2->CR1 |= (1 << 6);//ʹ�ܷ�������ж�
	}else{
//		USART2->CR1 &= ~(1 << 6);//�رշ�������ж�
		USART2->CR1 &= ~(1 << 3);//�رշ���  TE 
//		USART2->SR &= ~(1 << 6);	//�����������жϵ�״̬	
	}
}


uint16_t  myUART1_send_multi_bytes(uint8_t *src,uint16_t len)
{
   //��������Լ��,���ڵķ���״̬���
   if( uart1.txstat == DEVICE_BUSY){
      return 0;
   }
   
   //ʹ�ܶ��ϴ��ڵķ���485�����뵽����ģʽ
//   SI32_UART_A_disable_rx(SI32_UART_1); 
   usart2_rx_irq_enable(DISABLE);   
   SP3485_IN_DE_SEND();
   
   //�����������ݵ��ڴ��ַ��uart1�ķ��ͻ�����,����Ϊ����״̬λæ
   uart1.txbuf = src;
   uart1.didex = 0x0;   
   uart1.dsize = len;  
   uart1.txstat = DEVICE_BUSY;
  
   Delay_ms(30);
   
//   //��շ���FIFO��ͬʱʹ�ܷ������ݵĿ�FIFO��������
//   SI32_UART_A_flush_tx_fifo(SI32_UART_1);   
//   SI32_UART_A_enable_tx_data_request_interrupt(SI32_UART_1);    
	 
	USART2->CR1 |= (1 << 7);//���ͻ��������ж�ʹ��
   for(;;){
      if(uart1.txstat == DEVICE_BUSY){
      
      }else{
         break;
      }
   }
   
	return len;
}


void USART2_IRQHandler(void)
{
/*-------���ڽ����ж�-------*/
	if(USART2->SR & (1 << 5)){
		uart1.recvBuff[uart1.count++] = USART2->DR;
		if(uart1.count >= 699){
			 uart1.count=698;     
		}
		
		USART2->SR &= ~(1 << 5);
		
		reload_timer(&uart1_timer,2);
		start_timer(&uart1_timer);    
	}
	
/*-------��������ж�-------*/
	if(USART2->SR & (1 << 6)){ 
		if(uart1.dsize > uart1.didex) {
			USART2->DR = uart1.txbuf[uart1.didex];
			uart1.didex++;
		}else{
			USART2->CR1 &= ~(1 << 7);//�رշ��ͻ�����Ϊ��

      uart1.txstat = DEVICE_IDLE;
      uart1.txbuf = NULL;
      uart1.didex = 0x0;   
      uart1.dsize = 0x0;      
      //ʹ��485�Ľ���
//      SI32_UART_A_enable_rx(SI32_UART_1);       
//      rs485_mode_set(0, RS485_RECV);   
	    usart2_rx_irq_enable(ENABLE); 	
	    SP3485_IN_DE_RECV();
			
		}
	}
	
}


//void USART2_IRQHandler(void)
//{
//	/*-------���ڽ����ж�-------*/
//	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET){
//		USART_ClearFlag(USART2, USART_FLAG_RXNE);
//		
//		
//	}
//	
//  /*-------��������ж�-------*/
//	if(USART_GetFlagStatus(USART2, USART_FLAG_TC) != RESET){
//		 USART_ClearFlag(USART2, USART_FLAG_TC);
//		
//		if(uart1.dsize > uart1.didex){
//			USART_SendData(USART2,uart1.txbuf[uart1.didex]);
//			uart1.didex++;
//		}else{
//			uart1.dsize = 0;
//			uart1.didex = 0;

//		}
//	}
//}

