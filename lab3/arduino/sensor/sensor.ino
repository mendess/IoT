#include "sensor.hpp"

static Sensor SENSORS[] = {
    Sensor(Temperature, 'T'),
    Sensor(Light, 'L'),
    Sensor(Potentiometer, 'P'),
};

void read_errors() {
    if (Serial.available()) {
        byte err[1];
        while (Serial.readBytes(err, sizeof err) < 1)
            ;
        digitalWrite(*err, HIGH);
        for(byte i = 0; i < *err; ++i) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    for (auto& s: SENSORS) {
        s.setup();
    }
}

void loop() {
    for (auto& s : SENSORS) {
        auto value = s.read();
        char buf[10];
        buf[0] = s.sensor_name;
        sprintf(buf + 1, "%hd\n", value);
        Serial.print(buf);
    }
    read_errors();
    delay(1);
}
