#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "rpi_dma_utils.h"
#include "st7796s_define.h"

static uint8_t rotation;
static uint16_t _width;
static uint16_t _height;

void st7796s_write_command(uint8_t cmd)
{
	digitalWrite(TFT_DC, LOW);
	if (wiringPiSPIDataRW(SPI_CHANNEL, &cmd, 1) == -1)
		printf("SPI write err_1\n");
}

void st7796s_write_data(uint8_t data)
{
	digitalWrite(TFT_DC, HIGH);
	if (wiringPiSPIDataRW(SPI_CHANNEL, &data, 1) == -1)
		printf("SPI write err_2\n");
}

void st7796s_write_bytes(uint8_t *data, uint8_t len)
{
	digitalWrite(TFT_DC, HIGH);
	if (wiringPiSPIDataRW(SPI_CHANNEL, data, len) == -1)
		printf("SPI write err_3\n");
}

void st7796s_scroll_to(uint16_t y)
{
  uint8_t data[2];
  data[0] = y >> 8;
  data[1] = y & 0xff;
  st7796s_write_command(ST7796_VSCRSADD);
  st7796s_write_bytes(data, 2);
}

void st7796s_set_scroll_margins(uint16_t top, uint16_t bottom)
{
  // TFA+VSA+BFA must equal 480
  if (top + bottom <= ST7796S_TFTHEIGHT) {
    uint16_t middle = ST7796S_TFTHEIGHT - top - bottom;
    uint8_t data[6];
    data[0] = top >> 8;
    data[1] = top & 0xff;
    data[2] = middle >> 8;
    data[3] = middle & 0xff;
    data[4] = bottom >> 8;
    data[5] = bottom & 0xff;
    st7796s_write_command(ST7796_VSCRDEF);
    st7796s_write_bytes((uint8_t *)data, 6);
  }
}

void st7796s_set_rotation(uint8_t m)
{
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
  case 0:
    m = (ST7796_MADCTL_MX | ST7796_MADCTL_RGB);
    _width = ST7796S_TFTWIDTH;
    _height = ST7796S_TFTHEIGHT;
    break;
  case 1:
    m = (ST7796_MADCTL_MV | ST7796_MADCTL_RGB);
    _width = ST7796S_TFTHEIGHT;
    _height = ST7796S_TFTWIDTH;
    break;
  case 2:
    m = (ST7796_MADCTL_MY | ST7796_MADCTL_ML | ST7796_MADCTL_RGB);
    _width = ST7796S_TFTWIDTH;
    _height = ST7796S_TFTHEIGHT;
    break;
  case 3:
    m = (ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_ML | ST7796_MADCTL_RGB);
    _width = ST7796S_TFTHEIGHT;
    _height = ST7796S_TFTWIDTH;
    break;
  }

  st7796s_write_command(ST7796_MADCTL);
  st7796s_write_data(m);
  
  st7796s_set_scroll_margins(0, 0); //.kbv
  st7796s_scroll_to(0);  
}

void st7796s_set_address_window(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h)
{
  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
  uint8_t data[4] = {x1 >> 8, x1 & 0xff, x2 >> 8, x2 & 0xff};
  st7796s_write_command(ST7796_CASET); // Column address set
  st7796s_write_bytes(data, 4);

  data[0] = y1 >> 8;
  data[1] = y1 & 0xff;
  data[2] = y2 >> 8;
  data[3] = y2 & 0xff;
  st7796s_write_command(ST7796_PASET); // Row address set
  st7796s_write_bytes(data, 4);
  st7796s_write_command(ST7796_RAMWR); // Write to RAM
}

void st7796s_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h,
  uint16_t color) {

  // rudimentary clipping (drawChar w/big text requires this)
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width)  w = _width  - x;
  if((y + h - 1) >= _height) h = _height - y;

  st7796s_set_address_window(x, y, x+w-1, y+h-1);

  uint8_t cxx[2] = {color >> 8, color & 0xff};
    
  digitalWrite(TFT_DC, HIGH);
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
	  cxx[0] = color >> 8;
	  cxx[1] = color & 0xff;
      wiringPiSPIDataRW(SPI_CHANNEL, cxx, 2);
    }
  }
}

void st7796s_fill_screen(uint16_t color) {
  st7796s_fill_rect(0, 0,  _width, _height, color);
}

int st7796s_init(void)
{
	uint8_t data[32];
	wiringPiSetupGpio();
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    
    int fd = wiringPiSPISetupMode(SPI_CHANNEL, SPI_CLOCK_SPEED, 0);
    if (fd == -1) {
        printf("Failed to init SPI communication.\n");
        return -1;
    }
    printf("SPI communication successfully setup.\n");
    
    usleep(120000);

	st7796s_write_command(0x01); //Software reset
	usleep(120000);

	st7796s_write_command(0x11); //Sleep exit                                            
	usleep(120000);

	st7796s_write_command(0xF0); //Command Set control                                 
	st7796s_write_data(0xC3);    //Enable extension command 2 partI
	
	st7796s_write_command(0xF0); //Command Set control                                 
	st7796s_write_data(0x96);    //Enable extension command 2 partII
	
	st7796s_write_command(0x36); //Memory Data Access Control MX, MY, RGB mode                                    
	st7796s_write_data(0x48);    //X-Mirror, Top-Left to right-Buttom, RGB  
	
	st7796s_write_command(0x3A); //Interface Pixel Format                                    
	st7796s_write_data(0x55);    //Control interface color format set to 16
	
	
	st7796s_write_command(0xB4); //Column inversion 
	st7796s_write_data(0x01);    //1-dot inversion

	st7796s_write_command(0xB6); //Display Function Control
	data[0] = 0x80;
	data[1] = 0x02;
	data[2] = 0x3B;
	st7796s_write_bytes(data, 3);    //Bypass


	st7796s_write_command(0xE8); //Display Output Ctrl Adjust
	data[0] = 0x40;
	data[1] = 0x8A;
	data[2] = 0x00;
	data[3] = 0x00;
	data[4] = 0x29;
	data[5] = 0x19;
	data[6] = 0xA5;
	data[7] = 0x33;
	st7796s_write_bytes(data, 8);
	
	st7796s_write_command(0xC1); //Power control2                          
	st7796s_write_data(0x06);    //VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)
	 
	st7796s_write_command(0xC2); //Power control 3                                      
	st7796s_write_data(0xA7);    //Source driving current level=low, Gamma driving current level=High
	 
	st7796s_write_command(0xC5); //VCOM Control
	st7796s_write_data(0x18);    //VCOM=0.9

	usleep(120000);
	
	//ST7796 Gamma Sequence
	st7796s_write_command(0xE0); //Gamma"+"
	data[0] = 0xF0;
	data[1] = 0x09;
	data[2] = 0x0B;
	data[3] = 0x06;
	data[4] = 0x04;
	data[5] = 0x15;
	data[6] = 0x2F;
	data[7] = 0x54;
	data[8] = 0x42;
	data[9] = 0x3C;
	data[10] = 0x17;
	data[11] = 0x14;
	data[12] = 0x18;
	data[13] = 0x1B;                                             
	st7796s_write_bytes(data, 14);
	 
	st7796s_write_command(0xE1); //Gamma"-"
	data[0] = 0xE0;
	data[1] = 0x09;
	data[2] = 0x0B;
	data[3] = 0x06;
	data[4] = 0x04;
	data[5] = 0x03;
	data[6] = 0x2B;
	data[7] = 0x43;
	data[8] = 0x42;
	data[9] = 0x3B;
	data[10] = 0x16;
	data[11] = 0x14;
	data[12] = 0x17;
	data[13] = 0x1B;                                               
	st7796s_write_bytes(data, 14);

	usleep(120000);
	
	st7796s_write_command(0xF0); //Command Set control                                 
	st7796s_write_data(0x3C);    //Disable extension command 2 partI

	st7796s_write_command(0xF0); //Command Set control                                 
	st7796s_write_data(0x69);    //Disable extension command 2 partII

	usleep(120000);

	st7796s_write_command(0x29); //Display on
	
	st7796s_set_rotation(0);
	
	return 0;
}
