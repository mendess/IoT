class Sensor {

  public:
    using Feedback = void (*)(Sensor const&, u16);
    int const sensor;
    int const led;

    constexpr explicit Sensor(int const sensor, int const led, Feedback const f)
        : sensor(sensor), led(led), feedback(f) {}

    void setup() const { pinMode(led, OUTPUT); }

    void update() const { (feedback)(*this, read_sensor()); }

  private:
    const Feedback feedback;
    u16 read_sensor() const { return analogRead(sensor); }
};
