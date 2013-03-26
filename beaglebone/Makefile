
TARGET = spi
SRCS   = main.c corr.c
OBJS   = $(SRCS:.c=.o)

CC = gcc
CCFLAGS = -Wall -Werror
LDFLAGS = -lpthread
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



