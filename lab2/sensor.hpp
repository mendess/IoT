class Sensor {

  public:
    using Feedback = void (*)(Sensor const&, u16);
    int const sensor;
    int const led;

    constexpr explicit Sensor(int const sensor, int const led, Feedback const f)
        : sensor(sensor), led(led), feedback(f) {}

    void setup() const { pinMode(led, OUTPUT); }

    void update() const { (feedback)(*this, get_sensor_voltage()); }

  private:
    const Feedback feedback;
    u16 get_sensor_voltage() const { return analogRead(sensor); }
};
