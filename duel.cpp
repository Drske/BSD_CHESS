#include "duel.h"
#include "user.h"
#include "constants.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>

using namespace std;

void Duel::leave_duel(User *user)
{
    cout << "Player " << user->get_name() << " is leaving duel" << endl;
    
    if (this->white_player == user)
    {
        this->white_player = nullptr;
    }

    if (this->black_player == user)
    {
        this->black_player = nullptr;
    }

    cout << "Leaving completed" << endl;
}

bool Duel::is_terminated()
{
    cout << "Checking if terminated" << endl;
    if (this->white_player == nullptr || this->black_player == nullptr)
    {
        cout << "Check terminated true" << endl;
        return true;
    }
    else
    {
        cout << "Check terrminated false" << endl;
        return false;
    }
}

void Duel::init_duel(User *user_one, User *user_two)
{
    srand(time(NULL));
    if (rand() % 2 == 0)
    {
        this->white_player = user_one;
        this->black_player = user_two;
    }
    else
    {
        this->white_player = user_two;
        this->black_player = user_one;
    }

    this->next_moving_player = white_player;

    for (int col = A; col <= H; col++)
    {
        this->chessboard[col][1].column = static_cast<Column>(col);
        this->chessboard[col][1].row = 1;
        this->chessboard[col][1].figure = pawn;
        this->chessboard[col][1].colour = white;
        this->chessboard[col][6].column = static_cast<Column>(col);
        this->chessboard[col][6].row = 7;
        this->chessboard[col][6].figure = pawn;
        this->chessboard[col][6].colour = black;
    }

    for (int fig = rook; fig < king; fig++)
    {
        this->chessboard[fig][0].column = static_cast<Column>(fig); // A,B,C on proper places in relation to rook, knight, bishop
        this->chessboard[fig][0].row = 0;
        this->chessboard[fig][0].figure = static_cast<Figure>(fig);
        this->chessboard[fig][0].colour = white;
        this->chessboard[7 - fig][0].column = static_cast<Column>(fig); // A,B,C on proper places in relation to rook, knight, bishop
        this->chessboard[7 - fig][0].row = 0;
        this->chessboard[7 - fig][0].figure = static_cast<Figure>(fig);
        this->chessboard[7 - fig][0].colour = white;
        this->chessboard[fig][7].column = static_cast<Column>(fig); // A,B,C on proper places in relation to rook, knight, bishop
        this->chessboard[fig][7].row = 7;
        this->chessboard[fig][7].figure = static_cast<Figure>(fig);
        this->chessboard[fig][7].colour = black;
        this->chessboard[7 - fig][7].column = static_cast<Column>(fig); // A,B,C on proper places in relation to rook, knight, bishop
        this->chessboard[7 - fig][7].row = 7;
        this->chessboard[7 - fig][7].figure = static_cast<Figure>(fig);
        this->chessboard[7 - fig][7].colour = black;
    }

    this->chessboard[D][0].column = D;
    this->chessboard[D][0].row = 0;
    this->chessboard[D][0].figure = king;
    this->chessboard[D][0].colour = white;

    this->chessboard[E][0].column = E;
    this->chessboard[E][0].row = 0;
    this->chessboard[E][0].figure = queen;
    this->chessboard[E][0].colour = white;

    this->chessboard[D][7].column = D;
    this->chessboard[D][7].row = 7;
    this->chessboard[D][7].figure = king;
    this->chessboard[D][7].colour = black;

    this->chessboard[E][7].column = E;
    this->chessboard[E][7].row = 7;
    this->chessboard[E][7].figure = queen;
    this->chessboard[E][7].colour = black;

    for (int col = A; col <= H; col++)
    {
        for (int row = 2; row <= 5; row++)
        {
            this->chessboard[col][row].column = static_cast<Column>(col);
            this->chessboard[col][row].row = row;
            this->chessboard[col][row].figure = fig_empty;
            this->chessboard[col][row].colour = col_empty;
        }
    }
}

string Duel::get_game_status(string type)
{
    string game_status;

    if (type == "WHITE")
    {
        for (int row = 7; row >= 0; row--)
        {
            for (int col = 0; col < 8; col++)
            {
                game_status += col_translator[this->chessboard[col][row].colour];
                game_status += fig_translator[this->chessboard[col][row].figure];
                game_status += ";";
            }
        }
    }

    if (type == "BLACK")
    {
        for (int row = 0; row < 8; row++)
        {
            for (int col = 7; col >= 0; col--)
            {
                game_status += col_translator[this->chessboard[col][row].colour];
                game_status += fig_translator[this->chessboard[col][row].figure];
                game_status += ";";
            }
        }
    }

    return game_status;
}

User *Duel::get_next_moving_player()
{
    return this->next_moving_player;
}

move_validate Duel::validate_figurehead_move(int from_col, int from_row, int to_col, int to_row)
{
    Colour player_color;
    Figure figure_from = this->chessboard[from_col][from_row].figure;
    Colour colour_from = this->chessboard[from_col][from_row].colour;
    Figure figure_to = this->chessboard[to_col][to_row].figure;
    Colour colour_to = this->chessboard[to_col][to_row].colour;

    // not your figurehead
    if (this->next_moving_player == this->white_player)
    {
        player_color = white;
    }
    else if (this->next_moving_player == this->black_player)
    {
        player_color = black;
    }
    if (player_color != colour_from)
    {
        return not_your_figurehead;
    }

    // capture your pawn
    if (player_color == colour_to)
    {
        return capture_your_pawn;
    }

    // From this point I am sure that my move is within the
    // chessboard (checked in next move), is in proper format
    // (also checked in next move), player is going to move
    // his pawn and is not going to capute his own.
    // All is left to check if the certain figurehead is
    // moved in proper way according to game rules.

    // pawn
    if (figure_from == pawn)
    {
        // straigt one
        if (player_color == white)
        {
            if (from_col == to_col && from_row + 1 == to_row && figure_to == fig_empty)
            {
                return valid;
            }
        }
        if (player_color == black)
        {
            if (from_col == to_col && from_row - 1 == to_row && figure_to == fig_empty)
            {
                return valid;
            }
        }

        // straght two
        if (player_color == white)
        {
            if (from_col == to_col && from_row + 2 == to_row)
            {
                for (int row = from_row + 1; row <= to_row; row++)
                {
                    if (this->chessboard[from_col][row].figure != fig_empty)
                    {
                        return not_valid;
                    }
                }
                return valid;
            }
        }
        if (player_color == black)
        {
            if (from_col == to_col && from_row - 2 == to_row)
            {
                for (int row = from_row - 1; row >= to_row; row--)
                {
                    if (this->chessboard[from_col][row].figure != fig_empty)
                    {
                        return not_valid;
                    }
                }
                return valid;
            }
        }

        // capture
        if (player_color == white)
        {
            if ((from_col == to_col - 1 || from_col == to_col + 1) && from_row + 1 == to_row && figure_to != fig_empty)
            {
                return valid;
            }
        }
        if (player_color == black)
        {
            if ((from_col == to_col - 1 || from_col == to_col + 1) && from_row - 1 == to_row && figure_to != fig_empty)
            {
                return valid;
            }
        }
    }

    // roook
    if (figure_from == rook)
    {
        // column move
        if (from_col == to_col && from_row != to_row)
        {
            int inc;
            to_row > from_row ? inc = 1 : inc = -1;

            for (int row = from_row + inc; row != to_row; row += inc)
            {
                if (this->chessboard[from_col][row].figure != fig_empty)
                {
                    return not_valid;
                }
            }
            return valid;
        }
        // row move
        if (from_row == to_row && from_col != to_col)
        {
            int inc;
            to_col > from_col ? inc = 1 : inc = -1;
            for (int col = from_col + inc; col != to_col; col += inc)
            {
                if (this->chessboard[col][from_row].figure != fig_empty)
                {
                    return not_valid;
                }
            }
            return valid;
        }
    }

    // bishop
    if (figure_from == bishop)
    {
        if (abs(to_row - from_row) == abs(to_col - from_col) && from_row != to_row && from_col != to_col)
        {
            int inc_col, inc_row;
            to_col > from_col ? inc_col = 1 : inc_col = -1;
            to_row > from_row ? inc_row = 1 : inc_row = -1;

            for (int col = from_col + inc_col, row = from_row + inc_row; col != to_col && row != to_row; col += inc_col, row += inc_row)
            {
                if (this->chessboard[col][row].figure != fig_empty)
                {
                    return not_valid;
                }
            }

            return valid;
        }
    }

    // knight
    if (figure_from == knight)
    {
        // vertical jump
        if (to_col + 1 == from_col && to_row == from_row + 2)
        {
            return valid;
        }
        if (to_col - 1 == from_col && to_row == from_row + 2)
        {
            return valid;
        }
        if (to_col + 1 == from_col && to_row == from_row - 2)
        {
            return valid;
        }
        if (to_col - 1 == from_col && to_row == from_row - 2)
        {
            return valid;
        }
        // horizontal jump
        if (to_col == from_col + 2 && to_row - 1 == from_row)
        {
            return valid;
        }
        if (to_col == from_col + 2 && to_row + 1 == from_row)
        {
            return valid;
        }
        if (to_col == from_col - 2 && to_row - 1 == from_row)
        {
            return valid;
        }
        if (to_col == from_col - 2 && to_row + 1 == from_col)
        {
            return valid;
        }
    }

    //queen
    if (figure_from == queen)
    {
        // skew
        if (abs(to_row - from_row) == abs(to_col - from_col) && from_row != to_row && from_col != to_col)
        {
            int inc_col, inc_row;
            to_col > from_col ? inc_col = 1 : inc_col = -1;
            to_row > from_row ? inc_row = 1 : inc_row = -1;

            for (int col = from_col + inc_col, row = from_row + inc_row; col != to_col && row != to_row; col += inc_col, row += inc_row)
            {
                if (this->chessboard[col][row].figure != fig_empty)
                {
                    return not_valid;
                }
            }

            return valid;
        }

        // column move
        if (from_col == to_col && from_row != to_row)
        {
            int inc;
            to_row > from_row ? inc = 1 : inc = -1;

            for (int row = from_row + inc; row != to_row; row += inc)
            {
                if (this->chessboard[from_col][row].figure != fig_empty)
                {
                    return not_valid;
                }
            }
            return valid;
        }

        // row move
        if (from_row == to_row && from_col != to_col)
        {
            int inc;
            to_col > from_col ? inc = 1 : inc = -1;
            for (int col = from_col + inc; col != to_col; col += inc)
            {
                if (this->chessboard[col][from_row].figure != fig_empty)
                {
                    return not_valid;
                }
            }
            return valid;
        }
    }

    if (figure_from == king)
    {
        if (abs(to_col - from_col) < 2 && abs(to_row - from_row) < 2)
        {
            return valid;
        }
    }

    return not_valid;
}

bool Duel::check_loss()
{
    Colour king_to_search;

    if (this->white_player == this->next_moving_player)
    {
        king_to_search = white;
    }
    else
    {
        king_to_search = black;
    }

    for (int col = 0; col < 8; col++)
    {
        for (int row = 0; row < 8; row++)
        {
            if (this->chessboard[col][row].figure == king && this->chessboard[col][row].colour == king_to_search)
            {
                return false;
            }
        }
    }

    return true;
}

move_validate Duel::next_move(string move)
{
    if (this->check_loss())
    {
        return lost;
    }

    // bad input format
    if (move.size() != 5)
    {
        return bad_input;
    }

    int from_col = int(move[0] - 65);
    int from_row = move[1] - '0' - 1;
    int to_col = int(move[3] - 65);
    int to_row = move[4] - '0' - 1;

    // Bad input
    if (from_col < 0 || from_col > 7)
    {
        return bad_input;
    }
    if (from_row < 0 || from_row > 7)
    {
        return bad_input;
    }
    if (to_col < 0 || to_col > 7)
    {
        return bad_input;
    }
    if (to_row < 0 || to_col > 7)
    {
        return bad_input;
    }

    Figure figure_from = this->chessboard[from_col][from_row].figure;
    Colour colour_from = this->chessboard[from_col][from_row].colour;
    Figure figure_to = this->chessboard[to_col][to_row].figure;
    Colour colour_to = this->chessboard[to_col][to_row].colour;

    // no figurehead on from field
    if (figure_from == fig_empty || colour_from == col_empty)
    {
        return not_valid;
    }

    move_validate figurehead_move = this->validate_figurehead_move(from_col, from_row, to_col, to_row);

    if (figurehead_move != valid)
    {
        return figurehead_move;
    }

    this->chessboard[to_col][to_row].figure = figure_from;
    this->chessboard[to_col][to_row].colour = colour_from;
    this->chessboard[from_col][from_row].colour = col_empty;
    this->chessboard[from_col][from_row].figure = fig_empty;

    if (this->next_moving_player == this->white_player)
    {
        this->next_moving_player = this->black_player;
    }
    else
    {
        this->next_moving_player = this->white_player;
    }

    if (figure_to == king)
    {
        return won;
    }

    return valid;
}