#include "sys.h"
#include "usart.h"
#include "sai.h"
#include "delay.h"
#include "led.h"
#include "lcd.h"
#include "malloc.h"
#include "fontupd.h"
#include "text.h"
#include "es8388.h"
#include "recorder.h"
#include "arm_math.h"

//static u8 fft_buf[16]={0,12,24,31,99,80,75,60,30,10,0,0,0,23,45};//������

//�����ֵΪ0~100
//void display_columu(u8 new_buf[])
//{
//	u8 i=0;
//	static u8 column_state_buf[16]={0};//�ϴε����Ӹ߶�
//	static u8 block_state_buf[16]={0};//�ϴεķ���߶�
//	
//	LCD_Clear(0x1863);
//	for(i=0; i<16; i++)
//	{
//		//�����µ���״ͼ��
//		LCD_Fill((i+4)*10,200-(new_buf[i]),(i+5)*10-3,200,GRAYBLUE);
//		//���ɫ��߶�С���µ���״ͼ�߶������ø߶�Ϊ���ڵ�ֵ+1����������жϣ����ɫ��߶ȵ����µĸ߶��򲻱仯�������һ��Ȼ�����ɫ�飻
//		if(new_buf[i] > column_state_buf[i])//�����α��ϴλ��ߣ��ͽ�ɫ��λ������Ϊ�µĸ߶�
//		{
//			block_state_buf[i] = new_buf[i];
//		}
//		else//�����κ��ϴ�һ������ϴεĵͣ��ж�ɫ���Ƿ񵽵ף�Ȼ��ɫ��߶ȼ�һ����λ�����򲻱�
//		{
//			if(block_state_buf[i] > column_state_buf[i])
//			{
//				block_state_buf[i] -= 4;
//			}
//		}
//		LCD_Fill((i+4)*10,200-(block_state_buf[i])-6,(i+5)*10-3,200-(block_state_buf[i])-2,YELLOW);
//		//�洢�ϴε�ɫ��߶ȡ�
//		column_state_buf[i] = new_buf[i];
//	}
//}

int main(void)
{
    u8 i=0xF7;
	
    HAL_Init();
    SystemClock_Config();		//��ʼ��ϵͳʱ��Ϊ80M
    delay_init(80); 			//��ʼ����ʱ����    80Mϵͳʱ��
    uart_init(115200);			//��ʼ�����ڣ�������Ϊ115200

    LED_Init();					//��ʼ��LED
    LCD_Init();					//��ʼ��LCD

    my_mem_init(SRAM1);			//��ʼ���ڲ�SRAM1�ڴ��
    my_mem_init(SRAM2);			//��ʼ���ڲ�SRAM2�ڴ��

    ES8388_Init();				//ES8388��ʼ��
    ES8388_Set_Volume(33);		//���ö���������С

		LCD_Clear(0x1863);
        
    while(1)
    {
			printf("open:%d,%d\r\n",symbol_tr(i),-90);
			wav_recorder();
//      		if(rec_sai_fifo_read(&pdatabuf))//����������
//					{
							//�����ݽ��д���
//						for(i=0;i<1024;i++)
//						{
//							fft_inputbuf_16[i]=pdatabuf[4*i];
//							fft_inputbuf_16[i]<<=8;
//							fft_inputbuf_16[i]=pdatabuf[4*i+1];
//						}
//						printf("\r\nget data:");
//							for(i=0;i<8;i++){printf("%d,",fft_inputbuf_16[i]);}//��ȡ��������,��С1KB
							//get data:208,255,208,255,192,255,192,255,209,255,209,255,226,255,226,255,
//           arm_cfft_radix4_f32(&scfft, &pdatabuf);	//FFT���㣨��4��
//           arm_cmplx_mag_f32(&pdatabuf, fft_outputbuf, FFT_LENGTH);	//��������������ģ�÷�ֵ
//					}  
			//Ƶ������ʱ�Ὣ�Ѿ���д��ϵ�����һ�����Ƴ���������FFT���㡣ÿ10ms��ȡһ��ǰ��10ms����Ƶ
//				display_columu(fft_buf);
//				delay_ms(200);
//				for(i=0; i<16; i++)
//				{			
//				fft_buf[i]=1;
//				}
    }
}


