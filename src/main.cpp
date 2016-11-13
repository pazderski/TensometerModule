#include "common.h"

App * pApp;
App app;

int main(void)
{

	pApp = &app;
	app.Init();
	Led::Init();

	__enable_irq();

	app.Run();
}


