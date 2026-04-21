#include "../include/catalogueManager.hpp"
#include <iostream>
#include <fstream>


// ADL serialization
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
        t.bpm    = j.at("bpm").get<float>();
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
    Track track = newTrack;

    if (!validateTrackData(track))
        return false;

    track.id = lastTrackId++;
    track.mixIds.clear();

    // TODO: use std::move? catalogue.emplace_back(std::move(t));
    catalogue.emplace_back(track);
    refresh();
    return true;
}

bool CatalogueManager::editTrack(const Track& editedTrack)
{
    
    Track track = editedTrack;

    if (!validateTrackData(track))
        return false;
    
    auto foundTrack = std::find_if(catalogue.begin(), catalogue.end(),
        [&track](const Track& t)
        {
            return t.id == track.id;
        });

    if (foundTrack == catalogue.end())
        return false;

    foundTrack->artist = track.artist;
    foundTrack->title = track.title;
    foundTrack->label = track.label;
    foundTrack->bpm = track.bpm;
    foundTrack->colour = track.colour;

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

const Track* CatalogueManager::getTrackForDisplay(int id)
{
    for (const Track& t : catalogue)
    {
        if (t.id == id)
            return &t;
    }

    return nullptr;
}

const std::vector<Track>& CatalogueManager::getCatalogueForDisplay() const
{
    return catalogue;
}

// TODO: Could return an emum entry like data OK or MISSING_X...
bool CatalogueManager::validateTrackData(Track& track)
{
    // Remove white spaces.
    track.artist = StringUtil::trim(track.artist);
    track.title = StringUtil::trim(track.title);
    track.label = StringUtil::trim(track.label);

    // Ensure artist and title fields are filled.
    if (track.artist.empty() || track.title.empty())
        return false;

    // Don't allow duplicate names (case-insensitive).
    std::string artist = StringUtil::toLower(track.artist);
    std::string title = StringUtil::toLower(track.title);
    std::string label = StringUtil::toLower(track.label);

    // TODO: Check if mix ids are legit?
    for (const Track& t : catalogue)
    {
        if (t.id == track.id)
            continue;

        if (StringUtil::toLower(t.artist) == artist &&
            StringUtil::toLower(t.title) == title &&
            StringUtil::toLower(t.label) == label)
        {
            return false;
        }
    }

    // TODO: Validate mixIds?

    // Ensure bpm is not negative.
    track.bpm = std::max(0.f, track.bpm);
    
    return true;
}