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
    std::array<uint8_t, 3> colour = Ui::RGB_DEFAULT;
    std::vector<int> mixIds = {}; 

};

void to_json(json& j, const Track& t);
void from_json(const json& j, Track& t);

class LibraryManager
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

    enum Sort
    {
       Artist,
       Title,
       Label,
       BPM
    };

    std::vector<Track> searchAndSort(const std::string& search, Sort sort);

protected:
    std::vector<Track> library;
    int lastTrackId = 0;

    bool validateTrackData(Track& t);
};


