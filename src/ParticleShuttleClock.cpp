/* 
 * Project: Particle Shuttle Clock
 * Author: Tyler Mueller
 * Date: 12/16/2023
*/

#include <Particle.h>
#include "../lib/Adafruit_ILI9341/src/Adafruit_ILI9341.h"

#if defined(PARTICLE)
  #define TFT_DC   D5
  #define TFT_CS   D4
#endif

// Define Variables
Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);

String getCurrentWeekDay(int offset);
String getAbbreviatedWeekDay(int offset);
String getCurrentDate();
String getCurrentTime();

extern uint8_t blank[], sunny[], cloudy[], fog[], rain[], snow[], thunderstorm[];
String yesterdayTemp, todayTemp, tomorrowTemp;
String yesterdayCode, todayCode, tomorrowCode;
uint8_t* getWeatherCode(String weatherCode);
void updateTemps();

int lastMinute = -1;
int lastHour = -1;
int lastDay = -1;
int dayOfWeek = 0;

int centerTextX(String text, int textSize);

// Retrieve Temperature
void temperatureHandler(const char *event, const char *tempData) {
  int index0, index1, index2;
  String tempString = String(tempData);

  index0 = tempString.indexOf('~');
  yesterdayTemp = String(round(tempString.substring(0, index0).toFloat())).toInt();

  index1 = tempString.indexOf('~', index0+1);
  todayTemp = String(round(tempString.substring(index0+1, index1).toFloat())).toInt();

  index2 = tempString.indexOf('~', index1+1);
  tomorrowTemp = String(round(tempString.substring(index1+1, index2).toFloat())).toInt();

  updateTemps();
}

// Retrieve WeatherCodeDay
void weatherCodeHandler(const char *event, const char *weatherCodeData) {
  int index0, index1, index2;
  String codeString = String(weatherCodeData);

  index0 = codeString.indexOf('~');
  yesterdayCode = codeString.substring(0, index0);

  index1 = codeString.indexOf('~', index0+1);
  todayCode = codeString.substring(index0+1, index1);

  index2 = codeString.indexOf('~', index1+1);
  tomorrowCode = codeString.substring(index1+1, index2);
  
  display.fillRect(15, 192, 60, 60, ILI9341_BLACK);
  display.fillRect(90, 192, 60, 60, ILI9341_BLACK);
  display.fillRect(165, 192, 60, 60, ILI9341_BLACK);

  display.drawBitmap(15, 192, getWeatherCode(yesterdayCode), 60, 60, ILI9341_WHITE);
  display.drawBitmap(90, 192, getWeatherCode(todayCode), 60, 60, ILI9341_WHITE);
  display.drawBitmap(165, 192, getWeatherCode(tomorrowCode), 60, 60, ILI9341_WHITE);
}

void setup() {
  Particle.subscribe("hook-response/getTemp", temperatureHandler, MY_DEVICES);
  Particle.subscribe("hook-response/weatherCode", weatherCodeHandler, MY_DEVICES);

  display.begin();

  display.fillScreen(ILI9341_BLACK);
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  display.drawRect(0, 0, 240, 320, ILI9341_WHITE);
  display.drawFastHLine(10, 80, 230 - 10, ILI9341_WHITE);
  display.drawFastHLine(10, 155, 230 - 10, ILI9341_WHITE);

  display.setTextSize(2);
  display.setCursor(28, 170); display.print(getAbbreviatedWeekDay(-1));
  display.setCursor(90, 170); display.print("Today");
  display.setCursor(178, 170); display.print(getAbbreviatedWeekDay(1));
  
  int centerX = centerTextX("Rapid City, South Dakota", 1);
  display.setCursor(centerX, 300);
  display.setTextSize(1);
  display.print("Rapid City, South Dakota");

  String data = String(10);
  Particle.publish("getTemp", data, PRIVATE);
  Particle.publish("weatherCode", data, PRIVATE);

  updateTemps();
}

void loop() {
  Time.zone(-7);

  int currentMinute = Time.minute();
  int currentHour = Time.hour();
  int currentDay = Time.day();

  if (currentDay != lastDay) {
    lastDay = currentDay;

    display.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

    int centerX = centerTextX(getCurrentWeekDay(0), 3);
    display.setCursor(centerX, 15);
    display.setTextSize(3);
    display.print(getCurrentWeekDay(0));

    centerX = centerTextX(getCurrentDate(), 2);
    display.setCursor (centerX, 50);
    display.setTextSize(2);
    display.print(getCurrentDate());
  }

  if (currentMinute != lastMinute || currentHour != lastHour) {
    lastMinute = currentMinute;
    lastHour = currentHour;

    display.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

    int centerX = centerTextX(getCurrentTime(), 4);
    display.setCursor(centerX, 105);
    display.setTextWrap(false);
    display.setTextSize(4);
    display.print(getCurrentTime());
  }
  delay(1000);
}

String getCurrentTime() {
  Time.zone(-7);
  return Time.format("%I:%M %p");
}

String getCurrentDate() {
  Time.zone(-7);
  return Time.format("%B%e, %Y");
}

String getCurrentWeekDay(int offset) {
  Time.zone(-7);
  dayOfWeek = (Time.weekday() + offset - 1 + 7) % 7 + 1;

  switch (dayOfWeek) {
    case 1: return "Sunday";
    case 2: return "Monday";
    case 3: return "Tuesday";
    case 4: return "Wednesday";
    case 5: return "Thursday";
    case 6: return "Friday";
    case 7: return "Saturday";
    default: return "Unknown";
  }
}

String getAbbreviatedWeekDay(int offset) {
  String fullDay = getCurrentWeekDay(offset);
  return fullDay.substring(0, 3);
}

int centerTextX(String text, int textSize) {
  int textWidth = text.length() * (textSize * 6);
  int screenWidth = display.width();
  return (screenWidth - textWidth) / 2;
}

void updateTemps() {
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK); display.setTextSize(2); display.setCursor(32, 265); display.print(yesterdayTemp); display.setTextSize(1); display.setCursor(32+26, 260); display.print("o"); display.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK); display.setCursor(32+26, 272); display.print("F");
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK); display.setTextSize(2); display.setCursor(107, 265); display.print(todayTemp); display.setTextSize(1); display.setCursor(107+26, 260); display.print("o"); display.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK); display.setCursor(107+26, 272); display.print("F");
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK); display.setTextSize(2); display.setCursor(182, 265); display.print(tomorrowTemp); display.setTextSize(1); display.setCursor(182+26, 260); display.print("o"); display.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK); display.setCursor(182+26, 272); display.print("F");
}

uint8_t* getWeatherCode(String weatherCode) {
  if (weatherCode == "1000" || weatherCode == "1100") return sunny;
  if (weatherCode == "1101" || weatherCode == "1102" || weatherCode == "1001") return cloudy;
  if (weatherCode == "2000" || weatherCode == "2100") return fog;
  if (weatherCode == "4000" || weatherCode == "4001" || weatherCode == "4200" || weatherCode == "4201") return rain;
  if (weatherCode == "5000" || weatherCode == "5001" || weatherCode == "5100" || weatherCode == "5101" || weatherCode == "6000" || weatherCode == "6001" || weatherCode == "6200" || weatherCode == "6201" || weatherCode == "7000" || weatherCode == "7101" || weatherCode == "7102") return snow;
  if (weatherCode == "8000") return thunderstorm;
  return blank;
}