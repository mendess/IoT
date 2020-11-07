#ifndef SAMPLES
#    define SAMPLES 16
#endif

constexpr auto integer_log_recurse(size_t const x, size_t const i) -> size_t {
    return x > 1 ? integer_log_recurse(x >> 1, i + 1) : i;
}

constexpr auto integer_log(size_t const x) -> size_t {
    return integer_log_recurse(x, 0);
}

constexpr auto SHIFT = integer_log(SAMPLES);
constexpr auto N_SAMPLES = 1 << SHIFT;

class Sensor {
  private:
    size_t read_index;
    u16 total;
    u16 readings[N_SAMPLES];
    u8 const pin;

  public:
    constexpr explicit Sensor(u8 const pin)
        : read_index{0}, total{0}, readings{}, pin{pin} {}

    /**
     * Reads a value from the analog pin. Every 16 reads (or another value
     *  defined by SAMPLES) the passed handler is called with the averaged out
     *  value.
     *
     * @param value_handler A handler for the value
     */
    auto read() -> u16 {
        // subtract the last reading:
        total -= readings[read_index];
        // read from the sensor:
        readings[read_index] = analogRead(pin);
        // add the reading to the total:
        total += readings[read_index];
        // advance to the next position in the array:
        read_index = (read_index + 1) & (N_SAMPLES - 1);

        // calculate the average:
        return total / N_SAMPLES;
    }
};
