#include "user.h"
#include <string>

using namespace std;

void User::set_name(string username){
    this->name = username;
}

string User::get_name(){
    return this->name;
}

void User::set_socket(int socket){
    this->socket = socket;
}

int User::get_socket(){
    return this->socket;
}

void User::set_duel_status(bool status){
    this->duel_status = status;
}

bool User::get_duel_status(){
    return this->duel_status;
}

// void User::set_current_duel(Duel* duel){
//     this->current_duel = duel;
// }

void User::set_shmkey(int shmkey){
    this->shmkey = shmkey;
}

int User::get_shmkey(){
    return this->shmkey;
}