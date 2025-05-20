#pragma once

#include "../include/ConnectionHandler.h"
#include <mutex>
#include <vector>
#include <thread>

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    int subIdCounter;
	int receiptId;
    std::mutex& mutexLock;
    ConnectionHandler* handler;
    std::thread socketWorker;
    
public:
    StompProtocol(std::mutex& mutex);

    void keyboardTask();
    void socketThreadTask();
    void processLogin(vector<string>& parts);
    void processJoin(vector<string>& parts);
    void processExit(vector<string>& parts);
    void processReport(vector<string>& parts);
    void processLogout();
    void processSummary(vector<string>& parts);
    string buildSendFrame(Event& event, string& game);
    string mapToString(const std::map<string,string>& map_);

    virtual ~StompProtocol();
    StompProtocol(const StompProtocol& other);
    StompProtocol& operator=(const StompProtocol& other);
};
