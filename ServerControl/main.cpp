//
//  main.cpp
//  ServerControl
//
//  Created by Josh Getter on 9/22/17.
//  Copyright © 2017 Josh Getter. All rights reserved.
//
#include <string>
#include <iostream>
#include <map>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
using namespace std;

class Server;
class ServerController;
pid_t rootPid;
const string CONTROLLER = "[CONTROLLER] ";
const string CHILD = "[CHILD] ";
const string GRANDCHILD = "[GRANDCHILD] ";
const int ABORTSERV = 0;
const int CREATEPROC = 1;
const int ABORTPROC = 2;
Server *serverInstance;

class Server{
public:
    string serverName;
    int minProcs;
    int maxProcs;
    int numActive;
    pid_t pid;
    vector<pid_t> processes;
    struct sigaction createProcSigAction;
    struct sigaction abortProcSigAction;
    struct sigaction exitProcSigAction;
    struct sigaction abortServSigAction;
    Server(string _serverName, int _minProcs, int _maxProcs){
        serverName = _serverName;
        minProcs = _minProcs;
        maxProcs = _maxProcs;
        numActive = 0;
        //Register child/grand-child signal handlers
        createProcSigAction.sa_sigaction = incrementProcess;
        abortProcSigAction.sa_sigaction = decrementProcess;
        exitProcSigAction.sa_sigaction = childExit;
        abortServSigAction.sa_sigaction = abortServer;
        sigaction(SIGRTMIN+CREATEPROC, &createProcSigAction, NULL);
        sigaction(SIGRTMIN+ABORTPROC, &abortProcSigAction, NULL);
        sigaction(SIGRTMIN+ABORTSERV, &abortServSigAction, NULL);
        sigaction(SIGINT,&exitProcSigAction,NULL);
        pid_t _pid = fork();
        if(!_pid){
            //child level process so create grand-children
            cout << CHILD + "I am main server process\n";
            pid = getpid();
            //Set process name
            prctl(PR_SET_NAME, serverName.substr(0,15).c_str(),NULL,NULL,NULL);
            //Update instance with childs pid
            createProcess(minProcs);
            //Finished creating grand-children now wait for signal and record reference to self.
            serverInstance = this;
            doNothing();
        }else{
            //Controller instance, update controllers version with PID.
            pid = _pid;
        }
    }
    static void incrementProcess(int sigNum, siginfo_t * sigInfo, void * context){
        serverInstance->incrementProcess(1);
    }
    static void decrementProcess(int sigNum, siginfo_t * sigInfo, void * context){
        serverInstance->incrementProcess(-1);
    }
    static void childExit(int sigNum, siginfo_t * sigInfo, void * context){
        exit(0);
    }
    static void abortServer(int sigNum,siginfo_t * sigInfo, void * context){
        serverInstance->createProcess(0 - serverInstance->numActive);
        exit(0);
    }
    void createProcess(int num){
        if(getpid() == pid){
            //Is child level server so continue operation
            if(num > 0){
                //Creating processes
                for(int created = 0; created < num; created++){
                    pid_t grandChildPid = fork();
                    if(grandChildPid){
                        //Still child level server
                        //Add newly created grand-child.
                        processes.push_back(grandChildPid);
                        numActive++;
                    }else{
                        //is grand child
                        cout << GRANDCHILD + "I am server instance\n";
                        doNothing();
                    }
                }
            }else{
                //removing processes
                for(int removed = 0; removed > num; removed--){
                    pid_t removingPid = processes.back();
                    kill(removingPid, SIGINT);
                    processes.pop_back();
                    numActive--;
                }
            }
            
        }
    }
    void incrementProcess(int inc){
        if(numActive + inc >= minProcs && numActive + inc <= maxProcs){
            createProcess(inc);
        }else{
            cout << CHILD << "Cannot perform operation, number of processes will be out of bounds\n";
        }
    }
    void doNothing(){
        pid_t _pPid = getppid();
        while(1){
            if(_pPid == pid){
                pause();
            }else{
                wait(NULL);
            }
        }
        
    }
};

class ServerController{
private:
    map<string, Server> serverMap;
    struct sigaction abortServSigAction;
    void start(){
        string input;
        while(1){
            if(getpid() == rootPid){
                cout << CONTROLLER + "Enter a value\n";
                getline(cin,input);
                istringstream inputStream(input);
                vector<string> tokens;
                string token;
                while(getline(inputStream,token, ' ')){
                    tokens.push_back(token);
                }
                if(tokens[0] == "createServer"){
                    //create server
                    if(tokens.size() != size_t(4)){
                        cout << CONTROLLER + "Invalid parameters for createServer\n";
                    }else if(serverMap.count(tokens[1]) >= size_t(1)){
                        cout << CONTROLLER << "That server already exists.\n";
                    }else{
                        cout << CONTROLLER + "Creating Server\n";
                        Server currServer = Server(tokens[1], atoi((tokens[2].c_str())), atoi(tokens[3].c_str()));
                        serverMap.insert(pair<string,Server>(tokens[1], currServer));
                    }
                }
                if(tokens[0] == "displayStatus"){
                    string rPid;
                    stringstream output;
                    output << rootPid;
                    rPid = output.str();
                    string cmd = "ps f -o pid,comm -g $(ps -o sid= -p " + rPid + ")";
                    system(cmd.c_str());
                }
                if(tokens[0] == "createProcess"){
                    if(tokens.size() != size_t(2)){
                        cout << CONTROLLER << "Invalid parameters for createProcess\n";
                    }else if(serverMap.count(tokens[1]) != size_t(1)){
                        cout << CONTROLLER << "That server doesn't exist\n";
                    }else{
                        kill(serverMap.at(tokens[1]).pid, SIGRTMIN + CREATEPROC);
                    }
                }
                if(tokens[0] == "abortProcess"){
                    if(tokens.size() != size_t(2)){
                        cout << CONTROLLER << "Invalid parameters for abortProcess\n";
                    }else if(serverMap.count(tokens[1]) != size_t(1)){
                        cout << CONTROLLER << "That server doesn't exist\n";
                    }else{
                        kill(serverMap.at(tokens[1]).pid, SIGRTMIN + ABORTPROC);
                    }
                }
                if(tokens[0] == "abortServer"){
                    if(tokens.size() != size_t(2)){
                        cout << CONTROLLER << "Invalid parameters for abortServer\n";
                    }else if(serverMap.count(tokens[1]) != size_t(1)){
                        cout << CONTROLLER << "That server doesn't exist\n";
                    }else{
                        kill(serverMap.at(tokens[1]).pid, SIGRTMIN + ABORTSERV);
                        wait(NULL);
                        serverMap.erase(tokens[1]);
                    }
                }
            }
            
        }
    };
public:
    ServerController(){
        start();
    }
};
int main(int argc, char * argv[]) {
    rootPid = getpid();
    ServerController();
    return 0;
}

