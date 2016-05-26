#include "XBee.h"


void initializeXbee(void){
	delay_1s();
	//Send the +++ (Entering command mode)
	Usart1SendString("+++");
	waitForOkResponse();
	//Send the ATRE<CR>(Restore defaults)
	Usart1SendString("ATRE\r");
	waitForOkResponse();
	//Setting RO if not using (GT + CC + GT)
	//Send the ATDL ... <CR>(Destination Address Low.)
	Usart1SendString("ATDL FFFF \r");
	waitForOkResponse();
	//Send the ATDH ... <CR>(Destination Address High.)
	Usart1SendString("ATDH 0\r");
	waitForOkResponse();
	//Send the ATSL ... <CR>(Serial Number Low.)
	Usart1SendString("ATSL AAAA\r");
	waitForOkResponse();
	//Send the ATSH ... <CR>(Serial Number High.)
	Usart1SendString("ATSH 0\r");
	waitForOkResponse();
	//Send the ATAC<CR> Apply Changes) command
	Usart1SendString("ATAC\r");
	waitForOkResponse();
	//Send the ATCN<CR>(Exit Command Mode) command
	Usart1SendString("ATCN\r");
	waitForOkResponse();
}


/*First xbee
 * ATDL FFFF
 * ATDH 0
 * ATSL AAAA
 * ATSH 0
 *
 * ATDL AAAA
 * ATDH 0
 * ATSL FFFF
 * ATSH 0
 *
 *
 */
