#ifndef _key_H
#define _key_H


#include "system.h"
 
#define KEY1_Pin    GPIO_Pin_8    
#define KEY2_Pin    GPIO_Pin_9    
 

#define KEY_Port (GPIOB) //定义端口


//使用位操作定义
#define KEY1 	PBin(8)
#define KEY2 	PBin(9)



//定义各个按键值  
#define KEY1_VALUE 	1
#define KEY2_VALUE 	2
  



void KEY_Init(void);
u8 KEY_Scan(u8 mode);
#endif
