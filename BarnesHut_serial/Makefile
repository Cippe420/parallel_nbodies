all: $(TARGET)

$(TARGET): $(OBJECT)
	gcc -g -Wall -O2 -o $@ $^ -lm

%.o: %.c
	gcc -g -Wall -O2 -c -o $@ $< 

clean:
	rm -f $(TARGET) $(OBJECT) 
