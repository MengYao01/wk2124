#include "wk2124s.h"
#include "wk2124s2.h"
#include "wk2124s.h"
#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "os_struct.h"
#include "bsp_timer2.h"
#include "soft_timer.h"
#include "special_buff.h"
#include "wk2124_epca1.h"


extern uint8_t uCom_send_dataBase[8][540];
extern struct soft_timer epca0_timer, epca1_timer, epca2_timer;
//OS_UART epca0 = {{'\0'},0,0,0};
OS_UART epca1 = {{'\0'},0,0,0};


/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void HW_WK2124_epca1_Init(void)
{
	/*�л���PAGE0ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S2_Write_Reg(SPAGE(1),0x00);

	/*�Ӵ��� 1 ���ƼĴ��� */
	EXHW_WK2412S2_Write_Reg(SPAGE0_SCR(1),0x03);	//�Ӵ���1 ����ʹ��  ����ʹ��

	/*�Ӵ��� 1 ���üĴ���*/
	EXHW_WK2412S2_Write_Reg(SPAGE0_LCR(1),0x00);	//�Ӵ���1 �������,��ͨģʽ,8λ����λ,0У��,1λֹͣλ

	/*�Ӵ��� 1 FIFO���ƼĴ���*/
	EXHW_WK2412S2_Write_Reg(SPAGE0_FCR(1),0x0F);	//�Ӵ���1 ���ʹ�����,���մ����� 
																								//ʹ�� ����,����FIFO ��λ���ͽ���FIFO
	/*�Ӵ��� 1 �ж�ʹ�ܼĴ���*/
	EXHW_WK2412S2_Write_Reg(SPAGE0_SIER(1),0x83); //�Ӵ���1 ʹ�ܽ���FIFO���ݴ����ж�
																								//��ֹ����FIFO���ж�
																								//��ֹ����FIFO�����ж�
																								//ʹ�ܽ���FIFO���ճ�ʱ�ж�
																								//ʹ�ܽ���FIFO���մ����ж�

	/*�л���PAGE1ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S2_Write_Reg(SPAGE(1),0x01);

	/*�Ӵ���1 ���������üĴ������ֽ� [Reg = 11.0592/(115200*16) = 6] */
	EXHW_WK2412S2_Write_Reg(SPAGE1_BAUD1(1),0x00);

	/*�Ӵ���1 ���������üĴ������ֽ� */
	EXHW_WK2412S2_Write_Reg(SPAGE1_BAUD0(1),0x05);

	/*�Ӵ���1 ���������üĴ���С������*/
	EXHW_WK2412S2_Write_Reg(SPAGE1_PRES(1),0x00);

	/*�л���PAGE0ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S2_Write_Reg(SPAGE(1),0x00);
}

/*******************************************************************************
* Function Name  : void WK2124_uart0_IRQHandler(void)
* Description    : �Ӵ���1�жϴ�����
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void WK2124_epca1_IRQHandler(void)
{
	  volatile uint8_t epca1_irq_stat = 0; 
    volatile uint16_t epca1_recv_cnt = 0;
	
		/*�ж��� ���� 1 ���������͵��ж�*/
		epca1_irq_stat = EXHW_WK2412S2_Read_Reg(SPAGE0_SIFR(1));
	
		if(epca1_irq_stat & (3 << 0)){//�Ӵ��� 1 ����FIFO�����жϱ�־ �� �Ӵ��� 1 ����FIFO��ʱ�жϱ�־
			 epca1_recv_cnt = Wk2124S2_6_GetBuf(epca1.recvBuff + epca1.count);
			 epca1.count += epca1_recv_cnt;
			
      if(epca1.count > 498){
         epca1.count = 498;
			}
	
			reload_timer(&epca1_timer,2);
			start_timer(&epca1_timer);   
			
		}

		if(epca1_irq_stat & (1 << 7)){//�Ӵ��� 1 ����FIFO���ݴ����жϱ�־
		
		}
		
		if(epca1_irq_stat & (1 << 2)){//�Ӵ��� 1 ����FIFO�����жϱ�־
		
		}
		if(epca1_irq_stat & (1 << 3)){//�Ӵ��� 1 ����FIFO���жϱ�־
		
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
void WK2124_epca1_Send_String(uint8_t *src, uint16_t len)
{
	uint16_t send_cnt = 0,send_i = 0;
	
	SP3485_06_DE_SEND();
	Delay_ms(30); // 30  --- 3ms 
	

		send_cnt = len/256;
		
		for(send_i = 0; send_i <= send_cnt ;send_i++){
			send_cnt = Wk2124S2_6_SendBuf(src + (256*send_i),(len - (send_cnt * send_i)));
			
			while(EXHW_WK2412S2_Read_Reg(SPAGE0_TFCNT(1))  > 0);
			while((EXHW_WK2412S2_Read_Reg(SPAGE0_FSR(1)) & 0x01) == 1);
		}
		SP3485_06_DE_RECV();
		Delay_ms(30); // 30  --- 3ms 
}


uint16_t Wk2124S2_6_SendBuf(uint8_t *sendbuf,uint16_t len)
{
	uint16_t ret = 0,tfcnt = 0,sendlen = 0;
	uint8_t  fsr = 0;
	
	fsr = EXHW_WK2412S2_Read_Reg(SPAGE0_FSR(1));
	if(~fsr & 0x02 )//�Ӵ��ڷ���FIFOδ��
	{
		tfcnt = EXHW_WK2412S2_Read_Reg(SPAGE0_TFCNT(1));//���Ӵ��ڷ���fifo�����ݸ���
		sendlen = 256 - tfcnt;//FIFO��д�������ֽ���

		if(sendlen < len){
			ret = sendlen; 
			EXHW_WK2412S2_Write_FIFO(SPAGE0_FDAT(1),sendbuf,sendlen);
		}else{
			EXHW_WK2412S2_Write_FIFO(SPAGE0_FDAT(1),sendbuf,len);
			ret = len;
		}
	}

	return ret;
}


uint16_t Wk2124S2_6_GetBuf(uint8_t *getbuf)
{
	uint16_t ret=0,rfcnt = 0;
	uint8_t fsr = 0;
	
	fsr = EXHW_WK2412S2_Read_Reg(SPAGE0_FSR(1));
	if(fsr & 0x08 )//�Ӵ��ڽ���FIFOδ��
	{
		rfcnt = EXHW_WK2412S2_Read_Reg(SPAGE0_RFCNT(1));//���Ӵ��ڷ���fifo�����ݸ���
		if(rfcnt == 0)//��RFCNT�Ĵ���Ϊ0��ʱ�������������������256������0�����ʱ��ͨ��FSR���жϣ����FSR��ʾ����FIFO��Ϊ�գ���Ϊ256���ֽ�
		{
			rfcnt = 256;
		}
		EXHW_WK2412S2_Read_FIFO(SPAGE0_FDAT(1),getbuf,rfcnt);
		ret = rfcnt;
	}
	 return ret;	
}

