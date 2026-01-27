#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 rtc;  // DS3231 RTC modul

const int RELAY_PIN = 7;

struct TimeSchedule {
  int onHour;
  int onMinute;
  int offHour;
  int offMinute;
};

TimeSchedule schedule = {6, 0, 22, 0};

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  
  if (!rtc.begin()) {
    Serial.println("RTC modul nenájdený!");
    while (1);
  }
  
  // Nastav čas pri prvom spustení (odkomentuj a nahraď aktuálnym časom)
  // rtc.adjust(DateTime(2026, 1, 27, 12, 0, 0));
  
  Serial.println("RTC časovač inicializovaný");
}

bool shouldBeOn(DateTime now) {
  int currentTimeInMinutes = now.hour() * 60 + now.minute();
  int onTime = schedule.onHour * 60 + schedule.onMinute;
  int offTime = schedule.offHour * 60 + schedule.offMinute;
  
  if (onTime < offTime) {
    return (currentTimeInMinutes >= onTime && currentTimeInMinutes < offTime);
  } else {
    return (currentTimeInMinutes >= onTime || currentTimeInMinutes < offTime);
  }
}

void loop() {
  DateTime now = rtc.now();
  
  bool relayState = shouldBeOn(now);
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  
  // Debug výpis každých 10 sekúnd
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 10000) {
    lastPrint = millis();
    Serial.print(now.year());
    Serial.print('/');
    Serial.print(now.month());
    Serial.print('/');
    Serial.print(now.day());
    Serial.print(' ');
    Serial.print(now.hour());
    Serial.print(':');
    Serial.print(now.minute());
    Serial.print(" | Relé: ");
    Serial.println(relayState ? "ZAP" : "VYP");
  }
  
  delay(1000);
}