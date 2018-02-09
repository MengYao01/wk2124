/**
  ******************************************************************************
  * @file    :order.c used for Potevio interface board
  * @author  :linux.lucian@gmail.com
  * @version : ����1.0
  * @date    :2014-08-23
  * @brief   :source file for interface board(sim3u146) update
  * 
  ******************************************************************************
  */
#include "s3c44b0x.h"
#include "cf8051.h"
#include "myService.h" 
//#include <SI32_PBHD_A_Type.h>
//#include <SI32_UART_A_Type.h>
//#include "myUART1.h"
#include "bsp_flash.h"
#include "bsp_timer2.h"
#include "bsp_usart2.h"

uint8_t TrayInfo[17] = {0x7e,0x00,0x0F,0x03,0x0f,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x01,0x00,0x5a};
uint8_t version[57]  = {0x7e,0x00,0x0b,0x05,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//��Ԫ����������
uint8_t cf8051update[40] = {0x7e,0x00,0x21,0x06,0x01,0x3,0x06}; //���������ط��ص�Ԫ�������� ���������ݰ�д�롢����״ֵ̬������
uint8_t update_trays[8]= {'\0'};
uint8_t update_trays_packet_status[8]= {'\0'};
uint8_t update_trays_startup_status[8]= {'\0'};
uint8_t update_trays_restart_status[8]= {'\0'};
extern uint8_t flashinfo[30];
extern volatile uint16_t lost_packet[8];
extern void resource_collect(void);//���ٷַ�����
extern void repost_data_to_cf8051_immediately(void);//���ٷַ�����
extern void repost_update_packet(void);

volatile uint8_t HUB_output = 0;
volatile uint8_t HUB_output_alarm = 0;
volatile uint8_t kaiji_cmd = 0;

volatile uint8_t read_version = 0;

extern volatile uint8_t version_flag;
extern volatile uint8_t zd_resource_flag;
extern volatile uint8_t zd_tray_num;

extern volatile uint8_t v_zd_whitch;


volatile uint8_t updating_flags = 0;

extern volatile uint8_t save_order_s;

                            //    1    6    2    0    1    6    1    2    1    2  type
uint8_t soft_hard_1[13] = {0x01,0x09,0x02,0x00,0x01,0x07,0x00,0x05,0x01,0x01,0x06,0x00,0x00};
	 
//==================================================================================================
//             �ӿڰ�               ������ҵ��������Լ��߼�����(���¼�������)
//==================================================================================================

/**
  * @brief  ��ȡ������ҽ��˼�������(��Ԫ��)
  * @retval �� ���� ������Ҫ�������
{0x7e,0x00,0x0d,0x03,0x0f,0x06,0x01,0x02,0x03,0x04,0x05,0x06,0x01,0x00,0x5a}
  */
static void read_tray_info(void)
{
   uint8_t i = 0;

   TrayInfo[4]  = board.id;
   TrayInfo[5]  = 0x0;

   for(i = 0;i < 8;i++){
			if(TrayInfo[6 + i] != 0x00){
				TrayInfo[5]++;
			}
   }
   TrayInfo[15] = crc8(TrayInfo,0x0f); 

   add_a_new_node(TrayInfo,DATA_NORMAL,(TrayInfo[1] << 8|TrayInfo[2] + 2),DATA_RECV);		
      
}


/**
  * @brief   ��ȡ�˿���Ϣ:��ȡĳ���˿�|ĳ����|ĳ�����port��Ϣ
  * @retval  �� ���� ������Ҫ�������
  */
static void read_410_ports_info(void)
{
   uint8_t pTrayNum = 0;

	 if(uart1.recvBuff[5] == 0){
		return;
	 }
   pTrayNum = uart1.recvBuff[5] - 1; //ȷ��Ҫ��ȡ��һ����

	 if(uart1.recvBuff[18] == 0x02){//��ʾͨ����ȡ�˿���Ϣ����ָ���ɼ�
			v_zd_whitch = 3;
			zd_tray_num = uart1.recvBuff[5];

			flashinfo[pTrayNum] = 0xff;//�̺����
			DeviceInfo.Tary_Num[pTrayNum] = 0xff;//�̺����
			flashinfo[14 + pTrayNum] = 0xFF;//˵����12�˿ڵĵ�Ԫ�� ���������
		 
			ports.traylost[pTrayNum] = 0;
			ports.portstat[pTrayNum] = 0;
   }
	 
   //��ȡ����ĳһ���˿�
   memset(&uCom_send_dataBase[pTrayNum][0],0, uart1.recvBuff[1]<<8|uart1.recvBuff[2]+2);                  
   memcpy(&uCom_send_dataBase[pTrayNum][0],&uart1.recvBuff[0],uart1.recvBuff[1]<<8|uart1.recvBuff[2]+2);
   repost_data_to_cf8051_immediately();	 
	 

	 start_timer(&yijian_read_version_timer);	//������Ӳ���汾�ŵĶ�ʱ��������ָ���̲ɼ�
	 //SI32_UART_A_disable_rx(SI32_UART_1);
   usart2_rx_irq_enable(DISABLE); 
}

/**
  * @brief  ��ȡ�汾��Ϣ����ȡ ��ǰ�����Ӳ���汾��Ϣ|���浥Ԫ��|ĳһ����Ԫ�����Ӳ���汾��Ϣ
  * @retval �� ���� ������Ҫ�������
  *
  * type:����������
  * 0x06 ------ 6�ڼ�����
  * 0x08 ------ 8�ڼ�����
  */
static void read_version_info(void)
{
   uint8_t pTrayNum = 0;	
	 uint8_t i = 0;
                         //    1    6    2    0    1    6    1    2    1    2  type
   uint8_t soft_hard[13] = {V_1,V_2,Y_1,Y_2,Y_3,Y_4,M_1,M_2,D_1,D_2,T_1,0x00,0x00};
   
   if(uart1.recvBuff[4] != 0x0 && uart1.recvBuff[5] == 0x00){
      //�������Ӳ��
      version[0] = 0x7e;
      version[1] = 0x00;
      version[2] = 0x37;
      version[3] = 0x05;
      version[4] = soft_hard[10];
      version[5] = board.id;
      version[6] = 0x0;	
		 
      soft_hard[11] = flashinfo[10];//ʵ�ʵĿ���̺�
      memcpy(&version[7], &soft_hard[0],13);
		  memset(&version[20] , 0 , 11);
      memcpy(&version[31],&soft_hard[0],13);
      memset(&version[44] , 0 , 11);
		 
      version[55] = crc8(version,55);
      version[56] = 0x5A;
      add_a_new_node(version,DATA_NORMAL,(version[1] << 8|version[2] + 2),DATA_RECV);		
	
   }else if((uart1.recvBuff[5] != 0x00) && (uart1.recvBuff[5] != 0xFF)){
      //���̵���Ӳ��
      pTrayNum = uart1.recvBuff[5] - 1;
      memset(&uCom_send_dataBase[pTrayNum],0,COM_TX_SIZE);                  
      memcpy(&uCom_send_dataBase[pTrayNum][0],uart1.recvBuff,8);					
      repost_data_to_cf8051_immediately();		
   }else if(uart1.recvBuff[5] == 0xFF){//һ����ȡ�汾��
		  kaiji_cmd = 1;	//2017-06-21�ſ�����Դ�ɼ����� ���û�п���������£�
			for(i = 0; i < 8; i++){
				memset(&uCom_send_dataBase[i][0],0,COM_TX_SIZE);                  
				memcpy(&uCom_send_dataBase[i][0],uart1.recvBuff,8);		
				uCom_send_dataBase[i][5] = (i+1);//��
				uCom_send_dataBase[i][6]= crc8(&uCom_send_dataBase[i][0],0x06);
		 }		
		 memset(reSource.rxbuf,0,sizeof(reSource.rxbuf));
		 read_version = 1;
		 start_timer(&yijian_read_version_timer);	//������ȡ��Ӳ���汾�ų�ʱ
		 v_zd_whitch = 2;//��ʾһ����ȡ�汾��
		// SI32_UART_A_disable_rx(SI32_UART_1);
		 usart2_rx_irq_enable(DISABLE); 
	 }   
}

/**
  * @brief  �������
  * @retval �� ���� ������Ҫ�������
  */
static void software_update_cf8051(void)
{
   unsigned short datalen = 0;                         
   unsigned char Trayidex = 0,entires = 0,i = 0,j = 0;
   /*���±����������ڵ�Ԫ������*/
   unsigned short start_idex = 0,end_idex = 0;

   datalen = uart1.recvBuff[1] << 8 | uart1.recvBuff[2];

   switch(uart1.recvBuff[4]){
      case 0x01:
         memset(&cf8051update[7],0x0e,24);
         cf8051update[31] = 0x00;//���ݰ������0
         cf8051update[32] = 0x00;//���ݰ������0
         memset(update_trays,0,sizeof(update_trays));//�������������������ȫ�����Ϊ0������Ĭ��ȫ��������
         memset(update_trays_startup_status,0xe,sizeof(update_trays_startup_status));//0xe:����Ĭ��ȫ��������
         start_idex = 7;
         end_idex   = 9 + (uart1.recvBuff[6] - 1)*3;
         cf8051update[7] = board.id;
         for(i = start_idex; i < end_idex; i+=3){
            if(uart1.recvBuff[i] == board.id){
               Trayidex = uart1.recvBuff[i+1];
               update_trays[Trayidex-1] = 1;//��1��ʾ���������Ҫ������
               cf8051update[7+(Trayidex-1)*3] = board.id;
               cf8051update[8+(Trayidex-1)*3] = Trayidex;
               cf8051update[9+(Trayidex-1)*3] = 0x0f;
               memset(&uCom_send_dataBase[Trayidex - 1][0],0,540); 
               uCom_send_dataBase[Trayidex - 1][0] = 0x7e;
               uCom_send_dataBase[Trayidex - 1][1] = 0x00;
               uCom_send_dataBase[Trayidex - 1][2] = 0x0c;
               uCom_send_dataBase[Trayidex - 1][3] = 0x06;
               uCom_send_dataBase[Trayidex - 1][4] = uart1.recvBuff[4];
               uCom_send_dataBase[Trayidex - 1][5] = uart1.recvBuff[5];
               uCom_send_dataBase[Trayidex - 1][6] = 1;
               uCom_send_dataBase[Trayidex - 1][7] = board.id;
               uCom_send_dataBase[Trayidex - 1][8] = Trayidex;
               uCom_send_dataBase[Trayidex - 1][9] = 0x0;
               uCom_send_dataBase[Trayidex - 1][10]= 0x0;
               uCom_send_dataBase[Trayidex - 1][11]= uart1.recvBuff[datalen-1];
               uCom_send_dataBase[Trayidex - 1][12]= crc8(&uCom_send_dataBase[Trayidex - 1][0],0x0c);
               uCom_send_dataBase[Trayidex - 1][13]= 0x5a;
            }
         }
         repost_update_packet();
         reload_timer(&update_startup_timer,9000);	
         start_timer(&update_startup_timer);
      break;
      case 0x03:
         memset(&cf8051update[7],0x0e,24);
         cf8051update[31] = 0xff;//�������ط��������ݰ���� 0xff:����������Ԫ��
         cf8051update[32] = 0xff;//�������ط��������ݰ���� 0xff:����������Ԫ��
         memset(update_trays_restart_status,0xe,sizeof(update_trays_restart_status)); //��־�Ǹ�����Ҫ����������
         start_idex = 7;
         end_idex   = 9 + (uart1.recvBuff[6] - 1)*3;
         cf8051update[7] = board.id;		
         for(i = start_idex; i < end_idex; i+=3,j++){
            j = uart1.recvBuff[i+1];//ȡ����ǰ������̺ţ��ж��ǲ����ϴ������ĵ�Ԫ��
            if(update_trays_startup_status[j-1] == 0x0){
               //ֻ�иղ��������ģ����������ɹ��ĵ�Ԫ����ܷ����ݸ�����
               if(uart1.recvBuff[i] == board.id){
                  Trayidex = uart1.recvBuff[i+1];
                  cf8051update[7+(Trayidex-1)*3] = board.id;
                  cf8051update[8+(Trayidex-1)*3] = Trayidex;
                  cf8051update[9+(Trayidex-1)*3] = 0x0f;
                  memset(&uCom_send_dataBase[Trayidex - 1][0],0,540); 
                  uCom_send_dataBase[Trayidex - 1][0] = 0x7e;
                  uCom_send_dataBase[Trayidex - 1][1] = 0x00;
                  uCom_send_dataBase[Trayidex - 1][2] = 0x0c;
                  uCom_send_dataBase[Trayidex - 1][3] = 0x06;
                  uCom_send_dataBase[Trayidex - 1][4] = uart1.recvBuff[4];
                  uCom_send_dataBase[Trayidex - 1][5] = uart1.recvBuff[5];
                  uCom_send_dataBase[Trayidex - 1][6] = 1;
                  uCom_send_dataBase[Trayidex - 1][7] = board.id;
                  uCom_send_dataBase[Trayidex - 1][8] = Trayidex;
                  uCom_send_dataBase[Trayidex - 1][9] = 0x0;
                  uCom_send_dataBase[Trayidex - 1][10]= 0x0;
                  uCom_send_dataBase[Trayidex - 1][11]= uart1.recvBuff[datalen-1];
                  uCom_send_dataBase[Trayidex - 1][12]= crc8(&uCom_send_dataBase[Trayidex - 1][0],0x0c);
                  uCom_send_dataBase[Trayidex - 1][13]= 0x5a;
               }
            }
         }
      repost_update_packet();
      reload_timer(&update_restart_timer,10000);
      start_timer(&update_restart_timer);
      break;
      case 0x02:
         entires = uart1.recvBuff[6];
         start_idex = 7;
         end_idex = 9 + (entires-1)*3;	
         cf8051update[31] = uart1.recvBuff[datalen-514];//�Ȼ�ȡ���ط��������ݰ����
         cf8051update[32] = uart1.recvBuff[datalen-513];//�Ȼ�ȡ���ط��������ݰ����
         memset(update_trays_packet_status,0xe,sizeof(update_trays_packet_status));
         for(i = start_idex;i < end_idex;i+= 3,j++){
            j = uart1.recvBuff[i+1];//ȡ����ǰ������̺ţ��ж��ǲ����ϴ������ĵ�Ԫ��
            if(update_trays_startup_status[j-1] == 0x0){
               //ֻ�иղ��������ģ����������ɹ��ĵ�Ԫ����ܷ����ݸ�����
               if(uart1.recvBuff[i] == board.id){
                  Trayidex = uart1.recvBuff[i+1];	
                  cf8051update[7+(Trayidex-1)*3] = board.id;
                  cf8051update[8+(Trayidex-1)*3] = Trayidex;
                  cf8051update[9+(Trayidex-1)*3] = 0x01;//��ʾ��������������
                  memset(&uCom_send_dataBase[Trayidex - 1][0],0,540); 
                  uCom_send_dataBase[Trayidex - 1][0] = 0x7e;
                  uCom_send_dataBase[Trayidex - 1][1] = 0x02;
                  uCom_send_dataBase[Trayidex - 1][2] = 0x0c;
                  uCom_send_dataBase[Trayidex - 1][3] = 0x06;
                  uCom_send_dataBase[Trayidex - 1][4] = uart1.recvBuff[4];
                  uCom_send_dataBase[Trayidex - 1][5] = uart1.recvBuff[5];
                  uCom_send_dataBase[Trayidex - 1][6] = uart1.recvBuff[6];
                  uCom_send_dataBase[Trayidex - 1][7] = board.id;
                  uCom_send_dataBase[Trayidex - 1][8] = Trayidex;
                  uCom_send_dataBase[Trayidex - 1][9] = 0x0;
                  uCom_send_dataBase[Trayidex - 1][10]= uart1.recvBuff[datalen-514];
                  uCom_send_dataBase[Trayidex - 1][11]= uart1.recvBuff[datalen-513];
                  memcpy(&uCom_send_dataBase[Trayidex - 1][12],&uart1.recvBuff[datalen-512],512);
                  uCom_send_dataBase[Trayidex - 1][524] = crc8(&uCom_send_dataBase[Trayidex - 1][0],524);
                  uCom_send_dataBase[Trayidex - 1][525] = 0x5a;
               } 
            }else if(update_trays_startup_status[j] == 0x1){
               //��������||����ʧ�ܵ� ���ǲ���Ҫ�����Ƿ����ݰ�
                  Trayidex = uart1.recvBuff[i+1];
                  cf8051update[7+(Trayidex-1)*3] = board.id;
                  cf8051update[8+(Trayidex-1)*3] = Trayidex;
                  cf8051update[9+(Trayidex-1)*3] = 0x01;	
            }
         }
         repost_update_packet();
         reload_timer(&update_write_packets_timer,10000);
         start_timer(&update_write_packets_timer);	
      break;

   }

}

/**
  * @brief  д��EID���˿ڣ���ĳ���˿�д��һ��EID��Ϣ
  * @retval �� ���� ������Ҫ�������
  */
static uint8_t write_EID_to_410(void)
{
   uint8_t pTrayNum = 0;

   pTrayNum = uart1.recvBuff[5];
   memset(&uCom_send_dataBase[pTrayNum - 1],0,COM_TX_SIZE);
   memcpy(&uCom_send_dataBase[pTrayNum - 1][0],uart1.recvBuff,137);
   repost_data_to_cf8051_immediately();	
   return 0;
}

/**
  * @brief  LED�Ʋ���
  * @retval �� ���� ������Ҫ�������
  */
static uint8_t led_guide_test(void)
{
   uint8_t pTrayNum = 0,i = 0;
   uint8_t temp_lost[25] = {'\0'};
	
	 if(uart1.recvBuff[7] == 0x05){//˵����һ����ȡ�����ʵ�����

			temp_lost[0] = 0x7e;
			temp_lost[1] = 0x00;
			temp_lost[2] = 0x15;
			temp_lost[3] = 0xFA;
			temp_lost[4] = uart1.recvBuff[4];//���

			for(i = 0;i < 8;i++){
				temp_lost[5 + (2*i) + 0] = ((lost_packet[i] >> 8) & 0xff);
				temp_lost[5 + (2*i) + 1] = ((lost_packet[i] >> 0) & 0xff);
			}
			temp_lost[21] = crc8(temp_lost,21);
			temp_lost[22] = 0x5A;
			add_a_new_node(temp_lost,DATA_NORMAL,(temp_lost[1] << 8|temp_lost[2] + 2),DATA_RECV);		
			return 0;
	 }
		
   if(uart1.recvBuff[5] == 0xff){
		 kaiji_cmd = 1;	//2017-06-21�ſ�����Դ�ɼ����� ���û�п���������£�
      //����Ʋ���
      for(;pTrayNum < 8;pTrayNum++){
         memset(&uCom_send_dataBase[pTrayNum],0,COM_TX_SIZE);
         memcpy(&uCom_send_dataBase[pTrayNum][0],uart1.recvBuff,10);
      }		
   }else if(uart1.recvBuff[5] > 0x0 && uart1.recvBuff[5] < 0x09){
      //�̵Ĳ���(������ĳ���˿�||������ĳ������)
      pTrayNum = uart1.recvBuff[5];
      memset(&uCom_send_dataBase[pTrayNum - 1],0,COM_TX_SIZE);
      memcpy(&uCom_send_dataBase[pTrayNum - 1][0],uart1.recvBuff,10);
   }
   repost_data_to_cf8051_immediately();
   return 0;
}


/**
  * @brief  ����׷�ٻ�����׷�� 
  * @retval �� ���� ������Ҫ�������
  */
static void unControl_wirteEID_info(void)
{
   uint8_t pTrayNum = 0;

   if((uart1.recvBuff[4] == 0x05) || (uart1.recvBuff[4] == 0x06)){
      if(uart1.recvBuff[5] == board.id){
            pTrayNum = uart1.recvBuff[6];
            if(pTrayNum != 0x0){//�Ͻ��Կ��ǣ�ֻ�п�ŷ������̺Ų�Ϊ0������£����ǲ���ӿڰ巢������
               memset(&uCom_send_dataBase[pTrayNum - 1][0],0,COM_TX_SIZE); 
               memcpy(&uCom_send_dataBase[pTrayNum - 1][0],uart1.recvBuff,(uart1.recvBuff[1] << 8 |uart1.recvBuff[2] + 2));
            }else{
               return ;
            }
      }else if(uart1.recvBuff[9] == board.id){
            pTrayNum = uart1.recvBuff[10];
            if(pTrayNum != 0x0){//�Ͻ��Կ��ǣ�ֻ�п�ŷ������̺Ų�Ϊ0������£����ǲ���ӿڰ巢������
               memset(&uCom_send_dataBase[pTrayNum - 1][0],0,COM_TX_SIZE); 
               memcpy(&uCom_send_dataBase[pTrayNum - 1][0],uart1.recvBuff,(uart1.recvBuff[1] << 8 |uart1.recvBuff[2] + 2));
            }else{
               return ;
            }
      }
               
   }else{
      pTrayNum = uart1.recvBuff[6];
      if(pTrayNum != 0x0){//�Ͻ��Կ��ǣ�ֻ�п�ŷ������̺Ų�Ϊ0������£����ǲ���ӿڰ巢������
         memset(&uCom_send_dataBase[pTrayNum - 1][0],0,COM_TX_SIZE); 
         memcpy(&uCom_send_dataBase[pTrayNum - 1][0],uart1.recvBuff,(uart1.recvBuff[1] << 8 |uart1.recvBuff[2] + 2));
      }else{
         return ;
      }
   }

   repost_data_to_cf8051_immediately();
}

/**
  * @brief  ���������½��е�ָ��
  * @retval �� ���� ������Ҫ�������
  */
static void Patch_guide_action(void)
{
   uint8_t i = 0;
   uint8_t pTrayNum = 0;

   if(0x88 == uart1.recvBuff[3]){
      for(i = 0;i<2;i++){
         pTrayNum = uart1.recvBuff[5+(3*i)];
         memset(&uCom_send_dataBase[pTrayNum - 1],0,COM_TX_SIZE); 
         memcpy(&uCom_send_dataBase[pTrayNum - 1][0],uart1.recvBuff,(uart1.recvBuff[1] << 8 |uart1.recvBuff[2] + 2));
      }
   }else{
         pTrayNum = uart1.recvBuff[5];
         memset(&uCom_send_dataBase[pTrayNum - 1],0,COM_TX_SIZE); 
         memcpy(&uCom_send_dataBase[pTrayNum - 1][0],uart1.recvBuff,(uart1.recvBuff[1] << 8 |uart1.recvBuff[2] + 2));
   }
   repost_data_to_cf8051_immediately();

}

/**
  * @brief   
  * @retval �� ���� ������Ҫ�������
  */
static uint8_t Load_Data(void)
{
   uint8_t retval = 0;
   uint8_t pTrayNum = 0;
	 uint8_t load2flash[30] = {'\0'};
	 uint8_t load[11] = {'\0'};
	
   if(uart1.recvBuff[4] == 0x03)//��Ԫ����Ҫ���ص�����
   {
       pTrayNum = uart1.recvBuff[7];
         memset(&uCom_send_dataBase[pTrayNum - 1],0,COM_TX_SIZE); 
         memcpy(&uCom_send_dataBase[pTrayNum - 1][0],uart1.recvBuff,(uart1.recvBuff[1] << 8 |uart1.recvBuff[2] + 2));
   }
	 /* 2017-07-06 zhuchengzhi add */
	 else if(uart1.recvBuff[4] == 0x02){
			memcpy(&load2flash[0],&uart1.recvBuff[9],30);
			cpu_disable_irq(0);
			myFLASHCTRL0_run_erase_page_mode(FLASH_TEST_PAGE_ADDR);
			myFLASHCTRL0_run_write_flash_mode(FLASH_TEST_PAGE_ADDR,load2flash,sizeof(load2flash));
			ReadFlashData(FLASH_TEST_PAGE_ADDR_PTR,flashinfo,sizeof(flashinfo));
			cpu_enable_irq();
		 
		  //�������Ӳ��
      load[0] = 0x7e;
      load[1] = 0x00;
      load[2] = 0x09;
      load[3] = 0x13;
      load[4] = 0x02;
      load[5] = uart1.recvBuff[9]; //��
      load[6] = 0x0;							 //��
		  load[7] = 0x0;               //�˿�
      load[8] = 0x0;							 //�ɹ�ʧ��
      load[9] = crc8(load,9);
      load[10] = 0x5A;
      add_a_new_node(load,DATA_NORMAL,(load[1] << 8|load[2] + 2),DATA_RECV);		
	 }
	 
   return retval;
}



static uint8_t Back_Order_To_Source(void)
{
   uint8_t retval = 0;
   uint8_t pTrayNum = 0;
   uint8_t index_i = 0;

   for(index_i=0;index_i<uart1.recvBuff[6];index_i++)
   {
      if(uart1.recvBuff[8+(4 * index_i)] == board.id)
      {
         pTrayNum = uart1.recvBuff[9+(4 * index_i)];
         memset(&uCom_send_dataBase[pTrayNum - 1],0,COM_TX_SIZE);
         memcpy(&uCom_send_dataBase[pTrayNum - 1],uart1.recvBuff,(uart1.recvBuff[1] << 8 | uart1.recvBuff[2]));
      }			
   }
   return retval;
}

/**
  * @brief  ʩ�������������
  * @retval �� ���� ������Ҫ�������
  */
static void orders_operations(void)
{
   uint8_t sub_cmd = 0, tray_num = 0;
   uint16_t i = 0;
   /* �����ⲿ�ֱ���ֻ�����βɼ������� */	
   uint16_t datalen = 0,recoverlen = 0;
   uint16_t frameidex_start = 0,frameidex_end = 0;
   uint16_t  idex[8] = {'\0'};
   uint8_t  Trayidex = 0;
   uint8_t wirte2flash[10], tray_stat[15];   

   sub_cmd = uart1.recvBuff[4];
	 
	 if(sub_cmd != 0x0A){
		 save_order_s = sub_cmd; //zhuchengzhi 2017-06-12 
	 }
	 
   switch(sub_cmd)
   {
      case 0x01://��Դ�ɼ�	
      case 0x11://2�βɼ�	
			   kaiji_cmd = 1;	//2017-06-21�ſ�����Դ�ɼ����� ���û�п���������£�
			
         for(;i < 8;i++){
            memset(&uCom_send_dataBase[i][0],0,COM_TX_SIZE);                  
            memcpy(&uCom_send_dataBase[i][0],uart1.recvBuff,8);						
         }			
         reSource.idex = 0;	
         reSource.flag = 0;

				 memset(&ports.traylost[0],0,8);
				 memset(&ports.portstat[0],0,8);
				 
         reSource.reSourceNow = 1;						
         memset(reSource.tray_units, TRAY_OFFLINE, 10);         
         memset(reSource.rxbuf,0,sizeof(reSource.rxbuf));
         reSource.type = uart1.recvBuff[4];
         start_timer(&resource_timer);	//�����ɼ��ļ�ʱ����ɼ���ʱ1.5��
         memset(&DeviceInfo.Tary_Num[0],0xff,8);
//         SI32_UART_A_disable_rx(SI32_UART_1); 
usart2_rx_irq_enable(DISABLE); 		 
         break;	
      case 0xb1:
      case 0xb2:			
      case 0x02://odf���ڵ�������            
      case 0x03://����
      case 0x04://odf���ڵ��˲��	
      case 0x05://˫���½�
      case 0x06://˫�˲��	
      case 0x07://�����½�	
      case 0x08://�ܼ��½�һ��
      case 0x09://�ܼ���һ��
      case 0x10://�ܼ��������
      case 0x18:
      case 0x19:
      case 0x30:		
      case 0x0a://ȷ��
      case 0xA2://ȡ������
      case 0x8c://���410��Ԫ��Զ�
      case 0x8a://ֱ�ӻ���	
      case 0x9a://���ߵ�ȡ��
         if(((sub_cmd == 0x07) || (sub_cmd == 0xA2)||(sub_cmd == 0x10)||(sub_cmd == 0x30)||(sub_cmd == 0xb2)) 
            &&  ((uart1.recvBuff[5] == 0x07) || (uart1.recvBuff[5] == 0x10)|| (uart1.recvBuff[5] == 0x30)|| (uart1.recvBuff[5] == 0x11)|| (uart1.recvBuff[5] == 0x12))
         )
         {
            //�����½��еĵ�ƺ�ȡ������
            goto patch_07_or_A2;
         }else{
            if(uart1.recvBuff[6] == uart1.recvBuff[10]){//ͬ��
               if(uart1.recvBuff[7] == uart1.recvBuff[11]){//ͬ�̣�����һ������
                  tray_num = uart1.recvBuff[7] - 1;
                  if(tray_num < 8){
                     memset(&uCom_send_dataBase[tray_num],0,COM_TX_SIZE);                  
                     memcpy(&uCom_send_dataBase[tray_num][0],uart1.recvBuff,uart1.recvBuff[2] + 2);                   
                  }
               }else{//��ͬ��,����2������	
                  tray_num = uart1.recvBuff[7] - 1;
                  if(tray_num < 8){
                     memset(&uCom_send_dataBase[tray_num],0,COM_TX_SIZE);                  
                     memcpy(&uCom_send_dataBase[tray_num][0],uart1.recvBuff,uart1.recvBuff[2] + 2);                   
                  }
                  
                  tray_num = uart1.recvBuff[11] - 1;
                  if(tray_num < 8){
                     memset(&uCom_send_dataBase[tray_num],0,COM_TX_SIZE);                  
                     memcpy(&uCom_send_dataBase[tray_num][0],uart1.recvBuff,uart1.recvBuff[2] + 2);                   
                  } 	
               }
            }else{//��ͬ��,ֻҪ����һ������
                  tray_num = uart1.recvBuff[7] - 1;
                  if(tray_num < 8){
                     memset(&uCom_send_dataBase[tray_num],0,COM_TX_SIZE);                  
                     memcpy(&uCom_send_dataBase[tray_num][0],uart1.recvBuff,uart1.recvBuff[2] + 2);                   
                  } 
            }
         }
         break;
      case 0x0c://���βɼ��ĸ�������
         datalen = uart1.recvBuff[1] << 8 | uart1.recvBuff[2]; //��������ֽ��ܳ���(������crc��0x5aλ)
         frameidex_start = 5;//��λ��ʼ�˿����±�5�Ŵ�
         frameidex_end = datalen - 1;//��ֹ���±��ܳ���(������crc��0x5aλ)		
      
         for(i = 0;i < 8;i++){
            idex[i] = 5;
         }
         
         for(i = frameidex_start; i < frameidex_end; i +=4){
            Trayidex = uart1.recvBuff[i + 1] - 1;
            memcpy(&uCom_send_dataBase[Trayidex][idex[Trayidex]],&uart1.recvBuff[i],4);
            idex[Trayidex] += 4;
         }
         
         for(i = 0; i < 8;i++){
            if(idex[i] != 5){
               recoverlen = idex[i];
               uCom_send_dataBase[i][0] = 0x7e;
               uCom_send_dataBase[i][1] = (((recoverlen) >> 8) & 0xff);
               uCom_send_dataBase[i][2] = ((recoverlen) & 0xff);
               uCom_send_dataBase[i][3] = 0x0d;
               uCom_send_dataBase[i][4] = 0x0c;
               uCom_send_dataBase[i][recoverlen] = crc8(&uCom_send_dataBase[i][0],recoverlen);
               uCom_send_dataBase[i][recoverlen+1] = 0x5a;
            }
         }
         
         //�������̣�ԭ�������ڣ�����Դ�ɼ����������ݣ�˵�����������̣�ͬʱ�������ýӿڰ帲�ǵ�������Ҫ��¼��flash��ȥ
         //����������ԭ���ڣ����ڲ����ˣ���Ҳ�ô��м��޳�ô��������
         for(i = 0; i < 8; i++){
            //�ϴβɼ�ʱ���˴���û�н��̵�
            if(DeviceInfo.Tary_Num[i] == 0xff){
               if(reSource.tray_units[i] == TRAY_ONLINE){
                  //���һ������������¼��ɹ���"�澯"��Ϣ�����أ���Ϊ֮ǰ���ϱ��˷Ƿ������̵�"�澯"
                  alarm.data[alarm.idex++] = board.id;
                  alarm.data[alarm.idex++] = i + 1;
                  alarm.data[alarm.idex++] = 0x00;
                  alarm.data[alarm.idex++] = 0x0C;
                  alarm.data[0] = 0x7e; 
                  alarm.data[3] = 0x09; 
                  alarm.flag = 1;
                  if(alarm.idex > 292)alarm.idex = 4; 
                  ports.portstat[i] = BACK_TO_NORMAL;
//                  ports.tray[i] = 0;
                  
                  DeviceInfo.Tary_Num[i] = i + 1;                  
               }
            
            }else if(DeviceInfo.Tary_Num[i] == (i + 1)){
            //�ϴβɼ�ʱ���˴��ǽ����̵�
               if(reSource.tray_units[i] == TRAY_OFFLINE){
                  //ɾ������
                  DeviceInfo.Tary_Num[i] = 0xff;
               }
            
            }
            
         }
         
         memset(wirte2flash,0xff,sizeof(wirte2flash));
         memcpy(wirte2flash,&DeviceInfo.Tary_Num[0],8);
         cpu_disable_irq(0);
         wirte2flash[6] = 0x1;//�Ѿ�����Դ�ɼ����˵�
         myFLASHCTRL0_run_erase_page_mode(FLASH_TEST_PAGE_ADDR);
         myFLASHCTRL0_run_write_flash_mode(FLASH_TEST_PAGE_ADDR,wirte2flash,sizeof(wirte2flash));
         cpu_enable_irq();
         
         memset(tray_stat, 0x0, 15);
         tray_stat[0] = 0x7e;
         tray_stat[1] = 0x00;
         tray_stat[2] = 0x0b;
         tray_stat[3] = 0x33;
         memcpy(&tray_stat[4], wirte2flash, 7);
         tray_stat[11] = crc8(tray_stat, 11);
         tray_stat[12] = 0x5a;   
         add_a_new_node(tray_stat,DATA_NORMAL,13,DATA_RECV);
      break;
      case 0xa4:
         Back_Order_To_Source();
      break;
      
      default:break;
    }

   if(sub_cmd == 0x01 || sub_cmd == 0x11){
      return;
   }else{
      repost_data_to_cf8051_immediately();	 
      return; 
   }
    
patch_07_or_A2:
   //���ٵ���/��� �������ּ������ÿһ�����ϵ����ݺ��·�
   datalen = uart1.recvBuff[1] << 8 | uart1.recvBuff[2]; //��������ֽ��ܳ���(������crc��0x5aλ)
   frameidex_start = 6;//��λ��ʼ�˿����±�6�Ŵ�
   frameidex_end = datalen - 1;//��ֹ���±��ܳ���(������crc��0x5aλ)		

   for(i = 0;i < 6;i++){
      idex[i] = 6;
   }
   
   for(i = frameidex_start; i < frameidex_end; i +=4){
      Trayidex = uart1.recvBuff[i + 1] - 1;
      memcpy(&uCom_send_dataBase[Trayidex][idex[Trayidex]],&uart1.recvBuff[i],4);
      idex[Trayidex] += 4;
   }
   
   for(i = 0; i < 8;i++){
      if(idex[i] != 6){
         recoverlen = idex[i];
         uCom_send_dataBase[i][0] = 0x7e;
         uCom_send_dataBase[i][1] = (((recoverlen) >> 8) & 0xff);
         uCom_send_dataBase[i][2] = ((recoverlen) & 0xff);
         uCom_send_dataBase[i][3] = 0x0d;
         uCom_send_dataBase[i][4] = uart1.recvBuff[4];
         uCom_send_dataBase[i][5] = uart1.recvBuff[5]; 
         uCom_send_dataBase[i][recoverlen] = crc8(&uCom_send_dataBase[i][0],recoverlen);
         uCom_send_dataBase[i][recoverlen+1] = 0x5a;
      }
   }
   repost_data_to_cf8051_immediately();
   return ;
}

/**
  * @brief  �����ӿڰ壬�˴�������BIOS��ȥ
  * @retval 
  */
static void software_update_sim3u1xx(void)
{
//   unsigned char flashinfo[30];

//   memset(flashinfo,0xff,sizeof(flashinfo));
   ReadFlashData(FLASH_TEST_PAGE_ADDR_PTR,flashinfo,sizeof(flashinfo));
   
   flashinfo[11] = 0x78;
   flashinfo[12] = 0x78;
   flashinfo[13] = 0x78;//��ʾ�Ҵ�app����bios���Ҫ��bios�����и�����һ�������ɹ��ı�־λ
   
   myFLASHCTRL0_run_erase_page_mode(FLASH_TEST_PAGE_ADDR);//0x7C00
   
   myFLASHCTRL0_run_write_flash_mode(FLASH_TEST_PAGE_ADDR,flashinfo,30);
   
   reset_mcu_enter_default_mode();
   
//   SI32_RSTSRC_A_generate_software_reset(SI32_RSTSRC_0);  //ǿ�������λ  // ��ת���û�����
}


/**
  * @brief  ֻ���������Դ�ɼ�������
  * @retval �� ���� ������Ҫ�������
  * @description: �ӿڰ巵�ظ�s3c2416x���ذ�����ݸ�ʽ���£�һ����10�ֽ�+(8+32*12)*6+2 = 2364���ֽ�
  -------------------------------------------------------------------------------------------------------------------------------------
  ����֡ͷ(1�ֽ�)+���ݸ�λ(1�ֽ�)+���ݵ�λ��1�ֽڣ�+��������(0x0d)+�ɼ�����(1���ֽ�)+����(1���ֽ�)+�����Ӳ���汾��(4�ֽ�)+
  -------------------------------------------------------------------------------------------------------------------------------------
  { 
      ������Ӳ���汾��(4���ֽ�)+�����������(1���ֽ�)+���������̺�(1���ֽ�)+�����ϼ����˿�1(1���ֽ�)+�����ϼ����˿�12(1���ֽ�)+
      32*12(һ�����̵�EID��Ϣ����) 
  } X 6��
  -------------------------------------------------------------------------------------------------------------------------------------
  �������crc(1���ֽ�)+֡����β0x5a(1���ֽ�)	
  -------------------------------------------------------------------------------------------------------------------------------------
  */
static void fulling_resource_trayNums_data(void)
{
	
   unsigned short didex=6, EID_infolen = 35;
   unsigned short datalen = 0x0;	
   unsigned char  port_id = 0;//,tray_id = 0; 
	 unsigned char i =1,j =1;
	
   /*ÿ�����ֵĺ��壺
         32	: EID_infolen ÿ��EID��ǩ��Ϣ�����ݳ���
         8	����Դ�ɼ�ʱ��ÿ�����̻᷵�س���EID��ǩ��Ϣ�⣬�����������������Ŀ�ţ��̺ţ���������ʼ�˿ڣ�0x1,0xC�����̵���Ӳ���汾��(4���ֽ�)
         12	��ÿ��������12���˿� 
         6	: ÿ����6������ 
         14	�������ֽڳ����ܺͣ���������֡ͷ��֡β�����ȸߵ�λ�����ţ��Լ���Ӳ���汾�ŵ�
   */
   //datalen = (EID_infolen*12)*6+6;

	 for(i=1;i<=8;i++){
	 	datalen += EID_infolen * back_trays_types(i);
	 }
	 datalen += 7;//6 + 1 ǰ��6���ֽں����һ���Լ��ֶ���ӵĿ�����0x06--6�� 0x08--8�ڼ�����
	 
	 if(datalen > 4800){//��ֹ�������
		reSource.idex = 0;
		reSource.flag = 0;
		DeviceInfo.resource = 0;
		return;
	 }
	 
	 
   reSource.rxbuf[0] = 0x7e;
   reSource.rxbuf[1] = ( datalen >> 8 ) & 0xff;
   reSource.rxbuf[2] = ( datalen & 0xff);
   reSource.rxbuf[3] = 0x0d;
   reSource.rxbuf[4] = reSource.type;	
   reSource.rxbuf[5] = board.id;

   /* ������ֻ�Ǽ򵥵ĸ�ֵ��ֻ���û�вɼ��������̻��߸����̴���ȱ������²���Ч*/
   //tray_id = 1;
   port_id = 1;
	 
	for(i=1;i<=8;i++){
		if(back_trays_types(i) == 0x0c){    //˵��������һ��12�˿ڵ���
			for(j=1;j<=12;j++){
				reSource.rxbuf[didex]   = board.id;
				reSource.rxbuf[didex+1] = i;//tray_id
				reSource.rxbuf[didex+2] = port_id;
				didex += 35;
				
				port_id++;
				if(port_id == 13){
					port_id = 1;     
				}
			}
		}else if(back_trays_types(i) == 0x11){//˵��������һ��17�˿ڵ���
			for(j=1;j<=17;j++){
				reSource.rxbuf[didex]   = board.id;
				reSource.rxbuf[didex+1] = i;//tray_id
				reSource.rxbuf[didex+2] = port_id;
				didex += 35;
				
				port_id++;
				if(port_id == 18){
					port_id = 1;     
				}
			}
		}else{
			//do nothing
		}
	}
	 reSource.rxbuf[datalen - 1] = 0x08;//�Լ��ֶ���ӵĿ�����0x06--6�� 0x08--8�ڼ�����
   reSource.rxbuf[datalen] = crc8(&reSource.rxbuf[0],datalen);  
   reSource.rxbuf[datalen+1] = 0x5a;
   reSource.reSourceNow = 0x0;		
}





static void fulling_version_trayNums_data(void)
{
	unsigned short datalen = 0x0;	

	                       //    1   6   2   0   1   6   1   2   1   2 type
	uint8_t soft_hard_1[13] = {V_1,V_2,Y_1,Y_2,Y_3,Y_4,M_1,M_2,D_1,D_2,T_1,0x00,0x00};
	
	soft_hard_1[11] = flashinfo[10];
	
	datalen = 439; //8���̵İ汾���ܳ���
	reSource.rxbuf[0] = 0x7e;
	reSource.rxbuf[1] = ( datalen >> 8 ) & 0xff;
	reSource.rxbuf[2] = ( datalen & 0xff);
	reSource.rxbuf[3] = 0xAA;
	reSource.rxbuf[4] = board.id;//frame
	reSource.rxbuf[5] = 0xff;    //tray
	reSource.rxbuf[6] = 0x08;    //�������HUB����ܽ���8����
	memcpy(&reSource.rxbuf[7], &soft_hard_1[0],13);
	memcpy(&reSource.rxbuf[31],&soft_hard_1[0],13);
	
	reSource.rxbuf[439] = crc8(reSource.rxbuf, 439);
	reSource.rxbuf[440] = 0x5a;
}

/*�����е�����  ������Ҫ�޸�*/
static void fulling_zd_tray_resource_trayNums_data(void)
{
	unsigned short datalen = 0x0;	
	
	datalen = 391; //8���̵İ汾���ܳ��� һ���̵����ݳ���
	reSource.rxbuf[0] = 0x7e;
	reSource.rxbuf[1] = ( datalen >> 8 ) & 0xff;
	reSource.rxbuf[2] = ( datalen & 0xff);
	reSource.rxbuf[3] = 0xFC;
	reSource.rxbuf[4] = board.id;       //frame
	reSource.rxbuf[5] = zd_tray_num;    //tray

	reSource.rxbuf[391] = crc8(reSource.rxbuf, 391);
	reSource.rxbuf[392] = 0x5a;
}
//==================================================================================================
//             �ӿڰ�                  ��֤�����������ؾ���Ĳ�������(����3������)
//==================================================================================================


uint8_t  Oops_verify_data_volume_frameid(uint8_t *dat)
{
   volatile uint8_t key = *(dat + 3), ret = 0;//uint8_t key = dat[3], ret = 0;

   switch(key){
      
      case 0x03:
      case 0x07:
      case 0x88://����ָ��,��Ч��EID
      case 0x89://����ָ��-��Ч��EID
         if(*(dat + 4) == board.id) ret = 1;
      break; 

			case 0x10:
      case 0x20:
		  case 0x04:
			case 0x05:
         if((board.id == 0xff) || (board.id != *(dat + 4))){
						if(*(dat + 4) > 0x11){
							ret = 0;
						}else{
							board.id = *(dat + 4);//dat[4];
							ret = 1;
						}

         }else{  
            if(*(dat + 4) == board.id)//if(dat[4] == board.id)
               ret = 1;   
         }

      break;
         
      case 0x06:
//         if(dat[7] == board.id)
            ret = 1;
      break;
      
      case 0x13:
         if(*(dat + 6) == board.id)//if(dat[6] == board.id) 
            ret = 1;
      break;
      
      case 0x0d://��������  
         switch(*(dat + 4)){//switch(dat[4]){
            case 0x01://һ�βɼ�
            case 0x11://���βɼ�
                board.id = *(dat + 5);//dat[5];//zhuchengzhi 2017-01-20  �������ز���Ҫ���þͿ���ֱ����Դ�ɼ���
                  ret = 1;
            break;

            case 0x0c://���βɼ�����
               if(*(dat + 5) == board.id)//if(dat[5] == board.id) 
                  ret = 1;
            break;
            case 0xb1:
            case 0xb2:
            case 0x02://�����½�
            case 0x03://���˸���
            case 0x04://���˲��	
            case 0x06://˫�˲��	
            case 0x07://�����½�	
            case 0x08://�ܼ�A���½�һ��
            case 0x09://�ܼ�A�˲��һ��
            case 0x10://�ܼ�A���������
            case 0x18://�ܼ�Z���½�һ��
            case 0x19://�ܼ�Z�˲��һ��
            case 0x30://�ܼ�Z���������
            case 0xA2://����ȡ��	
            case 0x9a://˫�˻��ߵ�ȡ��
            case 0x8a://����
            case 0x05://˫���½�
            case 0x8c://�������
            case 0x0a://ȷ��ָ�� --- �� �������½��������ǲ���������Ǹ���
               if(*(dat + 6) == board.id )ret = 1;//if(dat[6] == board.id )ret = 1;
            break;
            case 0xa4://ȷ��ָ�� --- �� �������½��������ǲ���������Ǹ���
               if(*(dat + 8) == board.id )ret = 1;//if(dat[8] == board.id )ret = 1;
            break;
            default:break;
         }
      break;
         
      case 0x0f:
         if((*(dat + 4) == 0x05) || (*(dat + 4) == 0x06)){
            if((*(dat + 5) == board.id) || (*(dat + 9) == board.id)) ret = 1;
         }else{
            if(*(dat + 5) == board.id) ret = 1;
         }
//         if((dat[4] == 0x05) || (dat[4] == 0x06)){
//            if((dat[5] == board.id) || (dat[9] == board.id)) ret = 1;
//         }else{
//            if(dat[5] == board.id) ret = 1;
//         }
      break;
            
      case 0xa6:
      case 0xf6:
         //if(dat[6] == board.id)ret = 1;
			   ret = 1;
      break;

      }

   return ret;
}


/**
  * @brief  ר��������������յ�������
  * @retval �� ���� ������Ҫ�������
  */
static void mainboard_A6_or_F6_msgs_handle(void)
{
   uint8_t src_cmd = 0, chain_cmd = 0, sub_cmd = 0;
   unsigned char wirte2flash[30];

   src_cmd   = uart1.recvBuff[3];
   chain_cmd = uart1.recvBuff[4];
   sub_cmd   = uart1.recvBuff[5];

   switch(src_cmd){
   case 0xa6:
      switch(chain_cmd){
      case 0x33://�����õ�
            remove_a_old_node();         
      break;
      case 0xBB://��������ɹ�
//					kaiji_cmd = 1;
//					remove_a_old_node();         
      break;
			case 0xAA:
			  memset(reSource.rxbuf,0,sizeof(reSource.rxbuf));  
				version_flag = 0;
			  read_version = 0;
			break;
      case 0x0d:
         switch(sub_cmd){
         case 0x01://��һ����Ϣ¼��,ֱ��д�뵽flash,���õȴ����ص�д��flash����
            memset(reSource.rxbuf,0,sizeof(reSource.rxbuf));  
            reSource.idex = 0;
            reSource.flag = 0;
            DeviceInfo.resource = 1;
            memset(wirte2flash,0xff,sizeof(wirte2flash));
            memcpy(wirte2flash,&DeviceInfo.Tary_Num[0],8);
				    memcpy(&wirte2flash[14],&flashinfo[14],8);
            wirte2flash[9] = DeviceInfo.resource;//flash[9] ����ÿ��Ƿ񱻲ɼ���
            wirte2flash[10] = board.id;//flash[10] ����ɼ���Ŀ��
            cpu_disable_irq(0);
            myFLASHCTRL0_run_erase_page_mode(FLASH_TEST_PAGE_ADDR);
            myFLASHCTRL0_run_write_flash_mode(FLASH_TEST_PAGE_ADDR,wirte2flash,sizeof(wirte2flash));
				    ReadFlashData(FLASH_TEST_PAGE_ADDR_PTR,flashinfo,sizeof(flashinfo));
				 
						
            cpu_enable_irq();
         break;
         case 0x11://�ڶ���֮��Ĳɼ�����ԴѲ�죬����ֱ��д�뵽flash�����ǵȴ�����Ҫ��д��ʱ��д��.
            memset(reSource.rxbuf,0,sizeof(reSource.rxbuf));  
            reSource.idex = 0;
            reSource.flag = 0;
         break;
         case 0x02:
         case 0x03:
         case 0x04:
         case 0x05:
         case 0x06:
         case 0x07:
         case 0x08://�ܼ�A���½�һ��
         case 0x09://�ܼ�A�˲��һ��
         case 0x10://�ܼ�A���������
         case 0x18://�ܼ�Z���½�һ��
         case 0x19://�ܼ�Z�˲��һ��
         case 0x30://�ܼ�Z���������
         case 0xb1://���·�������½�
         case 0xb2:
         case 0x0a:
         case 0x8c:
         case 0x8a:
         case 0x88:
         case 0x89:
         case 0xEE:
				 case 0xA2:
               remove_a_old_node();
         break;
         default:break;
         }
      break;//end of 0x0d
      case 0x09://�����Ĵ����е�΢����������ϱ�ʱ��¼�ķ��������±꣬
                //����ǰʵ�ʵ��±����˱Ƚϣ��п������ϱ����ݺ��ڵȴ����ػظ�
                //0xa6/0xf6��ʱ�򣬽ӿڰ��ֻ�ȡ���µı������ݣ��������˺��棬�����б����б�Ҫ�ġ�
         if(alarm.idex == alarm.pendingBytes){
            alarm.flag = 0;
            alarm.idex = 4;
            alarm.pendingBytes = alarm.idex;
            memset(alarm.data,0,sizeof(alarm.data));					
         }else if(alarm.idex > alarm.pendingBytes){
            memset(&alarm.data[4],0,alarm.pendingBytes - 4);
            memcpy(&alarm.data[4],&alarm.data[alarm.pendingBytes],alarm.idex - alarm.pendingBytes);
            alarm.idex = alarm.idex - alarm.pendingBytes + 4;
            alarm.pendingBytes = 4;
         }
      break;//end of 0x09
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x0f:
      case 0x13:
      case 0x07:
			case 0xFA:
         remove_a_old_node();
      break;
			case 0xFC://ָ���̲ɼ�
				  zd_resource_flag = 0;
					memset(reSource.rxbuf,0,sizeof(reSource.rxbuf)); 
			    memcpy(&wirte2flash[0],&flashinfo[0],30);
					cpu_disable_irq(0);
					myFLASHCTRL0_run_erase_page_mode(FLASH_TEST_PAGE_ADDR);
					myFLASHCTRL0_run_write_flash_mode(FLASH_TEST_PAGE_ADDR,wirte2flash,sizeof(wirte2flash));
					ReadFlashData(FLASH_TEST_PAGE_ADDR_PTR,flashinfo,sizeof(flashinfo));
					cpu_enable_irq();
			break;
			
      case 0x10:
				 kaiji_cmd = 1;//��������ɹ�
         remove_a_old_node();
      break;
      case 0x06:
         remove_a_old_node();
         if(flashinfo[13] == 0x78){
            memset(flashinfo, 0xff, sizeof(flashinfo));
            ReadFlashData(FLASH_TEST_PAGE_ADDR_PTR, flashinfo, sizeof(flashinfo));    
            flashinfo[11] = 0xff;
            flashinfo[12] = 0xff;
            flashinfo[13] = 0xff;    
            myFLASHCTRL0_run_erase_page_mode(FLASH_TEST_PAGE_ADDR);//0x7C00        
            myFLASHCTRL0_run_write_flash_mode(FLASH_TEST_PAGE_ADDR, flashinfo, sizeof(flashinfo));         
         }
      break;   
      default:break;
      }
   break;//end of 0xa6 
   case 0xf6:
      
   break;//end of 0xf6

   default:break;
   }
}


/**
  * @brief  �������ط�����������,�����ɷ�����
  * @retval �� ���� ������Ҫ�������

      case 0x04://��ȡ�˿���Ϣ
         read_ports_info();
				 read_i++;
      break; 
  */
static uint8_t part3_repost_s3c44bx0_data_to_cf8051(void)
{
   uint8_t retval = 0; //����ҵ������ͣ�����Ҫ���ǻظ�0xA6���
   uint8_t src_cmd = uart1.recvBuff[3];

   switch(src_cmd)
   {
      case 0x03://����Ҫ��ȡ����Ϣ
         read_tray_info();					
      break;
			case 0x04:
		   read_410_ports_info();
			break;
      case 0x05://��ȡ��Ӳ���汾��Ϣ
         read_version_info();	
      break;
      case 0x06://�������
         if(uart1.recvBuff[5] == 0x02){
            software_update_sim3u1xx();
         }else{
            software_update_cf8051();
         }
      break;    
      case 0x07://д����ӱ�ǩ��Ϣ���豸
         write_EID_to_410();
      break;     
      case 0x0d://ʩ����������
         orders_operations();
      break;
      case 0x10://LEDָʾ�Ʋ���
         led_guide_test();
      break;
      case 0x0f://�ܿ�д��EID��Ϣ
         unControl_wirteEID_info();
      break;
      case 0x88://����ָ��,��Ч��EID
      case 0x89://����ָ��-��Ч��EID
         Patch_guide_action();
      break;
      case 0x13:
         Load_Data();
      break;		
      default:break;
   }

   return retval;
}

static void sim3u146_updata_ok_to_s3c2416(void)
{
	   unsigned char dfu_reply[18];
		//�豸��ǰ��������״̬,�����մ�BIOS����APP��Ӧ��Ҫ��������һ�������ɹ���״̬��Ϣ
		dfu_reply[0] = 0x7e;
		dfu_reply[1] = 0x00;
		dfu_reply[2] = 0x0e;
		dfu_reply[3] = 0x06;
		dfu_reply[4] = 0x03;
		dfu_reply[5] = 0x02;//update type
		dfu_reply[6] = 0x01;//update nums
		dfu_reply[7] = board.id;
		dfu_reply[8] = 0x00;//tray id,
		dfu_reply[9] = 0x00;//excute result
		dfu_reply[10] = 0x00;
		dfu_reply[11] = 0x00;
		dfu_reply[12] = 0x00;
		dfu_reply[13] = 0x00;  
		dfu_reply[14] = crc8(dfu_reply, 14);
		dfu_reply[15] = 0x5a;
	  add_a_new_node(dfu_reply, DATA_NORMAL, 16, DATA_RECV); 
		return;
}

#if 1
static void sim3u146_kaiji_cmd_to_s3c2416(void)
{
	unsigned char dfu_reply[18];
	//�豸�Ŀ�������ֻ���豸�����ɹ��˲���������������
	dfu_reply[0] = 0x7e;
	dfu_reply[1] = 0x00;
	dfu_reply[2] = 0x0e;
	dfu_reply[3] = 0x10;    //��������
	dfu_reply[4] = 0x02;    //���ͣ�01:��Ԫ�� 02:HUB 03:����
	dfu_reply[5] = board.id;//���
	dfu_reply[6] = 0x00;    //�̺�
	dfu_reply[7] = 0x00;    //�������� 01��APP����  02������������� 03��BIOS���������������(��������BIOS��)
	dfu_reply[8] = 0x00;    //���ͣ�01:���ط������������  02:HUB�Լ�����  
	dfu_reply[9] = 0x00;    
	dfu_reply[10] = 0x00;
	dfu_reply[11] = 0x00;
	dfu_reply[12] = 0x00;
	dfu_reply[13] = 0x00;  //excute result
	dfu_reply[14] = crc8(dfu_reply, 14);
	dfu_reply[15] = 0x5a;
	add_a_new_node(dfu_reply, DATA_NORMAL, 16, DATA_RECV); 
	return;
}
#endif


/**
  * @brief  �����ط�����ǰ���е�����״̬
  * @retval �� ���� ������Ҫ�������
  */
static void part3_feedback_msgs_to_s3c44b0x(void)
{
   uint16_t datalen = 0;
	 static volatile unsigned char kaiji_first = 0;
	
	 if(kaiji_first == 0){
		  kaiji_first = 1;
		  kaiji_cmd = 0;
			sim3u146_kaiji_cmd_to_s3c2416();
	 }
	
	 if(flashinfo[13] == 0x78){
			sim3u146_updata_ok_to_s3c2416();
	 }
	
   if(reSource.flag){                                    //��Դ�ɼ�||��ԴѲ�� ������ȼ��ϴ�		
      fulling_resource_trayNums_data();
      myUART1_send_multi_bytes(&reSource.rxbuf[0], reSource.rxbuf[1]<<8|reSource.rxbuf[2]+2); 		
   }else if(alarm.flag){ //�������ڶ����ȼ����أ��ղ������ص�ȷ��0xa6������� 
      datalen = alarm.idex;
      alarm.data[1] = (datalen >> 8) & 0xff; 
      alarm.data[2] =  datalen & 0xff;
      alarm.data[datalen] = crc8(alarm.data,datalen);
      datalen += 1;
      alarm.data[datalen] = 0x5a;
      myUART1_send_multi_bytes(alarm.data,datalen+1);
    alarm.pendingBytes = alarm.idex;
   }else if(EMPTY != Is_this_EmptyList()){               //���������񹤵����ݣ��������ȼ����أ����ݴ���pNewHeadΪ��ͷ��������				
      OS_NEWTASKLIST* p = NULL;
      p = pNewHead->next;
      myUART1_send_multi_bytes(p->data,p->dataLen);
      p->NodeStat = DATA_SEND;
   }else if (version_flag == 1){//һ����ȡ�汾��
      fulling_version_trayNums_data();
      myUART1_send_multi_bytes(&reSource.rxbuf[0], reSource.rxbuf[1]<<8|reSource.rxbuf[2]+2); 	
	 }else if (zd_resource_flag == 1){//ָ���̲ɼ�
		 fulling_zd_tray_resource_trayNums_data();
		 myUART1_send_multi_bytes(&reSource.rxbuf[0], reSource.rxbuf[1]<<8|reSource.rxbuf[2]+2); 	
	 }else{
		 if(flashinfo[10] != uart1.recvBuff[4]){//2017-04-20 �����ж�HUB�Լ��Ƿ��ڷǷ�������
				if(HUB_output < TRAY_TIMES_3){
					HUB_output++;
					if(HUB_output == TRAY_TIMES_3){
						HUB_output = TRAY_TIMES_3;
						HUB_output_alarm = 1;
						memset(&ports.traylost[0], 0 ,sizeof(ports.traylost));
					}
				}
			}else{
				HUB_output = 0;
				HUB_output_alarm = 0;
			}

      frame_run_ok[4] = board.id;
		 	frame_run_ok[7] = flashinfo[10];
      frame_run_ok[9] = crc8(frame_run_ok,9);
      myUART1_send_multi_bytes(frame_run_ok,11); //��ͼ�������ʲô���ݶ�û�еģ��򷵻ؽӿڰ���������ָ��	           
   }

}


/**
  * @brief  s3c44b0x�����ݴ������
            ��Ҫ�����ط����������������ȼ���ŶԲ��ԣ�Ȼ�����CRC������Ҫ���˲�����������
  * @retval �� ���� ������Ҫ�������
  */
// 2017-04-20 �������ж�CRC ������޸�
uint8_t mainboard_service_routine(void)
{
   uint8_t is_this_frame = 0;	
   uint8_t is_crc_right = 0,main_cmd = 0x0;
	
	is_crc_right = app_calccrc8(&uart1.recvBuff[0], uart1.count - 2);
	if(RIGHT == is_crc_right){
		/* ���CRC�ԵĻ���ȡ�����*/
		is_this_frame = Oops_verify_data_volume_frameid(&uart1.recvBuff[0]);
		if(WRONG == is_this_frame){
			memset(&uart1.recvBuff[0],0, sizeof(uart1.recvBuff));
			uart1.count = 0x0;
			uart1.recvFlag = 0;
			return 0;
		}
		
		//step2.�����������ݣ���������������ж��봦��
		main_cmd = uart1.recvBuff[3];
		if(0x20 == main_cmd){//step2.2  ��������Ч���жϳ����ط��͵���Ѳ������� �����֣�0x20
			part3_feedback_msgs_to_s3c44b0x();
		}else if(0xa6 == main_cmd || 0xf6 == main_cmd){//��������Ч���жϳ����ط��͵�������ȷ������� �����֣�0xa6/0xf6 
			mainboard_A6_or_F6_msgs_handle();
		}else{//��������Ч����������������ҵ���߼������������Ҫ�ӿڰ�ֱ�ӻظ�����ת������Ԫ��
			if(kaiji_cmd == 1){//ֻ�п����ɹ��˲��ܴ�������
				frame_is_right[4] = board.id;
				frame_is_right[5] = crc8(frame_is_right, 5); 
				myUART1_send_multi_bytes(frame_is_right,7);

				part3_repost_s3c44bx0_data_to_cf8051();
			}
		}
	}else{
	
	}
	
	uart1.count = 0x0;
	uart1.recvFlag = 0x0;   

	return 0;

}

//-----------------------------------eof------------------------------------------	

