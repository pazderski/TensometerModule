#include "uart_communication_interface.h"

void UartCommunicationInterface::HardwareInit()
{

	// zalaczenie portu PD
	RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
	// zalaczenie zegara bloku USART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	// zalaczenie zegara kontrolera DMA1
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	// zalaczenie remapowania
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	// PA2 - UART2_TX (out), PA3 - UART_RX (in)
	// brak remapowania linii wg dokumentacji ogólnej
	AFIO->MAPR &= ~AFIO_MAPR_USART2_REMAP;

	GPIOD->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
	GPIOD->CRL |= GPIO_CRL_CNF5_1 | GPIO_CRL_MODE5_0;

	// Parametry transmisji
	USART2->BRR = 0x138; //115,2kbs
	USART2->CR1 = USART_CR1_RE | USART_CR1_TE;
	USART2->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;
	USART2->CR1 |= USART_CR1_UE;

	// DMA1: USART2 RX : stream 6,  USART2 TX : stream 7

	// RX - urzadzenie->pamiec, automatyczna inkrementacja wskaznika pamieci, znaki 8-bitowe, bufor cyckliczny
	DMA_USART_RX->CCR = DMA_CCRx_MINC | DMA_CCRx_CIRC;
	DMA_USART_RX->CNDTR = RX_BUF_SIZE;
	DMA_USART_RX->CPAR = (int) &(USART2->DR);
	DMA_USART_RX->CMAR = (int) rxBuf;

	//TX - pamiec->urzadzenie, automatyczna inkrementacja wskaznika pamieci
	DMA_USART_TX->CCR = DMA_CCRx_MINC | DMA_CCRx_DIR;
	DMA_USART_TX->CPAR = (int) &(USART2->DR);

	// zerowanie bitow statusowych (w tym flag b³êdów) kontrolera DMA1
	DMA1->IFCR = DMA_IFCR_CGIF6 | DMA_IFCR_CGIF7;

	// zezwolenie na obsluge przerwan od kanalu DMA
	DMA_USART_RX->CCR |= DMA_CCRx_EN;
}

void UartCommunicationInterface::Init()
{
	HardwareInit();

	isFrameSending = false;
	isFrameReceived = false;

	rxDmaCounterPrev = DMA_USART_RX->CNDTR;
	txData = txBuf + 3;
	txBuf[0] = txBuf[1] = 0xAA; txBuf[2] = 0x00;	// inicjalizacja poczatku ramki nadawanej
}

void UartCommunicationInterface::Send(uint16_t size)
{
	// reset kanalu DMA
	DMA_USART_TX->CCR = 0;
	DMA_USART_TX->CCR = DMA_CCRx_MINC | DMA_CCRx_DIR;
	DMA_USART_TX->CNDTR = size + 3;
	DMA_USART_TX->CMAR = (int) txBuf;

	// czyszczenie bitow statusowych (wraz z flagami bledow) dla DMA1
	DMA1->IFCR = DMA_IFCR_CGIF7;
	USART2->SR &= ~(USART_SR_TC);

	isFrameSending = true;

	// zezwolenie na przerwanie od kanalu DMA
	DMA_USART_TX->CCR |= DMA_CCRx_TCIE;
	// zalaczenie kanalu DMA i start transmisji
	DMA_USART_TX->CCR |= DMA_CCRx_EN;
}

void UartCommunicationInterface::PeriodicUpdate()
{

  // sprawdzenie, czy ramka zostala obsluzona oraz czy nie zachodzi wysy³anie ramki - jesli nie to brak analizy nastepnej
  if (isFrameReceived || isFrameSending) return;

  // czytanie licznika kanalu DMA
  auto rxDmaCounter = DMA_USART_RX->CNDTR;

  // sprawdzenie, czy w buforze sa nowe dane
  if (rxDmaCounter != rxDmaCounterPrev) {

	  rxDmaCounterPrev = rxDmaCounter;
	  rxBufIndexWrite = RX_BUF_SIZE - rxDmaCounter;

	  // sprawdzenie, czy odbior ramki nie trwa zbyt dlugo
	  if (rxState != FR_IDLE)
	  {
		  clock++;
		  if (clock > RX_TIMEOUT)
		  {
			  // jesli czas jest przekroczony, to ramka jest ignorowana
			  rxState = FR_IDLE;
		  }
	  }

	  // przegladanie bufora odebranych znaków
	  while ((rxBufIndexWrite != rxBufIndexRead) && (!isFrameReceived))
	  {
		  // pobranie znaku z buforu DMA
		  auto c = rxBuf[rxBufIndexRead];

		  switch (rxState)
		  {
		  	  case FR_IDLE:
		  		  if (c == 0xAA)
		  		  {
		  			  rxState = FR_START_1;
		  			  clock = 0;
		  		  }
		  	  break;

		  	  case FR_START_1:
		  		  if (c == 0xAA)
		  		  {
		  			  rxState = FR_START_2;
		  		  }
		  		  else rxState = FR_IDLE;
		  	  break;

		  	  case FR_START_2:
		  		  if (c == 0x00)
		  		  {
		  			  rxState = FR_DATA;
		  			  rxDataIndex = 0;
		  		  }
		  		  else rxState = FR_IDLE;
		  	  break;

		  	  case FR_DATA:
		  		  rxData[rxDataIndex++] = c;
		  		  if (rxDataIndex == rxDataSize)
		  		  {
		  			  rxState = FR_IDLE;
		  			  isFrameReceived = 1;
		  		  }
		  	  break;
		  }
      rxBufIndexRead = (++rxBufIndexRead) & (RX_BUF_SIZE-1);
	  }
  }
}


