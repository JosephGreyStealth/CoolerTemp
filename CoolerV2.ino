/*
 * Record the temperatures in a cooler.  Display the current and highest/lowest temp in the last 24 hrs 
 *  using a circular buffer.
 *
 * Copyright Joseph Grey and the person who invented him
 * 
 * Version 1.0 -- August 2021
 * Version 2.0 -- Sept 2023 -- 
 *
 */

//  LIBRARIES
//#include <stdio.h>
//#include <stdlib.h>
#include <CircularBuffer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"  // Adafruit RTClib version 1.12.5 (and its dependencies). Install this through the library manager in this IDE

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SENSOR_PIN D3 // Arduino pin connected to DS18B20 sensor's DQ pin

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // create SSD1306 display object connected to I2C
OneWire oneWire(SENSOR_PIN);         // setup a oneWire instance
DallasTemperature tempSensor(&oneWire); // pass oneWire to DallasTemperature library

String tempString;

int largest = 0;
int smallest = 120;
long randNumber;
int warningFlag = 0;
int dangerFlag = 0;

//  CONSTANTS
CircularBuffer<int, 144> buffer; // Maximum size of buffer This will hold a recorded temp every ten minutes for 24 hours

// 10 min = 600000 ms

void setup() {
  Serial.begin(9600);

  // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000);         // wait for initializing
  oled.clearDisplay(); // clear display
  oled.display();

  oled.setTextSize(2);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(15, 1);        // position to display
  oled.print("How Cool");
  oled.setCursor(19,16);
  oled.print("is the ");
  oled.setCursor(15,29);
  oled.print("cooler?");
  oled.setTextSize(2); 
  oled.setTextSize(1);
  oled.setCursor(15,47);
  oled.print("a Joseph Grey");
  oled.setCursor(24,56);
  oled.print("Production");
  oled.display();

  tempSensor.begin();     // initialize the sensor
  tempString.reserve(10); // to avoid fragmenting memory when using String

  Serial.println("Looking for Real Time Clock...");
  
  if (! rtc.begin())
    {
    Serial.println("Could NOT find RTC");
    Serial.flush();
    abort();
    }
  else
    {
    Serial.println("Found RTC");
    }

  if (! rtc.isrunning())
    {
    Serial.println("\nReal Time Clock is NOT running, Setting the time now...");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("Time has been set");
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
  else
    {
    Serial.println("\nReal Time Clock is already running - NOT setting the time now.");
    }

    randomSeed(analogRead(0));
}

void loop() {

  // check tthe time.  
  // if time/10 = 0 then it is on the ten minute mark
  // put the temp into the buffer
  // update display

  Serial.println("Checking time...");
  Serial.flush();
  DateTime now = rtc.now();
  Serial.println("Got the time");
  Serial.println(now.minute());
  if((now.minute() % 10) == 0){
    Serial.println("inside the ten minute flag");
    tempSensor.requestTemperatures();             // send the command to get temperatures
    float tempF = tempSensor.getTempFByIndex(0);  // read temperature 
    Serial.println("writing data");

	buffer.push(tempF);

		for (byte i = 0; i < buffer.size(); i++) {
      Serial.println(buffer[i]);
      
      if (buffer[i]>largest){
        largest=buffer[i];
        if (largest>40){
          warningFlag++;
          if (warningFlag>5){
            dangerFlag=1;
          }
        } else {
          if (warningFlag<3){
            warningFlag=0;
          }
        }
      }
      if (buffer[i]<smallest){
        smallest=buffer[i];
      }
    }  
    
    
		Serial.print("The largest is: ");
		Serial.println(largest);
    Serial.print("The smallest is: ");
		Serial.println(smallest);
    Serial.println("Clearing OLED display");
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setTextColor(WHITE,BLACK);
    
    Serial.println("Printing Data tto OLED");
    randNumber = random(1, 30);
    oled.clearDisplay(); // clear display
    //oled.display();

    oled.setCursor(randNumber,1);
    oled.print("Cooler");
    oled.setTextSize(3);
    oled.setCursor(80,20);
    oled.print(smallest);
    oled.setCursor(0,20);
    oled.print(largest);
    if (warningFlag>0) {
      oled.setCursor(35, 24);
      if (dangerFlag>0){
        oled.print("!!");
      } else {
        oled.print("*");
      }
    }
    oled.setTextSize(1);
    oled.setCursor(0,50);
    oled.print("Current ");
    oled.setCursor(45,50);
    oled.print(int(tempF));
    oled.display();


	}
  delay(60000);
}
