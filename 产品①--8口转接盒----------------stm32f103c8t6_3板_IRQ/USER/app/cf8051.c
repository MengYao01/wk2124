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
 | ���ʱ�䣺2014-07-15 
 |------------------------------------------------------------------------------
 | ԭ�汾 �� 1.0
 | ��  �ߣ�  MingLiang.lu
 | ���ʱ�䣺2013-05-20
 |------------------------------------------------------------------------------
 | �������⣬����ϵ: linux.lucian@gmail.com
 |------------------------------------------------------------------------------
 */

//#include "myCPU.h"
#include "cf8051.h"
#include "s3c44b0x.h"
#include "bsp_timer2.h"
#include "bsp_flash.h"

OS_PORTLOST    ports = {{'\0'},{'\0'},{'\0'},{'\0'},{'\0'},0};
OS_BOARDINFO   board = {0x100,0x100,0x0};
OS_ALARMINFO   alarm = {0x0004,0x0000,0x0,{'\0'}};
OS_RESOURCE    reSource = {0x0,0x0,0x0,0x0,{0x7e,0x08,0x8d,0x0d,0x0}};

uint8_t poll_410_cmd[8] = {0x7e,0x00,0x06,0x20,0x01,0x00,0x00,0x5a};
uint8_t frame_stats[10] = {0x7e,0x00,0x08,0x09,0x01,0x00,0x00,0x00,0x00,0x5a};

volatile uint8_t T_type[8] = {0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C}; //zhuchengzhi2017-01-25
volatile uint16_t lost_packet[8] = {'\0'};

extern volatile uint8_t HUB_output_alarm;
extern volatile uint8_t HUB_output;

extern uint8_t flashinfo[30];

extern volatile uint8_t read_version;
extern volatile uint8_t updating_flags;
extern volatile uint8_t v_zd_whitch;


uint8_t check_0xA6_count[6] = {'\0'};

volatile uint8_t save_order_s = 0;


static OS_UART* detect_uarts(uint8_t uartnum);
static void cf8051_boards_info_handler(uint8_t *src,uint8_t com);
static uint8_t cf8051_recv_data_crc_check(uint8_t *src);
static void communication_logs(uint8_t communication_port,uint8_t connect_times, uint8_t cf8051_tray_number,uint8_t cf8051_board_number);
static void cf8051_send_data_pthread(uint8_t serivced_serial,uint8_t com_n,uint8_t communications);
static int  cf8051_recv_handle_pthread(OS_UART* uart, uint8_t serivced_serial,uint8_t com_n);

//------------------------------------------------------------------------------
//          System application Sim3U1xxx devices families.(cf8051 parts)
//------------------------------------------------------------------------------

/**
  * @brief  �ӿڰ�Ե�Ԫ�������������
  * @param   
  * @retval None.
  */
void cf8051_service_routine( void ) 
{
   static uint8_t serial = 1;
   uint8_t communication_times = 0, crc_stat = 0;
   uint8_t s_index = serial - 1, cf8051_tray_index = 0;	
   uint8_t time_stamp = 0;
//2017-08-22 zhuchnegzhi 
   uint8_t cf8051_board_index = 0;
//END
   
   OS_UART *p = detect_uarts(serial);

   //step1.������ǰ��Ԥ��׼������״̬λ,���һЩ״̬��־λ
   p->sendFlag = 0;
   p->recvFlag = 0;
  
   //ʹ�����̵����ݷ���
   mySerial_enable_tx(serial);  
   cf8051_send_data_pthread(serial,s_index,communication_times);
   mySerial_enable_rx(serial);   
   
   //step3.���ݷ��͵�ĳ������������Ϻ󣬵ȴ����̻ظ����ݣ������ճ�ʱʱ��ֹͣ�ȴ����ȴ�����ÿ����120ms.
   if(reSource.reSourceNow == 0x01){
      Delay_ms(1450);
   }else{
      for(time_stamp = 0; time_stamp < 120; time_stamp++){
         if(uart1.recvFlag == 1 && uart1.recvBuff[3] == 0x20){
            mainboard_service_routine();
         }else{
//            imprecise_msdelay();//����ȫ��ȷ��ʱ1ms
							Delay_ms(11);
         }
      }
   }
  
   //step4.�ھ���һ��ʱ��ȴ��󣬲鿴�Ƿ����յ����̻ظ�������,����У����봦�����û��ֱ������
	 if((p->recvFlag)){
      p->recvFlag = 0;
      crc_stat = cf8051_recv_handle_pthread(p,serial,s_index);
	 //													��ѯ����  ����APP��  	 �澯����
      if(crc_stat && 
				(((p->recvBuff[3] == 0x20) && (p->recvBuff[7] == 0x01))|| (p->recvBuff[3] == 0x82))){
						
         communication_times += 1;
         cf8051_tray_index = p->recvBuff[5];//�̺�
				 cf8051_board_index = p->recvBuff[4];//���
				 TrayInfo[6 + s_index] = s_index+1;
      }	
   }else{
			TrayInfo[6 + s_index] = 0x00;
		  lost_packet[s_index]++;  //2017-03-26����˶����ʵļ��
		  if(lost_packet[s_index] > 0xfffe){
				lost_packet[s_index] = 0x00;
			}
	 }
	 
   //step5.���ݴ�����ڣ��ض���һ485�������еĲ���������һЩ��¼
//   mySerial_disable_tx(serial);
//   mySerial_disable_rx(serial);
   
   if((0x01 == DeviceInfo.resource) && (HUB_output_alarm == 0)){
      //����û����Ѿ���Դ�ɼ�����
      communication_logs(s_index,communication_times, cf8051_tray_index,cf8051_board_index);// ��¼˫��ͨ�ŵĴ���
   }else{
      //do nothing 
   }

   //step6,��������̵�һ��ͨ�ź��ۼ����̱�ţ�������һ�����̵ķ���
   serial++;
   if(serial > 8){
      serial = 1;
   }

}

/**
  * @brief  ʶ�𱾿������������� 12�˿� �� 17�˿�
  * @retval 
  */
uint8_t back_trays_types(uint8_t dat)
{
	uint8_t ret = 0;
	if(flashinfo[14 + (dat - 1)] == 0x11){
		ret = 0x11;//17���˿ڵ�
	}else{
		ret = 0x0c;//12���˿ڵ�
	}
	return ret;
}

/**
  * @brief  �ǳ���Ҫ��һ��ģ�飬�����ݴ浱ǰ��Ԫ�巴����״̬��Ϣ��
  * @retval �� ���� ������Ҫ�������
  */
static void cf8051_boards_info_handler(uint8_t *src,uint8_t com)
{
    uint8_t IsEffect = 0,i = 0,Trayidex = 0x0;  
    uint16_t srcLen = 0x0;
		static uint8_t tray_types = 0;
	  static uint8_t tray_index = 0;
		tray_types = 0;
		tray_index = 0;
	
   IsEffect = cf8051_recv_data_crc_check(src);  

   if(IsEffect){
      srcLen = src[1] << 8|src[2] + 2;
      switch(src[3]){

         case 0x07://д����ӱ�ǩ
         case 0x0f://�ܿ�д����ӱ�ǩ
         case 0x10://LEDָʾ�Ʋ���	
         case 0x13:
				 case 0x04:
               *(src + srcLen - 1) = 0x5a;
               add_a_new_node(src,DATA_NORMAL,srcLen,DATA_RECV);
         break;
				 case 0xfc://��ȡ�˿���Ϣ	
					 memcpy(&reSource.rxbuf[6], &src[6], srcLen - 8);
				 
				   if(v_zd_whitch == 3){//˵������һ��ָ���̲ɼ��������0˵��ָʾ��ȡ�˿���Ϣ
					   flashinfo[(*(src + 5)) - 1] = *(src + 5);           //��Ԫ�巵�ص��̺�
					   DeviceInfo.Tary_Num[(*(src + 5)) - 1] = *(src + 5); //��Ԫ�巵�ص��̺�
				 
					   if(srcLen == 393){ 
						   flashinfo[14 + ((*(src + 5)) - 1)] = 0x0C;//˵����12�˿ڵĵ�Ԫ��
					   }else{
							 flashinfo[14 + ((*(src + 5)) - 1)] = 0x11;//˵����17�˿ڵĵ�Ԫ��
						 }
				   }
				 break;
				 
				case 0x05://��ȡ��Ӳ���汾��	
					if(read_version == 1){
						memcpy(&reSource.rxbuf[55 + (48 * (src[6]-1))], &src[7], 48);
						if(src[6] == 8){
							read_version = 1;
						}
					}else{
						 *(src + srcLen - 1) = 0x5a;
						 add_a_new_node(src,DATA_NORMAL,srcLen,DATA_RECV);
					}

				 break;
				
         case 0x06://�������	
               if(update_trays[com-1] == 0x0)break;
               if(src[4] == 0x01){//�������� <-----------------------------
                  Trayidex = com;
                  cf8051update[9+(Trayidex-1)*3] = src[9];
                  update_trays_startup_status[Trayidex-1] = src[9];
                  for(i = 0;i < 8;i++){
                     if((update_trays[i] == 1) && ((update_trays_startup_status[i] == 0xe)))
                     { //���������־�ڣ�����Ԫ��������𸴻�û���룬�Ͳ��ظ�������ʱ
                        break;
                     }
                     
                     if(i == 7){
                        stop_timer(&update_startup_timer);	
                        cf8051update[4] = 0x01;								
                        cf8051update[33] = crc8(cf8051update,33);
                        cf8051update[34] = 0x5a;
                        add_a_new_node(cf8051update,DATA_NORMAL,35,DATA_RECV);				
                     }
                  }
               }else if(src[4] == 0x02){//������... <-----------------------------
                  Trayidex = src[8];
                  cf8051update[9+(Trayidex-1)*3] = src[9];
                  update_trays_packet_status[Trayidex-1] = src[9];
                  for(i = 0;i < 8;i++){
                     if((update_trays_startup_status[i] == 0x0) && ((update_trays_packet_status[i] == 0xe)))
                     { //���������־�ڣ�����Ԫ��������𸴻�û���룬�Ͳ��ظ�������ʱ
                        break;
                     }						
                     if(i == 7){	
                        stop_timer(&update_write_packets_timer);		
                        cf8051update[4] = 0x02;		
                        cf8051update[33] = crc8(cf8051update,33);
                        cf8051update[34] = 0x5a;
                        add_a_new_node(cf8051update,DATA_NORMAL,35,DATA_RECV);			
                     }
                  }	
               }else if(src[4] == 0x03){//������ɣ�����������... <-----------------------------
                  Trayidex = src[8];
                  cf8051update[9+(Trayidex-1)*3] = src[9];
                  update_trays_restart_status[Trayidex-1] = src[9];
                  for(i = 0;i < 8;i++){
                     if((update_trays_startup_status[i] == 0x0) && ((update_trays_restart_status[i] == 0xe)))
                     { //���������־�ڣ�����Ԫ��������𸴻�û���룬�Ͳ��ظ�������ʱ
                        break;
                     }						
                     if(i == 7){	
                        stop_timer(&update_restart_timer);		
                        cf8051update[4] = 0x03;		
                        cf8051update[33] = crc8(cf8051update,33);
                        cf8051update[34] = 0x5a;
                        add_a_new_node(cf8051update,DATA_NORMAL,35,DATA_RECV);		
                        memset(update_trays_startup_status,0,sizeof(update_trays_startup_status));//������������ǲ������ģ���Ҫ���ϱ�����ʧ��������								
                     }
                  }
								  update_trays[Trayidex-1] = 0x00;//zhuchengzhi 2015-05-18
               }
//							 updating_flags = 0;//zhuchengzhi 2015-05-18 ���������ʱ���и澯
							ports.traylost[Trayidex-1] = 0;//2017-06-15 ����ʧ�ܺ��̸澯Ҫ���ٴ�ʶ��
							ports.portstat[Trayidex-1] = 0;
         break;
         case 0x0d://��������
            switch(src[4]){
               case 0x02://�����½�
               case 0x03://�������							
               case 0x04://���˲��						
               case 0x05://˫���½�	
               case 0x06://˫�˲��
               case 0x07://�����½�	
               case 0x08://�ܼ�A���½�һ��
               case 0x09://�ܼ�A�˲��һ��
               case 0x10://�ܼ�A���������	
               case 0x18://�ܼ�Z���½�һ��
               case 0x19://�ܼ�Z�˲��һ��
               case 0x30://�ܼ�Z���������
               case 0xb1://���·�������½�
               case 0xb2://���·�������½�
               case 0x8a://
               case 0xa2://ȡ��
               case 0x0a://ȷ��
               case 0x8c://ʩ���аγ�	
               case 0xee:
                  *(src + srcLen - 1) = 0x5a;
                  add_a_new_node(src,DATA_NORMAL,(src[1] << 8 |src[2] + 2),DATA_RECV);	
                  break;
               case 0x01:
               case 0x11://��Դ�ɼ�
//								     tray_types = *(src + srcLen - 3);//��ȡ������ ��12�˿ڵĻ���17�˿ڵ�  Ĭ����12�˿ڵ�
//								 	   //tray_types = back_trays_types(src[6]);
//										 tray_index = (*(src+6) - 1);
//							       flashinfo[14 + tray_index] = tray_types;//������Դ�ɼ���������̵�����
//                     reSource.idex = tray_index * (35 * tray_types) + 6;
//                     reSource.tray_units[tray_index] = TRAY_ONLINE;//��¼������̱��ɼ�����
//                     DeviceInfo.Tary_Num[tray_index] = *(src + 6);//�����̲ɼ���
//                     memcpy(&reSource.rxbuf[reSource.idex], &src[5], (35*tray_types));
							 
										//zhuchengzhi2017-01-25 TODO		
										if((srcLen <= 427)){//���������û�м��������͵��̻ᵼ������
											return ;
										}
										tray_types = *(src + srcLen - 3);//��ȡ������ ��12�˿ڵĻ���17�˿ڵ�  Ĭ����12�˿ڵ�
										tray_index = (*(src+6) - 1);
										flashinfo[14 + tray_index] = tray_types;//������Դ�ɼ���������̵�����
										reSource.tray_units[tray_index] = TRAY_ONLINE;//��¼������̱��ɼ�����
										DeviceInfo.Tary_Num[tray_index] = *(src + 6);//�����̲ɼ���
							 
										switch(*(src + 6)){
											case 0x01:
												memcpy(&reSource.rxbuf[6], &src[5], (35*tray_types));
												break;
											case 0x02:
												reSource.idex = (35 * T_type[0]) + 6;
												memcpy(&reSource.rxbuf[reSource.idex], &src[5], (35 * tray_types));
												break;
											case 0x03:
												reSource.idex = 35 * (T_type[0] + T_type[1]) + 6;
												memcpy(&reSource.rxbuf[reSource.idex], &src[5], (35 * tray_types));
												break;
											case 0x04:
												reSource.idex = 35 * (T_type[0] + T_type[1] + T_type[2]) +6;
												memcpy(&reSource.rxbuf[reSource.idex], &src[5], (35 * tray_types));
												break;
											case 0x05:
												reSource.idex = 35 * (T_type[0] + T_type[1] + T_type[2]+ T_type[3]) +6;
												memcpy(&reSource.rxbuf[reSource.idex], &src[5], (35 * tray_types));
												break;
											case 0x06:
												reSource.idex = 35 * (T_type[0] + T_type[1] + T_type[2]+ T_type[3] + T_type[4]) + 6;
												memcpy(&reSource.rxbuf[reSource.idex], &src[5], (35 * tray_types));
												break;
											case 0x07:
												reSource.idex = 35 * (T_type[0] + T_type[1] + T_type[2]+ T_type[3] + T_type[4]+ T_type[5]) + 6;
												memcpy(&reSource.rxbuf[reSource.idex], &src[5], (35 * tray_types));
												break;
											case 0x08:
												reSource.idex = 35 * (T_type[0] + T_type[1] + T_type[2]+ T_type[3] + T_type[4]+ T_type[5] + T_type[6]) + 6;
												memcpy(&reSource.rxbuf[reSource.idex], &src[5], (35 * tray_types));
												break;
											
											default:
												break;
										}

										break;
            }
            break;       
         case 0x82:
					  tray_types = back_trays_types(*(src + 5));//�������;��������澯���ݵĳ���
				    if(tray_types == 0x11){
							tray_types = 23;
					  }else{
					    tray_types = 18;
					  }
            for(i = 6;i < tray_types;i++){//for(i = 6;i < 18;i++){
               if(*(src + i) != 0x0){//�Ӹ��̵�һ�Ŷ˿ڵ�12�Ŷ˿ڣ����а���ɨ��ʶ��˭������ͼ�¼������ȥ
                  alarm.data[alarm.idex++] = board.id;
                  alarm.data[alarm.idex++] = *(src + 5);
                  alarm.data[alarm.idex++] = i - 5;						
                  alarm.data[alarm.idex++] = *(src + i);
                  if(alarm.idex > 292)alarm.idex = 4; 
               }
            }
            alarm.data[0] = 0x7e; 				
            alarm.data[3] = 0x09; 	
            alarm.flag = 1;
            break;
         default:
            break;
      }
   }
}

/**
  * @brief  HUB���ڹ������͵ĳ�ʱ������ݷ��� main_cmd = 0x0D,sub_cmd = 0x9C			
  * @retval �� ���� ������Ҫ�������
  */
void cmd_0x0d_sub_cmd_xx_timeout_func(uint8_t *src, uint8_t c_tray)
{
	uint8_t sub_cmd = 0;
	uint8_t num_tray = (c_tray + 1);
	uint8_t ofs = 0;
	uint8_t len = 0,i = 0;
	uint8_t order_timer_out[60] = {'\0'};
	sub_cmd = *(src + 4);
	switch(sub_cmd){
		case 0x02://�����½�
		case 0x03://����
		case 0x04://���˲��
			
		
		case 0x05://˫���½�
		case 0x08://�ܼ�˫���½�����
		case 0x18://�ܼ�˫���½��ӻ�
			
		
//	case 0x41://˫�˷��ͶԶ�ʱ��ʧ��
		case 0x06://˫�˲��
		case 0x09://�ܼ�˫�˲������
		case 0x19://�ܼ�˫�˲���ӻ�
		
		case 0xA2://����ȡ��
		case 0x0A://����ȷ��
			order_timer_out[6]  = sub_cmd;//���������� 
			if(sub_cmd == 0x0A){
				sub_cmd = save_order_s;
			}
			order_timer_out[4]  = sub_cmd;//������
			memcpy(&order_timer_out[7] , (src + 5) , 9);//[����]��[A�� A�� A�� A״̬]��[Z�� Z�� Z�� Z״̬]
		  if(num_tray == src[7]){//A��
				order_timer_out[11] = 0x0e;
			}
		  if(num_tray == src[11]){//z��
				order_timer_out[15] = 0x0e;
			}
			ofs = 0x11;
			break;
		case 0xB1://���·�������½�
		  order_timer_out[4]  = sub_cmd;//������
			memcpy(&order_timer_out[7] , (src + 5) , 5);
			memset(&order_timer_out[12], 0 , 4);
			break;
		
		case 0x07: //�����Ļ���
		case 0x10: //�ܼ����� ����
		case 0x30: //�ܼ����� �ӻ�
			order_timer_out[6]  = sub_cmd;//���������� 
			if(sub_cmd == 0x0A){
				sub_cmd = save_order_s;
			}
			order_timer_out[4]  = sub_cmd;//������	
		
			len = (src[2] - 5);
			memcpy(&order_timer_out[7] , (src + 5) , len);
		  ofs = (len + 8);
		  for(i = 0;i < ((src[2] - 6)/4);i++){
				if(num_tray == src[7 + (4*i)]){
					order_timer_out[11 + (4*i)] = 0x0e;
				}
			}
			break;
		
		default:
			break;
	}
	
	order_timer_out[0]  = 0x7e;
	order_timer_out[1]  = 0x00;
	order_timer_out[2]  = ofs;//ofs = 0x11;
	order_timer_out[3]  = 0x0D;//������
//order_timer_out[4]  = 0x05;//������
	order_timer_out[5]  = 0xAc;//����������
	order_timer_out[6]  = 0xAc;//���������� 
	
	order_timer_out[ofs-1] = 0x00;//Ԥ��״̬
	order_timer_out[ofs] = crc8(order_timer_out, ofs);//CRC8
	order_timer_out[ofs+1] = 0x5A;//5A
	add_a_new_node(order_timer_out, DATA_NORMAL, (ofs+2), DATA_RECV); 
	
	save_order_s = 0;
}


/**
  * @brief  �ӿڰ��cf8051��Ԫ���|| ���� ||�������				
  * @retval �� ���� ������Ҫ�������
  */
static int cf8051_recv_handle_pthread(OS_UART* uart, uint8_t serivced_serial,uint8_t com_n)
{
   uint8_t crc_status = 0;
	 crc_status = cf8051_recv_data_crc_check(uart->recvBuff);
	 if(crc_status == CRC_OK){
			if(uart->recvBuff[3] == 0x20){
					if(uart->recvBuff[6] == 0x11){
							T_type[uart->recvBuff[5] - 1] = 0x11;
					}else{
							T_type[uart->recvBuff[5] - 1] = 0x0C;
					}
					return 1;
			 }
			
      //��Ԫ�巵�ص�����crc��ȷ
      if(((uart->recvBuff[3] != 0x20) && (uart->recvBuff[3] != 0xf6) )&& (uart->recvBuff[3] != 0x00)){
         if(uart->recvBuff[3] == 0xa6){
            memset(&uCom_send_dataBase[com_n][0],0,COM_TX_SIZE);
            memset(uart->recvBuff,0,sizeof(uart->recvBuff));
         }else{
            if(0x01 == DeviceInfo.resource || reSource.reSourceNow == 0x1||uart->recvBuff[3] == 0x05|| \
							uart->recvBuff[3] == 0x06||uart->recvBuff[3] == 0xfc||uart->recvBuff[3] == 0x04)
            {//˵������Դ�ɼ�����
							 //mySerial_send_string(serivced_serial,frame_is_right);							//��ȷ���յ���Ԫ��������Ϣ��cf8051
               if(ports.portstat[com_n] == PORT_RUNNING_NORMAL || reSource.reSourceNow == 0x1|| \
									uart->recvBuff[3] == 0x05||uart->recvBuff[3] == 0x06|| \
									uart->recvBuff[3] == 0xfc||uart->recvBuff[3] == 0x04)
               {
                  //�����ʱ���������е��̣�����Ӧ����������������е���ʧ���ģ��Ͳ�care
                  cf8051_boards_info_handler(uart->recvBuff,serivced_serial);
               }else{
                  return 1;
               }
            }else{//˵������û�б��ɼ�,�������κ����ݸ�����
               memset(uart->recvBuff,0,sizeof(uart->recvBuff));
            }
            mySerial_send_string(serivced_serial,frame_is_right);							//��ȷ���յ���Ԫ��������Ϣ��cf8051
         }
      }else if( uart->recvBuff[3] == 0xf6 ){
         if(uCom_send_dataBase[com_n][3] != 0x0){
            mySerial_send_string(serivced_serial,&uCom_send_dataBase[com_n][0]);
         }else{
            mySerial_send_string(serivced_serial,poll_410_cmd);
         }
      }else{
         memset(uart->recvBuff,0,sizeof(uart->recvBuff));
      }
      
      return 1; 
	 }else{
      //��Ԫ�巵�ص�����crc����ȷ��������������յ�������
      //mySerial_send_string(serivced_serial,frame_is_wrong);	//2017-05-08 zhuchengzhi
      //uart->sendFlag = 1;
      
			TrayInfo[6 + serivced_serial - 1] = 0x00;
			lost_packet[serivced_serial - 1]++;  //2017-03-26����˶����ʵļ��
			if(lost_packet[serivced_serial - 1] > 0xfffe){
				lost_packet[serivced_serial - 1] = 0x00;
			}
			
			memset(uart->recvBuff,0,sizeof(uart->recvBuff)); //2017-05-08 zhuchengzhi

      return 0;
	 }
}


/**
  * @brief  �ӿڰ��cf8051��Ԫ��Ľ��մ������
  * @retval �� ���� ������Ҫ�������
  */
static void cf8051_send_data_pthread(uint8_t serivced_serial,uint8_t com_n,uint8_t communications)
{

   if(uCom_send_dataBase[com_n][3] != 0x0)
   {
      mySerial_send_string(serivced_serial,&uCom_send_dataBase[com_n][0]);

   }else{
      poll_410_cmd[4] = board.id;
      poll_410_cmd[5] = serivced_serial;
      poll_410_cmd[6] = crc8(poll_410_cmd,6);
      mySerial_send_string(serivced_serial,poll_410_cmd);
   }
}

/**
  * @brief  ����ת�����ݸ���Ԫ��
  * @retval none
  */
void repost_update_packet(void)
{
   uint8_t unit_board = 1,loop_times = 0;	

   for(loop_times = 0; loop_times < 18;loop_times++){  //18/6���� = 3��/��		
      if(uCom_send_dataBase[unit_board-1][3] != 0x0){
         OS_UART *p = detect_uarts(unit_board);
         p->sendFlag = 0;
         p->recvFlag = 0;
         
         mySerial_enable_tx(unit_board);  
         mySerial_enable_rx(unit_board);   
         
         if(!p->sendFlag){
            mySerial_send_string(unit_board,&uCom_send_dataBase[unit_board-1][0]);
            p->sendFlag = 1;
         }
         Delay_ms(1450);//�ȴ�����Ľ��ձ�־λ��ʱ�ж�����

         if(p->recvFlag){
            p->recvFlag = 0;
            if(p->recvBuff[3] == 0xa6){
               memset(&uCom_send_dataBase[unit_board-1][0],0,COM_TX_SIZE);
            }
         }
         
         mySerial_disable_tx(unit_board);  
//         mySerial_disable_rx(unit_board);           

      }//end of if(uCom_send_dataBase[unit_board_idex][3] != 0x0)
      
      unit_board += 1;
      if(unit_board > 8)unit_board = 1;
   }

   for(unit_board = 0; unit_board < 8;unit_board++)
         memset(&uCom_send_dataBase[unit_board][0],0,COM_TX_SIZE);	

}

/**
  * @brief  ����ת�����ݸ���Ԫ��
  * @retval none
  */
void repost_data_to_cf8051_immediately(void)
{
   uint8_t unit_board = 0, out_loop = 0;	

   for(out_loop = 0; out_loop < 5; out_loop++){ //2017-07-11 zhuchenzhi �޸ĵ�ѭ��
      for(unit_board = 1; unit_board < 9; unit_board++){     
         if(uCom_send_dataBase[unit_board-1][3] != 0x0){
            OS_UART *p = detect_uarts(unit_board);
            p->sendFlag = 0;
            p->recvFlag = 0;
            
            mySerial_enable_tx(unit_board);  
            mySerial_enable_rx(unit_board);         
             
            if(!p->sendFlag){
               mySerial_send_string(unit_board, &uCom_send_dataBase[unit_board-1][0]);
               p->sendFlag = 1;
            }
            
            Delay_ms(1450);//�ȴ�����Ľ��ձ�־λ��ʱ�ж�����
            
            if(p->recvFlag){
               p->recvFlag = 0;
               if(p->recvBuff[3] == 0xa6){
                  if(uCom_send_dataBase[unit_board-1][3] == 0x0d && uCom_send_dataBase[unit_board-1][4] == 0xa6){
                     Delay_ms(450);//�ȴ�����Ľ��ձ�־λ��ʱ�ж�����                           
                  }
                  memset(&uCom_send_dataBase[unit_board-1][0],0x0, COM_TX_SIZE);
               }else{
                  p->sendFlag = 0;
               }
            }
            
            mySerial_disable_tx(unit_board);  
//            mySerial_disable_rx(unit_board);   
            
         }//end of if(uCom_send_dataBase[unit_board_idex][3] != 0x0)   
      }
   }
   
	for(unit_board = 1; unit_board < 9;unit_board++){
		if(uCom_send_dataBase[unit_board-1][3] == 0x0D){//���ڹ���������Ҫ��ʱ����
				cmd_0x0d_sub_cmd_xx_timeout_func(&uCom_send_dataBase[unit_board-1][0],unit_board-1);//׼���ϱ���ODFĳ�������·�ʧ��
				memset(&uCom_send_dataBase[unit_board-1][0],0, COM_TX_SIZE);  //������ݷ������飬׼��������ѯ
				check_0xA6_count[unit_board-1] = 0;													 //�������ʧ�ܼ���

		}
	}	 
	 
   for(unit_board = 0; unit_board < 8;unit_board++)
      memset(&uCom_send_dataBase[unit_board][0],0,COM_TX_SIZE);
}

/**
  * @brief  usart0�������ݴ�����.
  * @retval None.
  */
static OS_UART* detect_uarts(uint8_t uartnum)
{
   OS_UART* p = NULL;

   switch(uartnum){
      case 8:
         p = &uart0;          
      break;
      case 7:
         p = &usart1;          
      break;
      case 6:
         p = &pca1;           
      break;
      case 5:
         p = &pca0;
      break;
      case 4:
         p = &epca2;         
      break;
      case 3:
         p = &epca1;
      break;       
      case 2:
         p = &epca0;       
      break;
      case 1:
         p = &usart0;
      break;
      default:
         break;		
   }
   
   return p;
}

/**
  * @brief  cf8051 port crcCheck~
  * @retval �� ���� ������Ҫ�������
  */
static uint8_t cf8051_recv_data_crc_check(uint8_t *src)
{
   uint16_t calced_len = 0;
   uint8_t  calced_crc = 0;
   uint8_t  calced_user = 0;
   
   if((src == NULL) || (*(src + 0) == 0x00))return 0;//����׳�Կ��ǣ�ָ�����ǻ���
   calced_len = ((*(src + 1) << 8)| (*(src + 2)));

   if(calced_len > 700){
         return 0;
   }else{
         calced_crc = crc8(src,calced_len);
         calced_user = *(src + calced_len);
         if((calced_crc == calced_user) && (*(src + 0) == 0x7E) && (*(src + calced_len + 1) == 0x5A)){
            return 1;
         }else{
            return 0;
         }
   }
}

/**
  * @brief  ���̵ķǷ����� ������09 ������04
  * @retval �� ���� ������Ҫ�������
  */
static void clear_cache_data(uint8_t serial_port)
{
   /* ������жϵ�ʧ���Ļ���
      ��û���������ʧ���ϱ������ͻָ��˵�*/
   if(ports.traylost[serial_port] > 0){
      ports.traylost[serial_port] = 0;
		  ports.portstat[serial_port] = 0;
    }
}

/**
  * @brief  ���̵ķǷ����� ������09 ������04
  * @retval �� ���� ������Ҫ�������
  */
static void alarm_module_tray_input_illeagle(
         uint8_t communication_port,
         uint8_t connect_times, 
         uint8_t cf8051_tray_number)
{
   if(ports.illegalaccess[communication_port] > TIMES_20){
      return ;     
   }else if(ports.illegalaccess[communication_port] == TIMES_20){
      alarm.data[alarm.idex++] = board.id;
      alarm.data[alarm.idex++] = communication_port + 1;
      alarm.data[alarm.idex++] = 0x00;
      alarm.data[alarm.idex++] = 0x04;
      alarm.data[0] = 0x7e; 
      alarm.data[3] = 0x09; 
      alarm.flag = 1;
      if(alarm.idex > 292)alarm.idex = 4;  
      ports.portstat[communication_port] |= ILLEGAL_ACCESS;
   }else{
      ports.illegalaccess[communication_port] += 1;
   }
}

/**
  * @brief  ���̵ķǷ�����Ļָ�
  * @retval �� ���� ������Ҫ�������
  */
static void alarm_module_tray_input_illeagle_return_to_normal(
         uint8_t communication_port,
         uint8_t connect_times, 
         uint8_t cf8051_tray_number)
{
   if(ports.illegalaccess_recover[communication_port] < 3){
      ports.illegalaccess_recover[communication_port] += 1;
   }else if(ports.illegalaccess_recover[communication_port] == 3){
      alarm.data[alarm.idex++] = board.id;
      alarm.data[alarm.idex++] = communication_port + 1;
      alarm.data[alarm.idex++] = 0x00;
      alarm.data[alarm.idex++] = 0x0C;
      alarm.data[0] = 0x7e; 
      alarm.data[3] = 0x09; 
      alarm.flag = 1;
      if(alarm.idex > 292)alarm.idex = 4; 
      
      ports.portstat[communication_port] &= ~ILLEGAL_ACCESS; 
      
      ports.illegalaccess_recover[communication_port] = 0;
      ports.illegalaccess[communication_port] = 0;      
   }else{
      return ;
   }
}


/**
  * @brief  ���̵ķǷ��γ�
  * @retval �� ���� ������Ҫ�������
  */
static void alarm_module_tray_output_illeagle(
         uint8_t communication_port,
         uint8_t connect_times, 
         uint8_t cf8051_tray_number)
{
   if(ports.traylost[communication_port] > TIMES_20){
      return;
   }else if(ports.traylost[communication_port] == TIMES_20){
      ports.traylost[communication_port] = TIMES_20 + 1;
      alarm.data[alarm.idex++] = board.id;
      alarm.data[alarm.idex++] = communication_port + 1;
      alarm.data[alarm.idex++] = 0x00;
      alarm.data[alarm.idex++] = 0x03;
      alarm.data[0] = 0x7e; 
      alarm.data[3] = 0x09; 
      alarm.flag = 1;
      if(alarm.idex > 292)alarm.idex = 4; 
      ports.portstat[communication_port] |= TRAY_LOST;
   }else{
      ports.traylost[communication_port] += 1;
   }   
}

/**
  * @brief  ���̵ķǷ��γ��Ļָ�
  * @retval �� ���� ������Ҫ�������
  */
static void alarm_module_tray_output_illeagle_return_to_normal(
         uint8_t communication_port,
         uint8_t connect_times, 
         uint8_t cf8051_tray_number)
{
   if(ports.traylost_recover[communication_port] < TIMES_3){
      ports.traylost_recover[communication_port] += 1;   
   }else if(ports.traylost_recover[communication_port] == TIMES_3){
      alarm.data[alarm.idex++] = board.id;
      alarm.data[alarm.idex++] = communication_port + 1;
      alarm.data[alarm.idex++] = 0x00;
      alarm.data[alarm.idex++] = 0x0d;
      alarm.data[0] = 0x7e; 
      alarm.data[3] = 0x09; 
      alarm.flag = 1;
      if(alarm.idex > 292)alarm.idex = 4; 
      ports.portstat[communication_port] &= ~TRAY_LOST; 
      
      ports.traylost_recover[communication_port] = 0;
      ports.traylost[communication_port] = 0;      
   }else{
      return;
   }
}

/* �������뵥Ԫ��û������Ӧ��,�˴���2�ֿ���:
   1.����������˿��ϴβɼ�ʱ��,û���̽��룬��ǰ����δ����״̬,
     �������֮ǰû�зǷ����������,�ǾͲ���
   2.�������������˿�Ҳû���̷Ƿ�����,�ǾͲ����ˣ�����������     
   */
static void alarm_module_tray_disconnect(
         uint8_t communication_port,
         uint8_t connect_times, 
         uint8_t cf8051_tray_number)
{
   if(DeviceInfo.Tary_Num[communication_port] == 0xff){
      /* ����û�вɼ� ��2�������
         1.һֱ���ȶ����У�û�н����κ��̵Ķ���
         2.֮ǰ�зǷ������������̣����˿̣������ڷǷ�����Ļָ���...
      */
      if(ports.portstat[communication_port] & TRAY_ILLEAGLE_INPUT_NOWING){
         /* �Ƿ����������һ */
         alarm_module_tray_input_illeagle_return_to_normal(communication_port,
                                          connect_times,cf8051_tray_number);
      }
			else
			{
				if(ports.portstat[communication_port] & TRAY_LOST){ //�Ƿ��γ��Ļָ� 2017-04-20 zhuchengzhi
						alarm_module_tray_output_illeagle_return_to_normal(
									 communication_port,
									 connect_times, 
									 cf8051_tray_number); 
				}
			}
   }else{
      /* ����û�вɼ� ��2�������
         1.�����̿������ڴ��ڼ���ʧ��״̬...
         2.֮ǰ�зǷ������������̣����˿̣������ڷǷ�����Ļָ���...
      */ 
      if(ports.portstat[communication_port] & TRAY_ILLEAGLE_INPUT_NOWING){
         /* �Ƿ����������һ */
         alarm_module_tray_input_illeagle_return_to_normal(
            communication_port,
            connect_times,
            cf8051_tray_number);      
      }else if(ports.portstat[communication_port] & TRAY_LOST){
         /* �Ѿ�ʧ����,�Ͳ��ù���*/
      }else{
         /* �⼴��ʧ����... */
         alarm_module_tray_output_illeagle(
            communication_port,
            connect_times,
            cf8051_tray_number);       
      }       
   }

}

static void alarm_module_tray_connect(
         uint8_t communication_port,
         uint8_t connect_times, 
         uint8_t cf8051_tray_number,
				 uint8_t cf8051_board_number)
{
   //����ֻ��2�������һ������ͨѶ�������Ƿ���������
   if(DeviceInfo.Tary_Num[communication_port] == 0xff){
   /* �ɼ�ʱû�̣�����ȴ�����ݣ�˵�����зǷ�������...*/
      if(ports.portstat[communication_port] 
         & TRAY_ILLEAGLE_INPUT_NOWING)
      {
      /* �������Լ����ڷǷ���������,�Ͳ�����...*/

      }else{
      /* ���̿��ܴ��ڷǷ���������,�澯����0x04...
         �������̷Ƿ����봦����*/         
      alarm_module_tray_input_illeagle(
         communication_port,
         connect_times, 
         cf8051_tray_number);
      }
   }else{ 
   /* ������2�������
      1.ͨѶ��,�յ��ĵ�Ԫ�巵�صĿ��==��Ȼѯ�ʵĿ��
      2.ͨѶ��,���յ���Ԫ�巵�صĿ��!=��ǰѯ�ʵĿ��,�������Ҫ�����Ƿ�����*/

      if(ports.portstat[communication_port] 
         & TRAY_ILLEAGLE_INPUT_NOWING )
      {
         /* �Ƿ������У�ʲô������... */
         if((DeviceInfo.Tary_Num[communication_port] == cf8051_tray_number)&&
					  (cf8051_board_number == board.id)) //2017-08-22 zhuchengzhi ������ڿ�Ų�һ�������̸澯������
         {
            /* ���̿��ܴ��ڷǷ���������,�澯����0x04...
               �������̷Ƿ����봦����*/         
            alarm_module_tray_input_illeagle_return_to_normal(
                  communication_port,
                  connect_times, 
                  cf8051_tray_number);                                 
         }         
      }else if(ports.portstat[communication_port] 
         & TRAY_LOST)
      {
         if((DeviceInfo.Tary_Num[communication_port] != cf8051_tray_number) ||
					  (cf8051_board_number != board.id))//2017-08-22 zhuchengzhi ������ڿ�Ų�һ�������̸澯������
         {
            /* ���̿��ܴ��ڷǷ���������,�澯����0x04...
               �������̷Ƿ����봦����*/         
            alarm_module_tray_input_illeagle(
                  communication_port,
                  connect_times, 
                  cf8051_tray_number);                                 
         }else{
            /* �Ƿ��γ��Ļָ���...*/
            alarm_module_tray_output_illeagle_return_to_normal(
               communication_port,
               connect_times, 
               cf8051_tray_number);             
         }
      }else{
         if((DeviceInfo.Tary_Num[communication_port] != cf8051_tray_number) ||
					   (cf8051_board_number != board.id))//2017-08-22 zhuchengzhi ������ڿ�Ų�һ�������̸澯������
         {
            /* ���̿��ܴ��ڱ��Ƿ��γ������������ַǷ���������,�澯����0x04...
               �������̷Ƿ����봦����*/         
            alarm_module_tray_input_illeagle(
                  communication_port,
                  connect_times, 
                  cf8051_tray_number);                                 
         }else{
            /* ��û�м�⵽ʧ����Ҳû�м�⵽�Ƿ������ʱ�򣬹۲컺������
               ����м����ģ���ʱ�����*/
            clear_cache_data(communication_port);
         }       
      }            
   }

}


/**
  * @brief  ������¼ͨ�ŵ�˫����һ��һ���ǲ��������������ʶ�û��ı�Ҫ��¼
  * @retval �� ���� ������Ҫ�������
  */

static void communication_logs(
            uint8_t communication_port,
            uint8_t connect_times, 
            uint8_t cf8051_tray_number,
						uint8_t cf8051_board_number)
{
   /* �����ǰ��ִ����Դ�ɼ�����ôֱ�ӷ��أ�
      ��Ϊ��Դ�ɼ���ʱ���ǲ��ϱ��κθ澯��Ϣ 
      ������������������...
      �����������������ڽ���bootloader��
      ����ʧ�����ϱ��澯 */
   if((reSource.reSourceNow == 0x1) 
      || (cf8051update[9+communication_port*3] == 0xf)
      || (cf8051update[9+communication_port*3] == 0x1)
	    || (update_trays[communication_port] == 0x01)
      )
   {
		  clear_cache_data(communication_port);
      return;
   }

   if(connect_times == 0){
      /* ����ͨ����Ӧ�� */   
      alarm_module_tray_disconnect(
          communication_port,
          connect_times, 
          cf8051_tray_number);     
   }else{
      /* ����ͨ����Ӧ�� */ 
      alarm_module_tray_connect(
          communication_port,
          connect_times, 
          cf8051_tray_number,
					cf8051_board_number); 
   }

}

/*
@������ʷ��
   2016��11��3�� 17:31:05 ������communication_logs���ж�����
                          ֻ����Ѳ������ʱ����ȡ�ж��Ƿ������ʧ�����߽���

   2016��11��4�� 16:30:16 �������յ��޸ģ����ɽ�������ָ澯���ʹ���
*/


// -------------------------------------------------eof----------------------------------------------------

