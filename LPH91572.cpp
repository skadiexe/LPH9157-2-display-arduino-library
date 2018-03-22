#include <Arduino.h>
#include <SPI.h>
#include "Symbols.h"
#include "LPH91572.h"

// to store previous RS value and not drive this pin while continious data streaming
uint8_t RS_old;

//===============================================================
//                        init
//===============================================================
void LCD_init(void)
{ SPI.begin();
  delay(20);
  pinMode(SS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  digitalWrite(SS, LOW);
  
 #ifdef _USE_HARDWARE_RESET
  pinMode(LCD_RESET, OUTPUT);
  digitalWrite(LCD_RESET, LOW);
  delay(20);
  digitalWrite(LCD_RESET, HIGH);
 #endif 

  delay(20);
  digitalWrite(LCD_RS, LOW);
  Send_to_lcd(CMD, 0x01); //soft reset
  delay(10);
  Send_to_lcd(DAT, 0x00);
  Send_to_lcd(CMD, 0x11); //wake up from sleep mode
  delay(20);
  //Memory Access Control (Направление заполнения области дисплея (памяти): 0bVHRXXXXX, V - заполнение по вертикали (0 - сверху-вниз, 1 - снизу-вверх),
  //H - заполнение по горизонтали (0 - слева-направо, 1 - справа-налево), R - меняются местами строки и столбцы (при этом заполнение остается сверху-вниз, слева-направо))
  Send_to_lcd(CMD, 0x36); 

  Send_to_lcd(CMD, 0x3a); //set color mode
#ifdef _8_BIT_COLOR
  Send_to_lcd(DAT, 0x02); //256 colors
#else
  Send_to_lcd(DAT, 0x05); //65536 colors
#endif
  delay(10);
  Send_to_lcd(CMD, 0x29); //power on display
  delay(10);
}

//===============================================================
//LCD driver function (RS==0 - command, RS==1 - data)
//===============================================================


void Send_to_lcd (uint8_t RS, uint8_t data)
{ static uint8_t old_RS = 0;
  if ((old_RS != RS) || (!RS && !old_RS)) {
    digitalWrite(LCD_RS, RS);
  }
  // 30MHz is maximum for that display
  // standart speed is 13MHz

#if defined(__AVR__)
SPI.beginTransaction(SPISettings(15000000L, MSBFIRST, SPI_MODE0));
#elif defined(ESP8266)
SPI.beginTransaction(SPISettings(30000000L, MSBFIRST, SPI_MODE0));
#endif

  SPI.transfer(data);
  // removed for better speed
  // delayMicroseconds(5);
  //digitalWrite(SS, HIGH); 
  SPI.endTransaction();
}


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


//===============================================================
//                          Рисуем точку
//===============================================================
void Put_Pixel (char x, char y, unsigned int color)
{
  SetArea( x, x, y, y );
  digitalWrite (LCD_RS, HIGH);

#ifdef _8_BIT_COLOR //(8-ми битовая цветовая палитра (256 цветов))
  Send_to_lcd( DAT, color ); //Данные - задаём цвет пикселя
#else             //(16-ти битовая цветовая палитра (65536 цветов))
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
#ifdef _8_BIT_COLOR //(8-ми битовая цветовая палитра (256 цветов))
            Send_to_lcd( DAT, t_color ); //Данные - задаём цвет пикселя
#else             //(16-ти битовая цветовая палитра (65536 цветов))
            Send_to_lcd( DAT, (t_color >> 8) ); Send_to_lcd( DAT, t_color );
#endif
          }
          else
          {
#ifdef _8_BIT_COLOR //(8-ми битовая цветовая палитра (256 цветов))
            Send_to_lcd( DAT, b_color ); //Данные - задаём цвет пикселя
#else             //(16-ти битовая цветовая палитра (65536 цветов))
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
//                  ЗАЛИВКА ЭКРАНА ЦВЕТОМ
//===============================================================
void LCD_FillScreen (unsigned int color)
{
  unsigned int x;
  SetArea( 0, 131, 0, 175 );   //Область всего экрана
  digitalWrite(LCD_RS, HIGH);

  //Данные - задаём цвет пикселя
  for (x = 0; x < 23232; x++)  // 23232 - это 132 * 176
  {
#ifdef _8_BIT_COLOR //(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( DAT, color ); //Данные - задаём цвет пикселя
#else     //(16-ти битовая цветовая палитра (65536 цветов))
    Send_to_lcd( DAT, (color >> 8) ); Send_to_lcd( DAT, color );
#endif
  }
}

//===============================================================
//                 ФУНКЦИЯ ВЫВОДА ИЗОБРАЖЕНИЯ
//===============================================================

  void LCD_Output_image (char x, char y, char width, char height, char *img, int rot)
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

//===============================================================
//          Функция для обеспечения вывода изображения
//===============================================================
//Вывод картинки с Image2Lcd и NokiaImageCreator должен выполняться слева-направо сверху-вниз.
//x, y - начало области вывода изображения; width и height - ширина и высота изображения

  void Send_Image (char x, char y, char width, char height, char *img, int rot)
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
    #ifdef _8_BIT_COLOR //(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( DAT, *img++ ); //Данные - задаём цвет пикселя
    #else     //(16-ти битовая цветовая палитра (65536 цветов))
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
    #ifdef _8_BIT_COLOR //(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( DAT, *img++ ); //Данные - задаём цвет пикселя
    #else     //(16-ти битовая цветовая палитра (65536 цветов))
    Send_to_lcd( DAT, *img++ ); Send_to_lcd( DAT, *img++ );
    #endif
   }
  }
  break;
  };
  }

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
//                  НАРИСОВАТЬ РАМКУ
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

  y = width * height;         //Количество пикселей в прямоугольнике
  for (x = 0; x < y; x++)
  {
#ifdef _8_BIT_COLOR //(8-ми битовая цветовая палитра (256 цветов))
    Send_to_lcd( DAT, color ); //Данные - задаём цвет пикселя
#else     //(16-ти битовая цветовая палитра (65536 цветов))
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