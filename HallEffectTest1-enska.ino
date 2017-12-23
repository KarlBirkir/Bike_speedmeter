/*
10k ohm resistor between pin 1 (5v) and pin 3 (signal) on Hall effect sensor
(Acts as pullup. Either at the sensor, or simply over the Arduino, from 5v pin/rail to signal pin).

Resistor value is not critial. I found slightly increased sensitivity with 
a much larger resistance. Anything above 2k or 3k is probably fine.

___________
\_H_A_L_L_/
  1  2  3

Hall sensor pinouts: 
- with 5v, GND, signal (1,2,3)
 
interrupt 0 = Arduino nano pin D2

  This program assumes two magnets on the wheel
  to trigger the hall effect sensor, and the 
  'flip' variable toggles between which one counts.
  This is to get more datapoints for more accurate
  calculations at lower/average speeds, and more
  frequent updates on the display.
  
  One magnet is flip=0, time0, speed0
  The other is flip=1, time1, speed1
  The final result is their average.

  If there's no trigger for more than 1800ms
  speed is set to 0. This can be adjusted.
  */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define diameter 0.695 // tire outermost diameter, in meters. 
// 695mm for my 700x35mm winter tyres, YM literally MV.

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

const byte intrPin = 0;

float speed0, speed1, maxSpeed, odometer = 0;
float circ = (3.14 * diameter); // circumference formula
String distUnit = "[m]"; // unit of distance, changes to [km] after 800m

// variables that are calculated together often have to have the same type
// especially here, millis() returns an unsigned long 
// so using other types can return strange results, 
// which sucks, for accurate time measurements
unsigned long time0, time1, nullTime = 0;
unsigned long timeToZero = 1800; // 1800 ms (1.8 seconds) of 'no trigger' => speed=0.

boolean flip = 0; // toggle between triggers
boolean engage = false; // set as true at each interrupt, false at each calculation


/* 
   The interrupt basically flips a switch, telling the next
   passover of the loop (which is very, very soon) to 
   calculate how long since tha last time it was flipped.
   Then, from the diameter, you can find your speed.
   Another switch toggles between the two timers.
 
*/

// -- FUNCTIONS

void magnet_detect() { // interrupt function
  flip = !flip;
  engage = true;
  // Serial.println("interrupt trig'd"); // debug
}

void drawDisplay(){
  display.clearDisplay();

  // x, y, (128,32). (0,0) is upper left

  // left column, speed
  display.setCursor(0,0);  
  display.print("Speed");
  display.setCursor(0,10);
  display.print(String((speed0+speed1)/2));
  display.setCursor(10,20);
  display.print("[km/hr]");

  //mid column, max speed
  display.setCursor(40,0);
  display.print("max");
  display.setCursor(40,10);
  display.print(maxSpeed);

  // right column, odometer
  display.setCursor(90,0);
  display.print("odo");  
  display.setCursor(90,10);

  
  // odometer, if >0,8km
  if(odometer > 800){
    display.print(String(odometer/1000, 2));
    if(distUnit == "[m]"){
    distUnit = "[km]";
    }
  }
    
  else{
    display.print(String(odometer, 1));
  } 
  display.setCursor(90,20);
  display.print(distUnit); 

  display.display();

  /*
  Serial.print(String("avg speed: "));
  Serial.println((hradi0+hradi1)/2);
  */
}

void setup(){
   Serial.begin(9600);
   attachInterrupt(intrPin, magnet_detect, FALLING);

   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
   display.setTextSize(1);
   display.setTextColor(WHITE);
   drawDisplay();
}
 
void loop(){
  
  // set speed=0 if no trigger for 1800 ms
  if (!engage && ((millis() - nullTime) > timeToZero)){ // nullTimi
    speed0 = 0; 
    speed1 = 0;
    delay(10);
    drawDisplay();
    nullTime = millis();
    }

  // calculate speed for trigger 0
  else if(engage && !flip){
    speed0 = (3600.0 * circ/(millis()-time0));
    maxSpeed = max(speed0, maxSpeed);
    odometer += circ; // adds one circumference to total distance
    drawDisplay();
    
    // finish up
    time0 = millis();
    nullTime = millis();
    engage=false;

    /*  //debug
    //Serial.print(String("speed 0: "));
    //Serial.println(speed0); 
    */
  }

  // calculate speed for trigger 1
  else  if(engage && flip){
    //Serial.println(timi1);
    speed1 = (3600.0 * circ/(millis()-time1));
    maxSpeed = max(speed0, maxSpeed);
    drawDisplay(); 

    time1 = millis();
    nullTime = millis();
    engage=false;

    /*  //debug  
    Serial.print(String("Speed 1: "));
    Serial.println(speed1); 
    */
  }
  
 } // end of loop
