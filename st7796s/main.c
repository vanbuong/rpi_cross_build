#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include "rpi_dma_utils.h"
#include "st7796s_define.h"

int main (int argc, char **argv)
{
	st7796s_init();
	
    printf("TFT LCD have beens setup.\n");
    uint16_t color;
    
    while (1)
    {   
        usleep(1000000);
		color = COLOR_HEX(0xff0000);
        st7796s_fill_screen(color);
        usleep(1000000);
        color = COLOR_HEX(0x00ff00);
        st7796s_fill_screen(color);
    }
}
