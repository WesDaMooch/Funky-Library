#pragma once

#include "../external/json/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

struct Track
{
    std::string artistName = "";
    std::string trackName = "";
};

void to_json(json& j, const Track& t);
void from_json(const json& j, Track& t);

class CatalogueManager
{
public:
    void load();
    void save() const;
    void addTrack(const Track& newTrack);
    const std::vector<Track>& getCatalogue() const;
    // removed track

protected:
    std::vector<Track> catalogue;
};

