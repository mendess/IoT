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
            Serial.println("C"); // print the temperature status
            analogWrite(s.led, value);
        }),
    Sensor(
        Light,
        RED,
        [](Sensor const& s, u16 value) {
            Serial.print("Analog reading = ");
            Serial.print(value); // the raw analog reading
            if (value < 10) {
                Serial.println(" - Dark");
            } else if (value < 200) {
                Serial.println(" - Dim");
            } else if (value < 500) {
                Serial.println(" - Light");
            } else if (value < 800) {
                Serial.println(" - Bright");
            } else {
                Serial.println(" - Very bright");
            }
            u8 i = 255 - map(value, 0, 1023, 0, 255);
            Serial.print("Setting red led to ");
            Serial.print(i);
            Serial.println("% power");
            analogWrite(s.led, i);
        }),
    Sensor(Potentiometer, GREEN, [](Sensor const& s, u16 value) {
        u8 i = map(value, 0, 1023, 0, 255);
        Serial.print("Setting green led to ");
        Serial.print(i);
        Serial.println("% power");
        analogWrite(s.led, i);
    })};

void setup() {
    for(auto& s: SENSORS) {
        s.setup();
    }
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

void loop() {
    for(auto& s: SENSORS) {
        s.update();
    }
    /* delay(1000); */
}
