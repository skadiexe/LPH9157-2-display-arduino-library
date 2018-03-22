//--------Display library LPH9157-2 from Siemens C75, МЕ75---------
//                 Green PCB
//                132х176 px
//          ==== Кизим Игорь ====
//-----------------------------------------------------

#ifndef __LPH91572_H__
#define __LPH91572_H__

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
void Put_Pixel (char x, char y, unsigned int color);
void Send_Symbol (unsigned char symbol, char x, char y, int t_color, int b_color, char zoom_width, char zoom_height, int rot);
void LCD_Putchar (char symbol, char x, char y, int t_color, int b_color, char zoom_width, char zoom_height, int rot);
void LCD_Puts(char *str, int x, int y,  int t_color, int b_color, char zoom_width, char zoom_height, int rot);
void Send_Symbol_Shadow (unsigned char symbol, char x, char y, int t_color, char zoom_width, char zoom_height, int rot);
void LCD_Putchar_Shadow (char symbol, char x, char y, int t_color, char zoom_width, char zoom_height, int rot);
void LCD_Puts_Shadow (char *str, int x, int y,  int t_color, char zoom_width, char zoom_height, int rot);
void LCD_FillScreen (unsigned int color);
void LCD_Output_image (char x, char y, char width, char height,  char *img, int rot);
void Send_Image (char x, char y, char width, char height,  char *img, int rot);
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

#endif


