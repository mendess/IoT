package main

import (
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
	PACKET_SIZE  = 3 * unsafe.Sizeof(uint16(42))
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
	} else {
		port = DEFAULT_PORT
	}
	l, err := net.Listen("tcp", "0.0.0.0:"+port)
	if err != nil {
		fmt.Println("Error listening: ", err.Error())
		os.Exit(1)
	}
	defer l.Close()
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
			go handleSensor(conn)
		case kind == "actuator" && !HAS_ACTUATOR:
			HAS_ACTUATOR = true
			fmt.Println("Actuator connected")
			go handleActuator(conn)
		default:
			fmt.Println("Invalid type", kind)
			conn.Write([]byte{0})
			conn.Close()
		}
	}
}

func handleSensor(conn net.Conn) {
	defer conn.Close()
	defer func() { HAS_SENSOR = false }()
	conn.Write([]byte{1})
	bytes := READ_VALUES
	conn.Write(bytes.Bytes[:])
	for {
		_, err := conn.Read(WRITE_VALUES.Bytes[:])
		if err != nil {
			fmt.Println("Sensor disconnecting: ", err.Error())
			break
		}
		swap_pointers()
	}
}

func handleActuator(conn net.Conn) {
	defer conn.Close()
	defer func() { HAS_ACTUATOR = false }()
	conn.Write([]byte{1})
	for {
		READ_MUTEX.Lock()
		var bytes = *READ_VALUES
		READ_MUTEX.Unlock()
		_, err := conn.Write(bytes.Bytes[:])
		if err != nil {
			fmt.Println("Actuator disconnecting: ", err.Error())
			break
		}
		s, err := conn.Read([]byte{1})
		if err != nil || s < 0 {
			fmt.Println("Failed to get ack")
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
			"bytes: %v | T%d | P%d | L%d\n",
			bytes,
			binary.LittleEndian.Uint16(bytes[0:2]),
			binary.LittleEndian.Uint16(bytes[2:4]),
			binary.LittleEndian.Uint16(bytes[4:6]),
		)
	}
}
