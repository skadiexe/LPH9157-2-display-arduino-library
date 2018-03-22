#include "LPH91572.h"

uint16_t color = 0;
uint32_t last_millis = 2000;

void setup() {
  LCD_init();
}

void loop() {

  if ((millis() - last_millis > 2000) {
    color = random (0xFFFF);
    LCD_FillScreen (color);
    LCD_Puts_Shadow("Hello", 6, 20, BLACK,  4, 4, 90);
    LCD_Puts_Shadow("Hello", 4, 18, WHITE,  4, 4, 90);
    
    LCD_Puts_Shadow("Hello", 4, 60, WHITE,  2, 2, 90);
    LCD_Puts_Shadow("Hello", 4, 90, BLACK,  1, 1, 90);
    last_millis = millis();
  }

}
