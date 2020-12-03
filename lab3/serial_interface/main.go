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
	"os/signal"
	"strconv"
	"syscall"
	"time"
	"unsafe"
)

const (
	SENSOR = iota
	ACTUATOR
)

const DUMMY uint16 = 42
const PACKET_SIZE = 4 * unsafe.Sizeof(DUMMY)

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
	sig_chan := make(chan os.Signal, 1)
	signal.Notify(sig_chan, syscall.SIGHUP, syscall.SIGINT, syscall.SIGQUIT)
	go func() {
		<-sig_chan
		fmt.Println("Signal received, shutting down")
		conn.Close()
		os.Exit(0)
	}()
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
		buffer[len(buffer)-1] = 4
		if err := write_until(conn, buffer[:]); err != nil {
			fmt.Println("Error reading from server", err.Error())
			break
		}
	}
}

func parseToSlice(s string, slice []byte) {
	v, err := strconv.Atoi(s)
	if err != nil {
		fmt.Printf("Error converting value from string: '%s'\n", s)
	} else {
		binary.LittleEndian.PutUint16(slice, uint16(v))
	}
}

func write_until(w io.Writer, buf []byte) error {
	written_so_far := 0
	for written_so_far < len(buf) {
		n, err := w.Write(buf[written_so_far:])
		if err == nil {
			return err
		}
		written_so_far += n
	}
	return nil
}

func handleActuator(conn net.Conn, port io.ReadWriteCloser) {
	defer port.Close()
	var buffer [PACKET_SIZE]byte
	var old [PACKET_SIZE]byte
	first_write := true
	go detect_errors(conn, port)
	for {
		if err := read_until(conn, buffer[:]); err != nil {
			fmt.Println("Error reading from server", err.Error())
			break
		}
		if buffer[len(buffer)-1] != 4 {
			// fmt.Println("Invalid packet")
			continue
		}
		if old != buffer {
			fmt.Printf("Writing %v\n", buffer)
			_, err := port.Write(buffer[:])
			if err != nil {
				fmt.Printf("Failed to write %v Reason: %v\n", buffer, err)
				return
			}
			if first_write {
				time.Sleep(2 * time.Second)
				_, err = port.Write(buffer[:])
				if err != nil {
					fmt.Printf(
						"Failed to make backup write %v Reason: %v\n",
						buffer,
						err,
					)
					return
				}
				first_write = false
			}
			copy(old[:], buffer[:])
		}
	}
}

func detect_errors(conn net.Conn, port io.ReadWriteCloser) {
	var buf [1]byte
	for {
		n, err := port.Read(buf[:])
		switch err {
		case io.EOF:
			fmt.Println("Terminating error detection")
			return
		case nil:
		default:
			fmt.Printf("Error reading '%v'\n", err)
			continue
		}
		switch buf[0] {
		case 5:
			fmt.Println("Yellow led failed")
		case 9:
			fmt.Println("Red led failed")
		case 11:
			fmt.Println("Green led failed")
		default:
			if n > 0 {
				fmt.Printf("Invalid byte received %v\n", buf[0])
			}
		}
	}
}

func read_until(r io.Reader, buf []byte) error {
	read_so_far := 0
	for read_so_far < len(buf) {
		n, err := r.Read(buf[read_so_far:])
		if err == nil {
			return err
		}
		read_so_far += n
	}
	return nil
}

// Mock an arduino, by reading stdin instead
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
