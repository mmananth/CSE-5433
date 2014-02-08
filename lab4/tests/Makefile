default: all
all: test10k test testIncremental testFullDump

test10k: test10k.c
	gcc -g -o test10k test10k.c

test: test.c
	gcc -g -o test test.c

testIncremental: testIncremental.c
	gcc -g -o testIncremental testIncremental.c

testFullDump: testFullDump.c
	gcc -g -o testFullDump testFullDump.c

clean:
	rm test test10k testIncremental testFullDump
