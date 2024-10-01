TARGET:= main
SRCS := $(wildcard *.c)
OBJECT:= $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECT)
	mpicc -g -Wall -flto -O2 -o $@ $^ -lm

%.o: %.c
	mpicc -g -Wall -flto -O2 -c -o $@ $< 

clean:
	rm -f $(TARGET) $(OBJECT) 

