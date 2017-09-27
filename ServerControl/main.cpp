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

using namespace std;

class Server{
public:
    string serverName;
    int minProcs;
    int maxProcs;
    int numActive;
    pid_t pid;
    
    Server(int _minProcs, int _maxProcs, string _serverName){
        serverName = _serverName;
        minProcs = _minProcs;
        maxProcs = _maxProcs;
        numActive = 0;
        pid = getpid(); //Should be updated/initialized after fork
    }
};


class ServerController{
private:
    map<string, Server> serverMap;
    void start(){
        string input;
        while(1){
            cout << "Enter a value\n";
            getline(cin,input);
            istringstream inputStream(input);
            vector<string> tokens;
            string token;
            while(getline(inputStream,token, ' ')){
                //Tokenize line
                tokens.push_back(token);
            }
            if(tokens[0] == "createServer"){
                cout << "Creating Server\n";
                //create server
                if(tokens.size() != sizeof(4)){
                    cout << "Invalid parameters for createServer";
                }else{
                    Server currServer = Server(atoi((tokens[1].c_str())), atoi(tokens[2].c_str()), tokens[3]);
                    serverMap.insert(pair<string,Server>(currServer.serverName,currServer));
                }
            }
        }
    };
public:
    ServerController(){
        start();
    }
};

int main(int argc, const char * argv[]) {
    ServerController();
    return 0;
}

