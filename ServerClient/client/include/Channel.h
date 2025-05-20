#pragma once
#include <map>
#include <vector>
#include "../include/event.h"

using std::string;
using std::vector;
using std::map;


class Channel{
    struct Comparator {
        bool operator()(const std::pair<bool, int>& lhs, const std::pair<bool, int>& rhs) const {
            if (lhs.first == rhs.first) {
                return lhs.second < rhs.second;
            } else {
                return lhs.first > rhs.first;
            }
        }
    };

    private:
        int subId;
        string team_a_name;
        string team_b_name;
        map<string,map<string,string>> general_stats;
        map<string,map<string,string>> team_a_stats;
        map<string,map<string,string>> team_b_stats;
        map<string,map<std::pair<bool, int>,vector<string>,Comparator>> gameReports;

    public:
        int getSubId();
        void receiveEvent(Event& event, string& user);
        Channel(int subId, string team_a, string team_b);
        string writeSummary(string& user);
        string mapToString(const std::map<string,string>& map_);
};
