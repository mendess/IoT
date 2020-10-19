template<typename T, typename... Ts>
void print(T&& t) {
    Serial.print(t);
}

template<typename T, typename... Ts>
void print(T&& t, Ts&&... ts) {
    Serial.print(t);
    print(ts...);
}

template<typename... Ts>
void println(Ts&&... ts) {
    print(ts...);
    Serial.println();
}
