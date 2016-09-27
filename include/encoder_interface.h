#pragma once

#include "stm32f10x.h"

class InputEncoders
{
	int16_t counterPrev;

	void HardwareInit()
	{

		// PA0, PA1 - TIM2
		// PA8, PA9 - TIM1

		// GPIOs settings
		RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
		GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0 | GPIO_CRL_CNF1 | GPIO_CRL_MODE1);
		GPIOA->CRL |= GPIO_CRL_CNF0_0 | GPIO_CRL_CNF1_0;
		GPIOA->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_MODE8 | GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
		GPIOA->CRH |= GPIO_CRH_CNF8_0 | GPIO_CRH_CNF9_0;

		// timers settings
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

		TIM1->ARR = 0xFFFF;             // autoreload value
		TIM1->SMCR = TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;
		TIM1->CCMR1 = TIM_CCMR1_CC2S_0 |TIM_CCMR1_CC1S_0 ; //TI2 mapped to IC2 TI1 mapped to IC1
		TIM1->CCER = TIM_CCER_CC1P;// ustawienie odpowiedniej rakcji na zbocze	 - zmiana kierunku zliczania

		TIM2->ARR = 0xFFFF;             // autoreload value
		TIM2->SMCR = TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1;
		TIM2->CCMR1 = TIM_CCMR1_CC2S_0 |TIM_CCMR1_CC1S_0 ; //TI2 mapped to IC2 TI1 mapped to IC1
		TIM2->CCER = TIM_CCER_CC1P;// ustawienie odpowiedniej rakcji na zbocze	 - zmiana kierunku zliczania
	}

public:

	int16_t direction;
	uint8_t state;

	void Init()
	{
		HardwareInit();
		TIM1->CNT = TIM2->CNT = 0;
		counterPrev = 0;
		direction = 0;
		TIM1->CR1 = TIM_CR1_CEN;
		TIM2->CR1 = TIM_CR1_CEN;

		state = 0;
	}

	void ReadCounters()
	{
		int16_t counter = (int16_t)TIM2->CNT; // - (int16_t)TIM1->CNT;
		auto deltaCounter = counter - counterPrev;

		if (deltaCounter > 0)
		{
			state++;
			direction = 1;
			counterPrev = counter;
		}
		if (deltaCounter < 0)
		{
			state--;
			direction = -1;
			counterPrev = counter;
		}
		state &= 0x03;
	}
};


class OutputEncoder
{
	static const uint16_t outputMap[4];

	void HardwareInit()
	{

		// PB14, PB15 - ENC_A, ENC_B

		// GPIOs settings
		RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

		GPIOB->ODR &= ~(GPIO_ODR_ODR14 | GPIO_ODR_ODR15);
		GPIOB->CRH &= ~(GPIO_CRH_CNF14 | GPIO_CRH_CNF15 | GPIO_CRH_MODE14 | GPIO_CRH_MODE15);
		GPIOB->CRH |= GPIO_CRH_MODE14_1 | GPIO_CRH_MODE15_1;
	}

public:

	void Init()
	{
		HardwareInit();
	}

	void SetOutput(uint8_t s)
	{
		auto data = GPIOB->ODR & ~(GPIO_ODR_ODR14 | GPIO_ODR_ODR15);
		data |= outputMap[s] << 14;
		GPIOB->ODR = data;
	}
};
