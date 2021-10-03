/*******************************************************************************
* 实验名称  	: ADC模数转换实验
* 实验说明  : 
* 接线说明  : 将外部0-3.3V的模拟信号接入到单片机底座脚P11（PA1）口。
						
* 实验现象	: 使用一根杜邦线，一端接到单片机底座脚P11（PA1）口，另一端
				可接到电源3.3V或GND端子测试，在串口助手上将输出ADC检测的
				AD值和电压。核心板上D1指示灯闪烁
*******************************************************************************/

#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "adc.h"

#define USART_TX_LEN 11	//数据包大小
uint8_t USART_TX_BUF[USART_TX_LEN];	//数据包缓存区
int k;
float vol;
u32 time=0;
u32 flagtime = 0; //判断最后触发时间
u32 historytime[] = {0,0,0,0,0,0,0,0,0,0};
/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void Float_to_Byte(float f,uint8_t *byte)
{
	unsigned long longdata = 0;
	longdata = *(unsigned long*)&f;          
	byte[3] = (longdata & 0xFF000000) >> 24;
	byte[2] = (longdata & 0x00FF0000) >> 16;
	byte[1] = (longdata & 0x0000FF00) >> 8;
	byte[0] = (longdata & 0x000000FF);
}

int ble_send(uint8_t *data,int len)
{
  for(k=0;k<USART_TX_LEN;k++)
	{
		USART_SendData(USART1,(u8)USART_TX_BUF[k]);	
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);
	}
	return 0;
}

void send(float vol, float heartrate)
{
	USART_TX_BUF[0] = 0xA5;
	Float_to_Byte(vol,&USART_TX_BUF[1]);
	Float_to_Byte(heartrate,&USART_TX_BUF[5]);
	USART_TX_BUF[9]=(uint8_t) ((USART_TX_BUF[1]+USART_TX_BUF[2]+USART_TX_BUF[3]+USART_TX_BUF[4]+USART_TX_BUF[5]+USART_TX_BUF[6]+USART_TX_BUF[7]+USART_TX_BUF[8])&0xff);
	USART_TX_BUF[10] = 0x5A;
	
	/*
	for(k=0;k<7;k++)
	{
		USART_SendData(USART1,(u8)USART_TX_BUF[k]);	
	}
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	*/
	
	ble_send(USART_TX_BUF,USART_TX_LEN);

}



void TIM4_Init(u16 per,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);//使能TIM4时钟
	
	TIM_TimeBaseInitStructure.TIM_Period=per;   //自动装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc; //分频系数
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //设置向上计数模式
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //开启定时器中断
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;//定时器中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4,ENABLE); //使能定时器	
}

void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4,TIM_IT_Update))
	{
		//led2=!led2;
		time++;
		
	}
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);	
}


int main()
{
	u8 i=0;
	u16 value=0;
	
	u8 cnt = 0;
	u8 k = 0;
	u8 flag = 0;
	u16 timeaverage = 0;
	float heartingrate = 0;
	
	TIM4_Init(7200-1,15-1);//不知为何要乘1.5 
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组 分2组
	LED_Init();
	USART1_Init(6400);
	ADCx_Init();
	
	while(1)
	{
		i++;
		/*
		if(i%20==0)
		{
			led1=!led1;
		}
		*/
		
		if(i%2 == 0)
		{
			value=Get_ADC_Value(ADC_Channel_1,5);
			//printf("检测AD值为：%d\r\n",value);
			vol=(float)value*(3.3/4096);
			//检测电压值：
			//printf("vol=%.2f,flag=%d\r\n,cnt=%d\r\n",vol,flag,cnt);
			//printf("%d\r\n",time);
			send(vol,heartingrate);
		}
		delay_us(100);	
		
//心率计算部分
		if(vol>2.5 && flag == 0)	//波型第一次高于3（类似上升沿触发）
		{
				cnt++;
				flag = 1;
				historytime[cnt] = time;
			  flagtime = time;
				
			if(cnt>=10)
				{
					for(k=0;k<9;k++)
						{
								historytime[k] = historytime[k+1];
						}
						historytime[9] = time;
						cnt = 9;
				}
			if(cnt<=3) timeaverage = (historytime[cnt] - historytime[0])/cnt;
			else if(cnt>3) timeaverage = 0.5*(historytime[cnt]-historytime[cnt-1])+0.3*(historytime[cnt-1]-historytime[cnt-2])+0.2*(historytime[cnt-2] - historytime[0])/(cnt-2);	 
				heartingrate = (int)(60*1000/timeaverage);
			
		}  

		if(vol<2.5 && flag == 1)
		{
				flag = 0;
		}
		
		if((time-flagtime)>5*1000)  
		{
			historytime[0] = flagtime+5*1000;
			for(k=1; k<10; k++)
				historytime[k] = 0; 
			cnt = 0;
			heartingrate = 0;
		}
		
		
			
//		if(i%50 == 0)  
//		{
//			if(heartingrate<40)  printf("TESTING\r\n");
//			else printf("vol=%.2f, flag=%d, cnt=%d, heartrate=%d\r\n",vol,flag,cnt,heartingrate);
//		}
		
	}
}