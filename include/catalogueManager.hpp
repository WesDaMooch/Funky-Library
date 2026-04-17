#pragma once

#include "../external/json/json.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

struct Track
{
    int id = 0; 
    std::string artistName = "";
    std::string trackName = "";
    std::vector<int> mixIds = {}; // Holds the id of other tracks
};

void to_json(json& j, const Track& t);
void from_json(const json& j, Track& t);

class CatalogueManager
{
public:
    void load();
    void save() const;
    void refresh();
    bool addTrack(const Track& newTrack);
    void addMix(int trackId, int mixId);
    void removeTrack(int id);
    void removeMix(int trackId, int mixId);

    const Track* getTrack(int id);
    const std::vector<Track>& getCatalogue() const;

    inline std::string toLower(std::string s)
    {
        std::transform(
            s.begin(),
            s.end(),
            s.begin(),
            [](unsigned char c) {return std::tolower(c); }
        );
        return s;
    }

    inline std::vector<std::string> splitWords(const std::string& text)
    {
        std::stringstream ss(text);
        std::vector<std::string> words;
        std::string word;

        while (ss >> word)
            words.push_back(word);

        return words;
    }

protected:
    std::vector<Track> catalogue;
    int lastTrackId = 0;
};


