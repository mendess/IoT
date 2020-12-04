#include "config.hpp"

class Actuator {
  public:
    using Feedback = void (*)(Actuator const&, u16);

    constexpr explicit Actuator(Config const conf, Feedback const f)
        : feedback{f}, led{conf.led}, check_led{conf.analog} {}

    void setup() const { pinMode(led, OUTPUT); }

    void update(u16 value) const { (feedback)(*this, value); }

    void checked_digital_write(bool on) const {
        digitalWrite(led, on);
        if (on) test_and_report(100);
    }

    void checked_analog_write(u8 value) const {
        analogWrite(led, value);
        /* if (value > 0) test_and_report(value); */
    }

  private:
    const Feedback feedback;
    Led const led;
    u8 const check_led;

    void test_and_report(u16 threshold) const {
        u16 voltage = analogRead(check_led);
        if (voltage < threshold) {
            u8 l = led;
            Serial.write(&l, sizeof led);
        }
    }
};