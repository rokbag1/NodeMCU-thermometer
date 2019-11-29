#include "Free_Fonts.h" // Include the font file
#include "Images.h" //Include the image file
#include "Moon_phase.h"
#include "SPI.h" //Screen lib
#include "TFT_eSPI.h" //Screen lib
// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();
unsigned long drawTime = 0;

// Arduino JSON library for JSON data
#include <ArduinoJson.h>

// ESP libraries
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

// BME libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <SPI.h>
#define D1 5
#define D2 4
#define D4 2
#define D3 0

// assign the SPI bus to pins
#define BME_SCK D1
#define BME_MISO D4
#define BME_MOSI D2
#define BME_CS D3
// initialize Adafruit BME280 library
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

// for round function
#include <math.h>

// time for moon phase
#include <time.h> //Time library for moon phase
int timezone = 3;
int dst = 0;
String nfm = ""; // days to next full moon
const int button = 16;
//WIFI username, password and client
char ssid[] = "Node";        
char pass[] = "weather1"; 
WiFiClient client;

//Setting up open weather
String Location= "klaipeda,LT"; 
int status = WL_IDLE_STATUS; 
String API_Key= "726f61710c107acb6b7240ae0e6a70a0"; 
int prevState = LOW;
int currState;
int nState = 0;


void setup(void) {
  Serial.begin(9600);
  pinMode(button, INPUT);
  Serial.println(F("BME280 test"));
  bool status;
  // default settings
  status =  bme.begin();
  while (!status) {
    delay(1000);
  }
    

  //TFT 
  tft.begin();
  tft.setRotation(1);
    
  //WIFI
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting");
  while( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  
}

void loop() {
   getHome();
   delay(10000);
   getWeather();
   delay(10000);
}

void getWeather() {
  if (WiFi.status() == WL_CONNECTED)  //Check WiFi connection status
  {
    HTTPClient http;  //Declare an object of class HTTPClient
 
    // specify request destination
    http.begin("http://api.openweathermap.org/data/2.5/weather?q=" + Location + "&APPID=" + API_Key);  // !!
 
    int httpCode = http.GET();  // send the request
 
    if (httpCode > 0)  // check the returning code
    {
      String payload = http.getString();   //Get the request response payload
 
      DynamicJsonBuffer jsonBuffer(512);
 
      // Parse JSON object
      JsonObject& root = jsonBuffer.parseObject(payload);
      if (!root.success()) {
        Serial.println(F("Parsing failed!"));
        return;
      }
      
      float temp         = (float)(root["main"]["temp"]) - 273.15;        // get temperature in °C
      int weather_status = root["weather"][0]["id"];                      // get weather status
      int   humidity     = root["main"]["humidity"];                      // get humidity in %
      float pressure     = (float)(root["main"]["pressure"]) / 1000;      // get pressure in bar
      float wind_speed   = root["wind"]["speed"];                         // get wind speed in m/s
      int  wind_degree   = root["wind"]["deg"];                           // get wind degree in °
 
      // send data to method
      displayElements(temp, weather_status, humidity, pressure, wind_degree,  wind_speed);
    }
    http.end();   //Close connection
  }
  
  time_t now = time(nullptr);
  splitTime(ctime(&now));
}

void displayElements(float temp, int weather_status, int humidity, float pressure, float wind_degree, int wind_speed) {
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF4);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  int t = round(temp);
  char str[8];
  itoa( t, str, 10 );

  if (strlen(str) == 2) {
    tft.drawString(str, 25, 20, FONT6);
    tft.drawBitmap(130, 15,  thermometer, 79, 80, TFT_WHITE);
    
  } else if (strlen(str) == 1) {
    tft.drawString(str, 50, 20, FONT6);
    tft.drawBitmap(100, 15,  thermometer, 79, 80, TFT_WHITE);
    
  } else if (strlen(str) == 3) {
    tft.drawString(str, 10, 20, FONT6);
    tft.drawBitmap(150, 15,  thermometer, 79, 80, TFT_WHITE);
  }
  
  checkWeatherStatus(weather_status);

//  //Small bar
//  tft.setTextSize(1);
  itoa( humidity, str, 10 );
  tft.setFreeFont(FF18);
  int px = responsiveNumbers(strlen(str)); 
  tft.drawString(str, px, 123, GFXFF);
  tft.drawBitmap(110, 115,  humid, 50, 50, TFT_WHITE);

  itoa( wind_speed, str, 10 );
  px = responsiveNumbers(strlen(str)); 
  tft.drawString(str, px, 187, GFXFF);
  tft.drawBitmap(110, 184,  wspeed, 50, 50, TFT_WHITE);
  
//  itoa( pressure, str, 10 );
//  px = responsiveNumbers(strlen(str)); 
//  tft.drawString(str, px, 170, GFXFF);
//  tft.drawBitmap(70, 170,  pressu, 35, 35, TFT_WHITE);
//  
//  itoa( wind_speed, str, 10 );
//  tft.drawString(str, 120, 170, GFXFF);
//  tft.drawBitmap(160, 168,  wspeed, 35, 35, TFT_WHITE);
//
//  itoa( wind_degree, str, 10 );
//  px = responsiveNumbers(strlen(str)); 
//  tft.drawString(str, 100 + px, 120, GFXFF);
//  tft.drawBitmap(170, 118,  degree, 35, 35, TFT_WHITE);
 }



// There follows a crude way of flagging that this example sketch needs fonts which
// have not been enbabled in the User_Setup.h file inside the TFT_HX8357 library.
//
// These lines produce errors during compile time if settings in User_Setup are not correct
//
// The error will be "does not name a type" but ignore this and read the text between ''
// it will indicate which font or feature needs to be enabled
//
// Either delete all the following lines if you do not want warnings, or change the lines
// to suit your sketch modifications.

#ifndef LOAD_GLCD
//ERROR_Please_enable_LOAD_GLCD_in_User_Setup
#endif

#ifndef LOAD_GFXFF
ERROR_Please_enable_LOAD_GFXFF_in_User_Setup!
#endif

void  checkWeatherStatus(int weather) {
    
  if (weather > 199 & weather < 233) {
       tft.drawBitmap(220 , 10,  thunder, 79, 79, TFT_WHITE);
  }
  
  if (weather > 299 & weather < 322) {
    tft.drawBitmap(220, 10, drizzle, 79, 79, TFT_WHITE); 
  }
  if (weather > 499 & weather < 532) {
    tft.drawBitmap(220, 10, rain, 79, 79, TFT_WHITE); 
  }
  if (weather > 599 & weather < 623) {
    tft.drawBitmap(220, 10, snow, 79, 79, TFT_WHITE); 
  }
  if (weather > 699 & weather < 801) {
    tft.drawBitmap(220, 10,  sun, 79, 79, TFT_WHITE); 
  }
  if (weather > 800) {
    tft.drawBitmap(220, 10,  cloud, 79, 79, TFT_WHITE); 
  }

}

void splitTime(String nowTime) {
  
   String monthString = nowTime.substring(4,7); 
   String dayInt = nowTime.substring(8,10); 
   String yearInt = nowTime.substring(20,25); 
   int monthInt = getMonthInt(monthString);
   Serial.println("diena");
   Serial.println(dayInt);
   Serial.println("metai");
   Serial.println(yearInt);
   Serial.println("menesis");
   Serial.println(monthInt);
   double julian = julianDat(yearInt.toInt(), monthInt, dayInt.toInt());
   int mp = moon_phase(julian);
   Serial.println("menulio laikrodis");
   Serial.println(mp);
   drawMoon(mp);
}

double julianDat(int year, int month, int day) {
  
  int timeZone = 3; //Taking our time zone
  double  zone = -(timeZone * 60 / 1440.0);
  
  if (month <= 2) {
    year -= 1;
    month += 12;
  }
  double day2 = day + zone + 0.5;
  double A = floor(year / 100.0);
  double B = 2 - A + floor(A / 4.0);
  double JD = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day2 + B - 1524.5;
  return JD;
}

int getMonthInt(String monthString){
  char *monthArray[]= {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  int arrSize = sizeof(monthArray);
  for (byte i = 0; i < arrSize; i = i + 1) {
    if (monthString == monthArray[i]) {
      return i+1;
    }
  }
}

int moon_phase(double julian){
  // calculates the age of the moon phase(0 to 7)
  // there are eight stages, 0 is full moon and 4 is a new moon
  double jd = 0; // Julian Date
  double ed = 0; //days elapsed since start of full moon
  int b= 0;
  jd = julian;
  jd = int(jd - 2244116.75); // start at Jan 1 1972
  jd /= 29.53; // divide by the moon cycle    
  b = jd;
  jd -= b; // leaves the fractional part of jd
  ed = jd * 29.53; // days elapsed this month
  nfm = String((int(29.53 - ed))); // days to next full moon
  b = jd*8 +0.5;
  b = b & 7; 
  return b;   
}

void drawMoon(int mp) {
  
  switch (mp){
    case 0:
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    tft.drawBitmap(200 , 95,  Full_Moon, 96,96, TFT_WHITE);
//    tft.drawString("Pilnatis", 210, 210, GFXFF);   
    tft.drawString("Full Moon", 205, 198, GFXFF);   
    break;
    
    case 1:
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    tft.drawBitmap(200 , 95,  Wanning_Gibbous, 96,96, TFT_WHITE);
//    tft.drawString("Delčia", 230, 210, GFXFF);    
    tft.drawString("Wanning Gibbous", 175, 196, GFXFF);    
    tft.drawString("Moon", 225, 215, GFXFF);  

    break;
    case 2:
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    tft.drawBitmap(200 , 95,  Third_Quater_Moom, 96,96, TFT_WHITE);
//    tft.drawString("Trečiasis ketvirtis", 210, 210, GFXFF);    
    tft.drawString("Third Quater", 180, 196, GFXFF);
    tft.drawString("Moon", 225, 215, GFXFF);     
        
    break;
    case 3:
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    tft.drawBitmap(200 , 95,  Old_Crescent_Moon, 96,96, TFT_WHITE); 
//    tft.drawString("Mažėjantis pusmėnulis", 210, 210, GFXFF);     
    tft.drawString("Old Crescent", 200, 196, GFXFF);
    tft.drawString("Moon", 225, 215, GFXFF);         
  
    break;     
    case 4:
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    tft.drawBitmap(200 , 95,  New_Moon, 96,96, TFT_WHITE);   
//    tft.drawString("Jaunatis",210, 210, GFXFF);  
    tft.drawString("New Moon", 205, 196, GFXFF);    
  
    break;  
    case 5:
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    tft.drawBitmap(200 , 95,  New_Crescent_Moon, 96,96, TFT_WHITE);   
//    tft.drawString("Vaško pusmenulis", 210, 210, GFXFF);   
    tft.drawString("New Crescent", 200, 196, GFXFF);
    tft.drawString("Moon", 225, 215, GFXFF);    

    break; 
    case 6:
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    tft.drawBitmap(200 , 95,  First_Quater_Moon, 96,96, TFT_WHITE);     
    tft.drawString("First Quater", 180, 196, GFXFF);
    tft.drawString("Moon", 230, 215, GFXFF);         
//    tft.drawString("Pirmasis ketvirtis", 210, 210, GFXFF);     
 
    break;
    case 7:
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    tft.drawBitmap(200 , 95,  Waxing_Gibbous_Moon, 96,96, TFT_WHITE);   
//    tft.drawString("Priešpilnis", 210, 210, GFXFF);     
    tft.drawString("Waxing Gibbous", 177, 196, GFXFF);
    tft.drawString("Moon", 225, 215, GFXFF);    
     
    break;   
  }
}

int responsiveNumbers(int str) {
  if (str == 1) {
    return 75;
    
  } else if (str == 2) {
    return 55;
    
  } else if (str == 3) {
   return 37;
  }
  
}

void getHome() {
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF4);
  tft.setFreeFont(FF18);
  char str[8];

  // read temperature, humidity and pressure from the BME280 sensor
  tft.setTextSize(1);
  tft.setTextColor(TFT_RED);
  tft.drawString("Temperature", 10, 10, GFXFF);
  tft.setTextColor(TFT_BLUE);
  tft.drawString("Humidity", 30, 145, GFXFF);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  float temp = bme.readTemperature();    // get temperature in °C
  dtostrf( temp, 1,2, str );
  tft.drawString(str, 15, 50, GFXFF);
  float humi = bme.readHumidity();       // get humidity in %
  dtostrf( humi, 1,2, str );
  tft.drawString(str, 15, 185, GFXFF);
  tft.drawBitmap(180 , 45,  house, 128,128, TFT_WHITE);   
  tft.drawLine(0, 120, 160, 120, TFT_WHITE);
  tft.drawLine(160, 0, 160, 320, TFT_WHITE);
}
