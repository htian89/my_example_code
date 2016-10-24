.PHONY: clean

CC = gcc
SUFFIX = c
CFLAGS =
SRCS = $(wildcard *.$(SUFFIX))
PROGS = $(patsubst %.$(SUFFIX), %, $(SRCS))

all: $(PROGS)

%: %.$(SUFFIX)
	$(CC) $(CFLAGS) -o $@.out $<
	

clean:
	rm -f *.out
	#find . -type f ! -name Makefile ! -name "*.cpp" -delete	
