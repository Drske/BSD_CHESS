#ifndef USER_H
#define USER_H

#include <string>
#include "duel.h"

using namespace std;


class Duel;
class User
{
public:
    void set_name(string);
    string get_name();
    void set_socket(int);
    int get_socket();
    void set_duel_status(bool);
    bool get_duel_status();
    // void set_current_duel(Duel*);
    void set_shmkey(int);
    int get_shmkey();
    // Duel* get_current_duel();

private:
    string name;
    int socket;
    bool duel_status;
    int shmkey;
    // Duel* current_duel;
};

#endif