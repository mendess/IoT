#include "actuators.hpp"

enum { TemperatureThreshold = 28 };

void temperature(Led led, u16 voltage) {
    auto degreesC = ((500 * (u32) voltage) >> 10) - 50;
    auto w = degreesC > TemperatureThreshold ? HIGH : LOW;
    digitalWrite(led, w);
}

void light(Led led, u16 voltage) {
    analogWrite(led, map(voltage, 1, 1021, 255, 0));
}

static u32 half_blink_interval = 100;

void potentiometer(Led led, u16 voltage) {
    (void) led;
    half_blink_interval = map(voltage, 0, 1023, 100, 1000);
}

void blink_potentiometer() {
    static u32 last_blink_time = 0UL;
    static bool led_on = false;

    u32 const now = millis();
    if (last_blink_time + half_blink_interval < now) {
        digitalWrite(Green, led_on = !led_on);
        last_blink_time = now;
    }
}

#define NUM_SENSORS 3

constexpr Actuator SENSORS[NUM_SENSORS] = {
    Actuator(Yellow, temperature),
    Actuator(Green, potentiometer),
    Actuator(Red, light)};

auto read_serial(u16* msg) -> bool {
    byte buf[NUM_SENSORS * 2];
    if (!Serial.available()) return false;

    size_t read_so_far = 0;
    while (read_so_far != sizeof buf)
        read_so_far +=
            Serial.readBytes(buf + read_so_far, sizeof(buf) - read_so_far);

    msg[0] = buf[0] | ((u16) buf[1]) << 8;
    msg[1] = buf[2] | ((u16) buf[3]) << 8;
    msg[2] = buf[4] | ((u16) buf[5]) << 8;
    return true;
}

void setup() {
    for (auto& s : SENSORS) s.setup();
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

static u16 msg[NUM_SENSORS];

void loop() {
    if (read_serial(msg))
        for (size_t i = 0; i < NUM_SENSORS; ++i) SENSORS[i].update(msg[i]);
    blink_potentiometer();
    delay(1);
}
