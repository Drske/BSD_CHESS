#ifndef DUEL_H
#define DUEL_H

#include "user.h"

using namespace std;

enum Colour {white, black, col_empty};
enum Figure {rook, knight, bishop, king, queen, pawn, fig_empty};
enum Column {A,B,C,D,E,F,G,H};
enum move_validate {valid, not_valid, bad_input, king_in_danger, not_your_figurehead, capture_your_pawn, mate, checkmate, won, lost};

struct ChessboardField {
    Column column;
    int row;
    Figure figure;
    Colour colour;
};

class User;

class Duel
{
public:
    void init_duel(User *, User *);
    bool players_ready();
    User *white_player;
    User *black_player;
    string get_game_status(string);
    move_validate next_move(string);
    User *get_next_moving_player();
    void leave_duel(User*);
    bool is_terminated();
    bool check_loss();

private:
    User *next_moving_player;
    ChessboardField chessboard[8][8];
    move_validate validate_figurehead_move(int,int,int,int);
};

#endif