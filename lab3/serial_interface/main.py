import time
import serial
import logging
import requests
import threading
from sys import argv

ROLE = 'sensor'  # sensor or actuator
BAUDRATE = 9600
DEVICE_PORT = "/dev/ttyACM0"
TIMEOUT = 10
POTENTIOMETER = 'potentiometer'
LIGHT = 'light'
TEMP = 'temp'
URL = "https://iot-lab3.herokuapp.com"


class Api:
    def __init__(self, url):
        self.url = url

    def getSensor(self, sensor):
        print('requesting...', end='')
        response = requests.get(self.url + f'/{sensor}')
        print(f'got {response.json()} from {sensor}')
        return response.json()['value']

    def setSensor(self, sensor, value):
        print(f'sending {value} to {sensor}')
        response = requests.post(self.url + f'/{sensor}',
                                 json={'value': value})
        return response.status_code


class UARTConsole:
    def __init__(self):
        self.ser = serial.Serial()  # open serial port
        self.ser.baudrate = BAUDRATE
        self.ser.port = DEVICE_PORT
        self.ser.timeout = TIMEOUT
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
        return self.ser.read_until()

    def write(self, name, value):
        print(f'writing: {name}:{value}')
        msg = name.encode() + value.to_bytes(2, byteorder='little')
        print(msg)
        self.ser.write(msg)


def infinite():
    while True:
        yield None


def main(role='sensor'):
    try:
        print('Opening serial port')
        uart = UARTConsole()
        print('Connecting to api')
        api = Api(URL)
        print('Initialized')
        print(f'Running as {role}')
        if role == 'sensor':
            sensor_values = {}
            sensor_values[POTENTIOMETER] = 0
            sensor_values[LIGHT] = 0
            sensor_values[TEMP] = 0

            def send_sensor_value(sensor):
                return lambda: [
                    api.setSensor(sensor, sensor_values[sensor])
                    for _ in infinite()
                ]

            threading.Thread(target=send_sensor_value(POTENTIOMETER)).start()
            threading.Thread(target=send_sensor_value(LIGHT)).start()
            threading.Thread(target=send_sensor_value(TEMP)).start()
            while True:
                analog_read = uart.read()
                if not analog_read: continue
                try:
                    name = chr(analog_read[0])
                    value = int(analog_read[1:].decode('utf-8').strip())
                    sensor = None
                    if name == 'P':
                        sensor = POTENTIOMETER
                    elif name == 'L':
                        sensor = LIGHT
                    elif name == 'T':
                        sensor = TEMP
                    else:
                        print(f'Invalid {analog_read[0]}:{name}')
                    if sensor:
                        sensor_values[sensor] = value
                except Exception as e:
                    print(e)
        elif role == 'actuator':
            potentiometer_current = None
            light_current = None
            temp_current = None
            while True:
                potentiometer_read = api.getSensor(POTENTIOMETER)
                if potentiometer_read != potentiometer_current:
                    potentiometer_current = potentiometer_read
                    uart.write('P', potentiometer_current)
                light_read = api.getSensor(LIGHT)
                if light_read != light_current:
                    light_current = light_read
                    uart.write('L', light_current)
                temp_read = api.getSensor(TEMP)
                if temp_read != temp_current:
                    temp_current = temp_read
                    uart.write('T', temp_current)
                time.sleep(0.1)
    except KeyboardInterrupt:
        exit(1)


if __name__ == "__main__":
    main(role=(argv[1] if len(argv) > 1 else ROLE))
