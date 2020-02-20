#include "common.h"

extern "C"
{
	void SysTick_Handler(void);
	void EXTI0_IRQHandler(void);
	void SPI2_IRQHandler(void);
	void DMA1_Channel4_IRQHandler(void);
	void USART1_IRQHandler(void);
}

void Delay(volatile uint32_t count)
{
	while(count--) {};
}


// Obsluga przerwania od nadajnika UART poprzez DMA
void DMA1_Channel4_IRQHandler()
{
	pApp->com.IrqDma();
}

// Obsluga przerwania od odbiornika/nadajnika USART2
void USART1_IRQHandler()
{
	pApp->com.IrqTx();
}

void HAL_IncTick(void)
{
	pApp->PeriodicUpdate();
}

void SPI2_IRQHandler(void)
{
	pApp->tensometer.Irq();
}

