#pragma once

#include "stm32f10x.h"

class Tensometer
{

	// Communication register
	static uint8_t const COMM_REG_RS0 = 0x01;
	static uint8_t const COMM_REG_RS1 = 0x02;
	static uint8_t const COMM_REG_RS2 = 0x04;
	static uint8_t const COMM_REG_RW0 = 0x10;
	static uint8_t const COMM_REG_RW1 = 0x20;
	static uint8_t const COMM_REG_WEN = 0x80;

	// Status register
	static uint8_t const STA_REG_MS0 = 0x01;
	static uint8_t const STA_REG_MS1 = 0x02;
	static uint8_t const STA_REG_MS2 = 0x04;
	static uint8_t const STA_REG_MS3 = 0x08;
	static uint8_t const STA_REG_NOREF = 0x10;
	static uint8_t const STA_REG_STBY = 0x20;
	static uint8_t const STA_REG_STDY = 0x40;
	static uint8_t const STA_REG_RDY = 0x80;

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
	static uint8_t const DAC_REG_DAC0 = 0x01;
	static uint8_t const DAC_REG_DAC1 = 0x02;
	static uint8_t const DAC_REG_DAC2 = 0x04;
	static uint8_t const DAC_REG_DAC3 = 0x08;
	static uint8_t const DAC_REG_DAC4 = 0x10;
	static uint8_t const DAC_REG_DAC5 = 0x20;



	// Definicje stanow automatu do obslugi akcelerometru
	enum FsmState
	{
		IDLE,
		RESET,
		FILTER_CONFIG,
		MODE_CONFIG,
		WAIT
	};

	volatile uint16_t u16Data;

	FsmState fsmState;

	static volatile unsigned long & SPI_CS()
	{
		return bitband_t m_BITBAND_PERIPH(&GPIOB->ODR, 12);
	}

	void HardwareInit()
	{
		//Ustawienia PA4-7
		//Ustawiamy zegar
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

		//Zerujemy bity wyjsciowe
		GPIOB->ODR&= ~(GPIO_ODR_ODR12 | GPIO_ODR_ODR13 | GPIO_ODR_ODR14 | GPIO_ODR_ODR15);
		// zerujemy rejestry PA4-7 Mode,CNF
		GPIOB->CRH &= ~(GPIO_CRH_CNF12 | GPIO_CRH_CNF13 |GPIO_CRH_CNF14 | GPIO_CRH_CNF15 | GPIO_CRH_MODE12 | GPIO_CRH_MODE13 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15);
		GPIOB->CRH |= GPIO_CRH_MODE12_0 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE15_0 | GPIO_CRH_CNF13_1 |GPIO_CRH_CNF14_0 | GPIO_CRH_CNF15_1;

		//SPI2
		SPI2->CR1 = SPI_CR1_CPOL | SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPHA | SPI_CR1_DFF;
		//W³¹czamy SPI1
		SPI2->CR1 |= SPI_CR1_SPE;

	}

public:

	volatile int8_t rawDataX;
	volatile int8_t rawDataY;
	volatile int8_t rawDataZ;

	float accVal[3];

	volatile bool isDataReady;

	// Wymiana danych na SPI z blokowaniem
	uint16_t WriteReadBlock(uint16_t data)
	{
		SPI_CS() = 0;
		__NOP(); __NOP();
		while(!(SPI2->SR & SPI_SR_TXE)) {};
		SPI2->DR = data;
	    while(SPI2->SR & SPI_SR_BSY) {};
	    while(!(SPI2->SR & SPI_SR_RXNE)) {};

	    SPI_CS() = 1;
	    __NOP(); __NOP();
	    data = SPI2->DR;
	    return data;
	}

	void WriteReadStart()
	{
		// fsmState = STATUS;
		SPI_CS() = 0; __NOP(); __NOP();
		// rozkaz wysylany
		SPI2->DR = 0x00;
	}

	void Init()
	{
		HardwareInit();

		isDataReady = false;
		fsmState = IDLE;
		u16Data = SPI2->DR;
		// ustawienie akcelerometru
		u16Data = WriteReadBlock(0x00);
		// zezwolenie na obsluge przerwan od odbiornika SPI
		SPI2->CR2 |= SPI_CR2_RXNEIE;
	}

	void Irq()
	{
	    uint16_t data;
		SPI_CS() = 1;
		data = (SPI2->DR) & 0x00FF;
		Fsm(data);
	}

	void Fsm(uint16_t);

	void ScaleData()
	{
		accVal[0] = (float)rawDataX;
		accVal[1] = (float)rawDataY;
		accVal[2] = (float)rawDataZ;
	}
};



