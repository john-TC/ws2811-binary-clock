#include <Arduino.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

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

CHSV orange = CHSV(17, 255, 255);
CHSV white = CHSV(0, 0, 235);
CHSV grey = CHSV(0, 0, 191);
CHSV lightBlue = CHSV(149, 166, 166);
CHSV darkBlue = CHSV(148, 255, 153);

int prevTime = -1;
int currentTime = -1;
int prevMins = -1;
int currentMins = -1;
int prevHours = -1;
int currentHours = -1;
int prevDate = -1;
int currentDate = -1;
int prevMonth = -1;
int currentMonth = -1;
int prevYear = -1;
int currentYear = -1;
bool updated = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, -25200);
CRGB leds[NUM_LEDS];

#define nBitsSec sizeof(ledPinsSec)/sizeof(ledPinsSec[0])
#define nBitsMin sizeof(ledPinsMin)/sizeof(ledPinsMin[0])
#define nBitsHr sizeof(ledPinsHr)/sizeof(ledPinsHr[0])
#define nBitsDate sizeof(ledPinsDate)/sizeof(ledPinsDate[0])
#define nBitsMonth sizeof(ledPinsMonth)/sizeof(ledPinsMonth[0])
#define nBitsYear sizeof(ledPinsYear)/sizeof(ledPinsYear[0])
#define nBitsDay sizeof(ledPinsDay)/sizeof(ledPinsDay[0])

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
  currentMins = timeClient.getMinutes();
  if (currentMins != prevMins) {
    prevMins = currentMins;
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
  currentHours = timeClient.getHours();
  if (currentHours != prevHours) {
    prevHours = currentHours;
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
  currentDate = timeClient.getDate();
  if (currentDate != prevDate) {
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
  currentMonth = timeClient.getMonth();
  if (currentMonth != prevMonth) {
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
  currentYear = timeClient.getYear();
  if (currentYear != prevYear) {
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

void setupWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(WIFI_LED, HIGH);
    delay(500);
    digitalWrite(WIFI_LED, LOW);
  }
  digitalWrite(WIFI_LED, HIGH);
  timeClient.update();
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
    // Serial.println(timeClient.getFormattedTime());
    prevTime = currentTime;
    if (updated) {
      updated = false;
    }
  }
  if (currentTime % 3600 == 0 && !updated) {
    // int oldTime = timeClient.getEpochTime();
    timeClient.forceUpdate();
    // int newTime = timeClient.getEpochTime();
    // Serial.print("Old time: ");
    // Serial.println(oldTime);
    // Serial.print("New time: ");
    // Serial.println(newTime);
    // Serial.print("Difference: ");
    // Serial.println(newTime - oldTime);
    updated = true;
  }
}
