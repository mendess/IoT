#include "actuators.hpp"

enum { TemperatureThreshold = 28 };

void yellow(Led led, u16 voltage) {
    auto degreesC = (((voltage / 1024.0) * 5.0) - 0.5) * 100.0;
    if (degreesC > TemperatureThreshold) {
        digitalWrite(led, HIGH);
    } else {
        digitalWrite(led, LOW);
    }
}

void red(Led led, u16 voltage) {
    analogWrite(led, map(voltage, 1, 1021, 255, 0));
}

static u32 half_blink_interval;

void green(Led led, u16 voltage) {
    (void) led;
    half_blink_interval = map(voltage, 0, 1023, 100, 1000);
}

void blink_green() {
    static u32 last_blink_time = 0UL;
    static bool led_on = false;

    u32 const now = millis();
    if (last_blink_time + half_blink_interval < now) {
        digitalWrite(Green, led_on);
        led_on = !led_on;
        last_blink_time = now;
    }
}

constexpr Actuator SENSORS[] = {
    Actuator(Yellow, yellow), Actuator(Red, red), Actuator(Green, green)};

struct Message {
    size_t actuator;
    u16 value;
    constexpr Message(): actuator{0}, value{0} {}
    constexpr Message(size_t a, u16 v): actuator{a}, value{v} {}
};

using Message = struct Message;

bool read_serial(Message* msg) {
    byte buf[3];
    if (!Serial.available() || Serial.readBytes(buf, sizeof buf) < 3) {
        return false;
    }
    u16 v = buf[2];
    v <<= 8;
    v |= buf[1];
    switch (*buf) {
        case 'T':
            *msg = {0, v};
            break;
        case 'L':
            *msg = {1, v};
            break;
        case 'P':
            *msg = {2, v};
            break;
        default:
            return false;
    }
    return true;
}

void setup() {
    for (auto& s : SENSORS) s.setup();
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
}

void loop() {
    Message msg;
    if (read_serial(&msg)) SENSORS[msg.actuator].update(msg.value);
    blink_green();
}
