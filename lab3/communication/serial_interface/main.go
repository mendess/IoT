package main

import (
	"communication/util"
	"encoding/binary"
	"flag"
	"fmt"
	"github.com/jacobsa/go-serial/serial"
	"io"
	"net"
	"os"
	"os/signal"
	"syscall"
)

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
	util.DoOrDie(util.WriteToEnd(conn, []byte(options.Mode)), "Error writting to socket: %v", err)

	var buffer [1]byte
	util.DoOrDie(util.ReadToEnd(conn, buffer[:]), "Error reading mode response: %v", err)
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
	go handle_signals(conn)
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

func handle_signals(conn io.Closer) {
	sig_chan := make(chan os.Signal, 1)
	signal.Notify(sig_chan, syscall.SIGHUP, syscall.SIGINT, syscall.SIGQUIT)
	<-sig_chan
	fmt.Println("Signal received, shutting down")
	conn.Close()
	os.Exit(0)
}

// Mock an arduino, by reading stdin and writting to stdout instead
type Terminal struct{}

func (Terminal) Write(p []byte) (n int, err error) {
	var s string
	if len(p) == 1 {
		s = fmt.Sprintf("Led failed %d\n", p[0])
	} else if len(p) == int(util.PacketSize) {
		s = fmt.Sprintf(">> %d:%d:%d\n",
			binary.LittleEndian.Uint16(p[0:2]),
			binary.LittleEndian.Uint16(p[2:4]),
			binary.LittleEndian.Uint16(p[4:6]),
		)
	} else {
		s = "Unknown write type: ["
		for _, b := range p {
			s += fmt.Sprintf("%d,", b)
		}
		s += "]"
	}
	return os.Stdout.Write([]byte(s))
}

func (Terminal) Read(p []byte) (n int, err error) {
	n, err = os.Stdin.Read(p)
	if len(p) == 1 {
		switch p[0] {
		case 'G':
			p[0] = util.GREEN_LED
		case 'Y':
			p[0] = util.YELLOW_LED
		case 'R':
			p[0] = util.RED_LED
		}
	}
	return n, err
}

func (Terminal) Close() error {
	return nil
}
