#include "uart_communication_interface.h"

//  16-bit CRC (based on Modbus RTU)
uint16_t UartCommunicationInterface::CRC16(const uint8_t *nData, uint16_t wLength)
{
  static const uint16_t wCRCTable[] =
  {
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
  };

  uint8_t nTemp;
  uint16_t wCRCWord = 0xFFFF;

  while (wLength--)
  {
     nTemp = *nData++ ^ wCRCWord;
     wCRCWord >>= 8;
     wCRCWord ^= wCRCTable[nTemp];
  }
  return wCRCWord;
}

void UartCommunicationInterface::HardwareInit()
{

	// zalaczenie portu PA
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// zalaczenie zegara bloku USART1
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	// zalaczenie zegara kontrolera DMA1
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	// zalaczenie remapowania
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	// PA9 - UART1_TX (out), PA10 - UART1_RX (in)
	// brak remapowania linii wg dokumentacji ogólnej
	AFIO->MAPR &= ~AFIO_MAPR_USART1_REMAP;

	GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9 | GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
	GPIOA->CRH |= GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_1 | GPIO_CRH_CNF10_0;

	// Parametry transmisji
	USART1->BRR = (uint16_t)(36000/921.6);
	USART1->CR1 = USART_CR1_RE | USART_CR1_TE;
	USART1->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR;
	USART1->CR1 |= USART_CR1_UE;

	// DMA1: USART1 RX : stream 5,  USART1 TX : stream 4

	// RX - urzadzenie->pamiec, automatyczna inkrementacja wskaznika pamieci, znaki 8-bitowe, bufor cyckliczny
	DMA_USART_RX->CCR = DMA_CCRx_MINC | DMA_CCRx_CIRC;
	DMA_USART_RX->CNDTR = RX_BUF_SIZE;
	DMA_USART_RX->CPAR = (int) &(USART1->DR);
	DMA_USART_RX->CMAR = (int) rxBuf;

	//TX - pamiec->urzadzenie, automatyczna inkrementacja wskaznika pamieci
	DMA_USART_TX->CCR = DMA_CCRx_MINC | DMA_CCRx_DIR;
	DMA_USART_TX->CPAR = (int) &(USART1->DR);

	// zerowanie bitow statusowych (w tym flag b³êdów) kontrolera DMA1
	DMA1->IFCR = DMA_IFCR_CGIF4 | DMA_IFCR_CGIF5;

	// zezwolenie na obsluge przerwan od kanalu DMA
	DMA_USART_RX->CCR |= DMA_CCRx_EN;
}

void UartCommunicationInterface::Init()
{
	HardwareInit();

	isFrameSending = false;
	isFrameReceived = false;

	rxDmaCounterPrev = DMA_USART_RX->CNDTR;
	txData = txBuf + 4;
	txBuf[0] = txBuf[1] = 0xAA; txBuf[2] = 0x00;	// inicjalizacja poczatku ramki nadawanej
}

void UartCommunicationInterface::Send(uint16_t size)
{

	txBuf[3] = size;
	auto crc = CRC16(txBuf+3, size+1);
	txBuf[size+4] = crc >> 8;
	txBuf[size+5] = crc & 0xFF;

	// reset kanalu DMA
	DMA_USART_TX->CCR = 0;
	DMA_USART_TX->CCR = DMA_CCRx_MINC | DMA_CCRx_DIR;
	DMA_USART_TX->CNDTR = size + 6;
	DMA_USART_TX->CMAR = (int) txBuf;

	// czyszczenie bitow statusowych (wraz z flagami bledow) dla DMA1
	DMA1->IFCR = DMA_IFCR_CGIF4;
	USART1->SR &= ~(USART_SR_TC);

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


