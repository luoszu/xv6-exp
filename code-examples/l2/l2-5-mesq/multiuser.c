#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "memlayout.h"


#define MAXUSERNUM 5

struct user{
    char *username;
    char *password;
    int  uid;
};

struct user userlist[MAXUSERNUM] = {
    {"popo","123",0},
    {"rua","546",0},
    {"kkk","345",0}

};

int loginstate[2] = {-1,-1};

char username[10];
char pwd[10];

int 
login(char *username, char *pwd)
{
    username[strlen(username)-1] = 0;
    pwd[strlen(pwd)-1] = 0;
    cprintf("username : %s, pwd : %s.\n",username,pwd);
    for (int i = 0; i < MAXUSERNUM; i++)
    {
        if(!strncmp(username,userlist[i].username,strlen(username))){
            if(!strncmp(pwd,userlist[i].password,strlen(pwd))){
                loginstate[getcurconsole()-1] = i;
                myproc()->uid = i;
                return 0;
            }
        }
    }
    return -1;
}

int 
getloginstate()
{
    myproc()->uid = loginstate[getcurconsole()-1];
    return loginstate[getcurconsole()-1];
}

int
getcuruid()
{
    return myproc()->uid;
}

void
logout()
{
    cprintf("logout : uid %d.\n",myproc()->uid);
    loginstate[getcurconsole()-1] = -1;
}
