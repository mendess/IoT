enum Led {
    Yellow = 5,
    Red = 6,
    Green = 9,
};

class Actuator {
  public:
    using Feedback = void (*)(Led l, u16);

    constexpr explicit Actuator(Led const led, Feedback const f)
        : feedback{f}, led{led} {}

    void setup() const { pinMode(led, OUTPUT); }

    void update(u16 value) const { (feedback)(led, value); }

  private:
    const Feedback feedback;
    Led const led;
};
