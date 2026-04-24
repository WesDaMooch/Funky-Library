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
void LibraryManager::load()
{
    std::ifstream file("catalogue.json");

    if (!file.is_open())
        return;

    json j;
    file >> j;

    file.close();

    library = j.value("tracks", std::vector<Track>{});

    // Update id
    for (const auto& track : library)
    {
        if (track.id >= lastTrackId)
            lastTrackId = track.id + 1;
    }
}

void LibraryManager::save() const
{
    json j;
    j["tracks"] = library;

    std::ofstream file("catalogue.json");
    file << j.dump(4);

    file.close();
}

void LibraryManager::refresh()
{
    save();
    load();
}

bool LibraryManager::addTrack(const Track& newTrack)
{
    // TODO: Check the data is good
    Track track = newTrack;

    if (!validateTrackData(track))
        return false;

    track.id = lastTrackId++;
    track.mixIds.clear();

    // TODO: use std::move? catalogue.emplace_back(std::move(t));
    library.emplace_back(track);
    refresh();
    return true;
}

bool LibraryManager::editTrack(const Track& editedTrack)
{
    
    Track track = editedTrack;

    if (!validateTrackData(track))
        return false;
    
    auto foundTrack = std::find_if(library.begin(), library.end(),
        [&track](const Track& t)
        {
            return t.id == track.id;
        });

    if (foundTrack == library.end())
        return false;

    foundTrack->artist = track.artist;
    foundTrack->title = track.title;
    foundTrack->label = track.label;
    foundTrack->bpm = track.bpm;
    foundTrack->colour = track.colour;

    refresh();
    return true;
}

void LibraryManager::addMix(int trackId, int mixId)
{
    if (trackId == mixId)
        return;

    bool trackFound = false;
    bool mixFound = false;

    for (Track& track : library)
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

void LibraryManager::removeTrack(int id)
{
    // Remove any refernce of id
    for (Track& track : library)
    {
        track.mixIds.erase(
            std::remove(track.mixIds.begin(),
                track.mixIds.end(),
                id),
            track.mixIds.end()
        );
    }
    // Remove
    library.erase(
        std::remove_if(library.begin(), library.end(),
            [&](const Track& track)
            {
                return track.id == id;
            }),
        library.end()
    );
    refresh();
}

void LibraryManager::removeMix(int trackId, int mixId)
{
    if (trackId == mixId)
        return;

    bool trackFound = false;
    bool mixFound = false;

    for (Track& track : library)
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

const Track* LibraryManager::getTrackForDisplay(int id)
{
    for (const Track& t : library)
    {
        if (t.id == id)
            return &t;
    }

    return nullptr;
}

const std::vector<Track>& LibraryManager::getCatalogueForDisplay() const
{
    return library;
}

std::vector<int> LibraryManager::getIdLibrary() const
{
    std::vector<int> idLibrary;
    idLibrary.reserve(library.size());

    for (const Track& track : library)
        idLibrary.emplace_back(track.id);

    return idLibrary;
}

// Returns a sorted and searched libary
std::vector<Track> LibraryManager::searchAndSort(const std::string& search, Sort sort)
{
    // TODO: Add bpm search

    std::vector<Track> outputLibrary;
    outputLibrary.reserve(library.size());

    bool doSeach = true;

    if (search.empty())
    {
        outputLibrary = library;
        doSeach = false;
    }

    // Search
    if (doSeach)
    {
        std::vector<std::string> searchWords = TextUtil::split(TextUtil::toLower(search));

        for (const Track& t : library)
        {
            std::string artist = TextUtil::toLower(t.artist);
            std::string title = TextUtil::toLower(t.title);
            std::string label = TextUtil::toLower(t.label);

            bool match = false;

            for (const auto& word : searchWords)
            {
                if (artist.find(word) != std::string::npos ||
                    title.find(word) != std::string::npos ||
                    label.find(word) != std::string::npos)
                {
                    outputLibrary.emplace_back(t);
                    break;
                }
            }
        }
    }

    // Sort
    switch (sort)
    {
    case Sort::Artist:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return TextUtil::toLower(a.artist) < TextUtil::toLower(b.artist);
            });
        break;

    case Sort::Title:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return TextUtil::toLower(a.title) < TextUtil::toLower(b.title);
            });
        break;

    case Sort::Label:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return TextUtil::toLower(a.label) < TextUtil::toLower(b.label);
            });
        break;

    case Sort::BPM: 
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return a.bpm < b.bpm;
            });
        break;
    }
    
    return outputLibrary;
}

// TODO: Could return an emum entry like data OK or MISSING_X...
bool LibraryManager::validateTrackData(Track& track)
{
    // Remove white spaces.
    track.artist = TextUtil::trim(track.artist);
    track.title = TextUtil::trim(track.title);
    track.label = TextUtil::trim(track.label);

    // Ensure artist and title fields are filled.
    if (track.artist.empty() || track.title.empty())
        return false;

    // Don't allow duplicate names (case-insensitive).
    std::string artist = TextUtil::toLower(track.artist);
    std::string title = TextUtil::toLower(track.title);
    std::string label = TextUtil::toLower(track.label);

    // TODO: Check if mix ids are legit?
    for (const Track& t : library)
    {
        if (t.id == track.id)
            continue;

        if (TextUtil::toLower(t.artist) == artist &&
            TextUtil::toLower(t.title) == title &&
            TextUtil::toLower(t.label) == label)
        {
            return false;
        }
    }

    // TODO: Validate mixIds?

    // Ensure bpm is not negative.
    track.bpm = std::max(0.f, track.bpm);
    
    return true;
}