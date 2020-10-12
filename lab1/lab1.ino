enum { BUTTON_PIN = 3, DELAY = 1000 };

enum STATE {
    OFF = 0,
    YELLOW = 4,
    BLUE = 5,
    RED = 6,
    GREEN = 7,
};

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

void loop() {
    constexpr STATE states[] = {RED, GREEN, BLUE, YELLOW, OFF};
    constexpr auto n_states = sizeof states / sizeof *states;
    static size_t state = 0;
    if (PAUSE) {
        return;
    } else if (states[state] == OFF) {
        for (auto led : states) digitalWrite(led, LOW);
    } else {
        if (state > 0) digitalWrite(states[state - 1], LOW);
        digitalWrite(states[state], HIGH);
    }
    state = (state + 1) % n_states;
    delay(DELAY);
}
