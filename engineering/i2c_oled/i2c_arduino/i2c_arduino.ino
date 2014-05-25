#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "SoftI2CMaster.h"
#include "ol_font.h"
#include "ol_samples.h"

#define SDA 7
#define SCL 6

#define OLEDaddr 0x3c

SoftI2CMaster _i2c = SoftI2CMaster(SDA, SCL);

/*-----------------------SYSTEM-----------------------*/
void ol_init(unsigned char reverse);
void ol_mem_reset(void);
void ol_send_command(unsigned char com);
/*----------------------------------------------------*/

/*----------------------GRAPHICS----------------------*/
void ol_setXY(unsigned char row, unsigned char col);
void ol_send_char(unsigned char data);
void ol_clear_display(void);
void ol_clear_area(unsigned char x1, unsigned char y1,
                   unsigned char x2, unsigned char y2);
void ol_send_str(unsigned char y, unsigned char x, char *string, unsigned char lag);
void ol_print_16n(unsigned char y, unsigned char x, unsigned char num);
void ol_print_char(unsigned char ascii);
/*----------------------------------------------------*/

void setup()
{
  ol_init(1);
  ol_clear_display();
  delay(50);
}

void loop()
{
  unsigned char posX = 0;
  unsigned char posY = 0;
  char str[10] = {0};
  char str1[4] = {0};

//  ol_send_str (0, 0, "stated on 01:00", 0);

while(1)
  {
    for (int i = 0; i < 24; i++)
    {
      for (int j = 0; j < 60; j++)
      {
        sprintf (str, "%d%d %d%d", (i - i%10)/10, i%10, (j - j%10)/10, j%10);
        printBigTime(str);
        for (int k = 0; k < 60; k++)
        {
           sprintf(str1, "%d%d", (k - k%10)/10, k%10);
           sendStrXY(str1, 0, 14);
//           delay(1000);
           printBigNumber(':', 2, 6);
           delay(500);
           printBigNumber(' ', 2, 6);
           delay(500);
        }
      }
    }
  }
}

/*-----------------------SYSTEM-----------------------*/

void ol_send_command(unsigned char com)
{
  _i2c.beginTransmission(OLEDaddr);
  _i2c.send(0x80);
  _i2c.send(com);
  _i2c.endTransmission();
}

void ol_init(unsigned char reverse)
{
/*  ol_send_command(0xae);    //display off
  delay(50); 
  if (reverse)
  {
    ol_send_command(0xa0);    //seg re-map 0->127(default)
    ol_send_command(0xa1);    //seg re-map 127->0
    ol_send_command(0xc8);
    delay(1000);
  }
  
  ol_send_command(0xaf);    //display on
  delay(50); 
*/
  ol_send_command(0xae);		//display off
  ol_send_command(0xa6);            //Set Normal Display (default)
    // Adafruit Init sequence for 128x64 OLED module
    ol_send_command(0xAE);             //DISPLAYOFF
    ol_send_command(0xD5);            //SETDISPLAYCLOCKDIV
    ol_send_command(0x80);            // the suggested ratio 0x80
    ol_send_command(0xA8);            //SSD1306_SETMULTIPLEX
    ol_send_command(0x3F);
    ol_send_command(0xD3);            //SETDISPLAYOFFSET
    ol_send_command(0x0);             //no offset
    ol_send_command(0x40 | 0x0);      //SETSTARTLINE
    ol_send_command(0x8D);            //CHARGEPUMP
    ol_send_command(0x14);
    ol_send_command(0x20);             //MEMORYMODE
    ol_send_command(0x00);             //0x0 act like ks0108
    
    ol_send_command(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg
    //ol_send_command(0xA0);
    
    ol_send_command(0xC8);            //COMSCANDEC  Rotate screen 180 Deg
    //ol_send_command(0xC0);
    
    ol_send_command(0xDA);            //0xDA
    ol_send_command(0x12);           //COMSCANDEC
    ol_send_command(0x81);           //SETCONTRAS
    ol_send_command(0xCF);           //
    ol_send_command(0xd9);          //SETPRECHARGE 
    ol_send_command(0xF1); 
    ol_send_command(0xDB);        //SETVCOMDETECT                
    ol_send_command(0x40);
    ol_send_command(0xA4);        //DISPLAYALLON_RESUME        
    ol_send_command(0xA6);        //NORMALDISPLAY             

  ol_clear_display();
  ol_send_command(0x2e);            // stop scroll
  //----------------------------REVERSE comments----------------------------//
  //  ol_send_command(0xa0);		//seg re-map 0->127(default)
  //  ol_send_command(0xa1);		//seg re-map 127->0
  //  ol_send_command(0xc8);
  //  delay(1000);
  //----------------------------REVERSE comments----------------------------//
  // ol_send_command(0xa7);  //Set Inverse Display  
  // ol_send_command(0xae);		//display off
  ol_send_command(0x20);            //Set Memory Addressing Mode
  ol_send_command(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
  //  ol_send_command(0x02);         // Set Memory Addressing Mode ab Page addressing mode(RESET)  
  
  ol_setXY(0,0);
  ol_send_command(0xaf);		//display on
}

void ol_mem_reset(void)
{
  ol_send_command(0x20);            //Set Memory Addressing Mode
  ol_send_command(0x02);            //Set Memory Addressing Mode ab Page addressing mode(RESET)
}

/*----------------------------------------------------*/

/*----------------------GRAPHICS----------------------*/
void ol_setXY(unsigned char row, unsigned char col)
{
  ol_send_command(0xb0 + row);                //set page address
  ol_send_command(0x00 + (8*col & 0x0f));       //set low col address
  ol_send_command(0x10 + ((8*col>>4) & 0x0f));  //set high col address
}

void ol_send_char(unsigned char data)
{
  _i2c.beginTransmission(OLEDaddr);
  _i2c.send(0x40);
  _i2c.send(data);
  _i2c.endTransmission();
}

void ol_clear_display(void)
{
  unsigned char i, k;
  for(k = 0; k < 8; k++)
  { 
    ol_setXY(k, 0);
    for(i = 0; i < 128; i++)
      ol_send_char(0);
  }
}

void ol_clear_area(unsigned char x1, unsigned char y1,
                   unsigned char x2, unsigned char y2)
{

}

void ol_send_str(unsigned char y, unsigned char x, char *string, unsigned char lag)
{
   unsigned char i=0;
  while(*string)
  {
    for(i=0;i<8;i++)
    {
      ol_send_char(pgm_read_byte(myFont[*string-0x20]+i));
      delay(lag);
    }
    *string++;
  }
}

static void printBigNumber(char string, int X, int Y)
{    
  ol_setXY(X,Y);
  int salto=0;
  for(int i=0;i<96;i++)
  {
    if(string == ' ') {
      ol_send_char(0);
    } else 
      ol_send_char(pgm_read_byte(bigNumbers[string-0x30]+i));
   
    if(salto == 23) {
      salto = 0;
      X++;
      ol_setXY(X,Y);
    } else {
      salto++;
    }
  }
}

static void printBigTime(char *string)
{

  int Y;
  int lon = strlen(string);
  if(lon == 3) {
    Y = 0;
  } else if (lon == 2) {
    Y = 3;
  } else if (lon == 1) {
    Y = 6;
  }
  
  int X = 2;
  while(*string)
  {
    printBigNumber(*string, X, Y);
    
    Y+=3;
    X=2;
    ol_setXY(X,Y);
    *string++;
  }
}

/*----------------------------------------------------*/

static void sendStrXY( char *string, int X, int Y)
{
  ol_setXY(X,Y);
  unsigned char i=0;
  while(*string)
  {
    for(i=0;i<8;i++)
    {
      ol_send_char(pgm_read_byte(myFont[*string-0x20]+i));
    }
    *string++;
  }
}
