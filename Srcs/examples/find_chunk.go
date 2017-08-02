package main

import (
	_ "bytes"
	"crypto/md5"
	"fmt"
	"io/ioutil"
	"log"
	"os"
)

func genmd5(b []byte) string {
	h := md5.New()
	h.Write(b)
	return fmt.Sprintf("%x", h.Sum(nil))
}

func main() {

	smallbuf := get_buf(os.Args[1])
	tofind := genmd5(smallbuf)
	log.Printf("Looking for %s in %s", tofind, os.Args[2])

	bigbuf := get_buf(os.Args[2])
	fmt.Println()
	for i := 0; i < len(bigbuf); i++ {
		buf := make([]byte, len(smallbuf))
		for j := 0; j < len(smallbuf); j++ {

			if j+i >= len(bigbuf) {
				break
			}

			buf[j] = bigbuf[i+j]
		}
		tomatch := genmd5(buf)
		if tomatch == tofind {
			log.Fatalf("Found @ offset %d", i)
		}
		fmt.Printf("\r progress: %d/%d", i, len(bigbuf))
	}
	fmt.Println()
}

func get_buf(path string) []byte {
	f, err := os.Open(path)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()
	b, err := ioutil.ReadAll(f)
	if err != nil {
		log.Fatal(err)
	}

	return b
}
