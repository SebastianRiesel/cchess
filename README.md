# CCHESS
This is a small chess game written in C.
It has a small and (very!) bare-bones SDL3 interface for playing and supports peer to peer playing over the network with tcp.

The game should support:
- Normal playing against an opponent
- Castling
- Pawn-Promotion
- Checkmate

Other capaibilites of the project which are currently not integrated into the main program include:
- A minimax-based chess-bot
- Move-Reversing
- Playing in a CLI
- a dependency-injection-like architecture to inject the different kinds of interface implementations into the game logic:
    - SDL3 GUI
    - Terminal CLI
    - Random Move
    - Minimax Bot
    - TCP Peer to Peer Connection
    


## Dependencies
The project is NOT platform-independent and currently targeted for x86-64 Linux machines.

Dependencies are:
- SDL3, installed as shared library
- SDL3_img, installed as shared library
- The Linux Sockets API

## Build
Please make sure you are on a x86-64 linux machine, and have the development versions of SDL3 and SDL3_img installed. Clone the project, and then run:

```
make all
```

## Running
After building the project, the game should be runnable with
```
./cchess
```

By default, the program will wait for incoming tcp connections, and when a connection has been established will start the game.

The other player should run the program as:

```
./cchess <ip address>
```

If there is a direct TCP connection possible, the game should then start on both clients.

## Notice
Feel free to report bugs or submit pull-requests, but since this is just a hobby project, I give no guarantuee to actually take a look at these or to maintain the project.


