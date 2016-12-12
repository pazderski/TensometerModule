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

	//volatile uint8_t

	// Definicje stanow automatu do obslugi akcelerometru
	enum FsmState
	{
		DAC_CONFIG,
		IDLE,
		RESET,
		FILTER_CONFIG,
		MODE_CONFIG,
		WAIT,
		RUN
	};

	volatile uint16_t u16Data;

	FsmState fsmState=FILTER_CONFIG;

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
		GPIOB->ODR&= ~(GPIO_ODR_ODR1 | GPIO_ODR_ODR12 | GPIO_ODR_ODR13 | GPIO_ODR_ODR14 | GPIO_ODR_ODR15);
		// zerujemy rejestry PA4-7 Mode,CNF
		GPIOB->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_MODE1);
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


public:
	uint32_t rcv_data = 0x00;
	uint8_t fsmSubState=1;
	int i=1;
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

	void SetCmdReadStatusRegister(uint32_t data, uint8_t size)
	{
		cmdTxBuf[0] = data & 0x000000ff;
		cmdTxBuf[1] = (data & 0x0000ff00) >> 8;
		cmdTxBuf[2] = (data & 0x00ff0000) >> 16;
		cmdTxBuf[3] = (data & 0xff000000) >> 24;
		cmdSize = size;
	}

	void TriggerBufferedTransmission()
	{
		SPI_CS() = 0;
		cmdReceived = false;
		txIndex = rxIndex = 0;
		SpiTxIrqEnable();
	}

	// Wymiana danych na SPI z blokowaniem
	uint8_t WriteReadContinue(uint8_t data)
	{
		__NOP(); __NOP();
		while(!(SPI2->SR & SPI_SR_TXE)) {};
		SPI2->DR = data;
	    while(SPI2->SR & SPI_SR_BSY) {};
	    while(!(SPI2->SR & SPI_SR_RXNE)) {};
	    data = SPI2->DR;
	    return data;
	}

	uint8_t WriteReadStart(uint8_t data)
	{
		//fsmState = STATUS;
		SPI_CS() = 0; __NOP(); __NOP();
		while(!(SPI2->SR & SPI_SR_TXE)) {};
		// rozkaz wysylany
		SPI2->DR = data;
	    while(SPI2->SR & SPI_SR_BSY) {};
	    while(!(SPI2->SR & SPI_SR_RXNE)) {};
	    data = SPI2->DR;
	    return data;
	}

	void Stop()
	{
		//fsmState = STATUS;
		__NOP(); __NOP(); SPI_CS() = 1;
	}

	void Init()
	{
		HardwareInit();

		isDataReady = false;
		cmdSize = 0;
		fsmState = FILTER_CONFIG;

		// read - just in case (reset RXNE flag)
		u16Data = SPI2->DR;

		SpiRxIrqEnable();
	}

	void Irq()
	{
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
				rcv_data=0x00;
				rcv_data = cmdRxBuf[0];

				for(i=1;i<(int)rxIndex;i++)
				{
					rcv_data=rcv_data<<8|cmdRxBuf[i];
				}

			}
		}
		Fsm();
	}

	void Fsm()
	{
	switch(fsmState)
	{
	case DAC_CONFIG:
		switch(fsmSubState)
				{
				case 1:
				SetCmdReadStatusRegister(0x04,1); //Next Operation as Write to 	DAC Register
				fsmSubState=2;
				break;
				case 2:
				SetCmdReadStatusRegister(0x23,1); //Write to DAC Register
				fsmState=MODE_CONFIG;
				fsmSubState=1;
				break;
				}
			break;
	case FILTER_CONFIG:
		switch(fsmSubState)
				{
				case 1:
				SetCmdReadStatusRegister(0x03,1); //Next Operation as Write to 	Filter Register
				fsmSubState=2;
				break;
				case 2:
				SetCmdReadStatusRegister(0x800010,3); //Write to 	Filter Register
				fsmState=DAC_CONFIG;
				fsmSubState=1;
				break;
				}
			break;
	case MODE_CONFIG:
		switch(fsmSubState)
				{
				case 1:
				SetCmdReadStatusRegister(0x02,1); //Next Operation as Write to 	Filter Register
				fsmSubState=2;
				break;
				case 2:
				SetCmdReadStatusRegister(0xB180,1); //Next Operation as Write to 	Filter Register
				fsmSubState=3;
				fsmState=WAIT;
				break;
				case 3:
				SetCmdReadStatusRegister(0x03,1); //Next Operation as Write to 	Filter Register
				fsmSubState=2;
				break;
				case 4:
				SetCmdReadStatusRegister(0x800010,3); //Write to 	Filter Register
				fsmState=DAC_CONFIG;
				fsmSubState=1;
				break;
				}
		break;
	case RUN:
		break;
	case WAIT:
		break;
	}
	}

	void Update()
	{
		TriggerBufferedTransmission();
	}

};



