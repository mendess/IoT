package main

import (
	"bufio"
	"encoding/binary"
	"flag"
	"fmt"
	"github.com/jacobsa/go-serial/serial"
	"io"
	"net"
	"os"
	"strconv"
	// "strings"
	"unsafe"
)

const (
	SENSOR = iota
	ACTUATOR
)

const DUMMY uint16 = 42
const PACKET_SIZE = 3 * unsafe.Sizeof(DUMMY)

type Options struct {
	Host string
	Port string
	Mode string
	Mock bool
}

func parseArgs() Options {
	var opt Options
	flag.StringVar(&opt.Host, "h", "iot-lab3.herokuapp.com", "the host to connect to")
	flag.StringVar(&opt.Port, "p", "80", "the port to connect to")
	flag.StringVar(&opt.Mode, "m", "sensor", "Either 'sensor' or 'actuator'")
	flag.BoolVar(&opt.Mock, "i", false, "read from terminal instead of arduino")
	flag.Parse()
	return opt
}

func main() {
	options := parseArgs()
	fmt.Println("Connecting")
	conn, err := net.Dial("tcp", fmt.Sprintf("%s:%s", options.Host, options.Port))
	if err != nil {
		fmt.Print("Error connecting to socket:", err.Error())
		os.Exit(1)
	}
	fmt.Println("Writing mode:", options.Mode)
	_, err = conn.Write([]byte(options.Mode))
	if err != nil {
		fmt.Print("Error writting to socket:", err.Error())
		os.Exit(1)
	}
	var buffer [1]byte
	_, err = conn.Read(buffer[:])
	if err != nil {
		fmt.Println("Error reading mode response:", err.Error())
		os.Exit(1)
	}
	if buffer[0] != 1 {
		fmt.Println("Invalid handshake")
		os.Exit(1)
	}
	fmt.Println("Connected to relay")
	var port io.ReadWriteCloser
	if options.Mock {
		port = Terminal{}
	} else {
		port = openSerial()
	}
	switch options.Mode {
	case "sensor":
		handleSensor(conn, port)
	case "actuator":
		handleActuator(conn, port)
	default:
		fmt.Println("Invalid type:", options.Mode)
		os.Exit(1)
	}
}

func openSerial() io.ReadWriteCloser {
	options := serial.OpenOptions{
		PortName:        "/dev/ttyACM0",
		BaudRate:        9600,
		DataBits:        8,
		StopBits:        1,
		MinimumReadSize: 1,
	}

	fmt.Println("Connecting to serial port")
	port, err := serial.Open(options)
	fmt.Println("Connected")
	if err != nil {
		fmt.Println("Failed to open serial port:", err.Error())
		os.Exit(1)
	}
	return port
}

func handleSensor(conn net.Conn, port io.ReadWriteCloser) {
	defer port.Close()
	bufReader := bufio.NewReaderSize(port, 10)
	var buffer [PACKET_SIZE]byte
	_, err := conn.Read(buffer[:])
	if err != nil {
		fmt.Println("Error getting current state", err.Error())
		return
	}
	fmt.Println("Read the initial state", buffer)
	for {
		line, err := bufReader.ReadString('\n')
		if err != nil {
			if err != io.EOF {
				fmt.Println("Error reading from port", err.Error())
			}
			break
		}
		switch line[0] {
		case 'T':
			parseToSlice(line[1:(len(line)-1)], buffer[0:2])
		case 'P':
			parseToSlice(line[1:(len(line)-1)], buffer[2:4])
		case 'L':
			parseToSlice(line[1:(len(line)-1)], buffer[4:6])
		}
		_, err = conn.Write(buffer[:])
		if err != nil {
			fmt.Println("Error reading from server", err.Error())
			break
		}
	}
}

func parseToSlice(s string, slice []byte) {
	v, err := strconv.Atoi(s)
	if err != nil {
		fmt.Printf("Error converting value from string: '%s'", s)
	} else {
		binary.LittleEndian.PutUint16(slice, uint16(v))
	}
}

func handleActuator(conn net.Conn, port io.ReadWriteCloser) {
	defer port.Close()
	var buffer [PACKET_SIZE]byte
	var old [PACKET_SIZE]byte
	for {
		_, err := conn.Read(buffer[:])
		if err != nil {
			fmt.Println("Error reading from server", err.Error())
			break
		}
		_, err = conn.Write([]byte{1})
		if err != nil {
			fmt.Println("Failed to write ack to server", err.Error())
			break
		}
		if old != buffer {
			fmt.Printf("Writing %v\n", buffer)
			_, err = port.Write(buffer[:])
			if err != nil {
				fmt.Printf("Failed to write %v Reason: %v\n", buffer, err)
				return
			}
			copy(old[:], buffer[:])
		}
	}
}

type Terminal struct{}

func (Terminal) Write(p []byte) (n int, err error) {
	s := fmt.Sprintf(">> %d:%d:%d\n",
		binary.LittleEndian.Uint16(p[0:2]),
		binary.LittleEndian.Uint16(p[2:4]),
		binary.LittleEndian.Uint16(p[4:6]),
	)
	return os.Stdout.Write([]byte(s))
}

func (Terminal) Read(p []byte) (n int, err error) {
	return os.Stdin.Read(p)
}

func (Terminal) Close() error {
	return nil
}
