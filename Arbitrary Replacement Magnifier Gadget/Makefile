.PHONY: all clean

all: arbitrary

arbitrary: arbitrary.c
			gcc -pthread -O3 $< -o $@ 

clean:
	rm -rf arbitrary
