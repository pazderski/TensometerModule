#pragma once

#include <string.h>

#include "stm32f10x.h"
#include "led_interface.h"
#include "encoder_interface.h"

#define CPU_CLK	((uint32_t)72000000)

class App
{

	// timers
	volatile uint32_t mainClock;
	volatile uint16_t clock1;
	volatile uint16_t clock2;

	volatile bool processSynch;

	// periods [ms]
	static const uint16_t period1 = 500;
	static const uint16_t period2 = 5;

public:

	InputEncoders encodersIn;
	OutputEncoder encoderOut;

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
	}

	void Init()
	{
		GeneralHardwareInit();
		encodersIn.Init();
		encoderOut.Init();
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
	}

	void Run()
	{
		while(1)
		{
			encodersIn.ReadCounters();
			__disable_irq();
			encoderOut.SetOutput(encodersIn.state);
			__enable_irq();

			if (encodersIn.direction == 1)
				Led::Led2() = 1;
			else
				Led::Led2() = 0;
		}
	}
};

extern App *pApp;
