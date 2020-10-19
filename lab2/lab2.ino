#include "sensor.hpp"

enum { TemperatureThreashold = 28 };

enum LEDS {
    YELLOW = 5,
    RED = 6,
    GREEN = 9,
};

enum Sensors {
    Temperature = A2,
    Light = A1,
    Potentiometer = A0,
};

const Sensor SENSORS[] = {
    Sensor(
        Temperature,
        YELLOW,
        [](Sensor const& s, u16 voltage) {
            auto degreesC = ((voltage * 0.004882814) - 0.5) * 100.0;
            if (degreesC > TemperatureThreashold) {
                Serial.print("Temperature: \x1b[32m");
                Serial.print(degreesC);
                Serial.println("C\x1b[0m");
                digitalWrite(s.led, HIGH);
            } else {
                Serial.print("Temperature: \x1b[31m");
                Serial.print(degreesC);
                Serial.println("C\x1b[0m");
                digitalWrite(s.led, LOW);
            }
        }),
    Sensor(
        Light,
        RED,
        [](Sensor const& s, u16 value) {
            analogWrite(s.led, 255 - map(value, 0, 1023, 0, 255));
        }),
    Sensor(Potentiometer, GREEN, [](Sensor const& s, u16 value) {
        analogWrite(s.led, map(value, 0, 1023, 0, 255));
    })};

void setup() {
    for (auto& s : SENSORS) {
        s.setup();
    }
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

void loop() {
    for (auto& s : SENSORS) {
        s.update();
    }
}
