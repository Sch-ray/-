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

//static u8 fft_buf[16]={0,12,24,31,99,80,75,60,30,10,0,0,0,23,45};//测试用

//输入的值为0~100
//void display_columu(u8 new_buf[])
//{
//	u8 i=0;
//	static u8 column_state_buf[16]={0};//上次的柱子高度
//	static u8 block_state_buf[16]={0};//上次的方块高度
//	
//	LCD_Clear(0x1863);
//	for(i=0; i<16; i++)
//	{
//		//绘制新的柱状图。
//		LCD_Fill((i+4)*10,200-(new_buf[i]),(i+5)*10-3,200,GRAYBLUE);
//		//如果色块高度小于新的柱状图高度则设置高度为现在的值+1，否则进行判断：如果色块高度等于新的高度则不变化，否则减一，然后绘制色块；
//		if(new_buf[i] > column_state_buf[i])//如果这次比上次还高，就将色块位置设置为新的高度
//		{
//			block_state_buf[i] = new_buf[i];
//		}
//		else//如果这次和上次一样或比上次的低，判断色块是否到底，然后色块高度减一个单位，否则不变
//		{
//			if(block_state_buf[i] > column_state_buf[i])
//			{
//				block_state_buf[i] -= 4;
//			}
//		}
//		LCD_Fill((i+4)*10,200-(block_state_buf[i])-6,(i+5)*10-3,200-(block_state_buf[i])-2,YELLOW);
//		//存储上次的色块高度。
//		column_state_buf[i] = new_buf[i];
//	}
//}

int main(void)
{
    u8 i=0xF7;
	
    HAL_Init();
    SystemClock_Config();		//初始化系统时钟为80M
    delay_init(80); 			//初始化延时函数    80M系统时钟
    uart_init(115200);			//初始化串口，波特率为115200

    LED_Init();					//初始化LED
    LCD_Init();					//初始化LCD

    my_mem_init(SRAM1);			//初始化内部SRAM1内存池
    my_mem_init(SRAM2);			//初始化内部SRAM2内存池

    ES8388_Init();				//ES8388初始化
    ES8388_Set_Volume(33);		//设置耳机音量大小

		LCD_Clear(0x1863);
        
    while(1)
    {
			printf("open:%d,%d\r\n",symbol_tr(i),-90);
			wav_recorder();
//      		if(rec_sai_fifo_read(&pdatabuf))//读到数据了
//					{
							//对数据进行处理，
//						for(i=0;i<1024;i++)
//						{
//							fft_inputbuf_16[i]=pdatabuf[4*i];
//							fft_inputbuf_16[i]<<=8;
//							fft_inputbuf_16[i]=pdatabuf[4*i+1];
//						}
//						printf("\r\nget data:");
//							for(i=0;i<8;i++){printf("%d,",fft_inputbuf_16[i]);}//获取到了数据,大小1KB
							//get data:208,255,208,255,192,255,192,255,209,255,209,255,226,255,226,255,
//           arm_cfft_radix4_f32(&scfft, &pdatabuf);	//FFT计算（基4）
//           arm_cmplx_mag_f32(&pdatabuf, fft_outputbuf, FFT_LENGTH);	//把运算结果复数求模得幅值
//					}  
			//频谱运行时会将已经读写完毕的其中一个复制出来，进行FFT计算。每10ms读取一次前面10ms的音频
//				display_columu(fft_buf);
//				delay_ms(200);
//				for(i=0; i<16; i++)
//				{			
//				fft_buf[i]=1;
//				}
    }
}


