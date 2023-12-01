CC=gcc
OPT=-Wall -Wextra -O2 #-pg -fprofile-arcs -ftest-coverage
DBG_OPT= #-g -fsanitize=address,undefined
LIBS=-lm
OBJS=bmp.o pHash.o verbose.o

all: libimg-dist.a

libimg-dist.a: $(OBJS)
	ar rcs libimg-dist.a $(OBJS)

%.o: %.c %.h
	$(CC) $(OPT) $(DBG_OPT) -c $< -o $@
