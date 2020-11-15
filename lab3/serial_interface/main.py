import time
import serial
import logging
import requests

ROLE = 'sensor' # sensor or actuator
POTENTIOMETER = 'potentiometer'
LIGHT = 'light'
TEMP = 'temp'
URL = "https://iot-lab3.herokuapp.com"


class Api:

    def __init__(self, url):
        self.url = url

    def getSensor(self, sensor):
        response = requests.get(self.url + f'/{sensor}')
        return response.json()['value']

    def setSensor(self, sensor, value):
        response = requests.post(self.url + f'/{sensor}', json={'value': value})
        return response.status_code


class UARTConsole:

    def __init__(self):
        self.ser = serial.Serial()  # open serial port
        self.ser.baudrate = 9600
        self.ser.port = "/dev/ttyACM0"
        self.ser.timeout = 10
        self.ser.setDTR(1)
        try:
            self.ser.open()
        except Exception as e:
            logging.error("Exception occurred", exc_info=True)
        logging.info(self.ser)  # check which port was really used

    def __del__(self):
        self.ser.close()

    def read(self):
        if not self.ser.is_open:
            logging.warning("Serial is closed. Is the device connected?")
            raise serial.SerialException
        bytes_to_read = self.ser.inWaiting()
        text_read = self.ser.read(bytes_to_read).decode(encoding='utf-8')
        return text_read

    def write(self, value):
        self.ser.write(value.encode())


def main():
    try:
        uart = UARTConsole()
        api = Api(URL)
        if ROLE == 'sensor':
            while True:
                analog_read = uart.read()
                if(analog_read != ""):
                    print(analog_read)
                value_read = analog_read.split(":")
                try:
                    if value_read[0] == "P":
                       api.setSensor(POTENTIOMETER, value_read[1])
                    elif value_read[0] == "L":
                        api.setSensor(LIGHT, value_read[1])
                    elif value_read[0] == "T":
                        print(api.setSensor(TEMP, value_read[1]))
                except Exception as e:
                    print(e)
                time.sleep(0.1)
        elif ROLE == 'actuator':
            while True:
                potentiometer = api.getSensor(POTENTIOMETER)
                light = api.getSensor(LIGHT)
                temp = api.getSensor(TEMP)
                uart.write(f"P:{potentiometer}")
                uart.write(f"L:{light}")
                uart.write(f"T:{temp}")
                time.sleep(0.1)
    except KeyboardInterrupt:
        exit(1)



if __name__ == "__main__":
    main()
