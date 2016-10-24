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
	static uint8_t const FIL_REG_CHP = 0x10;
	static uint8_t const FIL_REG_AC = 0x20;
	static uint8_t const FIL_REG_FAST = 0x100;
	static uint8_t const FIL_REG_SKIP = 0x200;

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
		RESET,
		FILTER_CONFIG,
		MODE_CONFIG,
		WAIT
	};

	volatile uint16_t u16Data;

	FsmState fsmState;

	static volatile unsigned long & SPI_CS()
	{
		return bitband_t m_BITBAND_PERIPH(&GPIOE->ODR, 3);
	}

	void HardwareInit()
	{
		//Ustawienia PA4-7
		//Ustawiamy zegar
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
		//Zerujemy bity wyjsciowe
		GPIOA->ODR&= ~(GPIO_ODR_ODR4 | GPIO_ODR_ODR5|GPIO_ODR_ODR6 | GPIO_ODR_ODR7);
		//Zerujemy rejestry PA4-7 Mode,CNF
		GPIOA->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_CNF5 |GPIO_CRL_CNF6 | GPIO_CRL_CNF7 | GPIO_CRL_MODE4 | GPIO_CRL_MODE5| GPIO_CRL_MODE6 | GPIO_CRL_MODE7);
	    //Ustawiamy PA4 PA6 PA7 w tryb wyjscia 10 MHz i Altarnate Function Push-pull
		GPIOA->CRL |= GPIO_CRL_MODE4_0 | GPIO_CRL_MODE6_0|GPIO_CRL_MODE7_0| GPIO_CRL_CNF4_1| GPIO_CRL_CNF6_1| GPIO_CRL_CNF7_1;

		//SPI1
		SPI1->CR1 = SPI_CR1_CPOL | SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPHA | SPI_CR1_DFF;
		//W³¹czamy SPI1
		SPI1->CR1 |= SPI_CR1_SPE;

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
		while(!(SPI1->SR & SPI_SR_TXE)) {};
		SPI1->DR = data;
	    while(SPI1->SR & SPI_SR_BSY) {};
	    while(!(SPI1->SR & SPI_SR_RXNE)) {};

	    SPI_CS() = 1;
	    __NOP(); __NOP();
	    data = SPI1->DR;
	    return data;
	}

	void WriteReadStart()
	{
		fsmState = STATE_STATUS;
		SPI_CS() = 0; __NOP(); __NOP();
		// rozkaz wysylany
		SPI1->DR = STATUS_REG;
	}

	void Init()
	{
		HardwareInit();

		isDataReady = false;
		fsmState = STATE_IDLE;
		u16Data = SPI1->DR;
		// ustawienie akcelerometru
		u16Data = WriteReadBlock(WHO_AM_I) & 0xFF;
		u16Data = WriteReadBlock(CTRL_REG1 + 0x47) & 0xFF;
		// zezwolenie na obsluge przerwan od odbiornika SPI
		SPI1->CR2 |= SPI_CR2_RXNEIE;
	}

	void Irq()
	{
	    uint16_t data;
		SPI_CS() = 1;
		data = (SPI1->DR) & 0x00FF;
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



