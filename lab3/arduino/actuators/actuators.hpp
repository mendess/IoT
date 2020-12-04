#include "config.hpp"

class Actuator {
  public:
    using Feedback = void (*)(Actuator const&, u16);

    constexpr explicit Actuator(Config const conf, Feedback const f)
        : feedback{f}, led{conf.led}, check_led{conf.analog} {}

    /** Setup this actuator's led */
    void setup() const { pinMode(led, OUTPUT); }

    /** Run the update function passing it the new value */
    void update(u16 value) const { (feedback)(*this, value); }

    /**
     * Digital write to a led pin, reading it's output to test
     * if the led failed or not
    */
    void checked_digital_write(bool on) const {
        digitalWrite(led, on);
        if (on) test_and_report(100);
    }

    /**
     * Analog write to a led pin, reading it's output to test
     * if the led failed or not.
     */
    void checked_analog_write(u8 value) const {
        analogWrite(led, value);
        if (value > 0) test_and_report(1);
    }

  private:
    const Feedback feedback;
    Led const led;
    u8 const check_led;

    // Check if this actuator failed and report this to the interface
    void test_and_report(u16 threshold) const {
        u16 voltage = analogRead(check_led);
        if (voltage < threshold) {
            u8 l = led;
            Serial.write(&l, sizeof led);
        }
    }
};
