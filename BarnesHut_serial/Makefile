TARGET:= main
SRCS := $(wildcard *.c)
OBJECT:= $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECT)
	gcc -g -Wall -o $@ $^ -lm

%.o: %.c
	gcc -g -Wall -c -o $@ $< 

clean:
	rm -f $(TARGET) $(OBJECT) 