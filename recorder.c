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
		
const u16 saiplaybuf[2] = {0X0000, 0X0000}; //2��16λ����,����¼��ʱSAI Block A��������.ѭ������0.

#define FFT_LENGTH		1024 		//FFT���ȣ�Ĭ����1024��FFT

u8 *sairecbuf;			//SAI1 DMA����BUF1

u8 data_buf[SAI_RX_DMA_BUF_SIZE]={0};
u8 data_flag=0;

int symbol_tr(u16 num)
{
if(num & 0x80){return -(0x7F&(~(num-1)));}
else{return num;}
}

/**
 * @brief	¼�� SAI_DMA�����жϷ�����.���ж�����д������
 * @return  void
 */
void rec_sai_dma_rx_callback(u16 data)
{
		SAI_Rec_Stop();
//    for(i = 0; i < SAI_RX_DMA_BUF_SIZE; i++)saififobuf[sairecfifowrpos][i] = sairecbuf[i + data]; //��������
    //for(i = 0; i < SAI_RX_DMA_BUF_SIZE; i++)data_buf[i] = *(sairecbuf + i + data); //��������
		data_flag++;
}

/**
 * @brief	����¼��ģʽ
 */
void recoder_enter_rec_mode(void)
{
    ES8388_ADDA_Cfg(1, 0);		//����ADC�ر�DAC
    ES8388_Input_Cfg(0);		//ADCͨ��1����
    ES8388_Output_Cfg(1);		//DACͨ��2���,�൱�ڹر���Ƶ���

    ES8388_I2S_Cfg(0, 3); //�����ֱ�׼,16λ���ݳ���

    SAIA_Init(SAI_MODEMASTER_TX, SAI_CLOCKSTROBING_RISINGEDGE, SAI_DATASIZE_16, REC_SAMPLERATE); //SAI1 Block A,������,16λ����
    SAIB_Init(SAI_MODESLAVE_RX, SAI_CLOCKSTROBING_RISINGEDGE, SAI_DATASIZE_16);	//SAI1 Block B��ģʽ����,16λ
    SAIA_TX_DMA_Init(1);	//����TX DMA,16λ
    __HAL_DMA_DISABLE_IT(&SAI1_TXDMA_Handler, DMA_IT_TC); //�رմ�������ж�(���ﲻ���ж�������)
    SAIB_RX_DMA_Init(1);//����RX DMA
    sai_rx_callback = rec_sai_dma_rx_callback;//��ʼ���ص�����ָsai_rx_callback

    HAL_SAI_Transmit(&SAI1A_Handler, (u8 *)&saiplaybuf[0], 2, 0);

    HAL_SAI_Receive_DMA(&SAI1B_Handler, sairecbuf, SAI_RX_DMA_BUF_SIZE);

    SAI_Rec_Start();			//��ʼSAI���ݷ���(����)��ΪSAI1Bͨ���ṩʱ��
}

//�����ֵΪ0~
void display_columu_fft(u8 *adc_buf)
{
	float fft_inputbuf[FFT_LENGTH * 2];	//FFT��������
	float fft_outputbuf[FFT_LENGTH * 2];	//FFT�������

	u16 i=0,j=0,cache=0;
	u32 count=0;
	static u8 fl=50;
	static int column_state_buf_now[32]={0};//�ϴε����Ӹ߶�
	static int column_state_buf[32]={0};//�ϴε����Ӹ߶�
	static int block_state_buf[32]={0};//�ϴεķ���߶�
	
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
	 arm_cfft_radix2_f32(&scfft, fft_inputbuf);	//FFT���㣨��2��
	 arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);	//��������������ģ�÷�ֵ
	//�������
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
	//��Ϊ32��Ƶ��
	for(i=0;i<32;i++)
	{
		count=0;
		for(j=0;j<((FFT_LENGTH/2)/32);j++)
		{
			count=count+fft_outputbuf[((FFT_LENGTH/2)/32)*i+j];//ע�������ǶԳƷֲ���ֻ��Ҫ512��
			//ʵ���ϵ�Ƶ��Ƶ���ǵȷֵģ������ټ��ⲿ��
			//��Ƶ��20HZ~150HZ
			//��Ƶ��150HZ~5KHZ
			//��Ƶ��5KHZ~20KHZ
		}
		count/=16;//ƽ��ֵ
		count/=(160);//��С160��
		if(count > 100){count = 100;}
		column_state_buf_now[i]=count*1.4;//����ʵ����ʾЧ��΢��
	}
//		printf("\r\nFFT Result:");
//		for(i = 0; i < 32; i++)
//		{
//				printf("%d,", column_state_buf_now[i]);
//		}
	LCD_Clear(0x1863);
	for(i=0; i<32; i++)
	{
		//�����µ���״ͼ��
		LCD_Fill((i+1)*7,200-(column_state_buf_now[i]),(i+2)*7-2,200,GRAYBLUE);
		//���ɫ��߶�С���µ���״ͼ�߶������ø߶�Ϊ���ڵ�ֵ+1����������жϣ����ɫ��߶ȵ����µĸ߶��򲻱仯�������һ��Ȼ�����ɫ�飻
		if(column_state_buf_now[i] > column_state_buf[i])//�����α��ϴλ��ߣ��ͽ�ɫ��λ������Ϊ�µĸ߶�
		{
			block_state_buf[i] = column_state_buf_now[i];
		}
		else//�����κ��ϴ�һ������ϴεĵͣ��ж�ɫ���Ƿ񵽵ף�Ȼ��ɫ��߶ȼ�һ����λ�����򲻱�
		{
			if(block_state_buf[i] > column_state_buf[i])
			{
				block_state_buf[i] -= 1;//��������ٶ�
			}
		}
		LCD_Fill((i+1)*7,200-(block_state_buf[i])-6,(i+2)*7-2,200-(block_state_buf[i])-2,YELLOW);
		//�洢�ϴε�ɫ��߶ȡ�
		column_state_buf[i] = column_state_buf_now[i];
	}
}

/**
 * @brief	WAV¼��
 */
void wav_recorder(void)
{
    arm_cfft_radix2_init_f32(&scfft, FFT_LENGTH, 0, 1); //��ʼ��scfft�ṹ�壬�趨FFT��ز���
    sairecbuf = mymalloc(SRAM1, SAI_RX_DMA_BUF_SIZE); //SAI¼���ڴ�����
		recoder_enter_rec_mode();	//����¼��ģʽ,��ʱ��������������ͷ�ɼ�������Ƶ0

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


