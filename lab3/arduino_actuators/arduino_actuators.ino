#include "actuators.hpp"

enum { TemperatureThreshold = 28 };

void temperature(Led led, u16 voltage) {
    auto degreesC = (((voltage / 1024.0) * 5.0) - 0.5) * 100.0;
    if (degreesC > TemperatureThreshold) {
        digitalWrite(led, HIGH);
    } else {
        digitalWrite(led, LOW);
    }
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

struct Message {
    u16 values[NUM_SENSORS];
};

using Message = struct Message;

constexpr byte ACK = 'A';

auto read_serial(Message* msg) -> bool {
    byte buf[NUM_SENSORS * 2];
    if (!Serial.available()) {
        return false;
    }
    size_t read_so_far = 0;
    while (read_so_far != sizeof buf) {
        read_so_far += Serial.readBytes(buf, sizeof(buf) - read_so_far);
    }
    Serial.write(&ACK, sizeof ACK);
    *msg = (Message){
        .values = {
            buf[0] | ((u16) buf[1]) << 8,
            buf[2] | ((u16) buf[3]) << 8,
            buf[4] | ((u16) buf[5]) << 8,
        }};
    return true;
}

void setup() {
    for (auto& s : SENSORS) s.setup();
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

void bbbbblink(u32 x) {
    digitalWrite(LED_BUILTIN, 1);
    delay(500);
    for (u32 i = 0; i < x; i++) {
        digitalWrite(LED_BUILTIN, 0);
        delay(200);
        digitalWrite(LED_BUILTIN, 1);
        delay(200);
    }
    digitalWrite(LED_BUILTIN, 0);
}

static Message msg = {0};

void loop() {
    if (read_serial(&msg)) {
        for (size_t i = 0; i < NUM_SENSORS; ++i)
            SENSORS[i].update(msg.values[i]);
    }
    blink_potentiometer();
    delay(1);
}
