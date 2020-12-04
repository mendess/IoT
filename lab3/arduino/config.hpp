enum Led : u8 {
    Yellow = 5,
    Red = 9,
    Green = 11,
};

enum Sensors: u8 {
    STemperature = A2,
    SLight = A1,
    SPotentiometer = A0,
};


struct Config {
    Led led;
    Sensors analog;
};

constexpr Config Potentiometer = {Green, SPotentiometer};
constexpr Config Light = {Red, SLight};
constexpr Config Temperature = {Yellow, STemperature};
