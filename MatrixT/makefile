CC = gcc
FLAGS = -D__USE_REENTRANT -lpthread
TARGET = MatrixT

$(TARGET): $(TARGET).c
	$(CC) -o $@ $< $(FLAGS) 

$(TARGET).o: $(TARGET).c
	$(CC) -o $@ -c $< 

clean:
	-rm *.o $(TARGET)
