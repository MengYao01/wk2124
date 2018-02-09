#include "wk2124_usart1.h"
#include "wk2124s.h"
#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "os_struct.h"
#include "bsp_timer2.h"
#include "soft_timer.h"
#include "special_buff.h"

//����ô��ڵ�֡���������
uint8_t usart1_frame_record;

//usart1�Լ����շ�����
extern uint8_t uCom_send_dataBase[8][COM_TX_SIZE];
extern uint8_t msg_recv_times[6];

extern struct soft_timer usart1_timer;
extern uint16_t recv_count[6];

OS_UART usart1 = {{'\0'},0,0,0};
/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void HW_WK2124_usart1_Init(void)
{
	/*�л���PAGE0ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S1_Write_Reg(SPAGE(1),0x00);

	/*�Ӵ��� 2 ���ƼĴ��� */
	EXHW_WK2412S1_Write_Reg(SPAGE0_SCR(1),0x03);	//�Ӵ���2 ����ʹ��  ����ʹ��

	/*�Ӵ��� 2 ���üĴ���*/
	EXHW_WK2412S1_Write_Reg(SPAGE0_LCR(1),0x00);	//�Ӵ���2 �������,��ͨģʽ,8λ����λ,0У��,1λֹͣλ

	/*�Ӵ��� 2 FIFO���ƼĴ���*/
	EXHW_WK2412S1_Write_Reg(SPAGE0_FCR(1),0x0F);	//�Ӵ���2 ���ʹ�����,���մ����� 
																								//ʹ�ܷ���,����FIFO ��λ���ͽ���FIFO
	/*�Ӵ��� 2 �ж�ʹ�ܼĴ���*/
	EXHW_WK2412S1_Write_Reg(SPAGE0_SIER(1),0x83); //�Ӵ���2 ʹ�ܽ���FIFO���ݴ����ж�
																								//��ֹ����FIFO���ж�
																								//��ֹ����FIFO�����ж�
																								//ʹ�ܽ���FIFO���ճ�ʱ�ж�
																								//ʹ�ܽ���FIFO���մ����ж�

	/*�л���PAGE1ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S1_Write_Reg(SPAGE(1),0x01);

	/*�Ӵ���1 ���������üĴ������ֽ� [Reg = 11.0592/(115200*16) = 6] */
	EXHW_WK2412S1_Write_Reg(SPAGE1_BAUD1(1),0x00);

	/*�Ӵ���1 ���������üĴ������ֽ� */
	EXHW_WK2412S1_Write_Reg(SPAGE1_BAUD0(1),0x05);

	/*�Ӵ���1 ���������üĴ���С������*/
	EXHW_WK2412S1_Write_Reg(SPAGE1_PRES(1),0x00);

	/*�л���PAGE0ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S1_Write_Reg(SPAGE(1),0x00);
}

/*******************************************************************************
* Function Name  : void WK2124_usart1_IRQHandler(void)
* Description    : �Ӵ���1�жϴ�����
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void WK2124_usart1_IRQHandler(void)
{
	  volatile uint8_t usart1_irq_stat = 0; 
    volatile uint16_t usart1_recv_cnt = 0;
	
		/*�ж��� ���� 1 ���������͵��ж�*/
		usart1_irq_stat = EXHW_WK2412S1_Read_Reg(SPAGE0_SIFR(1));
	
		if(usart1_irq_stat & (3 << 0)){//�Ӵ��� 1 ����FIFO�����жϱ�־ �� �Ӵ��� 1 ����FIFO��ʱ�жϱ�־
			 usart1_recv_cnt = Wk2124S1_2_GetBuf(usart1.recvBuff + usart1.count);
			 usart1.count += usart1_recv_cnt;
			
      if(usart1.count > 498){
         usart1.count = 498;
			}
	
			reload_timer(&usart1_timer,2);
			start_timer(&usart1_timer);   
			
		}

		if(usart1_irq_stat & (1 << 7)){//�Ӵ��� 1 ����FIFO���ݴ����жϱ�־
		
		}
		
		if(usart1_irq_stat & (1 << 2)){//�Ӵ��� 1 ����FIFO�����жϱ�־
		
		}
		if(usart1_irq_stat & (1 << 3)){//�Ӵ��� 1 ����FIFO���жϱ�־
		
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
void WK2124_usart1_Send_String(uint8_t *src, uint16_t len)
{
	uint16_t send_cnt = 0,send_i = 0;
	
	SP3485_02_DE_SEND();
	Delay_ms(30); // 30  --- 3ms 
	
	send_cnt = len/256;
	
	for(send_i = 0; send_i <= send_cnt ;send_i++){
		send_cnt = Wk2124S1_2_SendBuf(src + (256*send_i),(len - (send_cnt * send_i)));
		
		while(EXHW_WK2412S1_Read_Reg(SPAGE0_TFCNT(1))  > 0);
		while((EXHW_WK2412S1_Read_Reg(SPAGE0_FSR(1)) & 0x01) == 1);
	}
	SP3485_02_DE_RECV();
	Delay_ms(30); // 30  --- 3ms 
}


uint16_t Wk2124S1_2_SendBuf(uint8_t *sendbuf,uint16_t len)
{
	uint16_t ret = 0,tfcnt = 0,sendlen = 0;
	uint8_t  fsr = 0;
	
	fsr = EXHW_WK2412S1_Read_Reg(SPAGE0_FSR(1));
	if(~fsr & 0x02 )//�Ӵ��ڷ���FIFOδ��
	{
		tfcnt = EXHW_WK2412S1_Read_Reg(SPAGE0_TFCNT(1));//���Ӵ��ڷ���fifo�����ݸ���
		sendlen = 256 - tfcnt;//FIFO��д�������ֽ���

		if(sendlen < len){
			ret = sendlen; 
			EXHW_WK2412S1_Write_FIFO(SPAGE0_FDAT(1),sendbuf,sendlen);
		}else{
			EXHW_WK2412S1_Write_FIFO(SPAGE0_FDAT(1),sendbuf,len);
			ret = len;
		}
	}

	return ret;
}


uint16_t Wk2124S1_2_GetBuf(uint8_t *getbuf)
{
	uint16_t ret=0,rfcnt = 0;
	uint8_t fsr = 0;
	
	fsr = EXHW_WK2412S1_Read_Reg(SPAGE0_FSR(1));
	if(fsr & 0x08 )//�Ӵ��ڽ���FIFOδ��
	{
		rfcnt = EXHW_WK2412S1_Read_Reg(SPAGE0_RFCNT(1));//���Ӵ��ڷ���fifo�����ݸ���
		if(rfcnt == 0)//��RFCNT�Ĵ���Ϊ0��ʱ�������������������256������0�����ʱ��ͨ��FSR���жϣ����FSR��ʾ����FIFO��Ϊ�գ���Ϊ256���ֽ�
		{
			rfcnt = 256;
		}
		EXHW_WK2412S1_Read_FIFO(SPAGE0_FDAT(1),getbuf,rfcnt);
		ret = rfcnt;
	}
	 return ret;	
}
