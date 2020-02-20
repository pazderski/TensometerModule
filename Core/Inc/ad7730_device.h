#pragma once

#include "stm32f1xx_hal.h"

class Tensometer
{

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
	static uint16_t const MOD_REG_MD0 = 0x20;
	static uint16_t const MOD_REG_MD1 = 0x40;
	static uint16_t const MOD_REG_MD2 = 0x80;

	//Filter register
	static uint16_t const FIL_REG_CHP = 0x10;
	static uint16_t const FIL_REG_AC = 0x20;
	static uint16_t const FIL_REG_FAST = 0x01;
	static uint16_t const FIL_REG_SKIP = 0x02;









	//DAC register
	static uint16_t const DAC_REG_DAC0 = 0x01;
	static uint16_t const DAC_REG_DAC1 = 0x02;
	static uint16_t const DAC_REG_DAC2 = 0x04;
	static uint16_t const DAC_REG_DAC3 = 0x08;
	static uint16_t const DAC_REG_DAC4 = 0x10;
	static uint16_t const DAC_REG_DAC5 = 0x20;

	uint8_t cmdTxBuf[8];
	uint8_t cmdRxBuf[8];
	volatile uint8_t cmdSize;
	volatile uint8_t txIndex;
	volatile uint8_t rxIndex;
	volatile bool txIrqEnable;
	volatile bool rxIrqEnable;
	volatile bool cmdReceived;

	uint16_t errorNumber;
	uint16_t Offset;
	uint32_t Table[1000];

	int number;
	int i;



	// Definicje stanow automatu do obslugi akcelerometru
	enum FsmState
	{
		FSM_RESET,
		FSM_INIT_WAIT,
		FSM_INIT_WAIT_CHECK,
		FSM_WAIT,
		FSM_WAIT_CHECK,
		FSM_FILTER_CONFIG,
		FSM_DAC_CONFIG,
		FSM_MODE_CONFIG,
		FSM_ADC_READ
	};

	FsmState fsmState;


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
		//W��czamy SPI2
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

	void TriggerTransmission()
	{
		SPI_CS() = 0;
		cmdReceived = false;
		SpiTxIrqEnable();
	}

	void Fsm()
	{
		switch(fsmState)
		{
			case FSM_RESET:
				cmdTxBuf[0] = 0xFF;
				cmdTxBuf[1] = 0xFF;
				cmdTxBuf[2] = 0xFF;
				cmdTxBuf[3] = 0xFF;
				cmdTxBuf[4] = 0xFF;
				cmdSize = 5;

				TriggerTransmission();

				fsmState = FSM_INIT_WAIT;
			break;

			case FSM_INIT_WAIT:			// Czekanie na inicjalizacje ukladu po resecie

				cmdTxBuf[0] = READ_STATUS;			// Odczyt rejestru STATUS
				cmdTxBuf[1] = 0x00;
				cmdSize = 2;

				TriggerTransmission();

				fsmState = FSM_INIT_WAIT_CHECK;
			break;

			case FSM_INIT_WAIT_CHECK:

				if((cmdRxBuf[1]&0x0F)!=0x0B)	//Sprawdzanie czy uk�ad si� zainicjalizowa�
				{
					errorNumber++;
					if (errorNumber > 10)
					{
						fsmState = FSM_RESET;		// wi�cej niz 10 z�ych odpowiedzi- RESET Uk�adu
						errorNumber = 0;
					}
					else
					{
						fsmState = FSM_INIT_WAIT;
					}
				}
				else
				{
					fsmState = FSM_DAC_CONFIG;		// Przejscie do DAC_CONFIG
					errorNumber=0;
				}

			break;

			case FSM_WAIT:										// Czekanie
				cmdTxBuf[0]=READ_STATUS;				//Odczyt rejestru  STATUS
				cmdTxBuf[1]=0x00;
				cmdSize=2;

				TriggerTransmission();

				fsmState = FSM_WAIT_CHECK;
			break;

			case FSM_WAIT_CHECK:
				if((cmdRxBuf[1]&0x0F)!=0x0B)	//Sprawdzanie czy uk�ad si� zainicjalizowa�
				{
					errorNumber++;
					if (errorNumber > 10)
					{
						fsmState = FSM_RESET;		// wi�cej niz 10 z�ych odpowiedzi- RESET Uk�adu
						errorNumber = 0;
					}
				}
				else
				{
					errorNumber=0;
					if (~(cmdRxBuf[1] & 0x80))
					{
						cmdTxBuf[0]=READ_DATA;						//Odczyt rejestru - DATA
						cmdTxBuf[1]=0x00;
						cmdTxBuf[2]=0x00;
						cmdSize=3;

						TriggerTransmission();
						fsmState=FSM_ADC_READ;		// Przejscie do READING_DATA
					}
				}
			break;

			case FSM_DAC_CONFIG:

				cmdTxBuf[0]=WRITE_DAC;							//Zmiana rejestru DAC
				cmdTxBuf[1]=DAC_REG_DAC5;
				cmdSize=2;

				TriggerTransmission();

				fsmState=FSM_FILTER_CONFIG;
			break;

			case FSM_FILTER_CONFIG:

				cmdTxBuf[0]=WRITE_FILTER;						//Zmiana rejestru FILTER
				cmdTxBuf[1]=0x80;
				cmdTxBuf[2]=FIL_REG_SKIP;
				cmdTxBuf[3]=0x00;
				cmdSize=4;

				TriggerTransmission();

				fsmState=FSM_MODE_CONFIG;
			break;

			case FSM_MODE_CONFIG:

				cmdTxBuf[0]=WRITE_MODE;							//Zmiana rejestru MODE
				cmdTxBuf[1]=MOD_REG_RN2;
				cmdTxBuf[2]=MOD_REG_MD2;
				cmdSize=3;

				TriggerTransmission();

				fsmState=FSM_WAIT;
			break;

			case FSM_ADC_READ:
			{
				uint16_t c = __builtin_bswap16(*(uint16_t*)(cmdRxBuf+1));

				forceValue = (int16_t)(c - Offset);
				isDataReady = true;
				//if(i%10==0)
				//{
				/*Table[number]=raw_data;
				number++;
				//}
				//i++;
				if(number==1000)
				{
					number=0;
					i=0;
				}*/

				fsmState=FSM_WAIT;
			}
			break;
		}
	}

public:

	volatile bool isDataReady;
	volatile uint16_t forceValue;


	void Init()
	{
		HardwareInit();
		isDataReady = false;
		cmdSize = 0;
		fsmState = FSM_INIT_WAIT;

		// read - just in case (reset RXNE flag)
		uint16_t c = SPI2->DR;

		errorNumber = 0;
		Offset=0x7600;
		number=0;
		i=0;
		SpiRxIrqEnable();
	}

	void Irq()
	{

		uint16_t status = SPI2->SR;
		// check a source of the interrupt

		if ((status & SPI_SR_TXE) && (txIrqEnable))	// transmitter
		{
			SPI2->DR = cmdTxBuf[txIndex++];
			if (txIndex >= cmdSize)
				SpiTxIrqDisable();
		}

		if (status & SPI_SR_RXNE)					// receiver
		{
			cmdRxBuf[rxIndex++] = 0xFF & SPI2->DR;
			if (rxIndex >= cmdSize)
			{
				cmdReceived = true;
				SPI_CS() = 1;
				txIndex = rxIndex = 0;
				Fsm();
			}
		}

	}

	void Update()
	{
		txIndex = rxIndex = 0;
		Fsm();
	}

};



