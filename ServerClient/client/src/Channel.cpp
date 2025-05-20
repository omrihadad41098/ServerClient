#include "../include/Channel.h"

Channel::Channel(int id, string team_a, string team_b): subId(id),team_a_name(team_a),team_b_name(team_b),general_stats(),team_a_stats(),team_b_stats(),gameReports(){}

void Channel::receiveEvent(Event& event, string& user){

    map<string,string> gameUpdates = event.get_game_updates();
    map<string,string> aUpdates = event.get_team_a_updates();
    map<string,string> bUpdates = event.get_team_b_updates();

    for (auto& pair : gameUpdates) {
        general_stats[user][pair.first] = pair.second;
    }
    for (auto& pair : aUpdates) {
        team_a_stats[user][pair.first] = pair.second;
    }
    for (auto& pair : bUpdates) {
        team_b_stats[user][pair.first] = pair.second;
    }

    if(general_stats[user]["before halftime"]=="true")  //check if the current event happend before or after halftime
        gameReports[user][std::make_pair(true,event.get_time())] = {event.get_name(),event.get_discription()};
    else
        gameReports[user][std::make_pair(false,event.get_time())] = {event.get_name(),event.get_discription()};

}

int Channel::getSubId(){
    return subId;
}

string Channel::writeSummary(string& user){

    string headline = team_a_name+" vs "+team_b_name+"\n";
    //summary all stats maps
    string generalStats = "General stats:\n"+mapToString(general_stats[user]);
    string teamA = team_a_name+" stats:\n"+mapToString(team_a_stats[user]);
    string teamB = team_b_name+" stats:\n"+mapToString(team_b_stats[user]);

    //summary game reports
    string reports = "Game event reports:\n";
    for (std::pair<std::pair<bool,int>,vector<string>> pair : gameReports[user]) {
        string time = std::to_string(pair.first.second);
        string event_name = pair.second[0];
        string description = pair.second[1];
        reports += time+" - "+event_name+":\n\n"+description+"\n\n\n";
    }
    string summary = headline+"Game stats:\n"+generalStats+teamA+teamB+reports;
    return summary;
}

string Channel::mapToString(const std::map<string,string>& map_){
    string result="";
    for (auto& pair : map_) {
        result+= pair.first+":"+pair.second+"\n";
    }   
    return result;
}