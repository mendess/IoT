#include "actuators.hpp"

enum { TemperatureThreashold = 28 };

void yellow(Led led, u16 voltage) {
    auto degreesC = (((voltage / 1024.0) * 5.0) - 0.5) * 100.0;
    if (degreesC > TemperatureThreashold) {
        digitalWrite(led, HIGH);
    } else {
        digitalWrite(led, LOW);
    }
}

void red(Led led, u16 voltage) {
    analogWrite(led, map(voltage, 1, 1021, 255, 0));
}

void green(Led led, u16 voltage) {
    static u32 last_blink_time = 0UL;
    static bool led_on = false;

    u32 const half_blink_interval = map(voltage, 0, 1023, 100, 1000);
    u32 const now = millis();
    if (last_blink_time + half_blink_interval < now) {
        digitalWrite(led, led_on);
        led_on = !led_on;
        last_blink_time = now;
    }
}

constexpr Actuator SENSORS[] = {
    Actuator(Yellow, yellow), Actuator(Red, red), Actuator(Green, green)};

void setup() {
    for (auto& s : SENSORS) s.setup();
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

void loop() {
    auto dummy_value = 42;
    for (auto& s : SENSORS) s.update(dummy_value);
}
