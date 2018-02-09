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
 | ���ʱ�䣺2014-07-24 
 |------------------------------------------------------------------------------
 | ԭ�汾 �� 1.0
 | ��  �ߣ�  MingLiang.lu
 | ���ʱ�䣺2013-05-20
 |------------------------------------------------------------------------------
 | �������⣬����ϵ: linux.lucian@gmail.com
 |------------------------------------------------------------------------------
 */

#ifndef __MYSERVICE_H__
#define __MYSERVICE_H__

#include <stdbool.h>
#include <stdint.h>

#define V_1		0x02
#define V_2		0x00
#define Y_1   0x02
#define Y_2	  0x00
#define Y_3		0x01
#define Y_4		0x07
#define M_1		0x01
#define M_2   0x01
#define D_1   0x01
#define D_2   0x04
#define T_1   0x08



void mySerial_disable_tx(uint8_t uart_nr);
void mySerial_enable_tx(uint8_t uart_nr);
void mySerial_disable_rx(uint8_t uart_nr);
void mySerial_enable_rx(uint8_t uart_nr);

//ʹ��ĳ�����ڽ�������̬
void mySerial_send_string(uint8_t uart_nr,uint8_t *src);
//��ֹ���д���
void mySerial_disable_all_serial(void);
void reset_mcu_enter_default_mode(void);

#endif //__MYSERVICE_H__

