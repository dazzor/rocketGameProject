/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "bmpheader.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int BUTT1 = 0;
const int BUTT2 = 14;
const int BUTT3 = 12;
const int BUTT4 = 13;

void setup() {
  Serial.begin(115200);

  pinMode(BUTT1, INPUT);
  pinMode(BUTT2, INPUT);
  pinMode(BUTT3, INPUT);
  pinMode(BUTT4, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

}

int x = SCREEN_WIDTH-ROCKET_WIDTH;
int y = (SCREEN_HEIGHT-ROCKET_HEIGHT) / 2;
int enemyx = 0;

void loop() {

  if(digitalRead(BUTT1) == 0){
    Serial.print("button 1 = "); Serial.println(digitalRead(BUTT1));
    Serial.print("y = "); Serial.println(y);
    y--;
    if(y<=0 - ROCKET_HEIGHT/2) y= 0-ROCKET_HEIGHT/2;
  }
  if(digitalRead(BUTT2) == 0){
    Serial.print("button 2 = "); Serial.println(digitalRead(BUTT2));
    Serial.print("y = "); Serial.println(y);
    y++;
    if(y >= SCREEN_HEIGHT-ROCKET_HEIGHT/2) y= SCREEN_HEIGHT-ROCKET_HEIGHT/2;
  }

  display.clearDisplay();

  display.drawBitmap( x, y, rocket, ROCKET_WIDTH, ROCKET_HEIGHT, 1);
  display.drawBitmap( enemyx, (display.height() - LOGO_HEIGHT) / 2, logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    
  display.display();

  enemyx++;
  if(enemyx >= SCREEN_WIDTH) enemyx = 0;
}

