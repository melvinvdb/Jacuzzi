#include "Computer.h"
#include "usart.h"
#include <stdio.h>

char rxBuffer[BUFFER_SIZE+1];
volatile int rxInIndex=0;
volatile int rxOutIndex=0;
char rxLine[BUFFER_SIZE+1];
bool responseAvailable=false;

void Computer::Init()
{

}

bool Computer::RegisterForCallback(Keys& callback)
{
	if (interruptListIndex > INTERRUPT_LIST_LENGTH)
	{
		return false;
	}
	interruptList[interruptListIndex] = InterruptCallback(&callback, &Keys::KeyDataReceived);
	++interruptListIndex;
	return true;
}

void Computer::CheckIncomingData()
{
	if (responseAvailable == true)
	{
		printf("Resp: %s\r\n", rxLine);
		responseAvailable = false;
	}
}

#ifdef __cplusplus
 extern "C" {
#endif

void ReadLine()
{
    int i = 0;
    // Start Critical Section - don't interrupt while changing global buffer variables
    USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    // Loop reading rx buffer characters until end of line character. Uses \r\n for line termination
    while ((i==0) || (rxLine[i-1] != '\n')) {
        rxLine[i] = rxBuffer[rxOutIndex];
        i++;
        rxOutIndex = (rxOutIndex + 1) % BUFFER_SIZE;
    }
    // End Critical Section
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    rxLine[i] = 0; //i-1 == \n
    responseAvailable = true;
}

/**********************************************************
 * USART1 interrupt request handler: on reception of a
 * character 't', toggle LED and transmit a character 'T'
 *********************************************************/
void USART1_IRQHandler(void)
{
    /* RXNE handler */
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
    	char c = (char)USART_ReceiveData(USART1);
        rxBuffer[rxInIndex] = c;
        rxInIndex = (rxInIndex + 1) % BUFFER_SIZE;
        if (c == '\n')
        {
        	ReadLine();
        }
    }
}

#ifdef __cplusplus
 }
#endif
