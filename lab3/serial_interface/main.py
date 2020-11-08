import time

import serial
import logging


class UARTConsole:

    def __init__(self):
        self.ser = serial.Serial()  # open serial port
        self.ser.baudrate = 9600
        self.ser.port = "/dev/ttyACM1"
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


def main():
    rssi_results = []
    try:
        uart = UARTConsole()
        while True:
            print(uart.read())
            time.sleep(1.0)
    except KeyboardInterrupt:
        print(rssi_results)
        exit(1)



if __name__ == "__main__":
    main()
