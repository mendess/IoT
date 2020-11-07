#include "sensor.hpp"

enum { TemperatureThreashold = 25 };

static auto TEMPERATURE = Sensor(A2);

void setup() {
    Serial.begin(9600);
    pinMode(5, OUTPUT);
}

void loop() {
    TEMPERATURE.read([](u16 voltage) {
        auto degreesC = ((500 * voltage) >> 10) - 50;
        Serial.println(degreesC);
        if (degreesC > TemperatureThreashold) {
            digitalWrite(5, HIGH);
        } else {
            digitalWrite(5, LOW);
        }
    });
}
