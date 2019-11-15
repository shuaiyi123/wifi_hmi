#CC =gcc
CC =arm-linux-gnueabihf-gcc

LIBS    = -lpthread -lrt 
CFLAGS  = -Wall -g
OBJS    = hmi_paraDef.o wifi_drive.o hmi_trans.o hmi_recv.o test.o 
TARGET  = wifi-hmi.exe

all : $(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(LIBS)

%.o:%.c 
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean
clean:
	-rm -rf $(TARGET) *.o 