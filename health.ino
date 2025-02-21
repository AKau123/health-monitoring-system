#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define WIFI_SSID "INCUBE"
#define WIFI_PASSWORD "Anurag@cvsr$"
#define THINGSPEAK_API_KEY "M2MV495PXJZO4EZX"
#define THINGSPEAK_URL "https://api.thingspeak.com/update"

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define REPORTING_PERIOD_MS 2000
PulseOximeter pox;
uint32_t lastReportTime = 0;

void onBeatDetected() {
    Serial.println("Beat detected!");
}

void connectToWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(" Connected!");
    } else {
        Serial.println(" Failed to connect to WiFi. Check credentials or signal strength.");
    }
}

void setup() {
    Serial.begin(9600);
    connectToWiFi();
    sensors.begin();
    Serial.println("Initializing MAX30100...");
    if (!pox.begin()) {
        Serial.println("FAILED to initialize MAX30100! Check connections.");
        while (1);
    } else {
        Serial.println("MAX30100 initialized successfully.");
    }
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void sendDataToThingSpeak(float temperature, float heartRate, float spo2) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Disconnected! Reconnecting...");
        connectToWiFi();
        return;
    }
    if (isnan(heartRate) || heartRate == 0 || isnan(spo2) || spo2 == 0) {
        Serial.println("Invalid heart rate or SpO2 detected. Skipping ThingSpeak update.");
        return;
    }
    HTTPClient http;
    String url = String(THINGSPEAK_URL) + "?api_key=" + THINGSPEAK_API_KEY +
                 "&field1=" + String(temperature) +
                 "&field2=" + String(heartRate) +
                 "&field3=" + String(spo2);
    Serial.print("Sending data to ThingSpeak: ");
    Serial.println(url);
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        Serial.print("ThingSpeak Response Code: ");
        Serial.println(httpResponseCode);
        Serial.println("Data uploaded successfully!");
    } else {
        Serial.print("Error sending data: ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
}

void loop() {
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" \u00B0C");
    pox.update();
    if (millis() - lastReportTime > REPORTING_PERIOD_MS) {
        lastReportTime = millis();
        float heartRate = pox.getHeartRate();
        float spo2 = pox.getSpO2();
        Serial.print("Heart Rate: ");
        Serial.print(heartRate);
        Serial.print(" BPM, SpO2: ");
        Serial.print(spo2);
        Serial.println(" %");
        sendDataToThingSpeak(temperatureC, heartRate, spo2);
    }
    delay(2000);
}
