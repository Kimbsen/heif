FROM ubuntu:16.10

RUN DEBIAN_FRONTEND=noninteractive apt-get update

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y git \
	cmake \
	clang 

WORKDIR /root

RUN git clone https://github.com/nokiatech/heif.git

RUN cd heif && mkdir build && cd build && cmake .. && make

RUN cp /root/heif/build/Srcs/*/*.a /root/heif/Srcs/examples/
RUN cp /root/heif/build/Srcs/*/*/*.a /root/heif/Srcs/examples/

WORKDIR /root/heif/Srcs/examples

RUN c++ -lstdc++ -I/root/heif/build -I/root/heif/Srcs/common -I/root/heif/Srcs/parser -I/root/heif/Srcs/parser/avcparser -I/root/heif/Srcs/parser/h265parser -I/root/heif/Srcs/reader/ -I/root/heif/Srcs/writer -std=c++11 -L/root/heif/Srcs/examples -lcommon -lheifreader -lheifwriter -ljson -lavcparser -lh265parser example.cpp