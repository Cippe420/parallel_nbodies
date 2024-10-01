TARGET:= main
SRCS := $(wildcard *.c)
OBJECT:= $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECT)
	mpicc -g -Wall -O2 -flto -o $@ $^ -lm

%.o: %.c
	mpicc -g -Wall -O2 -flto -c -o $@ $< 

clean:
	rm -f $(TARGET) $(OBJECT) 

