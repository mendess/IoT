#include "sensor.hpp"

static Sensor SENSORS[] = {
    Sensor(Temperature, 'T'),
    Sensor(Light, 'L'),
    Sensor(Potentiometer, 'P'),
};

void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    for (auto& s : SENSORS) {
        auto value = s.read();
        char buf[10];
        buf[0] = s.sensor_name;
        sprintf(buf + 1, "%hd\n", value);
        Serial.print(buf);
    }
    delay(1);
}
