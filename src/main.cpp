#include "common.h"

App * pApp;

int main(void)
{
	App app;
	pApp = &app;
	app.Init();
	Led::Init();

	__enable_irq();

	app.Run();
}


