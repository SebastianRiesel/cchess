# CCHESS
This is a small chess game written in C.
It has a small and (very!) bare-bones SDL3 interface for playing and supports peer to peer playing over the network with tcp.

The game should support:
- Normal playing against an opponent
- Castling
- Pawn-Promotion
- Checkmate
- A minimax-based chess-bot
- TCP Peer to Peer Connection
- SDL3 GUI (bare-bones!)

Other capaibilites of the project which are currently not integrated into the main program include:
- Move-Reversing
- Playing in a CLI


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
The following options are availaible

```
./cchess local
./cchess local_vs_bot
./cchess host
./cchess connect <ip_addr>
```

In a network where you can establish a direct peer-to-peer TCP connection over IPv4, one player can run with the `host` option to wait for a connection, while the other player can run with `connect <ip_addr>` to connect.


## Notice
Feel free to report bugs or submit pull-requests, but since this is just a hobby project, I give no guarantuee to actually take a look at these or to maintain the project.

## Roadmap
Possible features for the future could include:
- Caching of future moves for (possibly) better Minimax performance
- Implementation of multiple difficulty levels of the chess-bot
- Porting the network implementation to a platform-independent layer for cross-platform compatibility
- Implementation of a UI with Qt or GTK, removing the need of command line options
- Implementation of a Relay-Server for playing games over the internet
- Exporting and Importing of positions in FEN 
- A Game Viewer with forwards and backwards navigation

