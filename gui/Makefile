
TARGET = gui
SRCS   = gui.c
OBJS   = $(SRCS:.c=.o)

CC = gcc
CCFLAGS = -Wall -Werror
LDFLAGS = -lglut -lGLU -lGL -lXext -lX11 -lm -lpthread #-lXmu 
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



