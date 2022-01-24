#pragma once
#ifndef PROTOCOL_H
#define PROTOCOL_H

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
#include <signal.h>
#include <iostream>

using namespace std;

static const char *MSG_INT = "101";
static const char *MSG_EXT = "102";
static const char *MSG_EXC = "202"; // confirm exit
static const char *MSG_GUL = "103"; // get_user_list
static const char *MSG_DRQ = "104"; // duel_request
static const char *MSG_DRL = "105"; // duel_requests_list
static const char *MSG_DAC = "106"; // duel_accept (info to server)
static const char *MSG_DNA = "207"; // duel could not be accepted
static const char *MSG_FFL = "107"; // fight_for_life (send to both users agreed to take part in duel)
static const char *MSG_CDS = "108"; // check_duel_status
static const char *MSG_IND = "109"; // in_duel
static const char *MSG_NID = "110"; // not_in_duel
static const char *MSG_UAC = "111"; // user accepted
static const char *MSG_UNA = "112"; // user not accepted
static const char *MSG_DST = "113"; // duel starded
static const char *MSG_MOV = "114"; // move message
static const char *MSG_MVR = "214"; // move request
static const char *MSG_RQM = "115"; // request move message
static const char *MSG_GSW = "116"; // game status for white player
static const char *MSG_GSB = "117"; // game status for black player
static const char *MSG_HSH = "000"; // handshake
static const char *MSG_DTE = "900"; // duel terminated
static const char *MSG_DWI = "906"; // duel won
static const char *MSG_DLO = "907"; // duel lost
static const char *MSG_MVV = "901"; // move valid
static const char *MSG_MNV = "902"; // move not valid
static const char *MSG_MBI = "903"; // move bad input
static const char *MSG_MNF = "904"; // move not your figurehead
static const char *MSG_MCP = "905"; // move capture your pawn

static void pretty_send(int socket, string message, int flag = 0)
{
    int length_message, bytes_to_send;

    length_message = message.size();
    bytes_to_send = sizeof(int);

    while (bytes_to_send != 0)
    {
        bytes_to_send -= send(socket, &length_message, sizeof(int), flag);
    }

    bytes_to_send = message.size();

    while (bytes_to_send != 0)
    {
        bytes_to_send -= send(socket, message.c_str(), message.size(), flag);
    }
}

static string pretty_recv(int socket, int flag = 0)
{
    vector<char> buffer(MAX_BUFFER_SIZE);
    int bytes_to_recv, length_message;
    string message;
    message.clear();

    bytes_to_recv = sizeof(int);

    while (bytes_to_recv != 0)
    {
        int n = recv(socket, &length_message, sizeof(length_message), flag);
        bytes_to_recv -= n;
    }

    bytes_to_recv = length_message;

    while (bytes_to_recv != 0)
    {
        int n = recv(socket, buffer.data(), buffer.size(), flag);
        bytes_to_recv -= n;
    }

    message.append(buffer.begin(), buffer.end());

    for (int i = 0; i < message.size(); i++)
    {
        if (int(message[i]) == 0)
        {
            message = message.substr(0, i);
            break;
        }
    }

    return message;
}

#endif