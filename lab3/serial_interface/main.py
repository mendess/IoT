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
        response = requests.get(self.url + f'/{sensor}')
        return response.json()['value']

    def setSensor(self, sensor, value):
        logging.info(f'sending {value} to {sensor}')
        response = requests.post(self.url + f'/{sensor}',
                                 json={'value': value})
        return response.status_code


class UARTConsole:
    def __init__(self):
        logging.info('Opening serial port')
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

    def write(self, current):
        msg = bytearray()
        for k in [TEMP, POTENTIOMETER, LIGHT]:
            msg += current[k].to_bytes(2, byteorder='little')
        logging.info(' '.join(map(str, msg)))
        self.ser.write(msg)

class TTYConsole:
    def __del__(self):
        pass

    def read(self):
        return input().encode('utf-8')

    def write(self, current):
        print(current)


def infinite():
    while True:
        yield None


def main(role, uart):
    try:
        logging.info('Connecting to api')
        api = Api(URL)
        logging.info('Initialized')
        logging.info(f'Running as {role}')
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

            threading.Thread(target=send_sensor_value(POTENTIOMETER), daemon=True).start()
            threading.Thread(target=send_sensor_value(LIGHT), daemon=True).start()
            threading.Thread(target=send_sensor_value(TEMP), daemon=True).start()
            while True:
                try:
                    analog_read = uart.read()
                    if not analog_read: continue
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
                        logging.error(f'Invalid {analog_read[0]}:{name}')
                    if sensor:
                        sensor_values[sensor] = value
                except EOFError:
                    break
                except Exception as e:
                    logging.error(e)
        elif role == 'actuator':
            current = {POTENTIOMETER: 0, LIGHT: 0, TEMP: 0}

            while True:

                def get_and_update(kind):
                    read = api.getSensor(kind)
                    if read != current[kind]:
                        current[kind] = read
                        return True
                    return False

                changed = (get_and_update(POTENTIOMETER)
                           or get_and_update(LIGHT) or get_and_update(TEMP))

                if changed:
                    uart.write(current)

    except KeyboardInterrupt:
        exit(1)


if __name__ == "__main__":
    role = argv[1] if len(argv) > 1 else ROLE
    console = TTYConsole() if len(argv) > 2 and argv[2] == '-i' else UARTConsole()
    main(role, console)
    if role == 'sensor': time.sleep(1) # wait for threads to send a few more values
