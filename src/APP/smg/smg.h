#ifndef _smg_H
#define _smg_H

#include "system.h"


/*  �����ʱ�Ӷ˿ڡ����Ŷ��� */
#define SMG_PORT 			GPIOB   
#define SMG_PIN 			(GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15)
#define SMG_PORT_RCC		RCC_APB2Periph_GPIOB


void SMG_Init(void);//����ܳ�ʼ��


#endif
