#pragma once

#include "stm32f1xx_hal.h"
#include "hdr_bitband.h"

class Led
{
	static void HardwareInit()
	{

		// PA0, PA0 - LEDy
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
		GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_CNF1 | GPIO_CRL_MODE0 | GPIO_CRL_MODE1);
		GPIOA->CRL |= GPIO_CRL_MODE0_1 | GPIO_CRL_MODE1_1;
	}

public:

	static void Init()
	{
		HardwareInit();
	}

	static volatile unsigned long & Led1()
	{
		return *(volatile unsigned long*) m_BITBAND_PERIPH(&GPIOA->ODR, 0);
	}

	static volatile unsigned long & Led2()
	{
		return *(volatile unsigned long*) m_BITBAND_PERIPH(&GPIOA->ODR, 1);
	}
};
