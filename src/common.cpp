#include "common.h"

extern "C"
{
	void SysTick_Handler(void);
	void EXTI0_IRQHandler(void);
	void SPI1_IRQHandler(void);
	void DMA1_Channel7_IRQHandler(void);
	void USART2_IRQHandler(void);
}

void Delay(volatile uint32_t count)
{
	while(count--) {};
}


// Obsluga przerwania od nadajnika UART poprzez DMA
void DMA1_Channel7_IRQHandler()
{
	//pApp->com.IrqDma();
}

// Obsluga przerwania od odbiornika/nadajnika USART2
void USART2_IRQHandler()
{
	//pApp->com.IrqTx();
}

void SysTick_Handler(void)
{
	pApp->PeriodicUpdate();
}


