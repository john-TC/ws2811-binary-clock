#include <Arduino.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define LED_TYPE WS2811
#define NUM_LEDS 50
#define COLOR_ORDER RGB
#define DATA_PIN 12
#define BRIGHTNESS 32
#define WIFI_LED 2

#define SSID "********"
#define PASSWORD "********"

int ledPinsSec[] = {20, 21, 22, 23, 24, 25};
int ledPinsMin[] = {31, 30, 29, 28, 27, 26};
int ledPinsHr[] = {44, 45, 46, 47, 48};
int ledPinsDate[] = {18, 17, 16, 15, 14};
int ledPinsMonth[] = {33, 34, 35, 36};
int ledPinsYear[] = {42, 41, 40, 39, 38};
int ledPinsDay[] = {19, 32, 43};
int ledPinsTempAbove[] = {8, 9, 10, 11, 12, 13};
int ledPinsTempBelow[] = {7, 6, 5, 4, 3, 2};

CHSV orange = CHSV(17, 255, 255);
CHSV white = CHSV(0, 0, 235);
CHSV grey = CHSV(0, 0, 191);
CHSV lightBlue = CHSV(149, 166, 166);
CHSV darkBlue = CHSV(148, 255, 153);

int prevTime = -1;
int currentTime = -1;
int prevMins = -1;
int prevHours = -1;
int prevDate = -1;
int prevMonth = -1;
int prevYear = -1;
int prevTemp;
bool updated = false;

WiFiUDP ntpUDP;
WiFiClient client;
HTTPClient http;
NTPClient timeClient(ntpUDP, -25200);
CRGB leds[NUM_LEDS];

#define nBitsSec sizeof(ledPinsSec)/sizeof(ledPinsSec[0])
#define nBitsMin sizeof(ledPinsMin)/sizeof(ledPinsMin[0])
#define nBitsHr sizeof(ledPinsHr)/sizeof(ledPinsHr[0])
#define nBitsDate sizeof(ledPinsDate)/sizeof(ledPinsDate[0])
#define nBitsMonth sizeof(ledPinsMonth)/sizeof(ledPinsMonth[0])
#define nBitsYear sizeof(ledPinsYear)/sizeof(ledPinsYear[0])
#define nBitsDay sizeof(ledPinsDay)/sizeof(ledPinsDay[0])
#define nBitsTempAbove sizeof(ledPinsTempAbove)/sizeof(ledPinsTempAbove[0])
#define nBitsTempBelow sizeof(ledPinsTempBelow)/sizeof(ledPinsTempBelow[0])

void dispBinarySec(byte nSec) {
  for (byte i = 0; i < nBitsSec; i++) {
    if (nSec & 1) {
      leds[ledPinsSec[i]] = grey;
      FastLED.show();
    }
    else {
      leds[ledPinsSec[i]] = CHSV(0, 0, 0);
      FastLED.show();
    }
    nSec /= 2;
  }
}

void dispBinaryMin(byte nMin) {
  if (nMin != prevMins) {
    prevMins = nMin;
    for (byte i = 0; i < nBitsMin; i++) {
      if (nMin & 1) {
        leds[ledPinsMin[i]] = lightBlue;
        FastLED.show();
      }
      else {
        leds[ledPinsMin[i]] = CHSV(0, 0, 0);
        FastLED.show();
      }
      nMin /= 2;
    }
  }
}

void dispBinaryHr(byte nHr) {
  if (nHr != prevHours) {
    prevHours = nHr;
    for (byte i = 0; i < nBitsHr; i++) {
      if (nHr & 1) {
        leds[ledPinsHr[i]] = darkBlue;
        FastLED.show();
      }
      else {
        leds[ledPinsHr[i]] = CHSV(0, 0, 0);
        FastLED.show();
      }
      nHr /= 2;
    }
  }
}

void dispBinaryDay(byte nDay) {
  if (nDay == 0) {
      nDay = 7;
  }
  for (byte i = 0; i < nBitsDay; i++) {
    if (nDay & 1) {
      leds[ledPinsDay[i]] = orange;
      FastLED.show();
    }
    else {
      leds[ledPinsDay[i]] = CHSV(0, 0, 0);
      FastLED.show();
    }
    nDay /= 2;
  }
}

void dispBinaryDate(byte nDate) {
  if (nDate != prevDate) {
    prevDate = nDate;
    dispBinaryDay(timeClient.getDay());
    for (byte i = 0; i < nBitsDate; i++) {
      if (nDate & 1) {
        leds[ledPinsDate[i]] = grey;
        FastLED.show();
      }
      else {
        leds[ledPinsDate[i]] = CHSV(0, 0, 0);
        FastLED.show();
      }
      nDate /= 2;
    }
  }
}

void dispBinaryMonth(byte nMonth) {
  if (nMonth != prevMonth) {
    prevMonth = nMonth;
    for (byte i = 0; i < nBitsMonth; i++) {
      if (nMonth & 1) {
        leds[ledPinsMonth[i]] = lightBlue;
        FastLED.show();
      }
      else {
        leds[ledPinsMonth[i]] = CHSV(0, 0, 0);
        FastLED.show();
      }
      nMonth /= 2;
    }
  }
}

void dispBinaryYear(byte nYear) {
  if (nYear != prevYear) {
    prevYear = nYear;
    for (byte i = 0; i < nBitsYear; i++) {
      if (nYear & 1) {
        leds[ledPinsYear[i]] = darkBlue;
        FastLED.show();
      }
      else {
        leds[ledPinsYear[i]] = CHSV(0, 0, 0);
        FastLED.show();
      }
      nYear /= 2;
    }
  }
}

int getTemp() {
  http.begin("http://api.openweathermap.org/data/2.5/weather?id={CITY-ID}&appid={API-KEY}&units=metric");
  http.GET();
  const size_t capacity = JSON_ARRAY_SIZE(3) + 2*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 480;
  DynamicJsonBuffer jsonBuffer(capacity);
  String json = http.getString();
  JsonObject& root = jsonBuffer.parseObject(json);
  JsonObject& main = root["main"];
  float main_temp = main["temp"];
  return round(main_temp);
}

void dispBinaryTemp(int nTemp) {
  if (nTemp != prevTemp) {
    if (nTemp >= 0) {
      prevTemp = nTemp;
      for (byte i = 0; i < nBitsTempBelow; i++) {
        leds[ledPinsTempBelow[i]] = CHSV(0, 0, 0);
        FastLED.show();
      }
      for (byte i = 0; i < nBitsTempAbove; i++) {
        if (nTemp & 1) {
          leds[ledPinsTempAbove[i]] = CHSV(0, 255, 255);
          FastLED.show();
        }
        else {
          leds[ledPinsTempAbove[i]] = CHSV(0, 0, 0);
          FastLED.show();
        }
        nTemp /= 2;
      }
    }
    else {
      nTemp = nTemp * -1;
      prevTemp = nTemp;
      for (byte i = 0; i < nBitsTempAbove; i++) {
        leds[ledPinsTempAbove[i]] = CHSV(0, 0, 0);
        FastLED.show();
      }
      for (byte i = 0; i < nBitsTempBelow; i++) {
        if (nTemp & 1) {
          leds[ledPinsTempBelow[i]] = CHSV(160, 255, 255);
          FastLED.show();
        }
        else {
          leds[ledPinsTempBelow[i]] = CHSV(0, 0, 0);
          FastLED.show();
        }
        nTemp /= 2;
      }
    }
  }
}

void setupWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(WIFI_LED, HIGH);
    delay(500);
    digitalWrite(WIFI_LED, LOW);
  }
  digitalWrite(WIFI_LED, HIGH);
  timeClient.update();
  dispBinaryTemp(getTemp());
}

void setup() {
  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, HIGH);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  WiFi.begin(SSID, PASSWORD);
  timeClient.begin();
  // Serial.begin(9600);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(WIFI_LED, HIGH);
    setupWifi();
  }
  currentTime = timeClient.getEpochTime();
  if (currentTime != prevTime) {
    dispBinarySec(timeClient.getSeconds());
    dispBinaryMin(timeClient.getMinutes());
    dispBinaryHr(timeClient.getHours());
    dispBinaryDate(timeClient.getDate());
    dispBinaryMonth(timeClient.getMonth());
    dispBinaryYear(timeClient.getYear() % 100);
    prevTime = currentTime;
    if (updated) {
      updated = false;
    }
  }
  if (currentTime % 900 == 0) {
    dispBinaryTemp(getTemp());
  }
  if (currentTime % 3600 == 0 && !updated) {
    timeClient.forceUpdate();
    updated = true;
  }
}
