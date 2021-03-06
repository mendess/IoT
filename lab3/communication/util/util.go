package util

import (
	"fmt"
	"io"
	"os"
	"unsafe"
)

const PacketSize = 4 * unsafe.Sizeof(uint16(0))

type packet = [PacketSize]byte

func MakePacket() packet {
	p := MakeEmptyPacket()
	p[len(p)-1] = 4
	p[len(p)-2] = 0xff
	return p
}

func MakeEmptyPacket() packet {
	var p packet
	return p
}

func IsPacketValid(p packet) bool {
	return p[len(p)-1] == 4 && p[len(p)-2] == 0xff
}

const (
	YELLOW_LED = 5
	RED_LED    = 9
	GREEN_LED  = 11
)

func ReadToEnd(r io.Reader, buf []byte) error {
	read_so_far := 0
	for read_so_far < len(buf) {
		n, err := r.Read(buf[read_so_far:])
		if err != nil {
			return err
		}
		read_so_far += n
	}
	return nil
}

func WriteToEnd(w io.Writer, buf []byte) error {
	written_so_far := 0
	for written_so_far < len(buf) {
		n, err := w.Write(buf[written_so_far:])
		if err != nil {
			return err
		}
		written_so_far += n
	}
	return nil
}

func DoOrDie(err error, f string, v ...interface{}) {
	if err != nil {
		fmt.Printf(f, v...)
		os.Exit(1)
	}
}
