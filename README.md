# LPH9157-2 display arduino library
Based on existing [AVR C project](http://cxem.net/mc/mc221.php)

Tested with 3.3v on ESP8266 and works fine

## Display pinout:
![display pinout](https://github.com/skadiexe/LPH9157-2-display-arduino-library/blob/master/lph9157-2%20pinout.jpg)

<table>
    <tr>
        <td>#</td>
        <td>Name</td>
        <td>Function</td>
    </tr>
    <tr>
    </tr>
</table>
	
1	RS	Low=CMD, High=DATA
2	~RST	Reset input, active low
3	~CS	Chip select, active low
4	SYNC	External frame synchorization input, unused by default
5	CLK	SPI Clock-in signal (High-to-Low)
6	DATA	SPI Data-in signal (MSB first)
7	VCC	Power supply, normally 2.9V (I tested with 3.3V)
8	GND	Ground
9	LED+	Backlight voltage, approx. 12V (depends on required current)
10	LED-	Backlight common pin

## Connection diagram:
![connection diagram](https://github.com/skadiexe/LPH9157-2-display-arduino-library/blob/master/connection%20diagram.png)
