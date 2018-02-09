#ifndef __BOARD_H_
#define __BOARD_H_

#include "stdint.h"

#define CNT_STAR 									 310							//��ʼλ����ֵ
//start bit 
#define START_BIT									(516 - CNT_STAR)//(ÿ�μ���CNT������һ���ж�)
#define BIT_TIME_START						(4294967296 - START_BIT)
#define LOW_TIME_START  					(BIT_TIME_START & 0xFFFF)
#define HIG_TIME_START					  ((BIT_TIME_START >> 16)&0xFFFF)

#define CNT_NORM 								   97							//����λ����ֵ
//norme bit
#define NORME_BIT									(344 - CNT_NORM)//(ÿ�μ���CNT������һ���ж�)
#define BIT_TIME_NORM						  (4294967296 - NORME_BIT)
#define LOW_TIME_NORM  					  (BIT_TIME_NORM & 0xFFFF)
#define HIG_TIME_NORM					    ((BIT_TIME_NORM >> 16)&0xFFFF)


#define ENABLE_RX			1
#define ENABLE_TX			2
			
		
typedef struct _analog_uartn {
	uint8_t rx_or_tx;
	uint8_t date_bit;
}ANALOG_UARTN;



#endif
