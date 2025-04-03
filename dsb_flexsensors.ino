
#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor Pins
const int flex3Thumb = A0;   // Thumb flex sensor
const int switchPin = 2;     // SPST switch input
const int buzzerPin = 3;     // Buzzer output

// DS18B20 Temperature Sensor Pin
#define ONE_WIRE_BUS 4       
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
    Serial.begin(9600);

    // Pin Modes
    pinMode(switchPin, INPUT_PULLUP);  // Internal pull-up for SPST switch
    pinMode(buzzerPin, OUTPUT);

    // Initialize Temperature Sensor
    sensors.begin();
}

void loop() {
    // Read Sensors
    int thumbValue = analogRead(flex3Thumb);
    int switchState = digitalRead(switchPin);
    sensors.requestTemperatures(); // Request temperature
    float temperatureC = sensors.getTempCByIndex(0);

    // Default status
    String status = "No Request";

    // Check SPST Switch (Emergency Trigger)
    if (switchState == LOW) {
        status = "EMERGENCY!";
        digitalWrite(buzzerPin, HIGH); // Activate buzzer
    } 
    else {
        digitalWrite(buzzerPin, LOW); // Turn off buzzer

        // Gesture-Based Requests
        if (thumbValue == 20) {
            status = "Needs Washroom";
        }
        else if (thumbValue == 18) {
            status = "Needs Food";
        }
    }

    // Print Output to Serial Monitor
    Serial.print("Thumb Value: "); Serial.print(thumbValue);
    Serial.print(" | Status: "); Serial.print(status);
    Serial.print(" | Temperature: "); Serial.print((temperatureC*9/5)+32); Serial.println(" F");

    delay(3000); // Delay before next reading
}
