package main

import (
	"communication/util"
	"fmt"
	"io"
	"net"
	"time"
)

func handleActuator(conn net.Conn, port io.ReadWriteCloser) {
	defer port.Close()
	buffer := util.MakeEmptyPacket()
	old := util.MakeEmptyPacket()
	first_write := true
	go detect_errors(conn, port)
	for {
		if err := util.ReadToEnd(conn, buffer[:]); err != nil {
			fmt.Println("Error reading from server", err.Error())
			break
		}
		if !util.IsPacketValid(buffer) {
			continue
		}
		if old != buffer {
			fmt.Printf("Writing %v\n", buffer)
			err := util.WriteToEnd(port, buffer[:])
			if err != nil {
				fmt.Printf("Failed to write %v Reason: %v\n", buffer, err)
				return
			}
			if first_write {
				time.Sleep(2 * time.Second)
				err = util.WriteToEnd(port, buffer[:])
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
	for {
		var buf [1]byte
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
		if n > 0 && contains([]byte{util.YELLOW_LED, util.RED_LED, util.GREEN_LED}, buf[0]) {
			util.WriteToEnd(conn, buf[:])
		}
	}
}

func contains(s []byte, e byte) bool {
	for _, a := range s {
		if a == e {
			return true
		}
	}
	return false
}
