#include "config.hpp"

#ifndef N_SAMPLES
#    define N_SAMPLES 16
#endif

class Sensor {
  private:
    size_t read_index;
    u16 total;
    u16 readings[N_SAMPLES];
    Sensors const pin;
    Led const led;

  public:
    char const sensor_name;

    constexpr explicit Sensor(Config const conf, char const sensor_name)
        : read_index{0},
          total{0},
          readings{},
          pin{conf.analog},
          led{conf.led},
          sensor_name{sensor_name} {}

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

    void setup() {
        pinMode(led, OUTPUT);
    }
};
