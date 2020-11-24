#include "sensor.hpp"

enum { TemperatureThreashold = 22 };

enum Sensors {
    Temperature = A2,
    Light = A1,
    Potentiometer = A0,
};

static Sensor SENSORS[] = {
    Sensor(Temperature, 'T'),
    Sensor(Light, 'L'),
    Sensor(Potentiometer, 'P'),
};

void setup() {
    Serial.begin(9600);
    pinMode(5, OUTPUT);
}

void loop() {
    for (auto& s : SENSORS) {
        char buf[10] = {s.sensor_name, ':'};
        sprintf(buf + 2, "%d\n", s.read());
        /*
        auto value = s.read();
        char buf[] = {s.sensor_name, ':', (char) (value & 0xff), (char) ((value >> 8) & 0xff)};
        */
        Serial.print(buf);
    }
    delay(1);
}
