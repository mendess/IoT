package main

import (
	"bufio"
	"communication/util"
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"strconv"
)

func handleSensor(conn net.Conn, port io.ReadWriteCloser) {
	defer port.Close()
	bufReader := bufio.NewReaderSize(port, 10)
	var buffer [util.PACKET_SIZE]byte
	err := util.ReadToEnd(conn, buffer[:])
	if err != nil {
		fmt.Println("Error getting current state", err.Error())
		return
	}
	fmt.Println("Read the initial state", buffer)
	go report_errors(conn, port)
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
		if err := util.WriteToEnd(conn, buffer[:]); err != nil {
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

func report_errors(conn net.Conn, port io.ReadWriteCloser) {
	for {
		var buf [1]byte
		n, err := conn.Read(buf[:])
		switch err {
		case io.EOF:
			fmt.Println("Terminating error detection")
			return
		case nil:
		default:
			fmt.Printf("Error reading '%v' terminating\n", err)
			return
		}
		switch buf[0] {
		case util.YELLOW_LED:
			fmt.Printf("Yellow led failed\n")
		case util.RED_LED:
			fmt.Printf("Red led failed\n")
		case util.GREEN_LED:
			fmt.Printf("Green led failed\n")
		default:
			if n > 0 {
				fmt.Printf("Invalid byte received %v\n", buf[0])
			}
			continue
		}
		util.WriteToEnd(port, buf[:])
	}
}
