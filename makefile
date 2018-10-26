LIBS = -lpthread
SERVER_INCS = -I server/
CLIENT_INCS = -I client/
CC = gcc
OPTIONS = -g -Wall
SERVER_BUILD = server_build
SERVER_OBJS = server/main.o server/minesweeper.o server/leaderboard.o server/threadpool.o server/comms.o
CLIENT_BUILD = client_build
CLIENT_OBJS = client/main.o client/minesweeper.o

default: server client
all: default

clean:
	@echo "Cleaning up..."
	rm -f server/*.o
	rm -f $(SERVER_BUILD)
	rm -f client/*.o
	rm -f $(CLIENT_BUILD)

server: $(SERVER_OBJS)
	@echo --------------------------------------
	@echo Linking...
	$(CC) $(SERVER_OBJS) $(LIBS) -o $(SERVER_BUILD)

client: $(CLIENT_OBJS)
	@echo --------------------------------------
	@echo Linking...
	$(CC) $(CLIENT_OBJS) -o $(CLIENT_BUILD)
	
server/%.o: server/%.c
	@echo --------------------------------------
	@echo Building server/$*.c...
	$(CC) $(OPTIONS) $(SERVER_INCS) -c server/$*.c -o $@
	
client/%.o: client/%.c
	@echo --------------------------------------
	@echo Building client/$*.c...
	$(CC) $(OPTIONS) $(CLIENT_INCS) -c client/$*.c -o $@

