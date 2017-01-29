#pragma once

#include <string.h>

#include "stm32f10x.h"
#include "led_interface.h"
#include "ad7730_device.h"
#include "uart_communication_interface.h"

#define CPU_CLK	((uint32_t)72000000)

class App
{

	// timers
	volatile uint32_t mainClock;
	volatile uint16_t clock1;
	volatile uint16_t clock2;
	volatile bool processSynch;


	volatile uint16_t Data;
	volatile uint8_t Data_write;
	volatile uint16_t Data_read;


	// periods [ms]
	static const uint16_t period1 = 500;
	static const uint16_t period2 = 5;


public:

	UartCommunicationInterface com;
	Tensometer tensometer;

	App()
	{
		mainClock = clock1 = clock2 = 0;

	};

	void GeneralHardwareInit()
	{
		// inicjalizacja mikrokontrolera
		SystemInit();

		// ustawienie zegara systemowego w programie
		if (SysTick_Config(CPU_CLK/1000))
		{
			while (1);
		}

		NVIC_EnableIRQ(DMA1_Channel4_IRQn);
		NVIC_EnableIRQ(USART1_IRQn);
		NVIC_EnableIRQ(SPI2_IRQn);
	}

	void Init()
	{
		GeneralHardwareInit();
		com.Init();
		tensometer.Init();

	}

	void PeriodicUpdate()
	{
		mainClock++;
		clock1++;
		clock2++;

		if (clock1 == period1)
		{
			Led::Led1()^= 1;
		  	clock1 = 0;
		}

		if (clock2 == period2)
		{
			clock2 = 0;
			processSynch = true;
			//tensometer.Update();

		}

		tensometer.Update();
		com.PeriodicUpdate();

	}

	void Run()
	{
		while(1)
		{
			if(com.isFrameReceived)
			{
				memcpy(com.txData, (void*)&tensometer.forceValue, sizeof(uint16_t));
				com.Send(sizeof(uint16_t));
				com.isFrameReceived = false;
			}
		}
	}
};

extern App *pApp;
