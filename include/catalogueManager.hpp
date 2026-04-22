#pragma once

#include "../external/json/json.hpp"
#include "UiConstants.hpp"
#include "StringUtils.hpp"

#include <string>
#include <array>
#include <vector>
#include <algorithm>



using json = nlohmann::json;

struct Track
{
    int id = -1;
    std::string artist = {};
    std::string title = {};
    std::string label = {};
    float bpm = 0;
    std::array<uint8_t, 3> colour = UI::RGB_DEFAULT;
    std::vector<int> mixIds = {}; 

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
    bool editTrack(const Track& editedTrack);
    void removeTrack(int id);
    void addMix(int trackId, int mixId);
    void removeMix(int trackId, int mixId);

    const Track* getTrackForDisplay(int id);
    const std::vector<Track>& getCatalogueForDisplay() const;
    std::vector<int> getIdLibrary() const;

protected:
    std::vector<Track> catalogue;
    int lastTrackId = 0;

    bool validateTrackData(Track& t);
};


