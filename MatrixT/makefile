CC = gcc
FLAGS = -D__USE_REENTRANT -lpthread
TARGET = MatrixT

$(TARGET): $(TARGET).c
	$(CC) $(FLAGS) -o $@ $<

$(TARGET).o: $(TARGET).c
	$(CC) -o $@ -c $< 

clean:
	-rm *.o $(TARGET)