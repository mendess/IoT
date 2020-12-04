#include "actuators.hpp"

enum { TemperatureThreshold = 28 };

/* Update state functions */

void temperature(Actuator const& self, u16 voltage) {
    int32_t degreesC = ((500 * (int32_t) voltage) >> 10) - 50;
    auto w = degreesC > TemperatureThreshold ? HIGH : LOW;
    self.checked_digital_write(w);
}

void light(Actuator const& self, u16 voltage) {
    self.checked_analog_write(map(voltage, 1, 1021, 255, 0));
}

static u32 half_blink_interval = 100;

void potentiometer(Actuator const& self, u16 voltage) {
    (void) self;
    half_blink_interval = map(voltage, 0, 1023, 100, 1000);
}

/* blink green led task */

void blink_potentiometer(Actuator const& self) {
    static u32 last_blink_time = 0UL;
    static bool led_on = false;

    u32 const now = millis();
    if (last_blink_time + half_blink_interval < now) {
        self.checked_digital_write(led_on = !led_on);
        last_blink_time = now;
    }
}

#define NUM_SENSORS 3

constexpr Actuator SENSORS[NUM_SENSORS] = {
    Actuator(Temperature, temperature),
    Actuator(Potentiometer, potentiometer),
    Actuator(Light, light)};

auto read_serial(u16* msg) -> bool {
    byte buf[(NUM_SENSORS + 1) * 2];
    if (!Serial.available()) return false;

    size_t read_so_far = 0;
    while (read_so_far != sizeof buf)
        read_so_far +=
            Serial.readBytes(buf + read_so_far, sizeof(buf) - read_so_far);

    if (buf[sizeof(buf) - 1] != 4 || buf[sizeof(buf) - 2] != 0xff) return false;


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

void loop() {
    static u16 msg[NUM_SENSORS + 1];
    if (read_serial(msg)) {
        for (size_t i = 0; i < NUM_SENSORS; ++i) SENSORS[i].update(msg[i]);
    }
    blink_potentiometer(SENSORS[1]);
}
