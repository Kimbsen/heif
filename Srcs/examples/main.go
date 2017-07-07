package main

import (
	"bytes"
	"fmt"
	//"io/ioutil"
	"log"
	"os"
	"os/exec"
)

func main() {
	tiles, err := getTiles()
	if err != nil {
		log.Println(err.Error())
		return
	}
	err = transcodeH265Tiles(tiles)
	if err != nil {
		log.Println(err.Error())
		return
	}
}

func getTiles() ([]string, error) {
	wd, err := os.Getwd()
	if err != nil {
		return nil, err
	}
	dir, err := os.Open(fmt.Sprintf("%s/tiles/", wd))
	if err != nil {
		return nil, err
	}
	tiles, err := dir.Readdirnames(0)
	if err != nil {
		return nil, err
	}
	tilesWithPath := []string{}
	for _, tile := range tiles {
		tilesWithPath = append(tilesWithPath, fmt.Sprintf("%s/tiles/%s", wd, tile))
	}

	return tilesWithPath, nil
}

func transcodeH265Tiles(tiles []string) error {
	for _, tile := range tiles {
		err := transcodeH265Tile(tile)
		if err != nil {
			return err
		}
		log.Printf("%s processed!", tile)
	}
	return nil
}

func transcodeH265Tile(tile string) error {
	cmd := exec.Command("ffmpeg", "-i", tile, "-loglevel", "fatal", "-frames:v", "1", "-vsync", "vfr", "-q:v", "1", "-an", fmt.Sprintf("%s.jpg", tile))
	var stdOut bytes.Buffer
	var stdErr bytes.Buffer

	cmd.Stdout = &stdOut
	cmd.Stderr = &stdErr
	err := cmd.Run()
	if err != nil {
		return err
	}

	return nil
}
