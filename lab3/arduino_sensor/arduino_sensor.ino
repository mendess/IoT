#include "sensor.hpp"

enum { TemperatureThreashold = 22 };

static auto TEMPERATURE = Sensor(A2);

void setup() {
    Serial.begin(9600);
    pinMode(5, OUTPUT);
}

void loop() {
    auto const voltage = TEMPERATURE.read();
    auto const degreesC = ((50 * voltage) / 102) - 50;
    Serial.println(degreesC);
    if (degreesC > TemperatureThreashold) {
        digitalWrite(5, HIGH);
    } else {
        digitalWrite(5, LOW);
    }
    delay(1);
}
