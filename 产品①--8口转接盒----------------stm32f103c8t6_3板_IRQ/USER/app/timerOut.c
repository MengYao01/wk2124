/**
  ******************************************************************************
  * @file    timerOut.c
  * @author  MingLiang.Lu
  * @version V1.1.0
  * @date    09-October-2014
  * @brief   used for sim3U1xxx devices families.
  * @descriptions: feel free to let me know if you have any questions,you can 
									 send e-mail to  linux.lucian@gmail.com
  ******************************************************************************
  */


#include <string.h>
#include <stdlib.h>

#include "crc8.h"
#include "os_struct.h"
#include "soft_timer.h"
//#include "linkerlist.h"
#include "newList.h"
#include "special_buff.h"
// application
#include "bsp_systick.h"
#include "bsp_flash.h"
#include "bsp_usart2.h"

#define  UART_RECV_DONE    0xAA

//�����ʱ��������
struct soft_timer  uart1_timer;

struct soft_timer  uart0_timer;//����1��
struct soft_timer  usart1_timer;//����2��
struct soft_timer  pca1_timer;//����3��
struct soft_timer  pca0_timer;//����4��
struct soft_timer  epca0_timer;//����5��
struct soft_timer  epca1_timer;//����6��
struct soft_timer  epca2_timer;//����7��
struct soft_timer  usart0_timer;//����8��

struct soft_timer  led_timer;

struct soft_timer  resource_timer;	
struct soft_timer  update_startup_timer,update_restart_timer,update_write_packets_timer;
struct soft_timer  yijian_read_version_timer;	

extern OS_UART    uart0, usart1,pca1, pca0, epca0, epca1, epca2,usart0;
extern OS_UART1	uart1;

extern OS_BOARDINFO  board;
extern OS_RESOURCE  reSource;	
//��Ԫ�������������
extern uint8_t cf8051update[40];
extern uint8_t update_trays[8];
extern uint8_t update_trays_packet_status[8];
extern uint8_t update_trays_startup_status[8];
extern uint8_t update_trays_restart_status[8];

volatile uint8_t version_flag = 0;
volatile uint8_t zd_resource_flag = 0;
volatile uint8_t zd_tray_num = 0;
volatile uint8_t v_zd_whitch = 0;

//���յ�����֡���ݷ���ָ��
uint8_t  uCom_send_dataBase[8][COM_TX_SIZE] = {{'\0'},{'\0'},{'\0'},{'\0'},{'\0'},{'\0'}};
uint8_t  frame_is_wrong[7] = {0x7e,0x00,0x05,0xf6,0x00,0xb8,0x5a};//��֡�쳣f6
uint8_t  frame_is_right[7] = {0x7e,0x00,0x05,0xa6,0x00,0x8d,0x5a};//��֡����a6
uint8_t    frame_run_ok[11] = {0x7e,0x00,0x09,0x20,0x00,0x08,0x01,0x00,0x00,0x8d,0x5a};//��֡����f0  0x06����������  ����BIOS����APP
														//4:��� 5:���� 6:app or bios 7:flash_��� 8��״̬���������澯�ȣ�
uint8_t      Tray_Error[8] = {0x7e,0x00,0x04,0xf6,0x8d,0x5a};  
uint8_t      ports_lost[8] = {'\0'};
uint8_t  flashinfo[30];

extern volatile uint8_t updating_flags;

extern struct myflash DeviceInfo;
extern uint8_t TrayInfo[17];
extern volatile uint8_t T_type[8]; //zhuchengzhi2017-01-25


//==============================================================================
//                             led
//==============================================================================
void led_timeover_proc(void)
{
	reload_timer(&led_timer, 1000);
	Run_Led_Flash();
	start_timer(&led_timer); 	
}


//==============================================================================
//                             ������7�����ڵĳ�ʱ������
//==============================================================================
/**
  * @brief  ����usart0 ���ճ�ʱ����
  * @retval None.
  */
void usart0_timeover_proc(void)
{	
//	while(SI32_USART_A_read_rx_fifo_count (SI32_USART_0) >> 0){
//		usart0.recvBuff[usart0.count++] = SI32_USART_A_read_data_u8(SI32_USART_0); 
//      if(usart0.count > 609){
//         usart0.count = 609;
//      }
//	}
   usart0.count = 0;   
	usart0.recvFlag = 1;
}

/**
  * @brief  ����usart1 ���ճ�ʱ����
  * @retval None.
  */
void usart1_timeover_proc(void)				
{
//	while(SI32_USART_A_read_rx_fifo_count (SI32_USART_1) >> 0){
//		usart1.recvBuff[usart1.count++] = SI32_USART_A_read_data_u8(SI32_USART_1); 
//      if(usart1.count > 609){
//         usart1.count = 609;
//      }      
//	}
   usart1.count = 0;    
	usart1.recvFlag = 1;
}

/**
  * @brief  ����epca0 ���ճ�ʱ����
  * @retval None.
  */
void epca0_timeover_proc(void)				
{
//   SI32_EPCACH_A_enable_negative_edge_input_capture(SI32_EPCA_0_CH0);  //�ָ�֮ǰ�Ĳ����� 
//   
   epca0.recvFlag = 1;
   epca0.count = 0;
}


/**
  * @brief  ����epca0 ���ճ�ʱ����
  * @retval None.
  */
void epca1_timeover_proc(void)				
{
//   SI32_EPCACH_A_enable_negative_edge_input_capture(SI32_EPCA_0_CH1);  //�ָ�֮ǰ�Ĳ����� 
//   
   epca1.recvFlag = 1;
   epca1.count = 0;
}

/**
  * @brief  ����epca0 ���ճ�ʱ����
  * @retval None.
  */
void epca2_timeover_proc(void)				
{
//   SI32_EPCACH_A_enable_negative_edge_input_capture(SI32_EPCA_0_CH2);  //�ָ�֮ǰ�Ĳ����� 
//   
	 epca2.count = 0;
   epca2.recvFlag = 1;

}


/**
  * @brief  ����pca0 ���ճ�ʱ����
  * @retval None.
  */
void pca0_timeover_proc(void)				
{
//   SI32_PCACH_A_enable_negative_edge_input_capture(SI32_PCA_0_CH0);  //�ָ�֮ǰ�Ĳ�����	
//   
   pca0.count = 0;
   pca0.recvFlag = 1;  
}

/**
  * @brief  ����pca1 ���ճ�ʱ����
  * @retval None.
  */

void pca1_timeover_proc(void)				
{
//   SI32_PCACH_A_enable_negative_edge_input_capture(SI32_PCA_1_CH0);  //�ָ�֮ǰ�Ĳ�����
//	
   pca1.count = 0;
   pca1.recvFlag = 1;
   
}

/**
  * @brief  ����uart0 ���ճ�ʱ����
  * @retval None.
  */
void uart0_timeover_proc(void)				
{
//   while(SI32_UART_A_read_rx_fifo_count(SI32_UART_0) > 0){
//      uart0.recvBuff[uart0.count++] = SI32_UART_A_read_data_u8(SI32_UART_0);
//      if(uart0.count > 609){
//           uart0.count = 609; 
//      }
//   }						

   uart0.count = 0;
   uart0.recvFlag = 1;
}

/**
  * @brief  ����uart1 ���ճ�ʱ����
  * @retval None.
  */
void uart1_timeover_proc(void)				
{ 
//   while(SI32_UART_A_read_rx_fifo_count(SI32_UART_1) > 0){
//		uart1.recvBuff[uart1.count++]= SI32_UART_A_read_data_u8(SI32_UART_1);   
//      
//      if(uart1.count >= 689)//699
//         uart1.count=688;    //698 
//   }
	
	
   uart1.count = 0;
   uart1.recvFlag = 1;   
}




//==============================================================================
//                             ��������Դ�ɼ��ĳ�ʱ������
//==============================================================================

/**
  * @brief  ��Դ�ɼ���ʱʱ��
  * @retval None.
  */
void resource_timeover_proc(void)
{
   uint8_t i = 0;
   reload_timer(&resource_timer,3000);	
   for(;i < 8;i++)
      memset(&uCom_send_dataBase[i][0],0,COM_TX_SIZE);
   
   reSource.flag = 1;
  
//  SI32_UART_A_enable_rx(SI32_UART_1);  
	  usart2_rx_irq_enable(ENABLE); 	

}

/**
  * @brief  һ����ȡ�汾�ų�ʱ
  * @retval None.
  */
void yijian_read_version_timer_proc(void)
{
   uint8_t i = 0;
   reload_timer(&yijian_read_version_timer,3000);	
   for(;i < 8;i++)
      memset(&uCom_send_dataBase[i][0],0,COM_TX_SIZE);
   
	 if(v_zd_whitch == 0x02){//��ʾһ����ȡ�汾��
		  v_zd_whitch = 0;
			version_flag = 1;
	 }
	 if(v_zd_whitch == 0x03){//ָ���̲ɼ�
		  v_zd_whitch = 0;
		  zd_resource_flag = 1;
	 }

//  
//  SI32_UART_A_enable_rx(SI32_UART_1);
	  usart2_rx_irq_enable(ENABLE); 	   
}

//==============================================================================
//                             ���µ�Ԫ������ʱ��ʱ������
//==============================================================================
/**
  * @brief  ��Ԫ��������ʱ��ʱ��
  * @retval None.
  */
void update_startup_proc(void)
{
   uint8_t i = 0;

   reload_timer(&update_startup_timer,9000);
   for(i = 0;i < 8;i++){
      if((update_trays[i] == 1) && ((update_trays_startup_status[i] == 0xe))) {
         update_trays_startup_status[i] = 0x01;
         cf8051update[9+i*3] = 0x01;
				 update_trays[i] = 0x00;//zhuchengzhi 2015-05-18
      }
   }
   cf8051update[4] = 0x01;	
   cf8051update[33] = crc8(cf8051update,33);
   cf8051update[34] = 0x5a;
   add_a_new_node(cf8051update,DATA_NORMAL,35,DATA_RECV);

   for(;i < 8;i++) {
      if(uCom_send_dataBase[i][3] == 0x06)
         memset(&uCom_send_dataBase[i][0],0,COM_TX_SIZE);
   }	
}

/**
  * @brief  ��Ԫ������������ʱ��ʱ��
  * @retval None.
  */
void update_restart_proc(void)
{
   uint8_t i = 0;

   reload_timer(&update_restart_timer,10000);		
   for(i = 0;i < 8;i++){
      if((update_trays[i] == 1) && ((update_trays_restart_status[i] == 0xe))) {
         update_trays_startup_status[i] = 0x01;
         cf8051update[9+i*3] = 0x01;	
				 update_trays[i] = 0x00;//zhuchengzhi 2015-05-18
      }
   }
   cf8051update[4] = 0x03;	
   cf8051update[33] = crc8(cf8051update,33);
   cf8051update[34] = 0x5a;
   add_a_new_node(cf8051update,DATA_NORMAL,35,DATA_RECV);
   memset(update_trays_startup_status,0,sizeof(update_trays_startup_status));//������������ǲ������ģ���Ҫ���ϱ�����ʧ��������

   for(;i < 8;i++) {
      if(uCom_send_dataBase[i][3] == 0x06)
         memset(&uCom_send_dataBase[i][0],0,COM_TX_SIZE);
   }
}

/**
  * @brief  ��Ԫ�����ݰ�д�볬ʱ��ʱ��
  * @retval None.
  */
void update_write_packets_proc(void)
{
   uint8_t i = 0;

   reload_timer(&update_write_packets_timer,8000);	

   for(i = 0;i < 8;i++){
      if((update_trays_startup_status[i] == 0x0) && ((update_trays_packet_status[i] == 0xe))){ 
         //���������־�ڣ�����Ԫ��������𸴻�û���룬�Ͳ��ظ�������ʱ
         update_trays_startup_status[i] = 0x01;
				 update_trays[i] = 0x00;//zhuchengzhi 2015-05-18
      }
   }
   cf8051update[4] = 0x02;		
   cf8051update[33] = crc8(cf8051update,33);
   cf8051update[34] = 0x5a;
   add_a_new_node(cf8051update,DATA_NORMAL,35,DATA_RECV);	

   for(i = 0;i < 8;i++) {
      if(uCom_send_dataBase[i][3] == 0x06)
         memset(&uCom_send_dataBase[i][0],0,COM_TX_SIZE);
   }
}                   

/**
  * @brief  �����ҵ������ʱ���͵�������(������s3c44b0x�㱨������)
  * @retval �� 
  */
void active_myPherial_from_reset( void )
{
   createNewList();															//����һ�������������ݵ�����
   soft_timer_list_reset();											   //��ʱ������λ	
   
   add_timer(&uart1_timer,uart1_timeover_proc, 1);    				//���uart1 ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms
   add_timer(&uart0_timer,uart0_timeover_proc, 1);    				//���uart0 ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms	
   add_timer(&usart1_timer,usart1_timeover_proc, 1);  				//���usart1���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms
   add_timer(&pca1_timer,pca1_timeover_proc, 1);      				//���pca1  ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms   
   add_timer(&pca0_timer,pca0_timeover_proc, 1);      				//���pca0  ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms
   add_timer(&epca0_timer,epca0_timeover_proc, 1);    				//���epca0 ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms
   add_timer(&epca1_timer,epca1_timeover_proc, 1);    				//���epca0 ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms
   add_timer(&epca2_timer,epca2_timeover_proc, 1);    				//���epca0 ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms
   add_timer(&usart0_timer,usart0_timeover_proc, 1);  				//���usart1���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1*1ms = 1ms
	
   add_timer(&resource_timer,resource_timeover_proc,3000);       //�����Դ�ɼ� ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ2s = 2000ms	
   add_timer(&led_timer,led_timeover_proc,1000);    				  //��led ���ճ�ʱ��ʱ���������ʱ���б�1����ʱ1000ms
  
   add_timer(&update_startup_timer,update_startup_proc,9000);     //��Ԫ������������ʱ��ʱ�� ��ʱ9s	
   add_timer(&update_write_packets_timer,update_write_packets_proc,9000); //��Ԫ������д���ݰ���ʱ�� ��ʱ9s	
   add_timer(&update_restart_timer,update_restart_proc,9000);     //��Ԫ��������ʱ��ʱ�� ��ʱ9s		

	
	 add_timer(&yijian_read_version_timer,yijian_read_version_timer_proc,3000);//һ����ȡ�汾��
	
   board.hard_version = 0x0106;
   board.soft_version = 0x0106;
   frame_is_right[4] = board.id;
   frame_is_right[5] = crc8(frame_is_right,5);		
   frame_is_wrong[4] = board.id;
   frame_is_wrong[5] = crc8(frame_is_wrong,5);		
   frame_run_ok[4] = board.id;
   frame_run_ok[9] = crc8(frame_run_ok,9);	

//   memset(flashinfo,0xff, sizeof(flashinfo));
//   ReadFlashData(FLASH_TEST_PAGE_ADDR_PTR,flashinfo,sizeof(flashinfo));
//   memcpy(&DeviceInfo.Tary_Num[0],flashinfo,8);
//   DeviceInfo.resource = flashinfo[9];//������¼������û�б��ɼ���
//   board.id = flashinfo[10]; //�鿴�Ƿ��п�ż�¼   


   memset(update_trays_startup_status,0,sizeof(update_trays_startup_status));//������������ǲ������ģ���Ҫ���ϱ�����ʧ��������	
   memset(update_trays,0,sizeof(update_trays));//�������������������ȫ�����Ϊ0����Ҫ���������ε�Ԫ����bios����������ϵ�ᱨ���������ɹ�������
   memset(&cf8051update[7],0x0e,24);
   reSource.reSourceNow = 0x0;/* ϵͳ��ʼ����Ĭ�ϲ��ɼ�*/
}

//------------------------------------------eof-----------------------
