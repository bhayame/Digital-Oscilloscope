/*Brandon Hayame
bhayame@ucsc.edu
CMPE 121 FALL 19

This program varies the baudrate between two UARTs to determine the maximum deviation in clock speed the UART
may posess and still be useful.
*/

#include "project.h"
#define BLOCK_SIZE 256

static uint8 transmitArray[BLOCK_SIZE - 1], receiveArray[BLOCK_SIZE - 1] = {0};
static uint16 transmitCount = 0, receiveCount = 0, mismatchCount = 0, errorFree = 0;


CY_ISR(primarytxISR){
    while((primaryUART_ReadTxStatus() & primaryUART_TX_STS_FIFO_NOT_FULL) == primaryUART_TX_STS_FIFO_NOT_FULL){
        primaryUART_PutChar(transmitArray[transmitCount]);
        transmitCount++;
        if(transmitCount == BLOCK_SIZE){
            transmitCount = 0;
        }
    }
}

CY_ISR(primaryrxISR){
    if((primaryUART_ReadRxStatus() & primaryUART_RX_STS_PAR_ERROR) != primaryUART_RX_STS_PAR_ERROR){
        if((primaryUART_ReadRxStatus() & primaryUART_RX_STS_STOP_ERROR) != primaryUART_RX_STS_STOP_ERROR){
            errorFree = 1;
        }
    }
    if(errorFree == 1){
        while((primaryUART_ReadRxStatus() & primaryUART_RX_STS_FIFO_NOTEMPTY) == primaryUART_RX_STS_FIFO_NOTEMPTY){
            receiveArray[receiveCount] = primaryUART_GetChar();
            if(receiveArray[receiveCount] != transmitArray[receiveCount]){
                mismatchCount++;
            }
            receiveCount++;
            if(receiveCount == BLOCK_SIZE){
                receiveCount = 0;
            }
        }
    }
}

CY_ISR(secondaryrxISR){
    while((secondaryUART_ReadRxStatus() & secondaryUART_RX_STS_FIFO_NOTEMPTY) == secondaryUART_RX_STS_FIFO_NOTEMPTY){
    secondaryUART_PutChar(secondaryUART_GetChar());
    }
}


int main(void)
{
    int i;
    for (i = 0; i<BLOCK_SIZE; i++){
        transmitArray[i] = i%256;
    }
    
    CyGlobalIntEnable;
    primaryUART_Start();
    secondaryUART_Start();
    primarytxInt_StartEx(primarytxISR);
    primaryrxInt_StartEx(primaryrxISR);
    secondaryrxInt_StartEx(secondaryrxISR);
    LCD_Char_Start();
    for(;;)
    {
        LCD_Char_ClearDisplay();
        LCD_Char_PrintString("MISMATCHES:");
        LCD_Char_PrintNumber(mismatchCount);
        CyDelay(100);
    }
}