#include <xc.h>
#include "blackbox.h"
//#include "main.h"
int seconds=10;

void __interrupt() isr(void)
{
	static unsigned short count;
     
	if (TMR0IF)
	{
		TMR0 = TMR0 + 8;

		if (count++ == 20000)
		{
			count = 0;
            seconds--;
		}
		TMR0IF = 0;
	}
}

