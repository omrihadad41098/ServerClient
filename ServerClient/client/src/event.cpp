#include "../include/event.h"
#include "../include/json.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
using json = nlohmann::json;
using std::string;

Event::Event(std::string team_a_name, std::string team_b_name, std::string name, int time,
             std::map<std::string, std::string> game_updates, std::map<std::string, std::string> team_a_updates,
             std::map<std::string, std::string> team_b_updates, std::string discription)
    : team_a_name(team_a_name), team_b_name(team_b_name), name(name),
      time(time), game_updates(game_updates), team_a_updates(team_a_updates),
      team_b_updates(team_b_updates), description(discription)
{
}

Event::~Event()
{
}

const std::string &Event::get_team_a_name() const
{
    return this->team_a_name;
}

const std::string &Event::get_team_b_name() const
{
    return this->team_b_name;
}

const std::string &Event::get_name() const
{
    return this->name;
}

int Event::get_time() const
{
    return this->time;
}

const std::map<std::string, std::string> &Event::get_game_updates() const
{
    return this->game_updates;
}

const std::map<std::string, std::string> &Event::get_team_a_updates() const
{
    return this->team_a_updates;
}

const std::map<std::string, std::string> &Event::get_team_b_updates() const
{
    return this->team_b_updates;
}

const std::string &Event::get_discription() const
{
    return this->description;
}

Event::Event(const std::string &frame_body) : team_a_name(""), team_b_name(""), name(""), time(0), game_updates(), team_a_updates(), team_b_updates(), description("")
{
    //build new event object from message frame body
    size_t start = 0;
    while(start < frame_body.length()){
        size_t endLine = frame_body.find("\n",start);
        string line = frame_body.substr(start,endLine-start);

        start = endLine+1;
        std::size_t delimiter = line.find(":");
        string key = line.substr(0,delimiter);

        if(key == "general games updates" || key == "team a updates" || key == "team b updates"){
            while(frame_body[start]=='\t'){ //read the body of the map (the updates)
                size_t endPair = frame_body.find("\n",start);
                size_t subDelimiter = frame_body.find(":",start);
                string subKey = frame_body.substr(start+1,subDelimiter-start-1);
                string subValue = frame_body.substr(subDelimiter+1,endPair-subDelimiter-1);
                if(key == "general games updates")
                    game_updates.insert({subKey,subValue});
                else{
                    if(key == "team a updates")
                        team_a_updates.insert({subKey,subValue});
                    else
                        team_b_updates.insert({subKey,subValue});
                }
                start = endPair+1;
            }
        }
        else{
            if(key == "description"){
                description = frame_body.substr(start);
                break;
            }
            else{
                string value = line.substr(delimiter+1);
                if(key == "team a"){
                    team_a_name = value;
                    continue;
                } 
                if(key == "team b"){
                    team_b_name = value;
                    continue;
                }
                if(key == "event name"){
                    name = value;
                    continue;
                }
                if(key == "time"){
                    time = std::stoi(value);
                }
            }
        }
    }
}


names_and_events parseEventsFile(std::string json_path)
{
    std::ifstream f(json_path);
    json data = json::parse(f);

    std::string team_a_name = data["team a"];
    std::string team_b_name = data["team b"];

    // run over all the events and convert them to Event objects
    std::vector<Event> events;
    for (auto &event : data["events"])
    {
        std::string name = event["event name"];
        int time = event["time"];
        std::string description = event["description"];
        std::map<std::string, std::string> game_updates;
        std::map<std::string, std::string> team_a_updates;
        std::map<std::string, std::string> team_b_updates;
        for (auto &update : event["general game updates"].items())
        {
            if (update.value().is_string())
                game_updates[update.key()] = update.value();
            else
                game_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team a updates"].items())
        {
            if (update.value().is_string())
                team_a_updates[update.key()] = update.value();
            else
                team_a_updates[update.key()] = update.value().dump();
        }

        for (auto &update : event["team b updates"].items())
        {
            if (update.value().is_string())
                team_b_updates[update.key()] = update.value();
            else
                team_b_updates[update.key()] = update.value().dump();
        }
        
        events.push_back(Event(team_a_name, team_b_name, name, time, game_updates, team_a_updates, team_b_updates, description));
        
    }
    names_and_events events_and_names{team_a_name, team_b_name, events};

    return events_and_names;
}