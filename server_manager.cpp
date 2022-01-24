#include "server_manager.h"
#include "user.h"
#include "protocol.h"
#include <string>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>

using namespace std;

void ServerManager::set_key_kernel(int kernel)
{
    this->key_kernel = kernel;
}

bool ServerManager::validate_user(User *user)
{
    cout << "Validating " << (*user).get_name() << endl;
    this->user_list_access.lock();
    for (int i = 0; i < this->user_list.size(); i++)
    {
        cout << "Comparing " << (*user).get_name() << " with " << (*this->user_list[i]).get_name() << endl;
        if ((*this->user_list[i]).get_name() == (*user).get_name())
        {
            this->user_list_access.unlock();
            return false;
        }
    }
    this->user_list_access.unlock();

    cout << (*user).get_name() << " validated true" << endl;
    return true;
}

void ServerManager::add_user(User *user)
{
    this->user_list_access.lock();
    this->user_list.push_back(user);
    this->user_list_access.unlock();
}

void ServerManager::del_user(User *user)
{
    cout << "DELETING " << user->get_name() << endl;
    this->user_list_access.lock();
    for (int i = 0; i < this->user_list.size(); i++)
    {
        if (this->user_list[i] == user)
        {
            cout << "Deleting user from user list " << user->get_name() << endl;
            this->user_list.erase(this->user_list.begin() + i);
            break;
        }
    }
    this->user_list_access.unlock();

    cout << "DELETING DUELS" << endl;

    this->duel_requests_list_access.lock();
    int i = 0;
    while (true)
    {
        if (i >= this->duel_requests_list.size())
        {
            break;
        }
        if (this->duel_requests_list[i].challenged == user || this->duel_requests_list[i].challenger == user)
        {
            cout << "Deleteing user form duels list " << user->get_name() << endl;
            this->duel_requests_list.erase(this->duel_requests_list.begin() + i);
            i -= 1;
        }
        i++;
    }
    this->duel_requests_list_access.unlock();
}

string ServerManager::get_user_list()
{
    string list = "";

    this->user_list_access.lock();
    for (int i = 0; i < this->user_list.size(); i++)
    {
        list += (*this->user_list[i]).get_name();
        list += ';';
    }
    this->user_list_access.unlock();
    cout << "User list size: " << list.size() << endl;
    return list;
}

void ServerManager::register_duel(User *challenger, string challenged)
{
    if (challenger->get_name() == challenged)
    {
        return;
    }

    DuelRequestType dr;
    dr.challenged = nullptr;
    dr.challenger = nullptr;
    this->user_list_access.lock();
    this->duel_requests_list_access.lock();
    dr.challenger = challenger;
    for (int i = 0; i < this->user_list.size(); i++)
    {
        if ((*this->user_list[i]).get_name() == challenged)
        {
            dr.challenged = this->user_list[i];
            break;
        }
    }
    if (dr.challenged != nullptr && dr.challenger != nullptr)
    {
        this->duel_requests_list.push_back(dr);
    }

    this->duel_requests_list_access.unlock();
    this->user_list_access.unlock();
}

void ServerManager::cancel_duel_request(User *challenger, string challenged_name)
{
    this->user_list_access.lock();
    this->duel_requests_list_access.lock();

    for (int i = 0; i < this->duel_requests_list.size(); i++)
    {
        if ((*this->duel_requests_list[i].challenger).get_name() == (*challenger).get_name() && (*this->duel_requests_list[i].challenged).get_name() == challenged_name)
        {
            this->duel_requests_list.erase(this->duel_requests_list.begin() + i);
            break;
        }
    }

    this->duel_requests_list_access.unlock();
    this->user_list_access.unlock();
}

string ServerManager::get_challengers_list(User *challenged)
{
    string list = "";

    this->duel_requests_list_access.lock();
    for (int i = 0; i < this->duel_requests_list.size(); i++)
    {
        if ((*this->duel_requests_list[i].challenged).get_name() == (*challenged).get_name())
        {
            list += (*this->duel_requests_list[i].challenger).get_name();
            list += ';';
        }
    }
    this->duel_requests_list_access.unlock();
    return list;
}

bool ServerManager::accept_duel(User *challenged, string challenger_name)
{
    User *challenger = nullptr;

    this->user_list_access.lock();
    this->duel_requests_list_access.lock();
    for (int i = 0; i < this->duel_requests_list.size(); i++)
    {
        if ((*this->duel_requests_list[i].challenged).get_name() == (*challenged).get_name() && (*this->duel_requests_list[i].challenger).get_name() == challenger_name)
        {
            challenger = this->duel_requests_list[i].challenger;
            this->duel_requests_list.erase(this->duel_requests_list.begin() + i);
        }
    }

    if (challenger == nullptr)
    {
        return false;
    }

    if (challenger->get_duel_status())
    {
        return false;
    }

    this->key_kernel = (this->key_kernel + 17) % 1000000;
    int shmid = shmget(this->key_kernel, sizeof(Duel), 0666 | IPC_CREAT);

    cout << "Shared memory id: " << shmid << endl;
    cout << "Key kernel: " << this->key_kernel << endl;

    challenger->set_shmkey(this->key_kernel);
    challenged->set_shmkey(this->key_kernel);

    cout << "Shmid set" << endl;

    cout << "Attaching accept " << endl;
    Duel *duel = (Duel *)shmat(shmid, (void *)0, 0);
    if (duel == (void *)-1)
    {
        cout << "caught error " << endl;
    }
    cout << "Duel shmat at init: " << duel << endl;
    printf("errno:%d\n", errno);
    if (strcmp(reinterpret_cast<const char *>(duel), "0xffffffff") == 0)
    {
        cout << "FINAFCUKINGLY" << endl;
    }
    else
    {
        cout << "Its a no no" << endl;
    }
    duel->init_duel(challenger, challenged);
    cout << "Deataching accept" << endl;
    shmdt(duel);

    cout << "Duel initated" << endl;

    (*challenger).set_duel_status(true);
    (*challenged).set_duel_status(true);

    cout << "End of accept" << endl;

    this->duel_requests_list_access.unlock();
    this->user_list_access.unlock();

    return true;
}
