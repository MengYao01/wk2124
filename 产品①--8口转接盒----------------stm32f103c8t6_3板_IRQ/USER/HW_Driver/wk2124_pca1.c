#include "wk2124_pca1.h"
#include "wk2124s.h"
#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "os_struct.h"
#include "bsp_timer2.h"
#include "soft_timer.h"
#include "special_buff.h"


extern uint8_t uCom_send_dataBase[8][540];
extern struct soft_timer pca1_timer;
OS_UART pca1 = {{'\0'},0,0,0};

/*******************************************************************************
* Function Name  : 
* Description    : 
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void HW_WK2124_pca1_Init(void)
{
	/*�л���PAGE0ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S1_Write_Reg(SPAGE(2),0x00);

	/*�Ӵ��� 2 ���ƼĴ��� */
	EXHW_WK2412S1_Write_Reg(SPAGE0_SCR(2),0x03);	//�Ӵ���2 ����ʹ��  ����ʹ��

	/*�Ӵ��� 2 ���üĴ���*/
	EXHW_WK2412S1_Write_Reg(SPAGE0_LCR(2),0x00);	//�Ӵ���2 �������,��ͨģʽ,8λ����λ,0У��,1λֹͣλ

	/*�Ӵ��� 2 FIFO���ƼĴ���*/
	EXHW_WK2412S1_Write_Reg(SPAGE0_FCR(2),0x0F);	//�Ӵ���2 ���ʹ�����,���մ����� 
																								//ʹ�ܷ���,����FIFO ��λ���ͽ���FIFO
	/*�Ӵ��� 2 �ж�ʹ�ܼĴ���*/
	EXHW_WK2412S1_Write_Reg(SPAGE0_SIER(2),0x83); //�Ӵ���2 ʹ�ܽ���FIFO���ݴ����ж�
																								//��ֹ����FIFO���ж�
																								//��ֹ����FIFO�����ж�
																								//ʹ�ܽ���FIFO���ճ�ʱ�ж�
																								//ʹ�ܽ���FIFO���մ����ж�

	/*�л���PAGE1ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S1_Write_Reg(SPAGE(2),0x01);

	/*�Ӵ���1 ���������üĴ������ֽ� [Reg = 11.0592/(115200*16) = 6] */
	EXHW_WK2412S1_Write_Reg(SPAGE1_BAUD1(2),0x00);

	/*�Ӵ���1 ���������üĴ������ֽ� */
	EXHW_WK2412S1_Write_Reg(SPAGE1_BAUD0(2),0x05);

	/*�Ӵ���1 ���������üĴ���С������*/
	EXHW_WK2412S1_Write_Reg(SPAGE1_PRES(2),0x00);

	/*�л���PAGE0ҳ�е��Ӵ��ڼĴ����� */
	EXHW_WK2412S1_Write_Reg(SPAGE(2),0x00);
}

/*******************************************************************************
* Function Name  : void WK2124_pca1_IRQHandler(void)
* Description    : �Ӵ���1�жϴ�����
*  
*
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void WK2124_pca1_IRQHandler(void)
{
	  volatile uint8_t pca1_irq_stat = 0; 
    volatile uint16_t pca1_recv_cnt = 0;
	
		/*�ж��� ���� 3 ���������͵��ж�*/
		pca1_irq_stat = EXHW_WK2412S1_Read_Reg(SPAGE0_SIFR(2));
	
		if(pca1_irq_stat & (3 << 0)){//�Ӵ��� 1 ����FIFO�����жϱ�־ �� �Ӵ��� 1 ����FIFO��ʱ�жϱ�־
			 pca1_recv_cnt = Wk2124S1_3_GetBuf(pca1.recvBuff + pca1.count);
			 pca1.count += pca1_recv_cnt;
			
      if(pca1.count > 498){
         pca1.count = 498;
			}
	
			reload_timer(&pca1_timer,2);
			start_timer(&pca1_timer);   
			
		}
//		if(pca1_irq_stat & (1 << 1)){//�Ӵ��� 1 ����FIFO��ʱ�жϱ�־
//			Wk2124S1_2_GetBuf(pca1.recvBuff);
//		}
		if(pca1_irq_stat & (1 << 7)){//�Ӵ��� 1 ����FIFO���ݴ����жϱ�־
		
		}
		
		if(pca1_irq_stat & (1 << 2)){//�Ӵ��� 1 ����FIFO�����жϱ�־
		
		}
		if(pca1_irq_stat & (1 << 3)){//�Ӵ��� 1 ����FIFO���жϱ�־
		
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
void WK2124_pca1_Send_String(uint8_t *src, uint16_t len)
{
	uint16_t send_cnt = 0,send_i = 0;
	
	SP3485_03_DE_SEND();
	Delay_ms(30); // 30  --- 3ms 
	
	send_cnt = len/256;
	
	for(send_i = 0; send_i <= send_cnt ;send_i++){
		send_cnt = Wk2124S1_3_SendBuf(src + (256*send_i),(len - (send_cnt * send_i)));
		
		while(EXHW_WK2412S1_Read_Reg(SPAGE0_TFCNT(2))  > 0);
		while((EXHW_WK2412S1_Read_Reg(SPAGE0_FSR(2)) & 0x01) == 1);
	}
	SP3485_03_DE_RECV();
	Delay_ms(30); // 30  --- 3ms 
}


uint16_t Wk2124S1_3_SendBuf(uint8_t *sendbuf,uint16_t len)
{
	uint16_t ret = 0,tfcnt = 0,sendlen = 0;
	uint8_t  fsr = 0;
	
	fsr = EXHW_WK2412S1_Read_Reg(SPAGE0_FSR(2));
	if(~fsr & 0x02 )//�Ӵ��ڷ���FIFOδ��
	{
		tfcnt = EXHW_WK2412S1_Read_Reg(SPAGE0_TFCNT(2));//���Ӵ��ڷ���fifo�����ݸ���
		sendlen = 256 - tfcnt;//FIFO��д�������ֽ���

		if(sendlen < len){
			ret = sendlen; 
			EXHW_WK2412S1_Write_FIFO(SPAGE0_FDAT(2),sendbuf,sendlen);
		}else{
			EXHW_WK2412S1_Write_FIFO(SPAGE0_FDAT(2),sendbuf,len);
			ret = len;
		}
	}

	return ret;
}


uint16_t Wk2124S1_3_GetBuf(uint8_t *getbuf)
{
	uint16_t ret=0,rfcnt = 0;
	uint8_t fsr = 0;
	
	fsr = EXHW_WK2412S1_Read_Reg(SPAGE0_FSR(2));
	if(fsr & 0x08 )//�Ӵ��ڽ���FIFOδ��
	{
		rfcnt = EXHW_WK2412S1_Read_Reg(SPAGE0_RFCNT(2));//���Ӵ��ڷ���fifo�����ݸ���
		if(rfcnt == 0)//��RFCNT�Ĵ���Ϊ0��ʱ�������������������256������0�����ʱ��ͨ��FSR���жϣ����FSR��ʾ����FIFO��Ϊ�գ���Ϊ256���ֽ�
		{
			rfcnt = 256;
		}
		EXHW_WK2412S1_Read_FIFO(SPAGE0_FDAT(2),getbuf,rfcnt);
		ret = rfcnt;
	}
	 return ret;	
}
