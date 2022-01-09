#include "recorder.h"
#include "malloc.h"
#include "usart.h"
#include "es8388.h"
#include "sai.h"
#include "led.h"
#include "lcd.h"
#include "delay.h"
#include "arm_math.h"

arm_cfft_radix2_instance_f32 scfft;
		
const u16 saiplaybuf[2] = {0X0000, 0X0000}; //2个16位数据,用于录音时SAI Block A主机发送.循环发送0.

#define FFT_LENGTH		1024 		//FFT长度，默认是1024点FFT

u8 *sairecbuf;			//SAI1 DMA接收BUF1

u8 data_buf[SAI_RX_DMA_BUF_SIZE]={0};
u8 data_flag=0;

int symbol_tr(u16 num)
{
if(num & 0x80){return -(0x7F&(~(num-1)));}
else{return num;}
}

/**
 * @brief	录音 SAI_DMA接收中断服务函数.在中断里面写入数据
 * @return  void
 */
void rec_sai_dma_rx_callback(u16 data)
{
		SAI_Rec_Stop();
//    for(i = 0; i < SAI_RX_DMA_BUF_SIZE; i++)saififobuf[sairecfifowrpos][i] = sairecbuf[i + data]; //拷贝数据
    //for(i = 0; i < SAI_RX_DMA_BUF_SIZE; i++)data_buf[i] = *(sairecbuf + i + data); //拷贝数据
		data_flag++;
}

/**
 * @brief	进入录音模式
 */
void recoder_enter_rec_mode(void)
{
    ES8388_ADDA_Cfg(1, 0);		//开启ADC关闭DAC
    ES8388_Input_Cfg(0);		//ADC通道1输入
    ES8388_Output_Cfg(1);		//DAC通道2输出,相当于关闭音频输出

    ES8388_I2S_Cfg(0, 3); //飞利浦标准,16位数据长度

    SAIA_Init(SAI_MODEMASTER_TX, SAI_CLOCKSTROBING_RISINGEDGE, SAI_DATASIZE_16, REC_SAMPLERATE); //SAI1 Block A,主发送,16位数据
    SAIB_Init(SAI_MODESLAVE_RX, SAI_CLOCKSTROBING_RISINGEDGE, SAI_DATASIZE_16);	//SAI1 Block B从模式接收,16位
    SAIA_TX_DMA_Init(1);	//配置TX DMA,16位
    __HAL_DMA_DISABLE_IT(&SAI1_TXDMA_Handler, DMA_IT_TC); //关闭传输完成中断(这里不用中断送数据)
    SAIB_RX_DMA_Init(1);//配置RX DMA
    sai_rx_callback = rec_sai_dma_rx_callback;//初始化回调函数指sai_rx_callback

    HAL_SAI_Transmit(&SAI1A_Handler, (u8 *)&saiplaybuf[0], 2, 0);

    HAL_SAI_Receive_DMA(&SAI1B_Handler, sairecbuf, SAI_RX_DMA_BUF_SIZE);

    SAI_Rec_Start();			//开始SAI数据发送(主机)，为SAI1B通道提供时钟
}

//输入的值为0~
void display_columu_fft(u8 *adc_buf)
{
	float fft_inputbuf[FFT_LENGTH * 2];	//FFT输入数组
	float fft_outputbuf[FFT_LENGTH * 2];	//FFT输出数组

	u16 i=0,j=0,cache=0;
	u32 count=0;
	static u8 fl=50;
	static int column_state_buf_now[32]={0};//上次的柱子高度
	static int column_state_buf[32]={0};//上次的柱子高度
	static int block_state_buf[32]={0};//上次的方块高度
	
	for(i=0;i<FFT_LENGTH;i++)
	{
		
		cache=*(adc_buf+(4*i+1));
		cache<<=8;
		cache=*(adc_buf+(4*i));
		fft_inputbuf[2*i]=symbol_tr(cache);
		fft_inputbuf[2*i+1]=0;
	}
//		printf("\r\ndata:");
//  	for(i=0; i<16; i++)
//		{
//			printf("%d,",*(adc_buf+i));
//		}
//			for(i=0; i<16; i++)
//		{
//			printf("%f,",fft_inputbuf[i]);
//		}
	 arm_cfft_radix2_f32(&scfft, fft_inputbuf);	//FFT计算（基2）
	 arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);	//把运算结果复数求模得幅值
	//调试输出
//	if(fl != 0){fl--;}
//	if(fl == 1)
//	{
//		printf("FFT Result:\r\n");
//		for(i = 0; i < FFT_LENGTH; i++)
//		{
//				printf("fft_outputbuf[%d]:%f\r\n", i, fft_outputbuf[i]);
//		}
//		fl=0;
//	}
	//分为32个频段
	for(i=0;i<32;i++)
	{
		count=0;
		for(j=0;j<((FFT_LENGTH/2)/32);j++)
		{
			count=count+fft_outputbuf[((FFT_LENGTH/2)/32)*i+j];//注意数列是对称分布，只需要512个
			//实际上低频高频不是等分的，后续再加这部分
			//低频：20HZ~150HZ
			//中频：150HZ~5KHZ
			//高频：5KHZ~20KHZ
		}
		count/=16;//平均值
		count/=(160);//缩小160倍
		if(count > 100){count = 100;}
		column_state_buf_now[i]=count*1.4;//根据实际显示效果微调
	}
//		printf("\r\nFFT Result:");
//		for(i = 0; i < 32; i++)
//		{
//				printf("%d,", column_state_buf_now[i]);
//		}
	LCD_Clear(0x1863);
	for(i=0; i<32; i++)
	{
		//绘制新的柱状图。
		LCD_Fill((i+1)*7,200-(column_state_buf_now[i]),(i+2)*7-2,200,GRAYBLUE);
		//如果色块高度小于新的柱状图高度则设置高度为现在的值+1，否则进行判断：如果色块高度等于新的高度则不变化，否则减一，然后绘制色块；
		if(column_state_buf_now[i] > column_state_buf[i])//如果这次比上次还高，就将色块位置设置为新的高度
		{
			block_state_buf[i] = column_state_buf_now[i];
		}
		else//如果这次和上次一样或比上次的低，判断色块是否到底，然后色块高度减一个单位，否则不变
		{
			if(block_state_buf[i] > column_state_buf[i])
			{
				block_state_buf[i] -= 1;//方块掉落速度
			}
		}
		LCD_Fill((i+1)*7,200-(block_state_buf[i])-6,(i+2)*7-2,200-(block_state_buf[i])-2,YELLOW);
		//存储上次的色块高度。
		column_state_buf[i] = column_state_buf_now[i];
	}
}

/**
 * @brief	WAV录音
 */
void wav_recorder(void)
{
    arm_cfft_radix2_init_f32(&scfft, FFT_LENGTH, 0, 1); //初始化scfft结构体，设定FFT相关参数
    sairecbuf = mymalloc(SRAM1, SAI_RX_DMA_BUF_SIZE); //SAI录音内存申请
		recoder_enter_rec_mode();	//进入录音模式,此时耳机可以听到咪头采集到的音频0

		while(1)
		{
			if(data_flag != 0)
			{
				//printf("write OK:%d\r\n",data_flag);
				display_columu_fft(sairecbuf);
				recoder_enter_rec_mode();
				data_flag=0;
			}
		}
}


