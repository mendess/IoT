#include "sensor.hpp"

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
        [](Sensor const& s, u16 value) {
            Serial.print("Temperature: ");
            Serial.print(value);
            Serial.print("C");
            analogWrite(s.led, value);
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
