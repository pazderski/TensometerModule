#pragma once

#include "stm32f10x.h"

class Tensometer
{

	uint8_t cmdTxBuf[8];
	uint8_t cmdRxBuf[8];
	volatile uint8_t cmdSize;
	volatile uint8_t txIndex;
	volatile uint8_t rxIndex;
	volatile bool txIrqEnable;
	volatile bool rxIrqEnable;
	uint32_t raw_data=0x00;
	uint32_t c = 0x00;
	int fsmSubState;
	int iteracja=0;
	int error=0;
	uint16_t Offset;
	uint32_t Table[1000];
	int number;
	int i;



	// Definicje stanow automatu do obslugi akcelerometru
	enum FsmState
	{
		RESET,
		INIT_WAIT,
		WAIT,
		FILTER_CONFIG,
		DAC_CONFIG,
		MODE_CONFIG,
		READING_DATA
	};

	volatile uint16_t u16Data;

	FsmState fsmState=WAIT;


	static volatile unsigned long & SPI_CS()
	{
		return bitband_t m_BITBAND_PERIPH(&GPIOB->ODR, 12);
	}

	static volatile unsigned long & nRESET()
	{
		return bitband_t m_BITBAND_PERIPH(&GPIOB->ODR, 1);
	}


	void HardwareInit()
	{
		//Ustawienia PA4-7
		//Ustawiamy zegar
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

		//Zerujemy bity wyjsciowe
		GPIOB->ODR&= ~(GPIO_ODR_ODR0 | GPIO_ODR_ODR1 | GPIO_ODR_ODR12 | GPIO_ODR_ODR13 | GPIO_ODR_ODR14 | GPIO_ODR_ODR15);
		// zerujemy rejestry PA4-7 Mode,CNF
		GPIOB->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_MODE1);
		GPIOB->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_CNF13 |GPIO_CRH_CNF14 | GPIO_CRH_CNF15 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15);
		GPIOB->CRL |= GPIO_CRL_MODE1_0;
		GPIOB->CRH |= GPIO_CRH_MODE12_0 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE15_0 | GPIO_CRH_CNF13_1 |GPIO_CRH_CNF14_0 | GPIO_CRH_CNF15_1;

		//SPI2 (8-bit mode)
		SPI2->CR1 = SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_SSM | SPI_CR1_SSI;
		//W³¹czamy SPI2
		SPI2->CR1 |= SPI_CR1_SPE;
		nRESET() = 1;

	}

	void SpiTxIrqEnable()
	{
		SPI2->CR2 |= SPI_CR2_TXEIE;
		txIrqEnable = true;
	}

	void SpiTxIrqDisable()
	{
		SPI2->CR2 &= ~SPI_CR2_TXEIE;
		txIrqEnable = false;
	}

	void SpiRxIrqEnable()
	{
		SPI2->CR2 |= SPI_CR2_RXNEIE;
	}


public:
	// Single Read
	static uint16_t const READ_STATUS =0x10;
	static uint16_t const READ_DATA=0x11;
	static uint16_t const READ_MODE=0x12;
	static uint16_t const READ_FILTER=0x13;
	static uint16_t const READ_DAC=0x14;
	static uint16_t const READ_OFFSET=0x15;
	static uint16_t const READ_GAIN=0x16;

	// Continuous Read
	static uint16_t const READ_STATUS_CONT =0x20;
	static uint16_t const READ_DATA_CONT=0x21;
	static uint16_t const READ_MODE_CONT=0x22;

	// Stop Continuous Read
	static uint16_t const STOP_READ_CONT =0x30;


	// Write
	static uint16_t const WRITE_COMM=0x00;
	static uint16_t const WRITE_MODE=0x02;
	static uint16_t const WRITE_FILTER=0x03;
	static uint16_t const WRITE_DAC=0x04;
	static uint16_t const WRITE_OFFSET=0x05;
	static uint16_t const WRITE_GAIN=0x06;


	// Communication register
	static uint16_t const COMM_REG_RS0 = 0x01;
	static uint16_t const COMM_REG_RS1 = 0x02;
	static uint16_t const COMM_REG_RS2 = 0x04;
	static uint16_t const COMM_REG_RW0 = 0x10;
	static uint16_t const COMM_REG_RW1 = 0x20;
	static uint16_t const COMM_REG_WEN = 0x80;

	// Status register
	static uint16_t const STA_REG_MS0 = 0x01;
	static uint16_t const STA_REG_MS1 = 0x02;
	static uint16_t const STA_REG_MS2 = 0x04;
	static uint16_t const STA_REG_MS3 = 0x08;
	static uint16_t const STA_REG_NOREF = 0x10;
	static uint16_t const STA_REG_STBY = 0x20;
	static uint16_t const STA_REG_STDY = 0x40;
	static uint16_t const STA_REG_RDY = 0x80;

	//Mode register
	static uint16_t const MOD_REG_CH0 = 0x01;
	static uint16_t const MOD_REG_CH1 = 0x02;
	static uint16_t const MOD_REG_RN1 = 0x10;
	static uint16_t const MOD_REG_RN2 = 0x20;
	static uint16_t const MOD_REG_MD0 = 0x2000;
	static uint16_t const MOD_REG_MD1 = 0x4000;
	static uint16_t const MOD_REG_MD2 = 0x8000;

	//Filter register
	static uint16_t const FIL_REG_CHP = 0x10;
	static uint16_t const FIL_REG_AC = 0x20;
	static uint16_t const FIL_REG_FAST = 0x100;
	static uint16_t const FIL_REG_SKIP = 0x200;

	//DAC register
	static uint16_t const DAC_REG_DAC0 = 0x01;
	static uint16_t const DAC_REG_DAC1 = 0x02;
	static uint16_t const DAC_REG_DAC2 = 0x04;
	static uint16_t const DAC_REG_DAC3 = 0x08;
	static uint16_t const DAC_REG_DAC4 = 0x10;
	static uint16_t const DAC_REG_DAC5 = 0x20;

	volatile bool cmdReceived;
	volatile bool isDataReady;

	void TriggerBufferedTransmission()
	{
		//if(iteracja==5)
		//{
		fsmSubState=1;
		iteracja=0;
		txIndex = rxIndex = 0;
		Fsm();
		//}
		//iteracja++;
	}

	void Init()
	{
		HardwareInit();
		fsmSubState = 1;
		isDataReady = false;
		cmdSize = 0;
		fsmState = INIT_WAIT;

		// read - just in case (reset RXNE flag)
		u16Data = SPI2->DR;
		iteracja=0;
		Offset=0x7600;
		number=0;
		i=0;
		SpiRxIrqEnable();
	}

	void Irq()
	{

		iteracja=0;
		uint16_t status = SPI2->SR;
		// check a source of the interrupt
		if ((status & SPI_SR_TXE) && (txIrqEnable))
		{
			SPI2->DR = cmdTxBuf[txIndex++];
			if (txIndex >= cmdSize)
				SpiTxIrqDisable();
		}

		if (status & SPI_SR_RXNE)
		{
			cmdRxBuf[rxIndex++] = 0xFF & SPI2->DR;
			if (rxIndex >= cmdSize)
			{
				cmdReceived = true;
				SPI_CS() = 1;
				txIndex = rxIndex = 0;
			}
		}
		Fsm();

	}

	void Fsm()
	{
	switch(fsmState)
	{
	case RESET:	//RESET Ukladu
		if(fsmSubState==1)
			{
			fsmSubState++;
			cmdTxBuf[0]=0xFF;
			cmdTxBuf[1]=0xFF;
			cmdTxBuf[2]=0xFF;
			cmdTxBuf[3]=0xFF;
			cmdTxBuf[4]=0xFF;
			cmdSize=5;				// Liczba bajtów do przes³ania
			SPI_CS() = 0;			// CS 0
			SpiTxIrqEnable();		// Zezwolnie na przesy³ danych
			}
			if(txIndex==0)
			{
			fsmState=INIT_WAIT;
			}
	break;

	case INIT_WAIT:									// Czekanie na inicjalizacje ukladu po resecie
		switch(fsmSubState)
				{
				case 1:
				cmdTxBuf[0]=READ_STATUS;			// Odczyt rejestru STATUS
				cmdTxBuf[1]=0x00;
				cmdSize=2;
				fsmSubState++;
				SPI_CS() = 0;
				SpiTxIrqEnable();
				break;
				case 2:
				fsmSubState=3;
				break;
				case 3:
					if((cmdRxBuf[1]&0x0F)!=0x0B)	//Sprawdzanie czy uk³ad siê zainicjalizowa³
						{
						error++;
						if (error > 10)
							{
							fsmState = RESET;		// wiêcej niz 10 z³ych odpowiedzi- RESET Uk³adu
							error = 0;
							}
						}
					else
						{
						fsmState=DAC_CONFIG;		// Przejscie do DAC_CONFIG
						error=0;
						}
				break;
				}
	break;


	case WAIT:										// Czekanie
		switch(fsmSubState)
		{
		case 1:
			cmdTxBuf[0]=READ_STATUS;				//Odczyt rejestru  STATUS
			cmdTxBuf[1]=0x00;
			cmdSize=2;
			fsmSubState++;
			SPI_CS() = 0;
			SpiTxIrqEnable();
		break;
		case 2:
			fsmSubState++;
		break;
		case 3:
			if((cmdRxBuf[1]&0x0F)!=0x0B)	//Sprawdzanie czy uk³ad siê zainicjalizowa³
				{
				error++;
				if (error > 10)
					{
					fsmState = RESET;		// wiêcej niz 10 z³ych odpowiedzi- RESET Uk³adu
					error = 0;
					}
				}
				else
					{
						error=0;
						if (~(cmdRxBuf[1] & 0x80))
						{
							fsmState=READING_DATA;		// Przejscie do READING_DATA
						}
					}
		break;
		}
	break;

	case DAC_CONFIG:
			if(fsmSubState==1)
			{
				fsmSubState++;
				cmdSize=2;
				cmdTxBuf[0]=WRITE_DAC;							//Zmiana rejestru DAC
				cmdTxBuf[1]=0x20;
				SPI_CS() = 0;
				SpiTxIrqEnable();
			}
			if(txIndex==0)
			{
				fsmState=FILTER_CONFIG;
			}
	break;

	case FILTER_CONFIG:
			if(fsmSubState==1)
			{
				fsmSubState++;
				cmdSize=4;
				cmdTxBuf[0]=WRITE_FILTER;						//Zmiana rejestru FILTER
				cmdTxBuf[1]=0x80;
				cmdTxBuf[2]=0x02;
				cmdTxBuf[3]=0x00;
				SPI_CS() = 0;
				SpiTxIrqEnable();
			}
			if(txIndex==0)
			{
				fsmState=MODE_CONFIG;
			}
	break;

	case MODE_CONFIG:
			if(fsmSubState==1)
			{
				fsmSubState++;
				cmdSize=3;
				cmdTxBuf[0]=WRITE_MODE;							//Zmiana rejestru MODE
				cmdTxBuf[1]=0x20;
				cmdTxBuf[2]=0x80;
				SPI_CS() = 0;
				SpiTxIrqEnable();
			}
			if(txIndex==0)
			{
				fsmState=WAIT;
			}
	break;

	case READING_DATA:
			if(fsmSubState==1)
			{
				fsmSubState++;
				cmdSize=3;
				cmdTxBuf[0]=READ_DATA;						//Odczyt rejestru - DATA
				cmdTxBuf[1]=0x00;
				cmdTxBuf[2]=0x00;
				SPI_CS() = 0;
				SpiTxIrqEnable();
			}
			if(txIndex==0)									//Obróbka danych
			{
				c=cmdRxBuf[1]*256 + cmdRxBuf[2];
				if(c)
				{
					if(c>Offset)
					{
						raw_data=c-Offset;
					}
					else
					{
						raw_data=Offset-c;
					}

				}
				else
				{
					raw_data=0;
				}
				//if(i%10==0)
				//{
				Table[number]=raw_data;
				number++;
				//}
				//i++;
				if(number==1000)
				{
					number=0;
					i=0;
				}

				fsmState=WAIT;
			}

	break;

	}
	}

	void Update()
	{
		TriggerBufferedTransmission();
	}

};



