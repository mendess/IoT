#ifndef SAMPLES
#    define SAMPLES 64
#endif

constexpr auto integer_log_recurse(size_t x, size_t i) -> size_t {
    return x > 1 ? integer_log_recurse(x >> 1, i + 1) : i;
}

constexpr auto integer_log(size_t x) -> size_t {
    return integer_log_recurse(x, 0);
}

constexpr auto SHIFT = integer_log(SAMPLES);
constexpr auto N_SAMPLES = 1 << SHIFT;

class Sensor {
  private:
    u8 const pin;
    u16 readings[N_SAMPLES];
    size_t n_readings;

  public:
    constexpr explicit Sensor(u8 const pin)
        : pin{pin}, readings{}, n_readings{0} {}

    /**
     * Reads a value from the analog pin. Every 16 reads (or another value
     *  defined by SAMPLES) the passed handler is called with the averaged out
     *  value.
     *
     * @param value_handler A handler for the value
     */
    template<typename F>
    void read(F value_handler) {
        readings[n_readings++] = analogRead(pin);
        if (n_readings == N_SAMPLES) {
            auto sum = 0;
            for (auto const r : readings) {
                sum += r;
            }
            n_readings = 0;
            value_handler(sum >> SHIFT);
        }
    }
};
