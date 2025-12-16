#
TARGET = spider

CROSS_COMPILE = arm-linux-gnueabihf-
CFLAGS = -g -Wall -std=gnu++11 -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include
LDFLAGS =  -g -Wall  -lstdc++  -lrt
CC = $(CROSS_COMPILE)g++

all: $(TARGET)

server: server.o
	$(CC) $(LDFLAGS) $^ -o $@

client: client.o
	$(CC) $(LDFLAGS) $^ -o $@

$(TARGET): Main.o Spider.o SpiderLeg.o ServoMotor.o MMap.o
	$(CC) $(LDFLAGS)  $^ -o $@ 

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~
