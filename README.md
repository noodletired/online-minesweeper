Assessment item for CAB403 - Systems Programming

#### Prompt
You have been commissioned to develop a client/server system for an online games provider to expand
their current offerings to registered clients. The company wishes to offer to their current clients 
the game of Minesweeper.

> Minesweeper is a single-player game, with the objective of placing flags on a grid of either empty
tiles, or tiles containing “mines”. The player does not know the location of these mines at the 
beginning of a round, yet must place the flags in the correct locations, which is the main challenge
of the game. A tile can be revealed to discover how many mines are adjacent to it, as a hint to 
where the mines lie. However, if a tile containing a mine is revealed, the game is over.

This game is simulated using a distributed system architecture. The server determines the
placement of the mines and maintains the state of the game grid. The client should therefore
receive input from the player, and relay this to the server. The server can then simulate the
player’s move and transmit the new state of the grid to the client. Only the visible tiles should
be sent to the client.

Both the client and server are implemented in the C programming language using BSD sockets
on the Linux operating system. The programs run in a terminal, reading input from the keyboard and
writing output to the screen.

#### Server
The server is responsible for ensuring only registered clients of the system can play
Minesweeper. A file named `Authentication.txt` contains the names and the passwords of all
registered clients. This file should be located in the same directory as the server binary file.

The server takes only one (1) command line parameter that indicates which `port number` the server 
is to listen on. If no port number is supplied the default port of 12345 is to be used by the server.
The following command will run the server program on port 12345.
```
./server 12345
```


#### Client
The client takes two (2) command line parameters: `hostname` and `port number`. The following command
will run the client program connecting to the server on port 12345.
```
./client server_IP_address 12345
```
The game of Minesweeper is only available to registered clients of the company’s online gaming system.
Once the client program starts the client must authenticate by entering a registered name and password 
which appear in the `Authentication.txt` file located on the server. After the server validates the
client’s credentials the client program can proceed to the main menu. If the client does not authenticate
correctly, the socket is immediately closed.
