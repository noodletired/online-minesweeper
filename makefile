LIBS = -lpthread
INCS = -I server/
CC = gcc
OPTIONS = -g -Wall
SERVER_BUILD = server_build
SERVER_OBJS = server/main.o server/minesweeper.o server/leaderboard.o server/threadpool.o server/comms.o

default: clean $(server_name) $(client_name)
all: default

clean:
	@echo "Cleaning up..."
	rm -f server/*.o
	rm -f $(SERVER_BUILD)
#	rm -f client/*.o
#	rm -f $(client_name)

server: $(SERVER_OBJS)
	@echo --------------------------------------
	@echo Linking...
	$(CC) $(SERVER_OBJS) $(LIBS) -o $(SERVER_BUILD)

%.o: %.c
	@echo --------------------------------------
	@echo Building $*.c...
	$(CC) $(OPTIONS) $(INCS) -c $*.c -o $@

