#include "print.hpp"
#include "sensor.hpp"

enum { TemperatureThreashold = 28 };

enum LEDS {
    Yellow = 5,
    Red = 6,
    Green = 9,
};

enum Sensors {
    Temperature = A2,
    Light = A1,
    Potentiometer = A0,
};

const Sensor SENSORS[] = {
    Sensor(
        Temperature,
        Yellow,
        [](Sensor const& s, u16 voltage) {
            auto degreesC = ((voltage * 0.004882814) - 0.5) * 100.0;
            if (degreesC > TemperatureThreashold) {
                println("Temperature: \x1b[32m", degreesC, "C\x1b[0m");
                digitalWrite(s.led, HIGH);
            } else {
                println("Temperature: \x1b[31m", degreesC, "C\x1b[0m");
                digitalWrite(s.led, LOW);
            }
        }),
    Sensor(
        Light,
        Red,
        [](Sensor const& s, u16 value) {
            analogWrite(s.led, map(value, 0, 1023, 255, 0));
        }),
    Sensor(Potentiometer, Green, [](Sensor const& s, u16 value) {
        static u32 last_blink_time = 0UL;
        static bool led_on = false;
        u32 const blink_interval = map(value, 0, 1023, 200, 2000);
        u32 const now = millis();
        if (last_blink_time + blink_interval < now) {
            digitalWrite(s.led, led_on);
            led_on = !led_on;
            last_blink_time = now;
        }
    })};

void setup() {
    for (auto& s : SENSORS) s.setup();
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

void loop() {
    for (auto& s : SENSORS) s.update();
}
