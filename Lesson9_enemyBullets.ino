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
#include <Adafruit_GFX.h>     // graphics header files
#include <Adafruit_SSD1306.h> // ssd1306 header files
#include "bmpheader.h"        // icon header file

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int BUTT1 = 0;  // declare buttons
const int BUTT2 = 14;
const int BUTT3 = 12;
const int BUTT4 = 13;

int rocketx = SCREEN_WIDTH-ROCKET_WIDTH;          // rocket x axis - does not change
int rockety = (SCREEN_HEIGHT-ROCKET_HEIGHT) / 2;  // rocket y axis - will change with button press
int enemyx = 0;                             // enemy x location
int enemyy = 0;                             // enemy y location
bool enemyleft = true;                      // left/right movement of the enemy
bool enemyup = true;                        // up/down movement of the enemy

bool debouncebutt = false;                  // button debounce
bool bullet[10] = {0};                      // bullet true or false array
int bx[10] = {0};                           // bullet x location
int by[10] = {0};                           // bullet y location
int bullet_counter = 0;                     // number of bullets in the array
const int bulletMax = 10;                   // max number of bullets is 10

bool enemybullet[10] = {0};                 // enemy bullet true or false array
int enemybx[10] = {0};                      // enemy bullet x location
int enemyby[10] = {0};                      // enemy bullet y location
int enemy_bullet_counter = 0;               // enemy number of bullets in the array
const int enemyBulletMax = 10;              // enemy max number of bullets is 10
unsigned long previousMillis = 0;
const long interval = 750;


int radius = 1;                             // radius of explosion
const int maxRadius = 30;                   // explosion max radius
int xEnd = 0;                               // display explosion pixel x
int yEnd = 0;                               // display explosion pixel y
int prevEx = 0;                             // location of enemy at time of hit x
int prevEy = 0;                             // location of enemy at time of hit y
bool hit = false;                           // did we kill the enemy?
bool ehit = false;                          // did we get killed?
int enemy_hit_counter = 30;                       // max number of hits before explosion 30/3=10 for drawing purposes
int rocket_hit_counter = 30;                // max number of hits before explosion 30/3=10 for drawing purposes

void setup() {
  Serial.begin(115200);                     // serial output initialization for testing purposes

  pinMode(BUTT1, INPUT);                    // initializing the buttons as outputs
  pinMode(BUTT2, INPUT);
  pinMode(BUTT3, INPUT);
  pinMode(BUTT4, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // initialize the display
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}

void loop() {
  unsigned long currentMillis = millis();
  display.clearDisplay();                                                 // clear the display

  if(digitalRead(BUTT1) == 0 && rockety>-ROCKET_HEIGHT/2) { rockety--; }              // move ship up
  if(digitalRead(BUTT2) == 0 && rockety<SCREEN_HEIGHT-ROCKET_HEIGHT/2) { rockety++; } // move ship down
  if(digitalRead(BUTT3) == 0){}                                           // for future use
  if(digitalRead(BUTT4) == 0) {                                           // load a bullet
    loadingButt();                                                        // function to load bullets
  } else debouncebutt = false;                                            // debounce button

  if(!ehit){
    display.drawBitmap( rocketx, rockety, rocket, ROCKET_WIDTH, ROCKET_HEIGHT, WHITE);  // draw the ship 
    checkShipCollision();
  } else {
    explosion(rocketx, rockety);
  }

  if(!hit){                                                               // if no hit do this
    display.drawBitmap( enemyx, enemyy, enemy, ENEMY_W, ENEMY_H, WHITE);  // draw the enemy
    checkEnemyCollision();                                                // check for collision
    moveEnemy();                                                          // move the enemy ship

    if(currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      loadEnemyButts();
    }
  } else {                                                                // if hit
    explosion(enemyx, enemyy);                                            // create explosion
  }

  shootingButt();                                                         // shooting the bullet
  shootEnemyButts();
  for(int q=0;q<=enemy_hit_counter;q+=3) { // 30/3=10 hits max (3 pixels for spacing purposes)
    display.drawRect(q, 0, 2, 4, WHITE);
  }
  for(int q=SCREEN_WIDTH-30;q<=rocket_hit_counter+SCREEN_WIDTH-30;q+=3) { // 30/3=10 hits max (3 pixels for spacing purposes)
    display.drawRect(q, 0, 2, 4, WHITE);
  }
  display.display();                                                      // draw everything to the screen
}

void explosion(int x, int y) {                                                        // explosion function
  for(int bunch = 0; bunch < 40; bunch++){
    for (int angle = 0; angle < 360; angle += 45) { // move around a circle at increments of 45 degrees - 8 pixels
      xEnd = x + radius * cos(radians(angle)); // cosine gives x coordinate
      yEnd = y + radius * sin(radians(angle)); // sin gives y coordinate
      display.drawPixel(xEnd, yEnd, WHITE);         // draw the pixel to the buffer to be displayed
    }
    radius++;                                       // increment radius
    if(radius >= maxRadius) {                       // check if radius is at max (10)
      hit = false;                                  // if so set hit to false to resume game
      ehit = false;
      radius = 1;                                   // prepare radius for next hit
    }
  }
  //delay(1000);
}

void checkEnemyCollision(){                            // collision detection function
  for(int i=0;i<10;i++){                          // check each bullet
    if(bullet[i] == true)                         // if the bullet exist
      if(bx[i]>enemyx && bx[i]<enemyx+ENEMY_W/2 && by[i]>enemyy && by[i]<enemyy+ENEMY_H){ // if bullet is inside enemy coordinates
        bullet[i]=false;            // delete bullet
        enemy_hit_counter -= 3;           // decrement bullet counter by 3 (used for x coordinate of display also)
        if(enemy_hit_counter == 0){
          hit = true;                 // set hit to true (to run explosion to maxRadius)
          prevEx = enemyx+ENEMY_W/2;  // center explosion to center of icon
          prevEy = enemyy+ENEMY_H/2;
          enemyx = 0-ENEMY_W*2;       // set the enemy off the screen
          enemyy=random(SCREEN_HEIGHT); // set enemy at random y
          enemy_hit_counter = 30; // reset hit counter to max
        }
      }
  }
}

void checkShipCollision(){                            // collision detection function
  for(int i=0;i<10;i++){                          // check each bullet
    if(enemybullet[i] == true)                         // if the bullet exist
      if(enemybx[i]>rocketx && enemybx[i]<rocketx+ROCKET_WIDTH && enemyby[i]>rockety && enemyby[i]<rockety+ROCKET_HEIGHT){ // if bullet is inside enemy coordinates
        enemybullet[i]=false;            // delete bullet
        rocket_hit_counter -= 3;           // decrement bullet counter by 3 (used for x coordinate of display also)
        if(rocket_hit_counter == 0){
          ehit = true;                 // set hit to true (to run explosion to maxRadius)
          prevEx = rocketx+ROCKET_WIDTH/2;  // center explosion to center of icon
          prevEy = rockety+ROCKET_HEIGHT/2;
          rocket_hit_counter = 30; // reset hit counter to max
        }
      }
  }
}

void moveEnemy() {                                              // moving the enemy function
  if(enemyleft == true && enemyx < SCREEN_WIDTH/4) {enemyx++;}  // only move the enemy to 1/4 the screen width
  if(enemyx >= SCREEN_WIDTH/4) {enemyleft = false;}             // let the enemy move all the way off the screen
  if(enemyleft == false && enemyx > 0-ENEMY_W*2) {enemyx--;}    // change direction of the enemy
  if(enemyx <= 0-ENEMY_W*2) {                                   // if the enemy is off the screen 
    enemyy=random(SCREEN_HEIGHT);                               //    randomize the y location
    enemyleft = true;                                           // change direction
  }
  if(enemyup == true && enemyy >0){enemyy--;}                   // move the enemy up
  if(enemyy <= 0) {enemyup = false;}                            // if at top of screen change direction
  if(enemyup == false && enemyy < SCREEN_HEIGHT) {enemyy++;}    // move enemy down
  if(enemyy >= SCREEN_HEIGHT) {enemyup = true;}                 // change enemy direction
}

void loadEnemyButts(){
  enemybullet[enemy_bullet_counter] = true;                   // add a bullet to the array
  enemybx[enemy_bullet_counter] = enemyx + ENEMY_W;           // nose of the ship
  enemyby[enemy_bullet_counter] = enemyy + ENEMY_H/2;        // centerd to height of the ship
  enemy_bullet_counter++;                                     // increment counter for next bullet
  if(enemy_bullet_counter == enemyBulletMax) enemy_bullet_counter = 0;   // limit to 10 bullets at a time
}

void shootEnemyButts(){
  for(int i=0; i<10; i++) {                                   // go through the array
    if(enemybullet[i] == true){                               // if the bullet exist
      enemybx[i]=enemybx[i]+4;                                // move the bullet by 4 pixels
      if(enemybx[i]<SCREEN_WIDTH){                            // as long as the bullet is on the screen
        display.drawCircle(enemybx[i], enemyby[i], 2, WHITE); // draw the bullet to the buffer
      } else {enemybullet[i]=false;}                          // if not on the screen - delete the bullet
    }
  }
}

void loadingButt(){               // bullet loading function
    if(debouncebutt == false){    // debounce the button
      debouncebutt = true;
      bullet[bullet_counter] = true;                        // add a bullet to the array
      bx[bullet_counter] = rocketx;                         // nose of the ship
      by[bullet_counter] = rockety + ROCKET_HEIGHT/2;       // centerd to height of the ship
      bullet_counter++;                                     // increment counter for next bullet
      if(bullet_counter == bulletMax) bullet_counter = 0;   // limit to 10 bullets at a time
    }
}

void shootingButt(){                                        // shooting bullets function
  for(int i=0; i<10; i++) {                                 // go through the array
    if(bullet[i] == true){                                  // if the bullet exist
      bx[i]=bx[i]-4;                                        // move the bullet by 4 pixels
      if(bx[i]>0){                                          // as long as the bullet is on the screen
        display.drawCircle(bx[i], by[i], 1, WHITE);         // draw the bullet to the buffer
      } else {bullet[i]=false;}                             // if not on the screen - delete the bullet
    }
  }
}
