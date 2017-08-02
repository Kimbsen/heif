#!/bin/sh

if [ "$1" == "" ]; then
	echo "Use as follows"
	echo "./build_pipe.sh /path/to/heic-image"
	exit 1
fi

if [[ -d build ]]; then
	rm -rf build
fi
mkdir build
cp *.a build/.
cp extractor build/extractor
cp main.go build/process_tiles.go

cp $1 build/.
cd build
echo "extracting tiles"
cat $1 | ./extractor 

mkdir tiles
mv *.tile tiles/.

echo "processing tiles"
go run process_tiles.go
exit

montage 1.tile.jpg \
2.tile.jpg \
3.tile.jpg \
4.tile.jpg \
5.tile.jpg \
6.tile.jpg \
7.tile.jpg \
8.tile.jpg \
9.tile.jpg \
10.tile.jpg \
11.tile.jpg \
12.tile.jpg \
13.tile.jpg \
14.tile.jpg \
15.tile.jpg \
16.tile.jpg \
17.tile.jpg \
18.tile.jpg \
19.tile.jpg \
20.tile.jpg \
21.tile.jpg \
22.tile.jpg \
23.tile.jpg \
24.tile.jpg \
25.tile.jpg \
26.tile.jpg \
27.tile.jpg \
28.tile.jpg \
29.tile.jpg \
30.tile.jpg \
31.tile.jpg \
32.tile.jpg \
33.tile.jpg \
34.tile.jpg \
35.tile.jpg \
36.tile.jpg \
 -geometry 512x512 -border 0 -tile 8x6 final.jpg