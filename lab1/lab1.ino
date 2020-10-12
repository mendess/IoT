enum { BUTTON_PIN = 3 };

typedef enum {
    NONE = 0,
    YELLOW = 4,
    BLUE = 5,
    RED = 6,
    GREEN = 7,
} LED;

volatile bool SAVE_LED = LOW;

void save_led() {
    SAVE_LED = HIGH;
}

void setup() {
    pinMode(YELLOW, OUTPUT);
    pinMode(BLUE, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), save_led, RISING);
}

LED const leds[] = { RED, GREEN, BLUE, YELLOW };
#define N_LEDS ((sizeof leds / sizeof(int)))
#define PREV(i) (((i) - 1) % N_LEDS)

void loop() {
    SAVE_LED = LOW;
    for (size_t i = 0; i < N_LEDS; ++i) {
        digitalWrite(leds[PREV(i)], SAVE_LED);
        SAVE_LED = LOW;
        digitalWrite(leds[i], HIGH);
        delay(1000);
    }
    for(size_t i = 0; i < N_LEDS; ++i) {
        digitalWrite(leds[i], LOW);
    }
    delay(1000);
}
