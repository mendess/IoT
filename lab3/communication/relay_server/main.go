package main

import (
	"communication/util"
	"encoding/binary"
	"fmt"
	"net"
	"os"
	"sync"
	"time"
	"unsafe"
)

const (
	DEFAULT_PORT = "80"
	PACKET_SIZE  = 4 * unsafe.Sizeof(uint16(42))
)

type Values struct {
	Bytes [PACKET_SIZE]byte
}

var VALUES1 Values
var VALUES2 Values
var READ_VALUES *Values = &VALUES1
var WRITE_VALUES *Values = &VALUES2
var READ_MUTEX sync.Mutex
var HAS_SENSOR bool
var HAS_ACTUATOR bool

func main() {
	go printState()
	var port string
	if len(os.Args) > 1 {
		port = os.Args[1]
	} else if p := os.Getenv("PORT"); p != "" {
		port = p
	} else {
		port = DEFAULT_PORT
	}
	l, err := net.Listen("tcp", "0.0.0.0:"+port)
	if err != nil {
		fmt.Println("Error listening: ", err.Error())
		os.Exit(1)
	}
	fmt.Printf("Listening on 0.0.0.0:%s\n", port)
	defer l.Close()
	error_channel := make(chan byte, 512)
	for {
		conn, err := l.Accept()
		if err != nil {
			fmt.Println("Error accepting: ", err.Error())
			continue
		}
		fmt.Println("Client connected")
		var buf [1024]byte
		reqLen, err := conn.Read(buf[:])
		if err != nil {
			fmt.Println("Error reading")
			continue
		}
		kind := string(buf[:reqLen])
		switch {
		case kind == "sensor" && !HAS_SENSOR:
			HAS_SENSOR = true
			fmt.Println("Sensor connected")
			go handleSensor(conn, error_channel)
		case kind == "actuator" && !HAS_ACTUATOR:
			HAS_ACTUATOR = true
			fmt.Println("Actuator connected")
			go handleActuator(conn, error_channel)
		default:
			fmt.Println("Invalid type or already has one connected", kind)
			fmt.Println("Has sensor: ", HAS_SENSOR)
			fmt.Println("Has actuator: ", HAS_ACTUATOR)
			if err := util.WriteToEnd(conn, []byte{0}); err != nil {
				fmt.Println("Couldn't send rejection byte")
			}
			conn.Close()
		}
	}
}

func handleSensor(conn net.Conn, error_channel chan byte) {
	defer conn.Close()
	defer func() { HAS_SENSOR = false }()
	if err := util.WriteToEnd(conn, []byte{1}); err != nil {
		fmt.Println("Couldn't send confirmation byte")
		return
	}
	bytes := READ_VALUES
	if err := util.WriteToEnd(conn, bytes.Bytes[:]); err != nil {
		fmt.Println("Failed to write initial state, continuing..")
	}
	quit := false
	go receive_errors(conn, error_channel, &quit)
	for {
		if err := util.ReadToEnd(conn, WRITE_VALUES.Bytes[:]); err != nil {
			fmt.Println("Sensor disconnecting: ", err.Error())
			quit = true
			HAS_SENSOR = false
			return
		}
		swap_pointers()
	}
}

func receive_errors(conn net.Conn, error_channel chan byte, quit *bool) {
	for {
		select {
		case err := <-error_channel:
			fmt.Println("Receiving error", err)
			util.WriteToEnd(conn, []byte{err})
		default:
			if *quit {
				fmt.Println("receive_errors stopping")
				return
			}
		}
	}
}

func handleActuator(conn net.Conn, error_channel chan byte) {
	defer conn.Close()
	defer func() { HAS_ACTUATOR = false }()
	if err := util.WriteToEnd(conn, []byte{1}); err != nil {
		fmt.Println("Couldn't send confirmation byte")
		return
	}
	go send_errors(conn, error_channel)
	for {
		READ_MUTEX.Lock()
		var bytes = *READ_VALUES
		READ_MUTEX.Unlock()
		if err := util.WriteToEnd(conn, bytes.Bytes[:]); err != nil {
			fmt.Println("Actuator disconnecting: ", err.Error())
			HAS_ACTUATOR = false
			return
		}
	}
}

func send_errors(conn net.Conn, error_channel chan byte) {
	var buf [1]byte
	for {
		if n, err := conn.Read(buf[:]); err != nil {
			return
		} else if n == len(buf) {
			fmt.Println("Sending error", buf[0])
			select {
			case error_channel <- buf[0]:
			default:
			}
		}
	}
}

func swap_pointers() {
	tmp := READ_VALUES
	READ_MUTEX.Lock()
	READ_VALUES = WRITE_VALUES
	READ_MUTEX.Unlock()
	WRITE_VALUES = tmp
}

func printState() {
	for {
		time.Sleep(5 * time.Second)
		bytes := &READ_VALUES.Bytes
		fmt.Printf(
			"bytes: %v | T%d | P%d | L%d | C%d\n",
			bytes,
			binary.LittleEndian.Uint16(bytes[0:2]),
			binary.LittleEndian.Uint16(bytes[2:4]),
			binary.LittleEndian.Uint16(bytes[4:6]),
			binary.LittleEndian.Uint16(bytes[6:8]),
		)
	}
}
