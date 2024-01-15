/* 
 * Project: Particle Shuttle Clock
 * Author: Tyler Mueller
 * Last Updated: 1/14/2024
*/


// Import Libraries & Define TFT Pins
#include <Particle.h>
#include "../lib/Adafruit_ILI9341/src/Adafruit_ILI9341.h"
#if defined(PARTICLE)
  #define TFT_DC   D5
  #define TFT_CS   D4
#endif


// Instance Variables
Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);

String yesterdayTemperature, todayTemperature, tomorrowTemperature, yesterdayCode, todayCode, tomorrowCode;
extern uint8_t blank[], sunny[], cloudy[], fog[], rain[], snow[], thunderstorm[];
int lastMinute, lastHour, lastDay = -1;
int dayOfWeek = 0;

uint8_t* getWeatherCode(String weatherCode) {
  if (weatherCode == "1000" || weatherCode == "1100") return sunny;
  if (weatherCode == "1101" || weatherCode == "1102" || weatherCode == "1001") return cloudy;
  if (weatherCode == "2000" || weatherCode == "2100") return fog;
  if (weatherCode == "4000" || weatherCode == "4001" || weatherCode == "4200" || weatherCode == "4201") return rain;
  if (weatherCode == "5000" || weatherCode == "5001" || weatherCode == "5100" || weatherCode == "5101" || weatherCode == "6000" || weatherCode == "6001" || weatherCode == "6200" || weatherCode == "6201" || weatherCode == "7000" || weatherCode == "7101" || weatherCode == "7102") return snow;
  if (weatherCode == "8000") return thunderstorm;
  return blank;
}
void centerText(String text, int textSize, int yPos, int offset) {
  int textWidth = text.length() * (textSize * 6);
  int screenWidth = display.width();
  int centeredX = (screenWidth - textWidth) / 2;
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  display.setCursor(centeredX + offset, yPos);
  display.setTextSize(textSize);
  display.print(text);
}
void screenLayout() {
  display.fillScreen(ILI9341_BLACK);

  display.drawRect(0, 0, 240, 320, ILI9341_WHITE);
  display.drawFastHLine(10, 80, 230-10, ILI9341_WHITE);
  display.drawFastHLine(10, 155, 230-10, ILI9341_WHITE);

  centerText("Rapid City, South Dakota", 1, 300, 0);
}
String getCurrentWeekDay(int offset) {
  dayOfWeek = (Time.weekday() + offset - 1 + 7) % 7 + 1;

  switch (dayOfWeek) {
    case 1: return "Sunday";
    case 2: return "Monday";
    case 3: return "Tuesday";
    case 4: return "Wednesday";
    case 5: return "Thursday";
    case 6: return "Friday";
    case 7: return "Saturday";
    default: return "N/A";
  }
}
String getCurrentDate() {
  return Time.format("%B %e, %Y");
}
String getCurrentTime() {
  return Time.format("%I:%M %p");
}
String getAbbreviatedWeekDay(int offset) {
  String fullDay = getCurrentWeekDay(offset);
  return fullDay.substring(0, 3);
}
void drawTempSymbols(int xPos, String temperature) {
  int offset;

  if (temperature.length() == 3) {
    offset = 8;
  } else if (temperature.length() == 1) {
    offset = -5;
  } else {
    offset = 0;
  }

  display.setTextColor(ILI9341_LIGHTGREY, ILI9341_BLACK);
  display.setTextSize(1);
  display.setCursor(xPos + offset, 260);
  display.print("o");
  display.setCursor(xPos + offset, 272);
  display.print("F");
}
void updateTemps() {
  centerText(getAbbreviatedWeekDay(-1), 2, 170, -75);
  centerText("Today", 2, 170, 0);
  centerText(getAbbreviatedWeekDay(1), 2, 170, 75);

  display.fillRect(10, 260, 220, 25, ILI9341_BLACK);

  centerText(yesterdayTemperature, 2, 265, -75);
  centerText(todayTemperature, 2, 265, 0);
  centerText(tomorrowTemperature, 2, 265, 75);

  drawTempSymbols(60, String(yesterdayTemperature));
  drawTempSymbols(135, String(todayTemperature));
  drawTempSymbols(210, String(tomorrowTemperature));
}
void temperatureHandler(const char *event, const char *data) {
  int index0, index1, index2;
  String tempString = String(data);

  index0 = tempString.indexOf('~');
  yesterdayTemperature = String(round(tempString.substring(0, index0).toFloat())).toInt();

  index1 = tempString.indexOf('~', index0+1);
  todayTemperature = String(round(tempString.substring(index0+1, index1).toFloat())).toInt();

  index2 = tempString.indexOf('~', index1+1);
  tomorrowTemperature = String(round(tempString.substring(index1+1, index2).toFloat())).toInt();

  updateTemps();
}
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
void callWebhooks() {
  String data = String(10);
  Particle.publish("getTemperatures", data, PRIVATE);
  Particle.publish("weatherCode", data, PRIVATE);
}

void setup() {
  display.begin();

  screenLayout();

  Particle.subscribe("hook-response/getTemperatures", temperatureHandler, MY_DEVICES);
  Particle.subscribe("hook-response/weatherCode", weatherCodeHandler, MY_DEVICES);

  callWebhooks();
}

void loop() {
  Time.zone(-7);

  int currentSecond = Time.second();
  int currentMinute = Time.minute();
  int currentHour = Time.hour();
  int currentDay = Time.day();

  if (currentDay != lastDay) {
    lastDay = currentDay;

    centerText(getCurrentWeekDay(0), 3, 15, 0);
    centerText(getCurrentDate(), 2, 50, 0);
  }
  if (currentMinute != lastMinute || currentHour != lastHour) {
    lastMinute = currentMinute;
    lastHour = currentHour;

    centerText(getCurrentTime(), 4, 105, 0);
  }
  if (currentMinute == 0 && currentSecond == 0) {
    callWebhooks();
  }
  delay(1000);
}