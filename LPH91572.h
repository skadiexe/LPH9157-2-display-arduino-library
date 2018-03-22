//--------Библиотека дисплея Siemens C75, МЕ75---------
//           Зеленый текстолит LPH9157-2
//              132х176 пикселей
//          ==== Кизим Игорь ====
//-----------------------------------------------------


#include "Symbols.cpp"
#include "LPH91572.cpp"

// uncomment to use 8 bit colors
//#define _8_BIT_COLOR

// allow to use geometrical functions  
#define _GEOMETRICAL  
// uncomment to use additional reset pin
//#define _USE_HARDWARE_RESET

//===============================================================
//		            Назначение выводов порта
//===============================================================

#ifdef _USE_HARDWARE_RESET
#define LCD_RESET B1 // hardware reset pin
#endif

#define LCD_RS 		D0	//RS pin (command/data select)

//*************************************************************
// command/data
#define CMD 0
#define DAT 1
// to store previous RS value and not drive this pin while continious data streaming
uint8_t RS_old;


#ifdef _8_BIT_COLOR
//8 bit color samples (256 colors)
#define GREEN       0x1C
#define DARK_GREEN  0x15
#define RED         0xE0
#define BLUE        0x1F
#define DARK_BLUE   0x03
#define YELLOW      0xFC
#define ORANGE      0xEC
#define VIOLET      0xE3
#define WHITE       0xFF
#define BLACK       0x00
#define GREY        0x6D

#else
//16 bit (RGB565) color samples (65536 colors)
#define    BLACK                0x0000 //
#define    WHITE                0xFFFF //
#define    GRAY                 0xE79C //
#define    GREEN                0x07E0
#define    BLUE                 0x001F
#define    RED                  0xF800
#define    SKY                  0x5d1c
#define    YELLOW               0xffe0
#define    MAGENTA              0xf81f
#define    CYAN                 0x07ff
#define    ORANGE               0xfca0
#define    PINK                 0xF97F
#define    BROWN                0x8200
#define    VIOLET               0x9199
#define    SILVER               0xa510
#define    GOLD                 0xa508
#define    BEGH                 0xf77b
#define    NAVY                 0x000F
#define    DARK_GREEN           0x03E0
#define    DARK_CYAN            0x03EF
#define    MAROON               0x7800
#define    PURPLE               0x780F
#define    OLIVE                0x7BE0
#define    LIGHT_GREY           0xC618
#define    DARK_GREY            0x7BEF
#endif

//*************************************************************
//functions prototypes

void LCD_init (void);
void Send_to_lcd (uint8_t RS, uint8_t data);
void SetArea (char x1, char x2, char y1, char y2);
uint16_t spectrumToRBG565 (uint16_t color);
void Put_Pixel (char x, char y, unsigned int color);
void Send_Symbol (unsigned char symbol, char x, char y, int t_color, int b_color, char zoom_width, char zoom_height, int rot);
void LCD_Putchar (char symbol, char x, char y, int t_color, int b_color, char zoom_width, char zoom_height, int rot);
void LCD_Puts(char *str, int x, int y,  int t_color, int b_color, char zoom_width, char zoom_height, int rot);
//void LCD_Putsf(flash char *str, int x, int y,  int t_color, int b_color, char zoom_width, char zoom_height, int rot);
void Send_Symbol_Shadow (unsigned char symbol, char x, char y, int t_color, char zoom_width, char zoom_height, int rot);
void LCD_Putchar_Shadow (char symbol, char x, char y, int t_color, char zoom_width, char zoom_height, int rot);
void LCD_Puts_Shadow (char *str, int x, int y,  int t_color, char zoom_width, char zoom_height, int rot);
//void LCD_Putsf_Shadow (flash char *str, int x, int y,  int t_color, char zoom_width, char zoom_height, int rot);
void LCD_FillScreen (unsigned int color);
//void LCD_Output_image (char x, char y, char width, char height, flash char *img, int rot);
//void Send_Image (char x, char y, char width, char height, flash char *img, int rot);
#ifdef _GEOMETRICAL
void LCD_DrawLine (char x1, char y1, char x2, char y2, int color);
void LCD_DrawRect (char x1, char y1, char width, char height, char size, int color);
void LCD_FillRect (char x1, char y1, char width, char height, int color);
void LCD_DrawCircle (char xcenter, char ycenter, char rad, int color);
void LCD_FillCircle (char xcenter, char ycenter, char rad, int color);
void LCD_DrawTriangle (char x1, char y1, char x2, char y2, char x3, char y3, int color);
void LCD_FillTriangle (char x1, char y1, char x2, char y2, char x3, char y3, int color);
void LCD_FillTriangleA (char x1, char y1, char x2, char y2, char x3, char y3, int color);
#endif

//===============================================================
//                        init
//===============================================================
void LCD_init(void)
{ SPI.begin();
  pinMode(SS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  digitalWrite(SS, LOW);
  digitalWrite(LCD_RS, LOW);

 #ifdef _USE_HARDWARE_RESET
  pinMode(LCD_RESET, OUTPUT);
  digitalWrite(LCD_RESET, LOW);
  delay(20);
  digitalWrite(LCD_RESET, HIGH);
 #endif 

  Send_to_lcd(CMD, 0x01); //soft reset
  Send_to_lcd(CMD, 0x36); //Memory Access Control (Направление заполнения области дисплея (памяти): 0bVHRXXXXX, V - заполнение по вертикали (0 - сверху-вниз, 1 - снизу-вверх),
                          //H - заполнение по горизонтали (0 - слева-направо, 1 - справа-налево), R - меняются местами строки и столбцы (при этом заполнение остается сверху-вниз, слева-направо))
  Send_to_lcd(DAT, 0x00);
  Send_to_lcd(CMD, 0x11); //wake up from sleep mode
  delay(20);
  Send_to_lcd(CMD, 0x3a); //set color mode
#ifdef _8_BIT_COLOR
  Send_to_lcd(DAT, 0x02); //256 colors
#else
  Send_to_lcd(DAT, 0x05); //65536 colors
#endif
  delay(20);
  Send_to_lcd(CMD, 0x29); //power on display
}

//===============================================================
//Функция записи команды/данных в LCD (RS==0 - команда, RS==1 - данные)
//===============================================================


void Send_to_lcd (uint8_t RS, uint8_t data)
{ static uint8_t old_RS = 0;
  if ((old_RS != RS) || (!RS && !old_RS)) {
    digitalWrite(LCD_RS, RS);
  }
  // 30MHz is maximum for that display
  // standart speed is 13MHz

#if defined(__AVR__)
SPI.beginTransaction(SPISettings(10000000L, MSBFIRST, SPI_MODE0));
#elif defined(ESP8266)
SPI.beginTransaction(SPISettings(30000000L, MSBFIRST, SPI_MODE0));
#endif

  SPI.transfer(data);
  // removed for better speed
  // delayMicroseconds(5);
  //digitalWrite(SS, HIGH); 
  SPI.endTransaction();
}


/*

  void Send_to_lcd (uint8_t RS, uint8_t data)
  {
  //unsigned char count;
  LCD_CLK = 0;
  LCD_DATA = 0;
  if ((RS_old != RS) || (!RS_old && !RS)) //проверяем старое значение RS (если поступают одни команды то дергаем CS)
  {
  LCD_CS=1;	// Установка CS
  LCD_RS=RS;
  LCD_CS=0;	// Сброс CS
  }

*/
//******************************************************************************
//Такой цикл обеспечивает более компактный код при записи байта в дисплей
//******************************************************************************
/*
  for (count = 0; count < 8; count++) //Цикл передачи данных
  {
  if(data&0x80)		LCD_DATA=1;
  else			    LCD_DATA=0;
  LCD_CLK=1;
  data <<= 1;
  LCD_CLK=0;
  }
*/

/*
  //******************************************************************************
  //Такой прямой код (без цикла) обеспечивает более быструю запись байта в дисплей
  //******************************************************************************
  LCD_DATA = 0;
  if ((data & 128) == 128)  LCD_DATA = 1;
  LCD_CLK=1;
  LCD_CLK=0;
  LCD_DATA = 0;
  if ((data & 64) == 64)  LCD_DATA = 1;
  LCD_CLK=1;
  LCD_CLK=0;
  LCD_DATA = 0;
  if ((data & 32) == 32)  LCD_DATA = 1;
  LCD_CLK=1;
  LCD_CLK=0;
  LCD_DATA = 0;
  if ((data & 16) ==16)  LCD_DATA = 1;
  LCD_CLK=1;
  LCD_CLK=0;
  LCD_DATA = 0;
  if ((data & 8) == 8)  LCD_DATA = 1;
  LCD_CLK=1;
  LCD_CLK=0;
  LCD_DATA = 0;
  if ((data & 4) == 4)  LCD_DATA = 1;
  LCD_CLK=1;
  LCD_CLK=0;
  LCD_DATA = 0;
  if ((data & 2) == 2)  LCD_DATA = 1;
  LCD_CLK=1;
  LCD_CLK=0;
  LCD_DATA = 0;
  if ((data & 1) == 1)  LCD_DATA = 1;
  LCD_CLK=1;
  LCD_CLK=0;

  RS_old=RS;  //запоминаю значение RS
  LCD_DATA = 0;
  }
*/
//===============================================================
//              Задание прямоугольной области экрана
//===============================================================
void SetArea(char x1, char x2, char y1, char y2)
{
  Send_to_lcd( CMD, 0x2A );  //задаем область по X
  Send_to_lcd( DAT, x1 );    //начальная
  Send_to_lcd( DAT, x2 );    //конечная

  Send_to_lcd( CMD, 0x2B );  //задаем область по Y
  Send_to_lcd( DAT, y1 );    //начальная
  Send_to_lcd( DAT, y2 );    //конечная

  Send_to_lcd( CMD, 0x2C );  //отправляем команду на начало записи в память и начинаем посылать данные
}



uint16_t spectrumToRBG565 (uint16_t color)
{

  const uint8_t lights[360] = {
    0,   0,   0,   0,   0,   1,   1,   2,
    2,   3,   4,   5,   6,   7,   8,   9,
    11,  12,  13,  15,  17,  18,  20,  22,
    24,  26,  28,  30,  32,  35,  37,  39,
    42,  44,  47,  49,  52,  55,  58,  60,
    63,  66,  69,  72,  75,  78,  81,  85,
    88,  91,  94,  97, 101, 104, 107, 111,
    114, 117, 121, 124, 127, 131, 134, 137,
    141, 144, 147, 150, 154, 157, 160, 163,
    167, 170, 173, 176, 179, 182, 185, 188,
    191, 194, 197, 200, 202, 205, 208, 210,
    213, 215, 217, 220, 222, 224, 226, 229,
    231, 232, 234, 236, 238, 239, 241, 242,
    244, 245, 246, 248, 249, 250, 251, 251,
    252, 253, 253, 254, 254, 255, 255, 255,
    255, 255, 255, 255, 254, 254, 253, 253,
    252, 251, 251, 250, 249, 248, 246, 245,
    244, 242, 241, 239, 238, 236, 234, 232,
    231, 229, 226, 224, 222, 220, 217, 215,
    213, 210, 208, 205, 202, 200, 197, 194,
    191, 188, 185, 182, 179, 176, 173, 170,
    167, 163, 160, 157, 154, 150, 147, 144,
    141, 137, 134, 131, 127, 124, 121, 117,
    114, 111, 107, 104, 101,  97,  94,  91,
    88,  85,  81,  78,  75,  72,  69,  66,
    63,  60,  58,  55,  52,  49,  47,  44,
    42,  39,  37,  35,  32,  30,  28,  26,
    24,  22,  20,  18,  17,  15,  13,  12,
    11,   9,   8,   7,   6,   5,   4,   3,
    2,   2,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
  };

  if (color > 360) color = 360;
  uint8_t red = lights[(color + 120) % 360];
  uint8_t green  = lights[color];
  uint8_t blue  = lights[(color + 240) % 360];

  return uint16_t((((31 * (red + 4)) / 255) << 11) | (((63 * (green + 2)) / 255) << 5) |  ((31 * (blue + 4)) / 255));

}


//===============================================================
//                          Рисуем точку
//===============================================================
void Put_Pixel (char x, char y, unsigned int color)
{
  signed char i;
  SetArea( x, x, y, y );
  digitalWrite (LCD_RS, HIGH);

#ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
  Send_to_lcd( DAT, color ); //Данные - задаём цвет пикселя
#else			        //(16-ти битовая цветовая палитра (65536 цветов))
  Send_to_lcd( DAT, (color >> 8) );
  Send_to_lcd( DAT, color );
#endif
}

//===============================================================
//           Функция прорисовки символа на дисплее
//===============================================================
void Send_Symbol (unsigned char symbol, char x, char y, int t_color, int b_color, char zoom_width, char zoom_height, int rot)
{
  unsigned char temp_symbol, a, b, zw, zh, mask;

  if (symbol > 127) symbol -= 64; //Убираем отсутствующую часть таблицы ASCII
  for ( a = 0; a < 5; a++) //Перебираю 5 байт, составляющих символ
  {
    temp_symbol = font_5x8[symbol - 32][a];
    zw = 0;
    while (zw != zoom_width) //Вывод байта выполняется zw раз
    {
      mask = 0x01;
      switch (rot)
      {
        case 0: case 180: SetArea( x + zw, x + zw, y, y + (zoom_height * 8) - 1 ); break;
        case 90: case 270: SetArea( x, x + (zoom_height * 8) - 1, y + zw, y + zw ); break;
      }
      digitalWrite (LCD_RS, HIGH); //Передаются данные
      for ( b = 0; b < 8; b++ ) //Цикл перебирания 8 бит байта
      {
        zh = zoom_height; //в zoom_height раз увеличится высота символа
        while (zh != 0) //Вывод пикселя выполняется z раз
        {
          if (temp_symbol & mask)
          {
#ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
            Send_to_lcd( DAT, t_color ); //Данные - задаём цвет пикселя
#else			        //(16-ти битовая цветовая палитра (65536 цветов))
            Send_to_lcd( DAT, (t_color >> 8) ); Send_to_lcd( DAT, t_color );
#endif
          }
          else
          {
#ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
            Send_to_lcd( DAT, b_color ); //Данные - задаём цвет пикселя
#else			        //(16-ти битовая цветовая палитра (65536 цветов))
            Send_to_lcd( DAT, (b_color >> 8) ); Send_to_lcd( DAT, b_color );
#endif
          }
          zh--;
        }
        mask <<= 1; //Смещаю содержимое mask на 1 бит влево;
      }
      zw++;
    }
    switch (rot)
    {
      case 0: case 180: x = x + zoom_width;  break; //Получить адрес начального пикселя по оси x для вывода очередного байта
      case 90: case 270: y = y + zoom_width; break; //Получить адрес начального пикселя по оси y для вывода очередного байта
    }
  }
}

//===============================================================
// Функция вывода одного символа ASCII-кода (из файла Symbols.h)
//===============================================================
void LCD_Putchar(char symbol, char x, char y, int t_color, int b_color, char zoom_width, char zoom_height, int rot)
{
  unsigned char m;
  if (zoom_width == 0)   zoom_width = 1;
  if (zoom_height == 0)  zoom_height = 1;
  switch (rot)
  {
    case 0:  //Начальный адрес осей Х и У - левый верхний угол дисплея
      Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
      break;
    //================================
    case 90:
      m = y; y = x; x = m;
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x40); //Начальный адрес осей Х и У - правый верхний угол дисплея
      Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00);
      break;
    //================================
    case 180:
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0xC0); //Начальный адрес осей Х и У - правый нижний угол дисплея
      Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00);
      break;
    //================================
    case 270:
      m = y; y = x; x = m;
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x80); //Начальный адрес осей Х и У - левый нижний угол дисплея
      Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00);
      break;
    //================================
    default:
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00); //Начальный адрес осей Х и У - левый верхний угол дисплея
      Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00);
      //=================================
  };
}

//===============================================================
//          Функция вывода строки, расположенной в ram
//===============================================================
void LCD_Puts(char *str, int x, int y,  int t_color, int b_color, char zoom_width, char zoom_height, int rot)
{
  unsigned char i = 0;

  if (zoom_width == 0)   zoom_width = 1;
  if (zoom_height == 0)  zoom_height = 1;

  while (str[i]) //x и y - адрес пикселя начальной позиции; с увеличением переменной i адрес вывода очередного символа смещается на i*6 (чем организуются столбцы дисплея)
  {
    LCD_Putchar(str[i], x + (i * 6 * zoom_width), y, t_color, b_color, zoom_width, zoom_height, rot);
    i++;
  }
}

//===============================================================
//          Функция вывода строки, расположенной во flash
//===============================================================
/*void LCD_Putsf(flash char *str, int x, int y,  int t_color, int b_color, char zoom_width, char zoom_height, int rot)
  {
  unsigned char i = 0;

  if (zoom_width == 0)   zoom_width = 1;
  if (zoom_height == 0)  zoom_height = 1;

  while (str[i]) //x и y - адрес пикселя начальной позиции; с увеличением переменной i адрес вывода очередного символа смещается на i*6 (чем организуются столбцы дисплея)
  {
    LCD_Putchar(str[i], x + (i * 6 * zoom_width), y, t_color, b_color, zoom_width, zoom_height, rot);
    i++;
  }
  }
*/
//===============================================================
//     Функция прорисовки символа на дисплее без цвета фона
//===============================================================
void Send_Symbol_Shadow (unsigned char symbol, char x, char y, int t_color, char zoom_width, char zoom_height, int rot)
{
  unsigned char temp_symbol, a, b, zw, zh, mask;
  char m, n;
  m = x;
  n = y;
  if (symbol > 127) symbol -= 64; //Убираем отсутствующую часть таблицы ASCII
  for ( a = 0; a < 5; a++) //Перебираю 5 байт, составляющих символ
  {
    temp_symbol = font_5x8[symbol - 32][a];
    zw = 0;
    while (zw != zoom_width) //Вывод байта выполняется zw раз
    {
      switch (rot)
      {
        case 0: case 180: n = y; break;
        case 90: case 270: m = x; break;
      }
      mask = 0x01;
      for ( b = 0; b < 8; b++ ) //Цикл перебирания 8 бит байта
      {
        zh = 0; //в zoom_height раз увеличится высота символа
        while (zh != zoom_height) //Вывод пикселя выполняется z раз
        {
          switch (rot)
          {
            case 0: case 180:
              if (temp_symbol & mask)
              {
                Put_Pixel (m + zw, n + zh, t_color);
              }
              break;
            case 90: case 270:
              if (temp_symbol & mask)
              {
                Put_Pixel (m + zh, n + zw, t_color);
              }
              break; //Получить адрес начального пикселя по оси y для вывода очередного байта
          }
          zh++;
        }
        mask <<= 1; //Смещаю содержимое mask на 1 бит влево;
        switch (rot)
        {
          case 0: case 180: n = n + zoom_height; break;
          case 90: case 270: m = m + zoom_height; break;
        }
      }
      zw++;
    }
    switch (rot)
    {
      case 0: case 180: m = m + zoom_width; break;
      case 90: case 270: n = n + zoom_width; break;
    }
  }
}

//===============================================================
// Функция вывода одного символа ASCII-кода без цвета фона
//===============================================================
void LCD_Putchar_Shadow (char symbol, char x, char y, int t_color, char zoom_width, char zoom_height, int rot)
{
  unsigned char m;
  if (zoom_width == 0)   zoom_width = 1;
  if (zoom_height == 0)  zoom_height = 1;
  switch (rot)
  {
    case 0:  //Начальный адрес осей Х и У - левый верхний угол дисплея
      Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
      break;
    //================================
    case 90:
      m = y; y = x; x = m;
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x40); //Начальный адрес осей Х и У - правый верхний угол дисплея
      Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00);
      break;
    //================================
    case 180:
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0xC0); //Начальный адрес осей Х и У - правый нижний угол дисплея
      Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00);
      break;
    //================================
    case 270:
      m = y; y = x; x = m;
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x80); //Начальный адрес осей Х и У - левый нижний угол дисплея
      Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00);
      break;
    //================================
    default:
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00); //Начальный адрес осей Х и У - левый верхний угол дисплея
      Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
      Send_to_lcd(CMD, 0x36);
      Send_to_lcd(DAT, 0x00);
      //=================================
  };
}

//===============================================================
//   Функция вывода строки, расположенной в ram без цвета фона
//===============================================================
void LCD_Puts_Shadow(char *str, int x, int y,  int t_color, char zoom_width, char zoom_height, int rot)
{
  unsigned char i = 0;

  if (zoom_width == 0)   zoom_width = 1;
  if (zoom_height == 0)  zoom_height = 1;

  while (str[i]) //x и y - адрес пикселя начальной позиции; с увеличением переменной i адрес вывода очередного символа смещается на i*6 (чем организуются столбцы дисплея)
  {
    LCD_Putchar_Shadow(str[i], x + (i * 6 * zoom_width), y, t_color, zoom_width, zoom_height, rot);
    i++;
  }
}

//===============================================================
// Функция вывода строки, расположенной во flash без цвета фона
//===============================================================
/*
  void LCD_Putsf_Shadow(flash char *str, int x, int y,  int t_color, char zoom_width, char zoom_height, int rot)
  {
  unsigned char i=0;

  if(zoom_width == 0)   zoom_width = 1;
  if(zoom_height == 0)  zoom_height = 1;

  while (str[i])
  {
  LCD_Putchar_Shadow(str[i], x+(i*6*zoom_width), y, t_color, zoom_width, zoom_height, rot);
  i++;
  }
  }
*/

//===============================================================
//                  ЗАЛИВКА ЭКРАНА ЦВЕТОМ
//===============================================================
void LCD_FillScreen (unsigned int color)
{
  unsigned int x;
  signed char i;
  SetArea( 0, 131, 0, 175 );   //Область всего экрана
  digitalWrite(LCD_RS, HIGH);

  //Данные - задаём цвет пикселя
  for (x = 0; x < 23232; x++)  // 23232 - это 132 * 176
  {
#ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( DAT, color ); //Данные - задаём цвет пикселя
#else			//(16-ти битовая цветовая палитра (65536 цветов))
    Send_to_lcd( DAT, (color >> 8) ); Send_to_lcd( DAT, color );
#endif
  }
}

//===============================================================
//                 ФУНКЦИЯ ВЫВОДА ИЗОБРАЖЕНИЯ
//===============================================================
/*
  void LCD_Output_image (char x, char y, char width, char height, flash char *img, int rot)
  {
  unsigned char m;
  switch (rot)
  {
  case 0:
  Send_Image (x, y, width, height, img, rot);
  break;
  //================================
  case 90:
  m=y; y=x; x=m;
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x40); //Начальный адрес осей Х и У - правый верхний угол дисплея
  Send_Image (x, y, width, height, img, rot);
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x00);
  break;
  //================================
  case 180:
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0xC0); //Начальный адрес осей Х и У - правый нижний угол дисплея
  Send_Image (x, y, width, height, img, rot);
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x00);
  break;
  //================================
  case 270:
  m=y; y=x; x=m;
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x80); //Начальный адрес осей Х и У - левый нижний угол дисплея
  Send_Image (x, y, width, height, img, rot);
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x00);
  break;
  //================================
  default:
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x00); //Начальный адрес осей Х и У - левый верхний угол дисплея
  Send_Image (x, y, width, height, img, rot);
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x00);
  //=================================
  };
  }
*/
//===============================================================
//          Функция для обеспечения вывода изображения
//===============================================================
//Вывод картинки с Image2Lcd и NokiaImageCreator должен выполняться слева-направо сверху-вниз.
//x, y - начало области вывода изображения; width и height - ширина и высота изображения
/*
  void Send_Image (char x, char y, char width, char height, flash char *img, int rot)
  {
  char x1, y1;

  switch (rot)
  {
  case 0: case 180:
  for(y1=y; y1<(y+height); y1++)
  {
   SetArea( x, x+(width-1), y1, y1 );
   for(x1=x; x1<x+width; x1++)
   {
    #ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( DAT, *img++ ); //Данные - задаём цвет пикселя
    #else			//(16-ти битовая цветовая палитра (65536 цветов))
    Send_to_lcd( DAT, *img++ ); Send_to_lcd( DAT, *img++ );
    #endif
   }
  }
  break;

  case 90: case 270:
  for(x1=x; x1<x+height; x1++)
  {
   SetArea( x1, x1, y, y+(width-1) );
   for(y1=y; y1<y+width; y1++)
   {
    #ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( DAT, *img++ ); //Данные - задаём цвет пикселя
    #else			//(16-ти битовая цветовая палитра (65536 цветов))
    Send_to_lcd( DAT, *img++ ); Send_to_lcd( DAT, *img++ );
    #endif
   }
  }
  break;
  };
  }
*/
#ifdef _GEOMETRICAL
//===============================================================
//                      НАРИСОВАТЬ ЛИНИЮ
//===============================================================
void LCD_DrawLine (char x1, char y1, char x2, char y2, int color)
{
  short  x, y, d, dx, dy, i, i1, i2, kx, ky;
  signed char flag;

  dx = x2 - x1;
  dy = y2 - y1;
  if (dx == 0 && dy == 0) Put_Pixel(x1, y1, color);  //Точка
  else      //Линия
  {
    kx = 1;
    ky = 1;
    if ( dx < 0 )
    {
      dx = -dx;
      kx = -1;
    }
    else if (dx == 0) kx = 0;
    if (dy < 0)
    {
      dy = -dy;
      ky = -1;
    }
    if (dx < dy)
    {
      flag = 0;
      d = dx;
      dx = dy;
      dy = d;
    }
    else flag = 1;
    i1 = dy + dy;
    d = i1 - dx;
    i2 = d - dx;
    x = x1;
    y = y1;

    for (i = 0; i < dx; i++)
    {
      Put_Pixel(x, y, color);
      if (flag) x += kx;
      else y += ky;
      if ( d < 0 ) d += i1;
      else
      {
        d += i2;
        if (flag) y += ky;
        else x += kx;
      }
    }
    Put_Pixel(x, y, color);
  }
}

//===============================================================
//			            НАРИСОВАТЬ РАМКУ
//===============================================================
void LCD_DrawRect (char x1, char y1, char width, char height, char size, int color)
{
  unsigned int i;
  char x2 = x1 + (width - 1), y2 = y1 + (height - 1); //Конечные размеры рамки по осям х и у
  for ( i = 1; i <= size; i++) // size - толщина рамки
  {
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    x1++; // Увеличиваю толщину рамки, если это задано
    y1++;
    x2--;
    y2--;
  }
}

//===============================================================
//              ЗАПОЛНИТЬ ПРЯМОУГОЛЬНИК ЦВЕТОМ COLOR
//===============================================================
void LCD_FillRect (char x1, char y1, char width, char height, int color)
{
  unsigned int x, y;

  SetArea( x1, x1 + (width - 1), y1, y1 + (height - 1) );
  digitalWrite(LCD_RS, HIGH);

  y = width * height;	        //Количество пикселей в прямоугольнике
  for (x = 0; x < y; x++)
  {
#ifdef _8_BIT_COLOR	//(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( DAT, color ); //Данные - задаём цвет пикселя
#else			//(16-ти битовая цветовая палитра (65536 цветов))
    Send_to_lcd( DAT, (color >> 8) ); Send_to_lcd( DAT, color );
#endif
  }
}

//===============================================================
//                  НАРИСОВАТЬ ОКРУЖНОСТЬ
//===============================================================
void LCD_DrawCircle (char xcenter, char ycenter, char rad, int color)
{
  signed char tswitch, x1 = 0, y1;
  char d;

  d = ycenter - xcenter;
  y1 = rad;
  tswitch = 3 - 2 * rad;
  while (x1 <= y1)
  {
    Put_Pixel(xcenter + x1, ycenter + y1, color);
    Put_Pixel(xcenter + x1, ycenter - y1, color);
    Put_Pixel(xcenter - x1, ycenter + y1, color);
    Put_Pixel(xcenter - x1, ycenter - y1, color);
    Put_Pixel(ycenter + y1 - d, ycenter + x1, color);
    Put_Pixel(ycenter + y1 - d, ycenter - x1, color);
    Put_Pixel(ycenter - y1 - d, ycenter + x1, color);
    Put_Pixel(ycenter - y1 - d, ycenter - x1, color);

    if (tswitch < 0) tswitch += (4 * x1 + 6);
    else
    {
      tswitch += (4 * (x1 - y1) + 10);
      y1--;
    }
    x1++;
  }
}

//===============================================================
//                 ЗАПОЛНИТЬ КРУГ ЦВЕТОМ COLOR
//===============================================================
void LCD_FillCircle (char xcenter, char ycenter, char rad, int color)
{
  signed int x1 = 0, y1, tswitch;
  y1 = rad;
  tswitch = 1 - rad;

  do
  {
    LCD_DrawLine(xcenter - x1, ycenter + y1, xcenter + x1, ycenter + y1, color);
    LCD_DrawLine(xcenter - x1, ycenter - y1, xcenter + x1, ycenter - y1, color);
    LCD_DrawLine(xcenter - y1, ycenter + x1, xcenter + y1, ycenter + x1, color);
    LCD_DrawLine(xcenter - y1, ycenter - x1, xcenter + y1, ycenter - x1, color);

    if (tswitch < 0)
      tswitch += 3 + 2 * x1++;
    else
      tswitch += 5 + 2 * (x1++ - y1--);
  } while (x1 <= y1);
}

//===============================================================
//                     НАРИСОВАТЬ ТРЕУГОЛЬНИК
//===============================================================
void LCD_DrawTriangle(char x1, char y1, char x2, char y2, char x3, char y3, int color)
{
  LCD_DrawLine(x1, y1, x2, y2, color);
  LCD_DrawLine(x3, y3, x1, y1, color);
  LCD_DrawLine(x3, y3, x2, y2, color);
}

//===============================================================
//              ЗАПОЛНИТЬ ТРЕУГОЛЬНИК ЦВЕТОМ COLOR
//===============================================================
void LCD_FillTriangle(char x1, char y1, char x2, char y2, char x3, char y3, int color)
{
  LCD_FillTriangleA(x1, y1, x2, y2, x3, y3, color);
  LCD_FillTriangleA(x3, y3, x1, y1, x2, y2, color);
  LCD_FillTriangleA(x3, y3, x2, y2, x1, y1, color);
}

//===============================================================
void LCD_FillTriangleA(char x1, char y1, char x2, char y2, char x3, char y3, int color)
{
  signed long x, y, addx, dx, dy;
  signed long P;
  int i;
  long a1, a2, b1, b2;
  if (y1 > y2)  {
    b1 = y2;
    b2 = y1;
    a1 = x2;
    a2 = x1;
  }
  else       {
    b1 = y1;
    b2 = y2;
    a1 = x1;
    a2 = x2;
  }
  dx = a2 - a1;
  dy = b2 - b1;
  if (dx < 0)dx = -dx;
  if (dy < 0)dy = -dy;
  x = a1;
  y = b1;

  if (a1 > a2)    addx = -1;
  else           addx = 1;

  if (dx >= dy)
  {
    P = 2 * dy - dx;
    for (i = 0; i <= dx; ++i)
    {
      LCD_DrawLine((int)x, (int)y, x3, y3, color);
      if (P < 0)
      {
        P += 2 * dy;
        x += addx;
      }
      else
      {
        P += 2 * dy - 2 * dx;
        x += addx;
        y ++;
      }
    }
  }
  else
  {
    P = 2 * dx - dy;
    for (i = 0; i <= dy; ++i)
    {
      LCD_DrawLine((int)x, (int)y, x3, y3, color);
      if (P < 0)
      {
        P += 2 * dx;
        y ++;
      }
      else
      {
        P += 2 * dx - 2 * dy;
        x += addx;
        y ++;
      }
    }
  }
}

//===============================================================
#endif



