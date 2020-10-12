enum { BUTTON_PIN = 3, DELAY = 1000 };

typedef enum {
    YELLOW = 4,
    BLUE = 5,
    RED = 6,
    GREEN = 7,
    OFF = 8,
} LED;

volatile bool PAUSE = false;

void toggle_pause() {
    PAUSE = !PAUSE;
}

void setup() {
    pinMode(YELLOW, OUTPUT);
    pinMode(BLUE, OUTPUT);
    pinMode(RED, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), toggle_pause, FALLING);
}

#define N_LEDS ((sizeof leds / sizeof(int)) - 1)
#define PREV(i) (((i) -1) % N_LEDS)

void loop() {
    LED const leds[] = {RED, GREEN, BLUE, YELLOW, OFF};
    static size_t state = 0;
    if (PAUSE) {
        return;
    } else if (leds[state] == OFF) {
        for (size_t i = 0; i < N_LEDS; ++i) digitalWrite(leds[i], LOW);
    } else {
        digitalWrite(leds[PREV(state)], LOW);
        digitalWrite(leds[state], HIGH);
    }
    state = (state + 1) % (N_LEDS + 1);
    delay(DELAY);
}
