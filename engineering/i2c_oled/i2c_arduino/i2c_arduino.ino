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
#define BUT 5

#define GO_OFF_TIME 10

#define OLED 0x3c
#define DS 0x68

enum {
      SHOW_DATE = 0,
      SLEEP_MODE,
      SET_HOURS,
      SET_MINUTES,
      SET_DAY,
      SET_DAYMONTH,
      SET_MONTH,
      SET_YEAR,
      STATE_MAX
};

typedef struct {
  unsigned char hours;
  unsigned char minutes;
  unsigned char seconds;
} time_t;

typedef struct {
  unsigned char year;
  unsigned char month;
  unsigned char day;
  unsigned char weekday;
} date_t;

typedef struct {
  time_t time;
  date_t date;
} clock_t;

clock_t clock;

SoftI2CMaster _i2c = SoftI2CMaster(SDA, SCL);

unsigned char working_time = 0;

int state = SHOW_DATE;

/*-------OLED START-----------------------------------*/
/*-----------------------SYSTEM-----------------------*/
void ol_display_on(void);
void ol_display_off(void);
void ol_init         (unsigned char reverse);
void ol_mem_reset    (void);
void ol_send_command (unsigned char com);
/*----------------------------------------------------*/

/*----------------------GRAPHICS----------------------*/
void ol_setXY         (unsigned char row, unsigned char col, int mode);
void ol_send_char     (unsigned char data);
void ol_clear_display (void);
void ol_clear_area    (unsigned char x1, unsigned char y1,
                       unsigned char x2, unsigned char y2);
void ol_send_str      (unsigned char y, unsigned char x, char *string, unsigned char lag);
void printBigNumber   (char string, int X, int Y);
void printBigTime     (char *string);
/*----------------------------------------------------*/
/*-------OLED END-------------------------------------*/

/*-------DS START-------------------------------------*/
byte bcdToDec       (byte val);
byte decToBcd       (byte val);
void ds_time_update ();
void ds_time_set    ();
/*-------DS END---------------------------------------*/

/*-------CLOCK START----------------------------------*/
void smart_delay(unsigned char lag);
void clock_sleep();
void clock_set_state (int t_state);
void clock_print_time();
/*-------CLOCK END------------------------------------*/

void setup()
{
  ol_init(1);
  ol_clear_display();
  delay(50);

  memset (&clock, 0, sizeof (clock_t));

  clock.time.hours = 0;
  clock.time.minutes = 31;
  clock.time.seconds = 20;
  
  ds_time_set();
  
  Serial.begin(9600);
}

void loop()
{
//  printBigTime("14:01");

  switch (state)
  {
    case SET_HOURS:    /*setHours();    clock_set_state (SET_MINUTES);*/  break;
    case SET_MINUTES:  /*setMinutes();  clock_set_state (SET_DAY);*/      break;
    case SET_DAY:      /*setDay();      clock_set_state (SET_DAYMONTH);*/ break;
    case SET_DAYMONTH: /*setDayMonth(); clock_set_state (SET_MONTH);*/    break;
    case SET_MONTH:    /*setMonth();    clock_set_state (SET_YEAR);*/     break;
    case SET_YEAR:     /*setYear();     clock_set_state (SHOW_DATE);*/    break;
    case SLEEP_MODE:   /*clock_sleep();*/                           break;
    case SHOW_DATE:    clock_print_time();                      break;
  }

 // working_time++;

  if (working_time == GO_OFF_TIME)
  {
    clock_set_state(SLEEP_MODE);
    working_time = 0;
  }

//  delay(100);
//  clock_button_check();
}

/*-------OLED START-----------------------------------*/

void ol_display_on(void)
{
    ol_send_command(0xaf);
}

void ol_display_off(void)
{
  ol_send_command(0xae);
}

void ol_send_command(unsigned char com)
{
  _i2c.beginTransmission(OLED);
  _i2c.send(0x80);
  _i2c.send(com);
  _i2c.endTransmission();
}

/*---------------------------------------------------*/

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
  
  ol_setXY(0,0,0);
  ol_send_command(0xaf);		//display on
}

/*---------------------------------------------------*/

void ol_mem_reset(void)
{
  ol_send_command(0x20);            //Set Memory Addressing Mode
  ol_send_command(0x02);            //Set Memory Addressing Mode ab Page addressing mode(RESET)
}

/*----------------------------------------------------*/

void ol_setXY(unsigned char row, unsigned char col, int mode)
{
  ol_send_command(0xb0 + row);                //set page address
//  ol_send_command(0x00 + (8*col & 0x0f));       //set low col address
//  ol_send_command(0x10 + ((8*col>>4) & 0x0f));  //set high col address

ol_send_command(0x00 + (((mode)? 1:8)*col & 0x0f));       //set low col address
ol_send_command(0x10 + ((((mode)? 1:8)*col>>4) & 0x0f));  //set high col address
}

/*---------------------------------------------------*/

void ol_send_char(unsigned char data)
{
  _i2c.beginTransmission(OLED);
  _i2c.send(0x40);
  _i2c.send(data);
  _i2c.endTransmission();
}

/*---------------------------------------------------*/

void ol_clear_display(void)
{
  unsigned char i, k;
  for(k = 0; k < 8; k++)
  { 
    ol_setXY(k, 0,0);
    for(i = 0; i < 128; i++)
      ol_send_char(0);
  }
}

/*---------------------------------------------------*/

void ol_clear_area(unsigned char x1, unsigned char y1,
                   unsigned char x2, unsigned char y2)
{
  do {
      ol_setXY(x1, 0, 0);
      for(int k = y1; k < 8*y2; k++)
        ol_send_char(0);
    x1++;
  } while (x1 <= x2);  
  

/*  for (int i = x2; i != x1; i++)
  {
    for (int j = y1; j != y2; j++)
    {
      ol_setXY(i, j, 0);
      for(int k = 0; k < 128; k++)
        ol_send_char(0);
    }
  }
  */
}

/*---------------------------------------------------*/

void ol_send_str(unsigned char y, unsigned char x, char *string, unsigned char lag)
{
  unsigned char i=0;
  ol_setXY(y, x, 0);
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

/*---------------------------------------------------*/

void printBigNumber(char string, int X, int Y)
{
  ol_setXY(X,Y,0);
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
      ol_setXY(X,Y,0);
    } else {
      salto++;
    }
  }
}

/*---------------------------------------------------*/

void printBigTime(char *string)
{
  int Y = 0;
  int lon = strlen(string);
/*  if(lon == 3) {
    Y = 0;
  } else if (lon == 2) {
    Y = 3;
  } else if (lon == 1) {
    Y = 6;
  }
  */
  int X = 1;
  while(*string)
  {
    ol_setXY(X,Y,0);
    printBigNumber(*string, X, Y);
    
    Y+=3;
    X=1;
    *string++;
  }
}

int prev_line_pos = 0;
int prev_sec = 0;
void printSecLine(int seconds)
{
  //seconds = 55;
  int end_pos = 0; 

  if ((!prev_line_pos) && seconds)
  {
    for (int i = 0; i < seconds; i++)
      end_pos += 2;
      
    if (end_pos > 120)
      end_pos = 120;
  } else {
    end_pos = prev_line_pos + 2;
  }

  if (prev_line_pos && ((seconds - prev_sec) < 0) || (prev_line_pos >= 120))
  {
    ol_clear_area (6, 0, 7, 16);
    prev_line_pos = 0;
    end_pos = 0;
    prev_sec = 0;
    return;
  }

  int i;
  for (i = prev_line_pos; i < end_pos; i++)
  {
    ol_setXY(7, i, 1);
    char str1[4] = {0};
    sprintf(str1, "%d%d", (seconds - seconds%10)/10, seconds%10);
 
    ol_send_char(0x7f);
    
    ol_setXY(6, (i < 12)? 0:i-12, 1);
    for(int j=0;j<8;j++)
    {
      ol_send_char(pgm_read_byte(myFont[str1[0]-0x20]+j));
      //delay(lag);
    }
    ol_setXY(6, (i < 12)? 8:i-4, 1);
    for(int k=0;k<8;k++)
    {
      ol_send_char(pgm_read_byte(myFont[str1[1]-0x20]+k));
      //delay(lag);
    }
  }

  prev_line_pos = i;
}

void printSecPoint (int mode, int seconds)
{
  int i;
  int start_pos = mode? 0:127;
  int end_pos = mode? 127:0;
  int delta = 1 * mode? 1:-1;
 
  for (i = start_pos; i != end_pos; i+=delta)
  {
    ol_setXY(7, i, 1);
    ol_send_char(0x7f);

    ol_setXY(7, mode? i-1:i+1, 1);
    ol_send_char(0);
  }
}

/*-------OLED END-------------------------------------*/

/*-------DS START-------------------------------------*/
byte bcdToDec(byte val)
{
  return ((val/16*10) + (val%16));
}

/*---------------------------------------------------*/

byte decToBcd(byte val)
{
  return ((val/10*16) + (val%10));
}

/*---------------------------------------------------*/

void ds_time_update ()
{
  _i2c.beginTransmission(DS);
  _i2c.send(0);
  _i2c.endTransmission();

  _i2c.requestFrom(DS);
//  _i2c.send(0x7); //FUCKING SHIT!!!!!!1111!!!111!!!11!!!
//  _i2c.send(0x10);

  clock.time.seconds  = bcdToDec(_i2c.receive() & 0x7f);
  clock.time.minutes  = bcdToDec(_i2c.receive());
  clock.time.hours    = bcdToDec(_i2c.receive() & 0x3f); //need to change this if 12 hour am/pm
  clock.date.weekday = bcdToDec(_i2c.receive());
  clock.date.day     = bcdToDec(_i2c.receive());
  clock.date.month   = bcdToDec(_i2c.receive());
  clock.date.year    = bcdToDec(_i2c.receive());
}

/*---------------------------------------------------*/

void ds_time_set ()
{
  _i2c.beginTransmission(DS);
  _i2c.send(0);
  _i2c.send(decToBcd(clock.time.seconds));
  _i2c.send(decToBcd(clock.time.minutes));
  _i2c.send(decToBcd(clock.time.hours));
  _i2c.send(decToBcd(clock.date.weekday));
  _i2c.send(decToBcd(clock.date.day));
  _i2c.send(decToBcd(clock.date.month));
  _i2c.send(decToBcd(clock.date.year));
  _i2c.endTransmission();
}

/*-------DS END---------------------------------------*/

/*-------CLOCK START----------------------------------*/
void smart_delay(unsigned char lag)
{
  unsigned char i;
  for (i = 0; i < lag; i++)
  {
    if (digitalRead(BUT))
      break;

    delay(100);
  }
}

/*---------------------------------------------------*/

void clock_sleep()
{
  ol_display_off();

  while (1)
  {
    if (digitalRead(BUT))
    {
      clock_set_state(SHOW_DATE);
      ol_display_on();
      break;
    }

    smart_delay(100);
  }
}

/*---------------------------------------------------*/

void clock_set_state (int t_state)
{
  state = t_state;
}

/*---------------------------------------------------*/
void clock_print_time()
{
  char str_time[6] = {0};

  unsigned char *hours;
  unsigned char *minutes;
  unsigned char *seconds;

  ds_time_update();

  hours = &clock.time.hours;
  minutes = &clock.time.minutes;
  seconds = &clock.time.seconds;

  if ((*hours <= 0   || *hours > 23)     ||
      (*minutes <= 0 || *minutes > 59)   ||
      (*seconds <= 0 || *seconds > 59))
  {
    ol_send_str(0, 1, "ds1307", 0);
    ol_send_str(1, 1, "return", 0);
    ol_send_str(2, 1, "invalid", 0);
    ol_send_str(3, 1, "time :(", 0);
    return;
  }

  sprintf (str_time, "%d%d:%d%d", (*hours - *hours%10)/10, *hours%10, (*minutes - *minutes%10)/10, *minutes%10);

  printBigTime(str_time);

    char str1[4] = {0};
    sprintf(str1, "%d%d", (*seconds - *seconds%10)/10, *seconds%10);
    ol_send_str(0, 14, str1, 0);

  pacman_animation();
//  clock_animation(seconds);
//  secLine_animation(seconds);
//  secPoint_animation(seconds);

}

/*-------CLOCK END------------------------------------*/
void secPoint_animation(int seconds)
{
  if (prev_sec == seconds)
    return;

  printSecPoint(seconds%2, seconds);
//  printSecPoint(0, seconds);

  prev_sec = seconds;
}
void secLine_animation(int seconds)
{
  if (prev_sec == seconds)
    return;

  printSecLine(seconds);
  
  prev_sec = seconds;
}
void clock_animation(int seconds)
{
  if (prev_sec == seconds)
    return;

  if (seconds%2)
    printBigNumber(':', 1, 6);
  else
    printBigNumber(' ', 1, 6);

  prev_sec = seconds;
}

void pacman_animation()
{
  printGhost(6, 0, 0);
  printPacman(6, 3, 1);
  dot_line();
  printGhost(6, 0, 1);
  printPacman(6, 3, 0);
}

void printPacman (unsigned char X, unsigned char Y, unsigned char mode)
{
  if (mode > 2)
    return;

  ol_setXY(X,Y,0);
  int salto=0;
  for(int i=0;i<32;i++)
  {
    ol_send_char(pgm_read_byte(pacman[mode]+i));

    if(salto == 15) {
      salto = 0;
      X++;
      ol_setXY(X,Y,0);
    } else {
      salto++;
    }
  }
}

void printGhost (unsigned char X, unsigned char Y, unsigned char mode)
{
  ol_setXY(X,Y,0);
  int salto=0;
  for(int i=0;i<32;i++)
  {
    ol_send_char(pgm_read_byte(ghost[!!mode]+i));

    if(salto == 15) {
      salto = 0;
      X++;
      ol_setXY(X,Y,0);
    } else {
      salto++;
    }
  }
}

void printDot (unsigned char X, unsigned char Y)
{
  ol_setXY(X,Y,0);
  int salto=0;
  for(int i=0;i<16;i++)
  {
    ol_send_char(pgm_read_byte(dot+i));

    if(salto == 7) {
      salto = 0;
      X++;
      ol_setXY(X,Y,0);
    } else {
      salto++;
    }
  }
}

static int first_dot_pos = 14;
static int mode = 0;
void dot_line ()
{  
  for (int i = 4; i != 15; i+=1)
  {
    if (!mode)
    {
     if (i%2)
     {     
       ol_send_str(6, i+1, " ", 0);
       ol_send_str(7, i+1, " ", 0);
       printDot(6, i); 
       printDot(6, 15);
     }
    } else
     if (!(i%2))
     {
       ol_send_str(6, i+1, " ", 0);
       ol_send_str(7, i+1, " ", 0);
       if (i != 4)
         printDot(6, i);
    }
  }

  if (mode){
    printPacman(6, 3, 2);
    delay(100);
  }

  if (first_dot_pos >= 4)
    first_dot_pos -= 1;

    mode = !mode;
}
