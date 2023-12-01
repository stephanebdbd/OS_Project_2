CC=gcc
OPT=-Wall -Wextra -std=gnu11 -O2
LIB_SERV=-L ./img-dist/ -limg-dist -lm -lpthread
LIB_CLIENT=

# Code du serveur (img-search)
DIR_SERV=serveur

# Code du client (pokedex-client)
DIR_CLIENT=client

# Code commun au client & serveur (optionnel)
DIR_COMMON=commun

OBJS_SERV=
OBJS_CLIENT=

all: img-search pokedex-client

libimg-dist.a:
	(cd img-dist ; make)

img-search: libimg-dist.a $(DIR_SERV)/main.c $(OBJS_SERV)
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
