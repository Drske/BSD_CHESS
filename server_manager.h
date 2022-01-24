#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include "user.h"
#include "types.h"
#include "duel.h"
#include <vector>
#include <mutex>

using namespace std;

struct DuelRequestType {
    User *challenger;
    User *challenged;
};

class ServerManager
{
public:
    bool validate_user(User*);
    void add_user(User*);
    void del_user(User*);
    string get_user_list();
    void register_duel(User*, string);
    void cancel_duel_request(User*, string);
    string get_challengers_list(User*);
    bool accept_duel(User*, string);
    vector<Duel*> active_duel_list;
    void set_key_kernel(int);

private:
    mutex user_list_access;
    mutex duel_requests_list_access;
    mutex active_duel_list_access;
    vector<User*> user_list;
    vector<DuelRequestType> duel_requests_list;
    int key_kernel;
};

#endif