#pragma once

#include "stm32f10x.h"
#include "stm32f10x_dma_extra.h"

// powiazania kanalow DMA z nadajnikiem i odbiornikiem UART
#define DMA_USART_RX	DMA1_Channel6
#define DMA_USART_TX	DMA1_Channel7

class UartCommunicationInterface
{

	// definicje stanow automatu do odbioru ramek komunikacyjnych
	enum FsmState
	{
		FR_IDLE,
		FR_START_1,
		FR_START_2,
		FR_DATA
	};

	static uint16_t const RX_TIMEOUT = 50;	// definicja maksymalnego czasu przesylania danych w ramce
	static uint16_t const RX_BUF_SIZE = 256;
	static uint16_t const TX_BUF_SIZE = 256;

	static uint8_t const rxDataSize = 4;

	volatile uint8_t  rxDataIndex;
	volatile FsmState  rxState;

	volatile uint16_t rxBufIndexRead;
	uint16_t rxBufIndexWrite;

	uint16_t rxDmaCounterPrev;
	volatile uint32_t clock;

	uint8_t rxBuf[RX_BUF_SIZE];
	uint8_t txBuf[TX_BUF_SIZE];

	void HardwareInit();

public:

	uint8_t rxData[RX_BUF_SIZE];
	uint8_t * txData;

	volatile bool isFrameSending;
	volatile bool isFrameReceived;

	void Init(void);
	void PeriodicUpdate();
	void Send(uint16_t size);

	void IrqDma()
	{
		DMA1->IFCR  = DMA_IFCR_CTCIF7 | DMA_IFCR_CHTIF7 | DMA_IFCR_CTEIF7;
		DMA_USART_TX->CCR &= ~DMA_CCR1_EN;
		USART2->CR1 |= USART_CR1_TCIE;
	}

	void IrqTx()
	{
		if (USART2->SR & USART_SR_TC)
		{
		    USART2->CR1 &= ~USART_CR1_TCIE;
		    isFrameSending = false;
		}
	}
};

