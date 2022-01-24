# BSD_CHESS
Repository connected with Computer Networks 2022 Course.

Client-Server based aplication allowing multiple users to connect with each other to play chess. Connections are implemented using BSD Sockets so any computer connected to local network may enter the server. Simple Terminal "GUI" allows users to check online users, send duel request, list user request, accept duel request and play with each other. Simple, but readible and colorful, unicode-based chessboards make the games quite pleasant to use.

## Compilation

**Client:** `g++ client.cpp`

optionally add output directory i.e:

`g++ client.cpp -o client.out`

**Server:** `g++ -pthread server.cpp server_manager.cpp duel.cpp user.cpp` 

optionally add output directory i.e

`g++ -pthread server.cpp server_manager.cpp duel.cpp user.cpp -o server.out`

## Setting up Host

Find IP of a computer who will be a server. In `constants.h` change `HOST` constant to you IP.

## Launching

1. Launch server application before running any client.
2. Launch client
3. Server will print most importat information about status during the launch time
4. As a client follow the steps displaying on the screen
5. Server supports multiple clients and pararell duels.

## Duel

1. Server support move validation. It will not allow for a move inconsistent with chess game rules.
2. Proceed with simple movements typing `<from> <to>`, i.e. `H5 H6`.
3. Terminal ''GUI'' will help you with orienting in the game status.

## Protocol

Clients and server exchange messagess according to protocol below.

```
MSG_INT = "101"; // initial message
MSG_EXT = "102"; // exit message
MSG_EXC = "202"; // confirm exit
MSG_GUL = "103"; // get_user_list
MSG_DRQ = "104"; // duel_request
MSG_DRL = "105"; // duel_requests_list
MSG_DAC = "106"; // duel_accept (info to server)
MSG_DNA = "207"; // duel could not be accepted
MSG_FFL = "107"; // fight_for_life (send to both users agreed to take part in duel)
MSG_CDS = "108"; // check_duel_status
MSG_IND = "109"; // in_duel
MSG_NID = "110"; // not_in_duel
MSG_UAC = "111"; // user accepted
MSG_UNA = "112"; // user not accepted
MSG_DST = "113"; // duel starded
MSG_MOV = "114"; // move message
MSG_MVR = "214"; // move request
MSG_RQM = "115"; // request move message
MSG_GSW = "116"; // game status for white player
MSG_GSB = "117"; // game status for black player
MSG_HSH = "000"; // handshake
MSG_DTE = "900"; // duel terminated
MSG_DWI = "906"; // duel won
MSG_DLO = "907"; // duel lost
MSG_MVV = "901"; // move valid
MSG_MNV = "902"; // move not valid
MSG_MBI = "903"; // move bad input
MSG_MNF = "904"; // move not your figurehead
MSG_MCP = "905"; // move capture your pawn
```
Every message exchanged is or starts with certain message code and both server and client act according to it.

## Technology

1. Server and client use BSD Sockets to exchange information.
2. Server uses threads (pthreads) to manage different users.
3. To display chessboards and figures special signs of unicode has been used.
4. Threads communicate with each other (during duels) using shared memory.
5. Whole server is managed by one server manager. Modification of user list are managed using semaphores (mutexes).

## Implementation

1. For the sake of the project two main functions, `pretty_send` and `pretty receive` has been implemented. They allow to send and receive messages via BSD Sockets in more pleasant way for the programmer and also allow any-sized messages to be sent and receive without problems, even when they are splitted on the way.

## Problems
1. Sending bigger amount of data between apllications.
2. Managing shared memory
