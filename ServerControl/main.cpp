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

using namespace std;

class Server;
class ServerController;
pid_t rootPid;
const string CONTROLLER = "[CONTROLLER] ";
const string CHILD = "[CHILD] ";
const string GRANDCHILD = "[GRANDCHILD] ";

class Server{
public:
    string serverName;
    int minProcs;
    int maxProcs;
    int numActive;
    pid_t pid;
    vector<pid_t> processes;
    Server(string _serverName, int _minProcs, int _maxProcs){
        serverName = _serverName;
        minProcs = _minProcs;
        maxProcs = _maxProcs;
        numActive = 0;
        pid_t _pid = fork();
        if(!_pid){
            //child level process so create grand-children
            cout << CHILD + "I am main server process\n";
            pid = getpid();
            //Update instance with childs pid
            createProcess(minProcs);
            //Finished creating grand-children now wait for signal
            pause();
        }
    }
    void createProcess(int count){
        if(getpid() == pid){
            for(int created = 1; created <= count; created++){
                //Is child level server so replicate
                pid_t grandChildPid;
                grandChildPid = fork();
                if(grandChildPid){
                    //Still parent server
                    //Add newly created grand - child.
                    processes.push_back(grandChildPid);
                }else{
                    //is grand child
                    cout << GRANDCHILD + "I am server instance\n";
                    pause();
                }
                
            }
        }
    }
};

class ServerController{
private:
    map<string, Server> serverMap;
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
                    cout << CONTROLLER + "Creating Server\n";
                    //create server
                    if(tokens.size() != sizeof(4)){
                        cout << CONTROLLER + "Invalid parameters for createServer\n";
                    }else{
                        Server currServer = Server(tokens[1], atoi((tokens[2].c_str())), atoi(tokens[3].c_str()));
                        serverMap.insert(pair<string,Server>(tokens[1], currServer));
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

