#include "../include/StompProtocol.h"
#include "../include/event.h"
#include <string>
#include <iostream>
#include <fstream>
#include <map>

using namespace std;
using std::string;
using std::map;

class Event;

StompProtocol::StompProtocol(std::mutex& mutex): subIdCounter(0),receiptId(0), mutexLock(mutex), handler(nullptr), socketWorker(){}

std::vector<string> split(const string s, string delimiter){
    std::vector<string> parts;
    size_t pos = 0;
    string str = s;
    while ((pos = str.find(delimiter)) != string::npos)
    {
        parts.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }
    parts.push_back(str);
    return parts;
}

void StompProtocol::keyboardTask(){

    const short bufsize = 1024;

    while(1){
        //read from keyboard
        char buf[bufsize];
        std::cin.getline(buf, bufsize);

        string line(buf);
        std::vector<string> parts = split(line," ");

        string command = parts[0];
        if(command == "login"){
            processLogin(parts);
            continue;
        }     
        if(command == "join"){
            processJoin(parts);
            continue;
        }
        if(command == "exit"){
            processExit(parts);
            continue;
        }
        if(command == "report"){
            processReport(parts);
            continue;
        }
        if(command == "logout"){
            processLogout();
            continue;
        }
        if(command == "summary"){
            processSummary(parts);
            continue;
        }
    }
}

void StompProtocol::processLogin(vector<string>& parts){
    if(handler != nullptr){
        std::cerr << "The client is already logged in, log out before trying again"<< std::endl;
    }
    else{
        int delimiterPos = parts[1].find(":");
        //extract headers
        string host = parts[1].substr(0,delimiterPos);
        short port = std::stoi(parts[1].substr(delimiterPos+1));
        string username = parts[2]; string password = parts[3];
        subIdCounter = 1;
        receiptId = 1;

        //connect to server
        handler = new ConnectionHandler(host,port,username);    

        if (!handler->connect()) {
            std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        }
        else{

            if (socketWorker.joinable()) {
                socketWorker.join();
            }
            socketWorker = std::thread(&StompProtocol::socketThreadTask,this); 

            //create connect frame
            string frame = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:"+username+"\npasscode:"+password+"\n\n";
            //try send frame
            if(!handler->sendLine(frame)) {
                std::lock_guard<std::mutex> lock(mutexLock);
                std::cout << "Disconnected. Exiting...\n" << std::endl;
            }
        }
    }
}
void StompProtocol::processJoin(vector<string>& parts){
    string gameName = parts[1];
    size_t delimiter = gameName.find('_');
    string team_a = gameName.substr(0,delimiter);
    string team_b = gameName.substr(delimiter+1);
    if(handler->isUserSubscribed(gameName)){
        std::lock_guard<std::mutex> lock(mutexLock);
        std::cout << "user is already subscribed to this channel" << std::endl;
    }
    else{
        string frame = "SUBSCRIBE\ndestination:"+gameName+"\nid:"+std::to_string(subIdCounter)+"\nreceipt:"+std::to_string(receiptId)+"\n\n";
        mutexLock.lock();
        handler->addReceipt(receiptId,"Joined channel "+gameName);
        handler->addChannel(gameName,Channel(subIdCounter,team_a,team_b));
        mutexLock.unlock();
        subIdCounter++; receiptId++;
        if (!handler->sendLine(frame)) {
            std::lock_guard<std::mutex> lock(mutexLock);
            std::cout << "Disconnected. Exiting...\n" << std::endl;
        }
    }
}
void StompProtocol::processExit(vector<string>& parts){
    string gameName = parts[1];
    if(!handler->isUserSubscribed(gameName)){
        std::cout << "user is not subscribed to this channel" << std::endl;
    }
    else{
        mutexLock.lock();
        int subId = handler->getChannel(gameName).getSubId();
        string frame = "UNSUBSCRIBE\nid:"+std::to_string(subId)+"\nreceipt:"+std::to_string(receiptId)+"\n\n";
        handler->addReceipt(receiptId,"Exited channel "+gameName);
        receiptId++;
        handler -> removeChannel(gameName);
        mutexLock.unlock();
        if (!handler->sendLine(frame)) {
            std::lock_guard<std::mutex> lock(mutexLock);
            std::cout << "Disconnected. Exiting...\n" << std::endl;
        }
    }
}
void StompProtocol::processReport(vector<string>& parts){
   
    names_and_events fileReport = parseEventsFile(parts[1]);
    string gameName = fileReport.team_a_name+"_"+fileReport.team_b_name;

    if(! handler->isUserSubscribed(gameName)){
        std::lock_guard<std::mutex> lock(mutexLock);
        std::cout << "the user is not subcribed to channel "+gameName+", subscibe before reporting" << std::endl;
    }
    else{
        std::vector<Event> events = fileReport.events;
        for(Event& event: events){
            string frame = buildSendFrame(event,gameName);
            if (!handler->sendLine(frame)) {
                std::lock_guard<std::mutex> lock(mutexLock);
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
        }
    }
}
void StompProtocol::processSummary(vector<string>& parts){
     
    string game = parts[1]; string user = parts[2]; string file = parts[3];
    
    if(! handler->isUserSubscribed(game)){
        std::lock_guard<std::mutex> lock(mutexLock);
        std::cout << "the user is not subcribed to channel "+game+", subscibe before asking for summary" << std::endl;
    }
    else{
        Channel relevantChannel = handler->getChannel(game);
        string summery = relevantChannel.writeSummary(user);

        std::ofstream outfile; // write to the file
        outfile.open(file,ofstream::out|ofstream::trunc);
        if(outfile.is_open()){
            outfile<<summery;
            outfile.close();
        } // if the file is already opened, writing over its content
    }
    
}
void StompProtocol::processLogout(){
    //create disconnect frame
    string frame = "DISCONNECT\nreceipt:"+std::to_string(receiptId)+"\n\n";

    mutexLock.lock();
    handler->addReceipt(receiptId,"Disconnected");
    receiptId++;
    mutexLock.unlock();
    if (!handler->sendLine(frame)) {
        std::lock_guard<std::mutex> lock(mutexLock);
        std::cout << "Disconnected. Exiting...\n" << std::endl;
    }
}

string StompProtocol::buildSendFrame(Event& event, string& game){

    string frame = "SEND\ndestination:"+game+"\n\n"+"user:"+handler->getUsername()+"\nteam a:"+event.get_team_a_name()+"\nteam b:"+event.get_team_b_name()+"\nevent name:"+event.get_name()+"\ntime:"+std::to_string(event.get_time())+"\n";

    std::map<string,string> generalUpdates = event.get_game_updates();
    std::map<string,string> aUpdates = event.get_team_a_updates();
    std::map<string,string> bUpdates = event.get_team_b_updates();

    frame+="general games updates:\n";
    if(!generalUpdates.empty())
        frame+=mapToString(generalUpdates);
    frame+="team a updates:\n";
    if(!aUpdates.empty())
        frame+=mapToString(aUpdates);
    frame+="team b updates:\n";
    if(!bUpdates.empty())
        frame+=mapToString(bUpdates); 
    
    return frame+"description:\n"+event.get_discription();  
}

string StompProtocol::mapToString(const std::map<string,string>& map_){
    string result = "";
    for(auto& pair : map_){
        result+="\t"+pair.first+":"+pair.second+"\n";
    }
    return result;
}

void StompProtocol::socketThreadTask(){
    while(1){
        string answerFrame;
        if (!handler->getLine(answerFrame)) {
            std::lock_guard<std::mutex> lock(mutexLock);
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            break;
        }

        string command = answerFrame.substr(0,answerFrame.find('\n'));

        if(command == "CONNECTED"){
            std::lock_guard<std::mutex> lock(mutexLock);
            std::cout<<"login successful"<<std::endl;
        }
        if(command == "RECEIPT"){

            string receiptHeader = "receipt-id:";
            size_t start = answerFrame.find(receiptHeader)+receiptHeader.length();
            size_t end = answerFrame.find('\n',start);
            int recId = std::stoi(answerFrame.substr(start,end-start)); //get receipt id

            std::lock_guard<std::mutex> lock(mutexLock);
            string meaning = handler->getReceiptMeaning(recId);
            std::cout << meaning << std::endl; //print the meaning
            if(meaning == "Disconnected"){
                delete handler;
                handler = nullptr;
                break;
            }          
        }
        if(command == "MESSAGE"){

            size_t start = answerFrame.find("destination:")+12;
            size_t end = answerFrame.find("\n",start);
            string destination = answerFrame.substr(start,end-start);
            
            string frame_body = answerFrame.substr(answerFrame.find("\n\n")+2);  
            size_t startUserPart = frame_body.find(":")+1;       
            size_t endUserPart = frame_body.find("\n");
            
            string username = frame_body.substr(startUserPart,endUserPart-startUserPart);
            frame_body = frame_body.substr(endUserPart+1);

            Event event(frame_body);

            std::lock_guard<std::mutex> lock(mutexLock);
            handler->getChannel(destination).receiveEvent(event,username);
        }
        if(command == "ERROR"){
            std::lock_guard<std::mutex> lock(mutexLock);
            if(handler != nullptr){
                delete handler;
                handler = nullptr;
            }
            std::cout << answerFrame << std::endl;
            std::cout<< "Disconnected..." <<std::endl;
            break;
        }
    }
}

StompProtocol::~StompProtocol(){
    if(handler!=nullptr){
        delete handler;
        handler = nullptr;
    }
}
StompProtocol::StompProtocol(const StompProtocol& other): subIdCounter(other.subIdCounter), receiptId(other.receiptId), mutexLock(other.mutexLock), handler(nullptr), socketWorker(){}
StompProtocol& StompProtocol::operator=(const StompProtocol& other){
    if(this != &other){
        subIdCounter = other.subIdCounter;
        receiptId = other.receiptId;
    }
    return *this;
}
