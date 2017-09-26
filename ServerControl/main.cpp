//
//  main.cpp
//  ServerControl
//
//  Created by Josh Getter on 9/22/17.
//  Copyright © 2017 Josh Getter. All rights reserved.
//
#include <string>
#include <iostream>

using namespace std;
int main(int argc, const char * argv[]) {
    string input;
    while(1){
        cout << "Enter a value\n";
        getline(cin,input);
        if(input.find("createServer") != -1){
            cout << "Creating Server\n";
            //create server
        }
    }
    return 0;
}
class Server{
    string serverName;
    int minProcs;
    int maxProcs;
    int numActive;
    
    Server(string _serverName, int _minProcs, int _maxProcs){
        serverName = _serverName;
        minProcs = _minProcs;
        maxProcs = _maxProcs;
    }
};
