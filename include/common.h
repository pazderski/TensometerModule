#pragma once

#include <string.h>

#include "stm32f10x.h"
#include "led_interface.h"
#include "encoder_interface.h"
#include "ad7730_device.h"
#include "uart_communication_interface.h"

#define CPU_CLK	((uint32_t)72000000)

class App
{

	// timers
	volatile uint32_t mainClock;
	volatile uint16_t clock1;
	volatile uint16_t clock2;
	volatile uint16_t Data;
	volatile uint16_t Data_write;
	volatile uint16_t Data_read;
	volatile bool processSynch;

	// periods [ms]
	static const uint16_t period1 = 500;
	static const uint16_t period2 = 5;

public:

	UartCommunicationInterface com;
	InputEncoders encodersIn;
	OutputEncoder encoderOut;
	Tensometer Tensometer1;

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

		NVIC_EnableIRQ(DMA1_Channel7_IRQn);
		NVIC_EnableIRQ(USART2_IRQn);
	}

	void Init()
	{
		GeneralHardwareInit();
		com.Init();
		encodersIn.Init();
		encoderOut.Init();
		Tensometer1.Init();
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
		}

		com.PeriodicUpdate();
	}

	void Run()
	{
		Tensometer1.WriteReadStart();
		while(1)
		{
			Data = Tensometer1.WriteReadBlock(0x0010);
			//encodersIn.ReadCounters();
			//__disable_irq();
			//encoderOut.SetOutput(encodersIn.state);
			//__enable_irq();

			if (encodersIn.direction == 1)
				Led::Led2() = 1;
			else
				Led::Led2() = 0;
		}
	}
};

extern App *pApp;
