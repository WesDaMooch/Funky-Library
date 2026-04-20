#include "../include/catalogueManager.hpp"
#include <iostream>
#include <fstream>


// ADL 
void to_json(json& j, const Track& t)
{
    j = {
        {"id", t.id},
        {"artist", t.artist},
        {"title", t.title},
        {"label", t.label},
        {"bpm", t.bpm},
        {"colour", t.colour},
        {"mixIds", t.mixIds}
    };
}

void from_json(const json& j, Track& t)
{
    try
    {
        t.id     = j.at("id").get<int>();
        t.artist = j.at("artist").get<std::string>();
        t.title  = j.at("title").get<std::string>();
        t.label  = j.at("label").get<std::string>();
        t.bpm    = j.at("bpm").get<int>();
        t.colour = j.at("colour").get<std::array<uint8_t, 3>>();
        t.mixIds = j.value("mixIds", std::vector<int>{});
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

    // Update id
    for (const auto& track : catalogue)
    {
        if (track.id >= lastTrackId)
            lastTrackId = track.id + 1;
    }
}

void CatalogueManager::save() const
{
    json j;
    j["tracks"] = catalogue;

    std::ofstream file("catalogue.json");
    file << j.dump(4);

    file.close();
}

void CatalogueManager::refresh()
{
    save();
    load();
}

bool CatalogueManager::addTrack(const Track& newTrack)
{
    // TODO: Check the data is good

    if (newTrack.artist.empty() || newTrack.title.empty())
        return false;

    // Dont allow duplicates names (not case sensative) to be added
    std::string newArtist = toLower(newTrack.artist);
    std::string newTitle = toLower(newTrack.title);
    std::string newLabel = toLower(newTrack.label);

    for (const Track& track : catalogue)
    {
        if (toLower(track.artist) == newArtist &&
            toLower(track.title) == newTitle &&
            toLower(track.label) == newLabel)

            return false; 
    }

    Track trackToAdd = newTrack;
    trackToAdd.id = lastTrackId++;
    trackToAdd.mixIds.clear();

    catalogue.emplace_back(trackToAdd);
    refresh();

    return true;
}

bool CatalogueManager::editTrack(const Track& editedTrack)
{
    // TODO: trim whitespace if (trim(editedTrack.artist).empty())
    if (editedTrack.artist.empty() || editedTrack.title.empty())
        return false;

    auto foundTrack = std::find_if(catalogue.begin(), catalogue.end(),
        [&editedTrack](const Track& t)
        {
            return t.id == editedTrack.id;
        });

    if (foundTrack == catalogue.end())
        return false;

    foundTrack->artist = editedTrack.artist;
    foundTrack->title = editedTrack.title;
    foundTrack->label = editedTrack.label;
    foundTrack->bpm = std::max(editedTrack.bpm, 0);
    foundTrack->colour = editedTrack.colour;

    refresh();
    return true;
}

void CatalogueManager::addMix(int trackId, int mixId)
{
    if (trackId == mixId)
        return;

    bool trackFound = false;
    bool mixFound = false;

    for (Track& track : catalogue)
    {
        if (track.id == trackId)
        {
            if (std::find(track.mixIds.begin(),
                track.mixIds.end(),
                mixId) == track.mixIds.end())
            {
                track.mixIds.emplace_back(mixId);
            }

            trackFound = true;
        }

        if (track.id == mixId)
        {
            if (std::find(track.mixIds.begin(),
                track.mixIds.end(),
                trackId) == track.mixIds.end())
            {
                track.mixIds.emplace_back(trackId);
            }

            mixFound = true;
        }

        if (trackFound && mixFound)
            break;
    }

    if (trackFound && mixFound)
        refresh();
}

void CatalogueManager::removeTrack(int id)
{
    // Remove any refernce of id
    for (Track& track : catalogue)
    {
        track.mixIds.erase(
            std::remove(track.mixIds.begin(),
                track.mixIds.end(),
                id),
            track.mixIds.end()
        );
    }
    // Remove
    catalogue.erase(
        std::remove_if(catalogue.begin(), catalogue.end(),
            [&](const Track& track)
            {
                return track.id == id;
            }),
        catalogue.end()
    );
    refresh();
}

void CatalogueManager::removeMix(int trackId, int mixId)
{
    if (trackId == mixId)
        return;

    bool trackFound = false;
    bool mixFound = false;

    for (Track& track : catalogue)
    {
        if (track.id == trackId)
        {
            track.mixIds.erase(
                std::remove(track.mixIds.begin(), track.mixIds.end(), mixId),
                track.mixIds.end());

            trackFound = true;
        }

        if (track.id == mixId)
        {
            track.mixIds.erase(
                std::remove(track.mixIds.begin(), track.mixIds.end(), trackId),
                track.mixIds.end());

            mixFound = true;
        }

        if (trackFound && mixFound)
            break;
    }

    if (trackFound || mixFound)
        refresh();
}

const Track* CatalogueManager::getTrack(int id)
{
    for (const Track& t : catalogue)
    {
        if (t.id == id)
            return &t;
    }

    return nullptr;
}

const std::vector<Track>& CatalogueManager::getCatalogue() const
{
    return catalogue;
}
