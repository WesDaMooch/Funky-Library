#include "../include/catalogueManager.hpp"
#include <iostream>
#include <fstream>


// ADL 
void to_json(json& j, const Track& t)
{
    j = {
        {"artistName", t.artistName},
        {"trackName", t.trackName}
    };
}

void from_json(const json& j, Track& t)
{
    try
    {
        t.artistName = j.at("artistName").get<std::string>();
        t.trackName = j.at("trackName").get<std::string>();
    }
    catch (const std::exception& e)
    {
        std::cout << "Invalid JSON format" << e.what() << std::endl;
    }
}

// Manager
void CatalogueManager::load()
{
    std::ifstream file("catalogue.json");

    if (!file.is_open())
        return;
   
    json j;
    file >> j;

    file.close();

    catalogue = j.value("tracks", std::vector<Track>{});
}

void CatalogueManager::save() const
{
    json j;
    j["tracks"] = catalogue;

    std::ofstream file("catalogue.json");
    file << j.dump(4);

    file.close();
}

void CatalogueManager::addTrack(const Track& track)
{
    catalogue.push_back(track);
    //save();
    //load();
}

const std::vector<Track>& CatalogueManager::getCatalogue() const
{
    return catalogue;
}
