package main

import (
	"bytes"
	"fmt"
	"log"
	"os"
	"os/exec"
)

const (
	HEIC_DIR = "/Users/erik/Documents/heif_conformance/conformance_files"
)

func main() {
	filenames, err := getFilenames()
	if err != nil {
		log.Println(err.Error())
		return
	}
	err = extractH265Thumbs(filenames)
	if err != nil {
		log.Println(err.Error())
		return
	}
}

func getFilenames() ([]string, error) {
	dir, err := os.Open(HEIC_DIR)
	if err != nil {
		return nil, err
	}
	filenames, err := dir.Readdirnames(0)
	if err != nil {
		return nil, err
	}
	return filenames, nil
}

func extractH265Thumbs(filenames []string) error {
	for _, filename := range filenames {
		if filename == "C034.heic" || filename == "C026.heic" || filename == "C029.heic" {
			continue
		}
		blob, err := extractH265Thumb(filename)
		if err != nil {
			return err
		}
		err = transcodeH265Thumb(blob, filename)
		if err != nil {
			return err
		}
		log.Println(filename)
	}
	return nil
}

func extractH265Thumb(filename string) ([]byte, error) {
	//log.Println(fmt.Sprintf("%s/%s", HEIC_DIR, filename))
	file, err := os.Open(fmt.Sprintf("%s/%s", HEIC_DIR, filename))
	if err != nil {
		return nil, err
	}
	var stdOut bytes.Buffer
	var stdErr bytes.Buffer
	cmd := exec.Command("./a.out")

	cmd.Stdin = file
	cmd.Stdout = &stdOut
	cmd.Stderr = &stdErr
	err = cmd.Run()
	if err != nil {
		return nil, err
	}

	return stdOut.Bytes(), nil
}

func transcodeH265Thumb(blob []byte, filename string) error {
	//log.Println(fmt.Sprintf("%s/%s", HEIC_DIR, filename))
	var stdOut bytes.Buffer
	var stdErr bytes.Buffer
	blobReader := bytes.NewReader(blob)
	cmd := exec.Command("ffmpeg", "-i", "pipe:0", "-loglevel", "fatal", "-frames:v", "1", "-vsync", "vfr", "-q:v", "1", "-an", fmt.Sprintf("%s.jpg", filename))

	cmd.Stdin = blobReader
	cmd.Stdout = &stdOut
	cmd.Stderr = &stdErr
	err := cmd.Run()
	if err != nil {
		return err
	}

	return nil
}
