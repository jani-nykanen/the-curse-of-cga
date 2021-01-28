package main

import (
	"encoding/binary"
	"encoding/csv"
	"encoding/xml"
	"fmt"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
)

type layer []int32

type tilemap struct {
	layers []layer
	width  int32
	height int32
}

type tmx struct {
	XMLName xml.Name   `xml:"map"`
	Width   int32      `xml:"width,attr"`
	Height  int32      `xml:"height,attr"`
	Layers  []layerXML `xml:"layer"`
}
type layerXML struct {
	XMLName xml.Name `xml:"layer"`
	Data    string   `xml:"data"`
}

func parseCSV(data string) []int32 {

	reader := csv.NewReader(strings.NewReader(data))

	out := make([]int32, 0)

	var line []string
	var err error
	var v int
	for {

		// Go might not like CSV format tiled has,
		// so let's ignore all the errors for now...
		line, _ = reader.Read()
		if line == nil || len(line) <= 0 {

			break
		}

		for _, s := range line {

			v, err = strconv.Atoi(s)
			if err == nil {

				out = append(out, int32(v))
			}

		}
	}

	return out
}

func parseTMX(fpath string) (*tilemap, error) {

	/*
	 * TODO: Layer IDs are ignored, which results
	 * that the layers might appear in wrong order.
	 * TODO2: I copied this from over 7 months old project,
	 * and the TODO above still exists. Laziness or what.
	 */

	var err error
	t := new(tilemap)
	t.layers = make([]layer, 0)

	file, err := os.Open(fpath)
	if err != nil {

		return nil, err
	}
	defer file.Close()

	byteValue, err := ioutil.ReadAll(file)
	if err != nil {

		return nil, err
	}

	var mapXML tmx
	xml.Unmarshal(byteValue, &mapXML)

	for _, l := range mapXML.Layers {

		t.layers = append(t.layers, parseCSV(l.Data))

	}
	t.width = mapXML.Width
	t.height = mapXML.Height

	return t, err
}

func layerToByte(t *tilemap, index int32) []byte {

	out := make([]byte, t.width*t.height)

	for i := int32(0); i < t.width*t.height; i++ {

		out[i] = byte(t.layers[index][i])
	}

	return out
}

func writeTilemapToFile(t *tilemap, path string) error {

	f, err := os.Create(path)
	if err != nil {

		return err
	}

	// Dimensions & layer count
	buffer := make([]byte, 2)
	binary.LittleEndian.PutUint16(buffer, uint16(t.width))
	f.Write(buffer)

	binary.LittleEndian.PutUint16(buffer, uint16(t.height))
	f.Write(buffer)

	binary.LittleEndian.PutUint16(buffer, uint16(len(t.layers)))
	f.Write(buffer)

	// Actual layer data
	for i := 0; i < len(t.layers); i++ {

		f.Write(layerToByte(t, int32(i)))
	}

	return nil
}

func main() {

	in := os.Args[1]
	out := os.Args[2]

	t, err := parseTMX(in)
	if err != nil {

		fmt.Println(err)
	}

	err = writeTilemapToFile(t, out)
	if err != nil {

		fmt.Println(err)
	}
}
