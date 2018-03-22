#include <SPI.h>
#include "LPH91572.h"

void setup() {
  Serial.begin(500000);
  // put your setup code here, to run once:
  LCD_init();
  LCD_FillScreen (WHITE);
  pinMode(LED_BUILTIN, OUTPUT);

}
uint16_t color = 0;
bool flag = 0;
char buf[20];
uint32_t last_millis = millis ();
void loop() {
  if (color > 360) color = 0;
  if (millis() - last_millis > 5000) {
    LCD_FillScreen (spectrumToRBG565(color));

    last_millis = millis();
  }
  /*if (flag) {
    LCD_Puts_Shadow("trololo", 10, 60, WHITE,  2, 2, 90);
    digitalWrite(LED_BUILTIN, !digitalRead (LED_BUILTIN));

    }
    else {
    LCD_Puts_Shadow("ololo", 10, 20, WHITE,  2, 2, 90);
    }
    sprintf(buf, "%d - color \0", color);
    LCD_Puts_Shadow(buf, 10, 100, WHITE,  2, 2, 90);
    delay(500);
    flag = !flag; */
  color = random(360);
  LCD_DrawCircle (random (176), random (132), random (65), spectrumToRBG565(color));
  delay (1000);
}


