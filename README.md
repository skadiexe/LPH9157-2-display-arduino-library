# LPH9157-2 display arduino library
Based on existing [AVR C project](http://cxem.net/mc/mc221.php)

Tested with 3.3v on ESP8266 and works fine
### Known issues:
No buffering<br>Try to use hardware reset if you sometimes got only white screen at powering on 

## Display pinout:
![display pinout](https://github.com/skadiexe/LPH9157-2-display-arduino-library/blob/master/lph9157-2%20pinout.jpg)

<table>
    	<tr><td>#</td><td>Name</td><td>Function</td></tr>
   	<tr><td>1</td><td>RS</td><td>Low=CMD, High=DATA</td></tr>
   	<tr><td>2</td><td>~RST</td><td>Reset input, active low</td></tr>
    	<tr><td>3</td><td>~CS</td><td>SPI chip select, active low</td></tr>
   	<tr><td>4</td><td>SYNC</td><td>External frame synchorization input, unused by default</td></tr>
	<tr><td>5</td><td>CLK</td><td>SPI Clock-in signal (High-to-Low)</td></tr>
	<tr><td>6</td><td>DATA</td><td>SPI Data-in signal (MSB first)</td></tr>
	<tr><td>7</td><td>VCC</td><td>Power supply, normally 2.9V (OK with 3.3V)</td></tr>
	<tr><td>8</td><td>GND</td><td>Ground</td></tr>
	<tr><td>9</td><td>LED+</td><td>Backlight voltage, approx. 12V (depends on required current)</td></tr>
	<tr><td>10</td><td>LED-</td><td>Backlight common pin</td></tr>
	
</table>
	

## Minimal connection diagram:
![connection diagram](https://github.com/skadiexe/LPH9157-2-display-arduino-library/blob/master/connection%20diagram.png)

## Usage:
Specify LCD_RS pin (and optionally -- LCD_RESET) in LPH91572.h

```Arduino
#include "LPH91572.h"

void setup() {
  LCD_init();
  LCD_FillScreen (WHITE);
  
}
  
void loop() {
  LCD_Puts_Shadow("Hello!", 10, 60, GREEN,  2, 2, 90); 
  delay(50);
}

```
