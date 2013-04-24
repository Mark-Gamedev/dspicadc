
TARGET = impac
SRCS   = main.c xcorr.c plotGraph.c peakDetect.c tcpServer.c calibration.c spi.c processData.c
OBJS   = $(SRCS:.c=.o)

CC = gcc
CCFLAGS = -Wall -Werror -g
LDFLAGS = -lm
LIBS    =

all: $(TARGET) 

.PHONY: all clean

$(TARGET): $(OBJS)
	$(CC) $(CCFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): %.o: %.c
	$(CC) $(CCFLAGS) -o $@ -c $< 

clean:
	-rm -f $(OBJS)
	-rm -f $(TARGET)



