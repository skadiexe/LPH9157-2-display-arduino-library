#include "LPH91572.h"

void setup() {
  LCD_init();
}

void loop() {

  LCD_FillScreen (random (0xFFFF));
  
  LCD_Puts_Shadow("Hello", 6, 20, BLACK,  4, 4, 90);
  
  LCD_Puts_Shadow("Hello", 4, 18, WHITE,  4, 4, 90);

  LCD_Puts_Shadow("Hello", 4, 60, WHITE,  2, 2, 90);
  LCD_Puts_Shadow("Hello", 4, 90, BLACK,  1, 1, 90);

}
