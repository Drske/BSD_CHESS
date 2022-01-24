#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "constants.h"
#include "types.h"
#include <string>
#include <iostream>
#include <vector>
#include "protocol.h"
#include "server_manager.h"
#include "user.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

using namespace std;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

ServerManager sm;

void chess_duel(User *current_user)
{
    string message;
    int shmid;

    Duel *duel = NULL;
    while (true)
    {
        shmid = shmget(current_user->get_shmkey(), sizeof(Duel), 0666 | IPC_CREAT);
        duel = (Duel *)shmat(shmid, (void *)0, 0);
        if (duel == (void *)-1)
        {
            cout << "caught error " << errno << endl;

            if (shmdt(duel) == -1)
            {
                cout << "Error" << errno << endl;
            }
            if (shmctl(shmid, IPC_RMID, NULL) == -1)
            {
                cout << "Error: " << errno << endl;
            }
            current_user->set_shmkey(0);
            current_user->set_duel_status(false);
            pretty_send(current_user->get_socket(), MSG_DTE);
            message = pretty_recv(current_user->get_socket());
            return;
        }
        else
        {
            break;
        }
    }

    cout << "IN GAME:" << endl;
    cout << current_user->get_name() << endl;
    cout << current_user->get_shmkey() << endl;
    cout << "White " << duel->white_player->get_name() << endl;
    cout << "Black " << duel->black_player->get_name() << endl;

    // initial game status

    if (current_user == duel->white_player)
    {
        message = MSG_GSW;
        message += duel->get_game_status("WHITE");
    }
    else if (current_user == duel->black_player)
    {
        message = MSG_GSB;
        message += duel->get_game_status("BLACK");
    }

    pretty_send(current_user->get_socket(), message);
    message = pretty_recv(current_user->get_socket());

    if (message == MSG_EXT)
    {
        duel->leave_duel(current_user);
        shmdt(duel);
        pretty_send(current_user->get_socket(), MSG_EXC);
        sm.del_user(current_user);
        pthread_exit(NULL);
        close(current_user->get_socket());
    }

    while (true)
    {
        if (duel->check_loss())
        {
            message = MSG_DLO;
            shmdt(duel);
            if (shmctl(shmid, IPC_RMID, NULL) == -1)
            {
                cout << "Error: " << errno << endl;
            }
            current_user->set_duel_status(false);
            current_user->set_shmkey(0);
            pretty_send(current_user->get_socket(), message);
            pretty_recv(current_user->get_socket());
            return;
        }

        if (duel->is_terminated())
        {
            shmdt(duel);
            if (shmctl(shmid, IPC_RMID, NULL) == -1)
            {
                cout << "Error: " << errno << endl;
            }
            current_user->set_duel_status(false);
            current_user->set_shmkey(0);
            pretty_send(current_user->get_socket(), MSG_DTE);
            message = pretty_recv(current_user->get_socket());
            if (message == MSG_HSH)
            {
                cout << "Returning from terminated duel" << endl;
            }
            return;
        }

        if (duel->get_next_moving_player() == current_user)
        {
            // Update game status
            if (current_user == duel->white_player)
            {
                message = MSG_GSW;
                message += duel->get_game_status("WHITE");
            }
            else if (current_user == duel->black_player)
            {
                message = MSG_GSB;
                message += duel->get_game_status("BLACK");
            }

            pretty_send(current_user->get_socket(), message);
            message = pretty_recv(current_user->get_socket());

            if (message == MSG_EXT)
            {
                duel->leave_duel(current_user);
                shmdt(duel);
                pretty_send(current_user->get_socket(), MSG_EXC);
                sm.del_user(current_user);
                pthread_exit(NULL);
                close(current_user->get_socket());
            }

            message = MSG_MVR;
            pretty_send(current_user->get_socket(), message);
            message = pretty_recv(current_user->get_socket());

            if (message == MSG_EXT)
            {
                duel->leave_duel(current_user);
                shmdt(duel);
                pretty_send(current_user->get_socket(), MSG_EXC);
                sm.del_user(current_user);
                pthread_exit(NULL);
                close(current_user->get_socket());
            }

            if (message.substr(0, 3) == MSG_MOV)
            {
                while (true)
                {
                    message.erase(0, 3);
                    move_validate mv = duel->next_move(message);

                    if (mv == valid)
                    {
                        if (current_user == duel->white_player)
                        {
                            message = MSG_GSW;
                            message += duel->get_game_status("WHITE");
                        }
                        else if (current_user == duel->black_player)
                        {
                            message = MSG_GSB;
                            message += duel->get_game_status("BLACK");
                        }

                        pretty_send(current_user->get_socket(), message);
                        message = pretty_recv(current_user->get_socket());

                        if (message == MSG_EXT)
                        {
                            duel->leave_duel(current_user);
                            shmdt(duel);
                            pretty_send(current_user->get_socket(), MSG_EXC);
                            sm.del_user(current_user);
                            pthread_exit(NULL);
                            close(current_user->get_socket());
                        }
                        break;
                    }

                    if (mv == not_valid)
                    {
                        message = MSG_MNV;
                    }

                    if (mv == bad_input)
                    {
                        message = MSG_MBI;
                    }

                    if (mv == not_your_figurehead)
                    {
                        message = MSG_MNF;
                    }

                    if (mv == capture_your_pawn)
                    {
                        message = MSG_MCP;
                    }

                    if (mv == won)
                    {
                        message = MSG_DWI;
                        shmdt(duel);
                        current_user->set_duel_status(false);
                        current_user->set_shmkey(0);
                        pretty_send(current_user->get_socket(), message);
                        pretty_recv(current_user->get_socket());
                        return;
                    }

                    pretty_send(current_user->get_socket(), message);
                    message = pretty_recv(current_user->get_socket());

                    if (message == MSG_EXT)
                    {
                        duel->leave_duel(current_user);
                        shmdt(duel);
                        pretty_send(current_user->get_socket(), MSG_EXC);
                        sm.del_user(current_user);
                        pthread_exit(NULL);
                        close(current_user->get_socket());
                    }
                }
            }
        }

        else
        {
            if (current_user == duel->white_player)
            {
                message = MSG_GSW;
                message += duel->get_game_status("WHITE");
            }
            else if (current_user == duel->black_player)
            {
                message = MSG_GSB;
                message += duel->get_game_status("BLACK");
            }

            pretty_send(current_user->get_socket(), message);
            message = pretty_recv(current_user->get_socket());

            if (message == MSG_EXT)
            {
                duel->leave_duel(current_user);
                shmdt(duel);
                pretty_send(current_user->get_socket(), MSG_EXC);
                sm.del_user(current_user);
                pthread_exit(NULL);
                close(current_user->get_socket());
            }
        }
        usleep(1000000);
    }
}

void *new_user_thread(void *arg)
{
    int new_socket = *((int *)arg);
    User current_user;
    string username;

    while (true)
    {
        username = pretty_recv(new_socket);

        if (username == MSG_EXT)
        {
            pretty_send(new_socket, MSG_EXC);
            pthread_exit(NULL);
            close(current_user.get_socket());
        }

        current_user.set_name(username);
        if (sm.validate_user(&current_user))
        {
            current_user.set_socket(new_socket);
            current_user.set_duel_status(false);
            sm.add_user(&current_user);
            pretty_send(new_socket, MSG_UAC);
            break;
        }
        else
        {
            pretty_send(new_socket, MSG_UNA);
        }
    }

    for (;;)
    {
        cout << "Name " << current_user.get_name() << endl;
        cout << "Shmid " << current_user.get_shmkey() << endl;
        cout << "Duel " << current_user.get_duel_status() << endl;
        string message = pretty_recv(new_socket);
        cout << "Message received: " << message << endl;

        if (message.substr(0, 3) == MSG_EXT)
        {
            if (current_user.get_shmkey() != 0)
            {
                int shmid = shmget(current_user.get_shmkey(), sizeof(Duel), 0666 | IPC_CREAT);
                Duel *duel = (Duel *)shmat(shmid, (void *)0, 0);
                if (duel == (void *)-1)
                {
                    cout << "caught error " << endl;
                }
                printf("errno:%d\n", errno);

                duel->leave_duel(&current_user);
                shmdt(duel);
            }
            pretty_send(current_user.get_socket(), MSG_EXC);
            sm.del_user(&current_user);
            pthread_exit(NULL);
            close(current_user.get_socket());
        }

        // user requested list of available users
        if (message.substr(0, 3) == MSG_GUL)
        {
            message.clear();

            string ul = sm.get_user_list();

            pretty_send(new_socket, ul);
        }

        if (message.substr(0, 3) == MSG_DRQ)
        {
            username = message.substr(3, message.size() - 3);
            sm.register_duel(&current_user, username);
            pretty_send(new_socket, MSG_HSH);
        }

        if (message.substr(0, 3) == MSG_DRL)
        {
            string cl = sm.get_challengers_list(&current_user);

            pretty_send(new_socket, cl);
        }

        if (message.substr(0, 3) == MSG_DAC)
        {
            if (current_user.get_duel_status())
            {
                pretty_send(current_user.get_socket(), MSG_DNA);
            }
            else
            {
                username = message.substr(3, message.size() - 3);
                bool acc_true = sm.accept_duel(&current_user, username);

                if (acc_true)
                {
                    pretty_send(current_user.get_socket(), MSG_DAC);
                }

                else
                {
                    pretty_send(current_user.get_socket(), MSG_DNA);
                }
            }
        }

        if (message.substr(0, 3) == MSG_CDS)
        {
            // checking if active duel has been terminated
            // before user entered
            if (current_user.get_shmkey() != 0)
            {
                int shmid = shmget(current_user.get_shmkey(), sizeof(Duel), 0666 | IPC_CREAT);
                Duel *duel = (Duel *)shmat(shmid, (void *)0, 0);
                if (duel == (void *)-1)
                {
                    cout << "Error: " << errno << endl;
                }
                if (duel->is_terminated())
                {
                    shmdt(duel);
                    if (shmctl(shmid, IPC_RMID, NULL) == -1)
                    {
                        cout << "Error: " << errno << endl;
                    }
                    current_user.set_duel_status(false);
                    current_user.set_shmkey(0);
                }
                shmdt(duel);
            }

            bool status = current_user.get_duel_status();

            if (status)
            {
                message = MSG_IND;
                pretty_send(new_socket, message);
                pretty_recv(new_socket);
                usleep(1000000);
                chess_duel(&current_user);
            }
            else
            {
                message = MSG_NID;
                pretty_send(new_socket, message);
                pretty_recv(new_socket);
            }
        }
    }
}

int main()
{
    int server_socket, new_socket;
    struct sockaddr_in server_address;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    //Create the socket.
    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    // Address family = Internet
    server_address.sin_family = AF_INET;

    //Set port number, using htons function to use proper byte order
    server_address.sin_port = htons(1100);

    //Set IP address to localhost
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    //Set all bits of the padding field to 0
    memset(server_address.sin_zero, '\0', sizeof server_address.sin_zero);

    //Bind the address struct to the socket
    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    sm.set_key_kernel(1);

    //Listen on the socket
    if (listen(server_socket, 50) == 0)
        printf("Listening\n");
    else
        printf("Error\n");

    pthread_t thread_id;

    while (1)
    {
        //Accept call creates a new socket for the incoming connection
        addr_size = sizeof serverStorage;
        new_socket = accept(server_socket, (struct sockaddr *)&serverStorage, &addr_size);

        if (pthread_create(&thread_id, NULL, new_user_thread, &new_socket) != 0)
            printf("Failed to create thread\n");

        pthread_detach(thread_id);
    }

    return 0;
}