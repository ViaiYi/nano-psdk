#include "Speaker.h"

uint8_t SYS6288Rx = 0;
void SYN_FrameInfo(uint8_t *HZdata)
{
/****************SYN8086**********************************/ 
		 unsigned  char Frame_Info[150]; 
		 unsigned  int  HZ_Length;  
		 //uint8_t word;
	     uint8_t i=0; 
		 HZ_Length =strlen(HZdata); 			
 		 
/*****************头帧**************************************/           
		 Frame_Info[0] = 0xFD ; 			
		 Frame_Info[1] = 0x00 ; 			
		 Frame_Info[2] = HZ_Length + 2; 		
		 Frame_Info[3] = 0x01 ; 				 		 
		 Frame_Info[4] = 0x05;       
 	  
     memcpy(&Frame_Info[5], HZdata, HZ_Length);
		 Usart_SendString((uint8_t *)Frame_Info,5+HZ_Length);
     //vTaskDelay(150 * (5 + HZ_Length));
		 //for(i=0;i<HZ_Length;i++){
			//word = Frame_Info[i];
			//Usart_SendString(word);
		 //}
		 //Usart_SendString(&Frame_Info); 
}

// void SYN_FrameInfo(u8 Music, u8 *HZdata)
// {
//   /****************需要发送的文本**********************************/
//   unsigned  char  Frame_Info[50];
//   unsigned  char  HZ_Length;
//   unsigned  char  ecc  = 0;       //定义校验字节
//   unsigned  int i = 0;
//   HZ_Length = strlen((char*)HZdata);      //需要发送文本的长度

//   /*****************帧固定配置信息**************************************/
//   Frame_Info[0] = 0xFD ;      //构造帧头FD
//   Frame_Info[1] = 0x00 ;      //构造数据区长度的高字节
//   Frame_Info[2] = HZ_Length + 3;    //构造数据区长度的低字节
//   Frame_Info[3] = 0x01 ;      //构造命令字：合成播放命令
//   Frame_Info[4] = 0x01 | Music << 4 ; //构造命令参数：背景音乐设定

//   /*******************校验码计算***************************************/
//   for(i = 0; i < 5; i++)          //依次发送构造好的5个帧头字节
//   {
//     ecc = ecc ^ (Frame_Info[i]);    //对发送的字节进行异或校验
//   }

//   for(i = 0; i < HZ_Length; i++)      //依次发送待合成的文本数据
//   {
//     ecc = ecc ^ (HZdata[i]);        //对发送的字节进行异或校验
//   }
//   /*******************发送帧信息***************************************/
//   memcpy(&Frame_Info[5], HZdata, HZ_Length);
//   Frame_Info[5 + HZ_Length] = ecc;
//   SendStr5(Frame_Info, 5 + HZ_Length + 1);
// }



UART_HandleTypeDef UartHandle;
 
void DEBUG_USART_Config(void)
{ 
  HAL_UART4_MspInit(UartHandle);
	
	
  UartHandle.Instance          = DEBUG_USART;
  
  UartHandle.Init.BaudRate     = DEBUG_USART_BAUDRATE;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  
  HAL_UART_Init(&UartHandle);
    
  HAL_UART_Receive_IT(&UartHandle, &SYS6288Rx, 1);
  //__HAL_UART_ENABLE_IT(&UartHandle,UART_IT_RXNE);  
	
	
}



void HAL_UART4_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  DEBUG_USART_CLK_ENABLE();
	
  DEBUG_USART_RX_GPIO_CLK_ENABLE();
  DEBUG_USART_TX_GPIO_CLK_ENABLE();
  
/**USART1 GPIO Configuration    
  PA10     ------> USART1_TX
  PA11    ------> USART1_RX 
  */
  /* ����Tx����Ϊ���ù���  */
  GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = DEBUG_USART_TX_AF;
  HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);
  
  /* ����Rx����Ϊ���ù��� */
  GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
  //GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  //GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = DEBUG_USART_RX_AF;
  HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct); 
 
  
  HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ );		    //ʹ��USART1�ж�ͨ��  
  HAL_NVIC_SetPriority(DEBUG_USART_IRQ ,1,1);	//��ռ���ȼ�0�������ȼ�1
}


/*****************  �����ַ��� **********************/
void Usart_SendString(uint8_t *str,uint8_t size)
{
	uint8_t k=0;
  do 
  {
      HAL_UART_Transmit(&UartHandle,(uint8_t *)(str + k) ,1,1000);
      k++;
  } while(k < size);
  
}
///�ض���c�⺯��printf������DEBUG_USART���ض�����ʹ��printf����
int fputc1(int ch, FILE *f)
{
	/* ����һ���ֽ����ݵ�����DEBUG_USART */
	HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 1000);	
	
	return (ch);
}

///�ض���c�⺯��scanf������DEBUG_USART����д����ʹ��scanf��getchar�Ⱥ���
int fgetc1(FILE *f)
{
		
	int ch;
	HAL_UART_Receive(&UartHandle, (uint8_t *)&ch, 1, 1000);	
	return (ch);
}

//音量控制
void Volinme(uint8_t Y_L){
  unsigned  char Frame_Info[50]; 
		 unsigned  int  HZ_Length;  
		 //uint8_t word;
	     uint8_t i=0; 
		 //HZ_Length =strlen(HZdata); 			
 		 
/*****************头帧**************************************/           
		 Frame_Info[0] = 0xFD ; 			
		 Frame_Info[1] = 0x00 ; 			
		 Frame_Info[2] = 0x06 ; 		
		 Frame_Info[3] = 0x01 ; 				 		 
		 Frame_Info[4] = 0x01 ;
     Frame_Info[5] = 0x5B ;
     Frame_Info[6] = 0x76 ;
     Frame_Info[7] = Y_L+48;
     Frame_Info[8] = 0x5D ; 
 	  
     //memcpy(&Frame_Info[5], HZdata, HZ_Length);
		 Usart_SendString((uint8_t *)Frame_Info,sizeof(Frame_Info));
     //USER_LOG_INFO("%d\n",Frame_Info[7]);
     //vTaskDelay(1000);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  if(huart->Instance == DEBUG_USART) // 仅处理UART5串口的接收中断
    {
      HAL_UART_Receive_IT(&UartHandle, &SYS6288Rx, 1);//启动下一次接收  
    }   
}

void talker(uint8_t Y_L){
  unsigned  char Frame_Info[50]; 
		 unsigned  int  HZ_Length;  
		 //uint8_t word;
	     uint8_t i=0; 
		 //HZ_Length =strlen(HZdata); 			
 		 
/*****************头帧**************************************/           
		 Frame_Info[0] = 0xFD ; 			
		 Frame_Info[1] = 0x00 ; 			
		 Frame_Info[2] = 0x07 ; 		
		 Frame_Info[3] = 0x01 ; 				 		 
		 Frame_Info[4] = 0x01 ;
     Frame_Info[5] = 0x5B ;
     Frame_Info[6] = 0x6D ;
     Frame_Info[7] = 0x35 ;
     Frame_Info[8] = Y_L+48;
     Frame_Info[9] = 0x5D ; 
 	  
     //memcpy(&Frame_Info[5], HZdata, HZ_Length);
		 Usart_SendString((uint8_t *)Frame_Info,sizeof(Frame_Info));
     //USER_LOG_INFO("%d\n",Frame_Info[7]);
     //vTaskDelay(1000);
}

/*********************************************END OF FILE**********************/



// void Usart_SYN6288_SendByte( uint16_t * pUSARTx, uint8_t ch){
//   HAL_UART_Transmit(pUSARTx,ch);
//   while (__HAL_UART_GET_FLAG(pUSARTx, UART_FLAG_TXE) == RESET);

// }

// void Usart_SSYN6288_endString( uint16_t * pUSARTx, char *str)
// {
// 	unsigned int k=0;
//   do 
//   {
//       Usart_SYN6288_SendByte( pUSARTx, *(str + k) );
//       k++;
//   } while(*(str + k)!='\0');
  
//   /* µÈ´ý·¢ËÍÍê³É */
//   while(HAL_UART_GetState(pUSARTx)==RESET)
//   {}
// }

// void SYN6288_Speech( uint16_t * pUSARTx,char * str )
// {
	
// 	if(SYN6288_Read_GPIO_IN()==Bit_RESET)/* x usºóÈÔÎª¸ßµçÆ½±íÊ¾Êý¾Ý¡°1¡± */
// 	{

// 				char * p = str;
// 		int len = 0,check=0xFD,i;
		
// 		while( *p++ != 0 )
// 		{
// 				len++;
		
// 		}
		
// 		len+=3;
		
// 		Usart_SYN6288_SendByte(DEBUG_USART_SYN6288,0xFD);
		
// 		Usart_SYN6288_SendByte( DEBUG_USART_SYN6288,len / 256 );
// 		Usart_SYN6288_SendByte( DEBUG_USART_SYN6288,len % 256 );
// 		check  = check ^ ( len / 256 ) ^ ( len % 256 );
		
// 		Usart_SYN6288_SendByte( DEBUG_USART_SYN6288,0x01 );
// 		Usart_SYN6288_SendByte( DEBUG_USART_SYN6288,0x01 );
// 		check = check ^ 0x01 ^ 0x01;
		
// 		for( i = 0; i < len-3; i++ )
// 		{
// 				Usart_SYN6288_SendByte(DEBUG_USART_SYN6288,*str);
// 				check ^= ( *str );
// 				str++;
// 		}
// 		Usart_SYN6288_SendByte(DEBUG_USART_SYN6288,check);   
				
// 		rt_thread_delay(150*len);	
// 	}	

// }



//ÓïÒôºÏ³É
// void SYN6288_Speech( USART_TypeDef * pUSARTx,char * str )
// {
	
// 	if(SYN6288_Read_GPIO_IN()==Bit_RESET)/* x usºóÈÔÎª¸ßµçÆ½±íÊ¾Êý¾Ý¡°1¡± */
// 	{

// 				char * p = str;
// 		int len = 0,check=0xFD,i;
		
// 		while( *p++ != 0 )
// 		{
// 				len++;
		
// 		}
		
// 		len+=3;
		
// 		Usart_SYN6288_SendByte(DEBUG_USART_SYN6288,0xFD);
		
// 		Usart_SYN6288_SendByte( DEBUG_USART_SYN6288,len / 256 );
// 		Usart_SYN6288_SendByte( DEBUG_USART_SYN6288,len % 256 );
// 		check  = check ^ ( len / 256 ) ^ ( len % 256 );
		
// 		Usart_SYN6288_SendByte( DEBUG_USART_SYN6288,0x01 );
// 		Usart_SYN6288_SendByte( DEBUG_USART_SYN6288,0x01 );
// 		check = check ^ 0x01 ^ 0x01;
		
// 		for( i = 0; i < len-3; i++ )
// 		{
// 				Usart_SYN6288_SendByte(DEBUG_USART_SYN6288,*str);
// 				check ^= ( *str );
// 				str++;
// 		}
// 		Usart_SYN6288_SendByte(DEBUG_USART_SYN6288,check);   
				
// 		rt_thread_delay(150*len);	
// 	}	

// }
