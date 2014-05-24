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

/*-----------------------SAMPLES----------------------*/
void print_hollow_heart(unsigned char y, unsigned char x, unsigned char lag);
void print_clock_dot(unsigned char y, unsigned char x, unsigned char on);
void blink_clock_dot(unsigned char y, unsigned char x, unsigned char lag);
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

  ol_send_str (0, 0, "stated on 01:00", 0);

  while(1)
  {
    for (int i = 0; i < 24; i++)
    {
      for (int j = 0; j < 60; j++)
      {
        for (int k = 0; k < 60; k++)
        {
          sprintf (str, "%d%d %d%d %d%d", (i - i%10)/10, i%10, (j - j%10)/10, j%10, (k - k%10)/10, k%10); 
          ol_send_str (3, 3, str, 0);

          blink_clock_dot (3, 5, 500);
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
  ol_send_command(0xae);    //display off
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
  unsigned char i;

  ol_setXY(y, x);

  while(*string)
  {
    for(i=0;i<8;i++)
    {
      ol_send_char(myFont[*string - 0x20][i]);
      delay(lag);
    }
    *string++;
  }
}

void ol_print_16n(unsigned char y, unsigned char x, unsigned char num)
{
  unsigned char max_num = 10*4; 
  unsigned char num_idx = num * 4;
  
  if ((num_idx > 0) && (num_idx >= (10 * 4 - 4)))
    return;
  
  unsigned char t_x = x;
  
  for (int i = num_idx; i <  num_idx+4; i++)
  {
    ol_setXY(y, t_x);
    for (int j = 0; j < 8; j++)
    {
      ol_send_char(num16[i][j]);
    } 
    t_x++;
    if ((i - num_idx) == 1)
    {
      y++;
      t_x = x;
    }
  }
}

void ol_print_char(unsigned char ascii)
{
  unsigned char i=0;
  for(i = 0; i < 8; i++)
    ol_send_char(myFont[ascii - 0x20][i]);
}

/*----------------------------------------------------*/

/*-----------------------SAMPLES----------------------*/

void print_hollow_heart(unsigned char y, unsigned char x, unsigned char lag)
{
  unsigned char i, j;

  ol_setXY(y, x);

  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < 8; j++)
    {
      ol_send_char (hollow_heart[i][j]);
      delay(lag);
    }
  }
}

void print_clock_dot(unsigned char y, unsigned char x, unsigned char on)
{
  if (on)
  {
    ol_setXY(y, x);
    for (int j = 0; j < 8; j++)
    {
      ol_send_char(dot[0][j]);
    }
  } else {
    ol_setXY(y,x);
    ol_print_char(' ');
    ol_setXY(y,x+3);
    ol_print_char(' ');
  }
}

void blink_clock_dot(unsigned char y, unsigned char x, unsigned char lag)
{
  print_clock_dot (y, x, 1);
  delay(lag);
  print_clock_dot (y, x, 0);
  delay(lag);
}

/*----------------------------------------------------*/
