package main

import (
	"bytes"
	"fmt"
	"io/ioutil"
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
		if filename == "C026.heic" || filename == "C029.heic" || filename == "C030.heic" {
			//continue
		}
		blob, err := extractH265Thumb(filename)
		if err != nil {
			return fmt.Errorf("%s failed @ extractH265Thumb, reason: %s", filename, err.Error())
		}
		err = transcodeH265Thumb(blob, filename)
		if err != nil {
			return fmt.Errorf("%s failed @ transcodeH265Thumb, reason: %s", filename, err.Error())
		}
		log.Printf("%s processed!", filename)
	}
	return nil
}

func extractH265Thumb(filename string) ([]byte, error) {
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
	tempFile, err := ioutil.TempFile("", "temp")
	if err != nil {
		return err
	}
	defer os.Remove(tempFile.Name())
	_, err = tempFile.Write(blob)
	if err != nil {
		return err
	}
	log.Println(tempFile.Name())
	var stdOut bytes.Buffer
	var stdErr bytes.Buffer
	//blobReader := bytes.NewReader(blob)
	var name string
	if filename == "C031.heic" {
		name = filename
	} else {
		name = tempFile.Name()
	}
	cmd := exec.Command("ffmpeg", "-i", name, "-loglevel", "fatal", "-frames:v", "1", "-vsync", "vfr", "-q:v", "1", "-an", fmt.Sprintf("%s.jpg", filename))

	//cmd.Stdin = blobReader
	cmd.Stdout = &stdOut
	cmd.Stderr = &stdErr
	err = cmd.Run()
	if err != nil {
		return err
	}

	return nil
}
