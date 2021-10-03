/*******************************************************************************
* ʵ������  	: ADCģ��ת��ʵ��
* ʵ��˵��  : 
* ����˵��  : ���ⲿ0-3.3V��ģ���źŽ��뵽��Ƭ��������P11��PA1���ڡ�
						
* ʵ������	: ʹ��һ���Ű��ߣ�һ�˽ӵ���Ƭ��������P11��PA1���ڣ���һ��
				�ɽӵ���Դ3.3V��GND���Ӳ��ԣ��ڴ��������Ͻ����ADC����
				ADֵ�͵�ѹ�����İ���D1ָʾ����˸
*******************************************************************************/

#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "adc.h"

#define USART_TX_LEN 11	//���ݰ���С
uint8_t USART_TX_BUF[USART_TX_LEN];	//���ݰ�������
int k;
float vol;
u32 time=0;
u32 flagtime = 0; //�ж���󴥷�ʱ��
u32 historytime[] = {0,0,0,0,0,0,0,0,0,0};
/*******************************************************************************
* �� �� ��         : main
* ��������		   : ������
* ��    ��         : ��
* ��    ��         : ��
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
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);//ʹ��TIM4ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Period=per;   //�Զ�װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc; //��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //�������ϼ���ģʽ
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE); //������ʱ���ж�
	TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;//��ʱ���ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4,ENABLE); //ʹ�ܶ�ʱ��	
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
	
	TIM4_Init(7200-1,15-1);//��֪Ϊ��Ҫ��1.5 
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //�ж����ȼ����� ��2��
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
			//printf("���ADֵΪ��%d\r\n",value);
			vol=(float)value*(3.3/4096);
			//����ѹֵ��
			//printf("vol=%.2f,flag=%d\r\n,cnt=%d\r\n",vol,flag,cnt);
			//printf("%d\r\n",time);
			send(vol,heartingrate);
		}
		delay_us(100);	
		
//���ʼ��㲿��
		if(vol>2.5 && flag == 0)	//���͵�һ�θ���3�����������ش�����
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