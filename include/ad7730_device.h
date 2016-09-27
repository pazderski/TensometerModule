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

	// Data register
	static uint8_t const DAT_REG_MS0 = 0x01;
	static uint8_t const DAT_REG_MS1 = 0x02;
	static uint8_t const DAT_REG_MS2 = 0x04;
	static uint8_t const DAT_REG_MS3 = 0x08;


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

	void HardwareInit();

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

