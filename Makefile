CC=gcc
OPT=-Wall -Wextra -std=gnu11 -O2
LIB_SERV=-L ./img-dist/ -limg-dist -lm -lpthread
LIB_CLIENT=
LIBS=-lm
IMG_DIST=./img-dist
OBJS= $(IMG_DIST)/bmp.o $(IMG_DIST)/pHash.o $(IMG_DIST)/verbose.o
# Code du serveur (img-search)
DIR_SERV=serveur

# Code du client (pokedex-client)
DIR_CLIENT=client

# Code commun au client & serveur (optionnel)
DIR_COMMON=commun

OBJS_SERV=
OBJS_CLIENT=

all: $(IMG_DIST)/libimg-dist.a img-search pokedex-client

$(IMG_DIST)/%.o: $(IMG_DIST)/%.c $(IMG_DIST)/%.h
	$(CC) $(OPT) $(DBG_OPT) -c $< -o $@

$(IMG_DIST)/libimg-dist.a: $(OBJS)
	ar rcs $(IMG_DIST)/libimg-dist.a $(OBJS)

img-search: $(IMG_DIST)/libimg-dist.a $(DIR_SERV)/main.c $(OBJS_SERV)
	$(CC) $(OPT) $(OPT) $(DIR_SERV)/main.c -o img-search $(OBJS_SERV) $(LIB_SERV)

pokedex-client: $(DIR_CLIENT)/main.c $(OBJS_SERV)
	$(CC) $(OPT) $(OPT) $(DIR_CLIENT)/main.c -o pokedex-client $(OBJS_CLIENT) $(LIB_CLIENT)

%.o: $(DIR_SERV)/%.c $(DIR_SERV)/%.h
	$(CC) $(OPT) $(DBG_OPT) -c $< -o $@

%.o: $(DIR_CLIENT)/%.c $(DIR_SERV)/%.h
	$(CC) $(OPT) $(DBG_OPT) -c $< -o $@

%.o: $(DIR_COMMON)/%.c (DIR_COMMON)/%.h
	$(CC) $(OPT) $(DBG_OPT) -c $< -o $@

.PHONY: clean

clean:
	rm -f *.o
	rm -f img-search
	rm -f pokedex-client
	rm -f img-dist/*.a img-dist/*.o
