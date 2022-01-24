#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <string>
#include "constants.h"
#include <iostream>
#include "types.h"
#include <vector>
#include "protocol.h"
#include <signal.h>
#include <map>

using namespace std;

enum MenuOption
{
    opt_get_user_list,
    opt_challenge_user,
    opt_list_duel_requests,
    opt_accept_duel_request
};
map<string, string> figure_to_unicode;

int client_socket;

void init_figures()
{
    figure_to_unicode["WK"] = "\33[32;1m\u2654";
    figure_to_unicode["WQ"] = "\33[32;1m\u2655";
    figure_to_unicode["WR"] = "\33[32;1m\u2656";
    figure_to_unicode["WB"] = "\33[32;1m\u2657";
    figure_to_unicode["WN"] = "\33[32;1m\u2658";
    figure_to_unicode["WP"] = "\33[32;1m\u2659";
    figure_to_unicode["BK"] = "\33[32;1m\u265A";
    figure_to_unicode["BQ"] = "\33[32;1m\u265B";
    figure_to_unicode["BR"] = "\33[32;1m\u265C";
    figure_to_unicode["BB"] = "\33[32;1m\u265D";
    figure_to_unicode["BN"] = "\33[32;1m\u265E";
    figure_to_unicode["BP"] = "\33[32;1m\u265F";
    figure_to_unicode["EE"] = " ";
}

void int_signal_handler(int signo)
{
    string message;
    message = MSG_EXT;
    pretty_send(client_socket, message);
    message = pretty_recv(client_socket); // receive exit confirmation or w/e
    close(client_socket);
    kill(getpid(), SIGQUIT);
}

void pretty_print_game_status_white(string game_status)
{
    string bg_white = "\033[47;1m"; // mageenta
    string bg_black = "\033[45;1m";
    string bg = bg_white;

    cout << "\033[1;36m  A B C D E F G H\033[0m" << endl;
    for (int row = 8; row >= 1; row--)
    {
        cout << "\033[1;36m" << row << "\033[0m ";
        for (int col = 0; col < 8; col++)
        {
            string figurehead = game_status.substr(0, game_status.find(';'));
            cout << bg << figure_to_unicode[figurehead] << " \033[0m";
            if (bg == bg_white)
            {
                bg = bg_black;
            }
            else
            {
                bg = bg_white;
            }
            game_status.erase(0, game_status.find(';') + 1);
        }
        if (bg == bg_white)
        {
            bg = bg_black;
        }
        else
        {
            bg = bg_white;
        }
        cout << "\033[1;36m " << row << "\033[0m " << endl;
    }
    cout << "\033[1;36m  A B C D E F G H\033[0m" << endl;
}

void pretty_print_game_status_black(string game_status)
{
    string bg_white = "\033[47;1m"; // mageenta
    string bg_black = "\033[45;1m";
    string bg = bg_white;

    cout << "\033[1;36m  H G F E D C B A\033[0m" << endl;
    for (int row = 1; row <= 8; row++)
    {
        cout << "\033[1;36m" << row << "\033[0m ";
        for (int col = 0; col < 8; col++)
        {
            string figurehead = game_status.substr(0, game_status.find(';'));
            cout << bg << figure_to_unicode[figurehead] << " \033[0m";
            if (bg == bg_white)
            {
                bg = bg_black;
            }
            else
            {
                bg = bg_white;
            }
            game_status.erase(0, game_status.find(';') + 1);
        }
        if (bg == bg_white)
        {
            bg = bg_black;
        }
        else
        {
            bg = bg_white;
        }
        cout << "\033[1;36m " << row << "\033[0m " << endl;
    }
    cout << "\033[1;36m  A B C D E F G H\033[0m" << endl;
}

void clear_screen()
{
    cout << "\033[2J\033[1;1H";
}

int main()
{
    init_figures();
    struct sockaddr_in server_addr;
    socklen_t addr_size;

    // Create the socket.
    client_socket = socket(PF_INET, SOCK_STREAM, 0);

    //Configure settings of the server address
    // Address family is Internet
    server_addr.sin_family = AF_INET;

    //Set port number, using htons function
    server_addr.sin_port = htons(1100);

    //Set IP address
    server_addr.sin_addr.s_addr = inet_addr(HOST);
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

    //Connect the socket to the server using the address
    addr_size = sizeof server_addr;
    connect(client_socket, (struct sockaddr *)&server_addr, addr_size);

    signal(SIGINT, int_signal_handler);

    cout << "Connected" << endl;

    // Ask for a username
    string username, message;
    string handshake = MSG_HSH;
    cout << "Welcome new User, please type your username:\n";
    while (true)
    {
        while (true)
        {
            cin >> username;
            if (username[0] < 65 || username[0] > 122)
            {
                cout << "Wrong username type: " << endl;
            }
            else
            {
                break;
            }
        }

        pretty_send(client_socket, username);
        message = pretty_recv(client_socket);

        if (message.substr(0, 3) == MSG_UAC)
        {
            break;
        }
        else if (message.substr(0, 3) == MSG_UNA)
        {
            cout << "Username exists, try another one:\n";
        }
    }

    for (;;)
    {
        MenuOption option;
        int chosen_option;

        message = MSG_CDS;
        pretty_send(client_socket, message);

        message = pretty_recv(client_socket);
        pretty_send(client_socket, handshake);

        if (message == MSG_IND)
        {
            while (true)
            {
                message = pretty_recv(client_socket);

                if (message.substr(0, 3) == MSG_DTE)
                {
                    pretty_send(client_socket, MSG_HSH);
                    clear_screen();
                    cout << "Sorry, your duel has been terminated" << endl;
                    break;
                }

                if (message.substr(0, 3) == MSG_GSW)
                {
                    clear_screen();
                    message.erase(0, 3);
                    pretty_print_game_status_white(message);
                    pretty_send(client_socket, handshake);
                }

                if (message.substr(0, 3) == MSG_GSB)
                {
                    clear_screen();
                    message.erase(0, 3);
                    pretty_print_game_status_black(message);
                    pretty_send(client_socket, handshake);
                }

                if (message.substr(0, 3) == MSG_MVR)
                {
                    cout << "Your move! <from> <to>\n";
                    string move;
                    getline(cin, move);
                    message = MSG_MOV + move;
                    pretty_send(client_socket, message);
                }

                if (message.substr(0, 3) == MSG_MNV)
                {
                    cout << "Move invalid, type again" << endl;
                    string move;
                    getline(cin, move);
                    message = MSG_MOV + move;
                    pretty_send(client_socket, message);
                }

                if (message.substr(0, 3) == MSG_MBI)
                {
                    cout << "Bad input, type again" << endl;
                    string move;
                    getline(cin, move);
                    message = MSG_MOV + move;
                    pretty_send(client_socket, message);
                }

                if (message.substr(0, 3) == MSG_MNF)
                {
                    cout << "You are trying to move not your figurehead! Type again" << endl;
                    string move;
                    getline(cin, move);
                    message = MSG_MOV + move;
                    pretty_send(client_socket, message);
                }

                if (message.substr(0, 3) == MSG_MCP)
                {
                    cout << "Yu are trying to capture your own pawn! Type again" << endl;
                    string move;
                    getline(cin, move);
                    message = MSG_MOV + move;
                    pretty_send(client_socket, message);
                }

                if (message.substr(0, 3) == MSG_DWI)
                {
                    cout << "Congrats, you have won!" << endl;
                    pretty_send(client_socket, MSG_HSH);
                    break;
                }

                if (message.substr(0, 3) == MSG_DLO)
                {
                    cout << "What a pity, you have lost..." << endl;
                    pretty_send(client_socket, MSG_HSH);
                    break;
                }
            }
        }

        cout << "What would you like to do?\n";
        cout << opt_get_user_list << ". Get user list\n";
        cout << opt_challenge_user << ". Challenge user\n";
        cout << opt_list_duel_requests << ". List duel requests\n";
        cout << opt_accept_duel_request << ". Accept duel\n";

        cin >> chosen_option;
        cout << chosen_option;

        option = static_cast<MenuOption>(chosen_option);

        clear_screen();

        if (option == opt_get_user_list)
        {
            message = MSG_GUL;
            pretty_send(client_socket, message);

            message.clear();
            message = pretty_recv(client_socket);

            for (int i = 0;; i++)
            {
                string username = message.substr(0, message.find(';'));
                cout << "> " << username << endl;
                message.erase(0, message.find(';') + 1);
                if (message.size() == 0)
                {
                    break;
                }
            }
        }

        if (option == opt_challenge_user)
        {
            message = MSG_DRQ;
            cout << "Type user login to challenge: ";
            cin >> username;
            message += username;

            pretty_send(client_socket, message);
            pretty_recv(client_socket);
        }

        if (option == opt_list_duel_requests)
        {
            message = MSG_DRL;
            pretty_send(client_socket, message);

            message.clear();
            message = pretty_recv(client_socket);

            for (int i = 0;; i++)
            {
                string username = message.substr(0, message.find(';'));
                cout << "[X] " << username << endl;
                message.erase(0, message.find(';') + 1);
                if (message.size() == 0)
                {
                    break;
                }
            }
        }

        if (option == opt_accept_duel_request)
        {
            cout << "Type user login to accept duel: ";
            message = MSG_DAC;
            cin >> username;
            message += username;

            pretty_send(client_socket, message);
            message = pretty_recv(client_socket);

            if (message.substr(0, 3) == MSG_DNA)
            {
                cout << "User is in duel already or no longer available, can't accept" << endl;
            }
            if (message.substr(0, 3) == MSG_DAC)
            {
                cout << "Going to duel" << endl;
            }
        }
    }

    close(client_socket);

    return 0;
}