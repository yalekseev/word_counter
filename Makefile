CC=g++
CFLAGS=--std=c++0x
LDFLAGS=-lboost_filesystem -lboost_system -lboost_thread

all: word_counter

main.o: main.cpp
	$(CC) $(CFLAGS) -c $^

word_counter.o: word_counter.cpp
	$(CC) $(CFLAGS) -c $^

word_counter: main.o word_counter.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) word_counter

clear: clean
	$(RM) main.o word_counter.o
