#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "memlayout.h"


#define MAXUSERNUM 5 //系统最大支持的用户数

struct user{
	char *username; //用户名
 	char *password; //密码
 	int uid; 		//对应的 uid
};

struct user userlist[MAXUSERNUM] = { //系统全局的用户表，可以在此添加用户
 	{"popo","123",0},
	{"rua","546",1}
};
//当前登录状态，目前只有两个虚拟终端所以数组长度为 2
int loginstate[2] = {-1,-1};
char username[10];
char pwd[10];

int 
login(char *username, char *pwd)
{
    username[strlen(username)-1] = 0;	//处理回车符号
    pwd[strlen(pwd)-1] = 0;				//处理回车符号
    cprintf("username : %s, pwd : %s.\n",username,pwd);
    for (int i = 0; i < MAXUSERNUM; i++)	//在用户表中查找
    {
        if(!strncmp(username,userlist[i].username,strlen(username))){
            if(!strncmp(pwd,userlist[i].password,strlen(pwd))){
                loginstate[getcurconsole()-1] = i;	//把当前终端的登录状态置为uid
                proc->uid = i;				//设置shell 进程 uid
                return 0;
            }
        }
    }
    return -1;
}

int 
getloginstate()
{
    proc->uid = loginstate[getcurconsole()-1];	//注意每次调用都会把当前进程的uid重置一下，主要是为了方便后面切换终端的操作
    return loginstate[getcurconsole()-1];		//返回当前的状态
}

int
getcuruid()
{
    return proc->uid;	//直接返回当前进程的uid
}

void
logout()
{
    cprintf("logout : uid %d.\n",proc->uid);
    loginstate[getcurconsole()-1] = -1;	//直接把当前终端的登录状态置为-1
}
