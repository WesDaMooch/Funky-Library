#include "../include/catalogueManager.hpp"
#include <iostream>
#include <fstream>


// ADL serialization
void to_json(json& j, const Mix& m)
{
    j = {
        {"id", m.id},
        {"direction", static_cast<int>(m.direction)},
        {"rating", m.rating},
        {"pitch", m.pitch },
        {"note", m.note }
    };
}

void from_json(const json& j, Mix& m)
{
    m.id = j.at("id").get<int>();

    int Jdirection = j.value("direction", static_cast<int>(MixDirection::InAndOut));
    m.direction = static_cast<MixDirection>(Jdirection);

    m.rating = j.value("rating", 0);
    m.pitch = j.value("pitch", 0);
    m.note = j.value("note", "");
}

void to_json(json& j, const Track& t)
{
    j = {
        {"id", t.id},
        {"artist", t.artist},
        {"title", t.title},
        {"label", t.label},
        {"release", t.release},
        {"position", t.position},
        {"bpm", t.bpm},
        {"rating", t.rating},
        {"colour", t.colour},
        {"mix", t.mix}
    };
}

// TODO: Use value for all but id, artist and title
void from_json(const json& j, Track& t)
{
    try
    {               
        t.id        = j.at("id").get<int>();
        t.artist    = j.at("artist").get<std::string>();
        t.title     = j.at("title").get<std::string>();
        t.label     = j.at("label").get<std::string>();
        t.release   = j.at("release").get<std::string>();
        t.position  = j.at("position").get<std::string>();
        t.bpm       = j.at("bpm").get<float>();
        t.rating    = j.at("rating").get<int>();
        t.colour    = j.at("colour").get<std::array<uint8_t, 3>>();
        t.mix       = j.value("mix", std::vector<Mix>{});
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

LibraryManager::ValidationResult LibraryManager::addTrack(const Track& newTrack)
{
    Track track = newTrack;

    ValidationResult result = ValidateTrack(track);
    
    if (result != ValidationResult::ValidTrack)
        return result;

    track.id = lastTrackId++;
    track.mix.clear();

    // TODO: use std::move? catalogue.emplace_back(std::move(t));
    library.emplace_back(track);
    refresh();

    return result;
}

LibraryManager::ValidationResult LibraryManager::editTrack(const Track& editedTrack)
{ 
    // TODO: could just use a non const edited track?
    Track track = editedTrack;

    ValidationResult result = ValidateTrack(track);

    if (result != ValidationResult::ValidTrack)
        return result;
    
    Track* foundTrack = getTrack(editedTrack.id);

    if (foundTrack == nullptr)
        return ValidationResult::TrackNotFound;

    foundTrack->artist = track.artist;
    foundTrack->title = track.title;
    foundTrack->label = track.label;
    foundTrack->release = track.release;
    foundTrack->position = track.position;
    foundTrack->bpm = track.bpm;
    foundTrack->rating = track.rating;
    foundTrack->colour = track.colour;
    
    save();
    return result;
}

void LibraryManager::addMix(int mixId, int parentTrackId)
{
    if (mixId == parentTrackId)
        return;

    Track* parentTrack = getTrack(parentTrackId);
    Track* childTrack = getTrack(mixId);

    if ((parentTrack == nullptr) || (childTrack == nullptr))
        return;

    bool changed = false;

    // Add child track to parent mix list
    bool parentFound = false;
    for (const Mix& m : parentTrack->mix)
    {
        if (m.id == mixId)
        {
            parentFound = true;
            break;
        }
    }

    if (!parentFound)
    {
        parentTrack->mix.emplace_back(Mix{ mixId, MixDirection::InAndOut, 0 });
        changed = true;
    }

    // Add parent track to child mix list
    bool childFound = false;
    for (const Mix& m : childTrack->mix)
    {
        if (m.id == parentTrackId)
        {
            childFound = true;
            break;
        }
    }

    if (!childFound)
    {
        childTrack->mix.emplace_back(Mix{ parentTrackId, MixDirection::InAndOut, 0 });
        changed = true;
    }

    if (changed)
        save();
}


void LibraryManager::editMix(Mix mix, int parentTrackId)
{
    ValidationResult result = ValidateMix(mix, parentTrackId);

    if (result != ValidationResult::ValidMix)
        return;

    Track* parentTrack = getTrack(parentTrackId);
    Track* childTrack = getTrack(mix.id);

    if ((parentTrack == nullptr) || (childTrack == nullptr))
        return;

    bool changed = false;

    // Edit parent track mix 
    bool parentFound = false;
    for (Mix& m : parentTrack->mix)
    {
        if (m.id == mix.id)
        {
            m.direction = mix.direction;
            m.rating = mix.rating;
            m.pitch = mix.pitch;
            m.note = mix.note;
            parentFound = true;
            changed = true;
            break;
        }
    }

    if (!parentFound)
    {
        parentTrack->mix.emplace_back(mix);
        changed = true;
    }

    // Edit child track mix
    bool childFound = false;
    for (Mix& m : childTrack->mix)
    {
        if (m.id == parentTrackId)
        {
            m.direction = InvertMixDirection(mix.direction);

            if (mix.direction != MixDirection::InAndOut)
            {
                m.rating = mix.rating;
                m.pitch = mix.pitch;
                m.note = mix.note;
            }
            else
            {
                m.pitch = -mix.pitch;
            }

            childFound = true;
            changed = true;
            break;
        }
    }

    if (!childFound)
    {
        childTrack->mix.emplace_back(Mix{ parentTrackId, InvertMixDirection(mix.direction), 0 });
        changed = true;
    }

    if (changed)
        save();
}

void LibraryManager::removeTrack(int id)
{
    // Remove any refernce of ID.
    for (Track& track : library)
    {
        if (track.id == id)
            continue;

        track.mix.erase(
            std::remove_if(
                track.mix.begin(),
                track.mix.end(),
                [&](const Mix& m)
                {
                    return m.id == id;
                }),
            track.mix.end()
        );
    }

    // Remove track.
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

    auto removeMixFromTrack = [](Track& track, int idToRemove)
    {
        track.mix.erase(
            std::remove_if(
                track.mix.begin(),
                track.mix.end(),
                [&](const Mix& m)
                {
                    return m.id == idToRemove;
                }),
            track.mix.end()
        );
    };

    bool trackFound = false;
    bool mixFound = false;

    for (Track& track : library)
    {
        if (track.id == trackId)
        {
            removeMixFromTrack(track, mixId);
            trackFound = true;
        }

        if (track.id == mixId)
        {
            removeMixFromTrack(track, trackId);
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


Track* LibraryManager::getTrack(int id)
{
    // TODO: Use find or find_if?
    for (Track& t : library)
    {
        if (t.id == id)
            return &t;
    }

    return nullptr;
}


std::vector<const Track*> LibraryManager::getLabel(const std::string& labelString) const
{
    // Find all tracks that share a label (case non-sensative).
    std::vector<const Track*> tracksInLabel{};
    std::string labelName = TextUtil::ToLower(labelString);

    for (const Track& t : library)
    {
        if (labelName == TextUtil::ToLower(t.label))
            tracksInLabel.emplace_back(&t);
    }

    return tracksInLabel;
}

std::vector<const Track*> LibraryManager::getRelease(const std::string& releaseString) const
{
    // Find all tracks that share a release (case non-sensative).
    std::vector<const Track*> tracksInRelease{};
    std::string releaseName = TextUtil::ToLower(releaseString);

    for (const Track& t : library)
    {
        if (releaseName == TextUtil::ToLower(t.release))
            tracksInRelease.emplace_back(&t);
    }

    // TODO: Sort release by position (if it has one)
    //std::sort(tracksInRelease.begin(), tracksInRelease.end());

    return tracksInRelease;
}

std::vector<const Track*> LibraryManager::getTag(const std::string& tagString) const
{
    // Find all tracks that share a tag.
    std::vector<const Track*> tracksInTag{};
    return tracksInTag;
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
std::vector<Track> LibraryManager::searchAndSort(const std::string& search, TrackSort sort)
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
        std::vector<std::string> searchWords = TextUtil::Split(TextUtil::ToLower(search));

        for (const Track& t : library)
        {
            std::string artist = TextUtil::ToLower(t.artist);
            std::string title = TextUtil::ToLower(t.title);
            std::string label = TextUtil::ToLower(t.label);

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
    case TrackSort::Artist:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return TextUtil::ToLower(a.artist) < TextUtil::ToLower(b.artist);
            });
        break;

    case TrackSort::Title:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return TextUtil::ToLower(a.title) < TextUtil::ToLower(b.title);
            });
        break;

    case TrackSort::Label:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return TextUtil::ToLower(a.label) < TextUtil::ToLower(b.label);
            });
        break;

    case TrackSort::BPM: 
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return a.bpm < b.bpm;
            });
        break;

    case TrackSort::Rating:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return a.rating < b.rating;
            });
        break;
    }
    
    return outputLibrary;
}


LibraryManager::ValidationResult LibraryManager::ValidateMix(Mix& m, int parentTrackId)
{
    // Mix ID is the same as the parent track.
    if (m.id == parentTrackId)
        return ValidationResult::InvalidMix;

    // No track found with given mix ID.
    bool matchingTrackFound = false;

    for (const Track& t : library)
    {
        if (m.id == t.id)
        {
            matchingTrackFound = true;
            break;
        }
    }

    if (!matchingTrackFound)
        return ValidationResult::InvalidMix;

    // Ensure direction.
    int direction = std::clamp(static_cast<int>(m.direction), 0, static_cast<int>(MixDirection::NumDirections) - 1);
    m.direction = static_cast<MixDirection>(direction);

    // Ensure rating is between 0 & 5.
    m.rating = std::clamp(m.rating, 0, 5);

    // Ensure pitch adjust in between -99 & 99.
    m.pitch = std::clamp(m.pitch, -99, 99);

    // Remove space padding from note.
    m.note = TextUtil::Trim(m.note);

    return ValidationResult::ValidMix;
}


LibraryManager::ValidationResult LibraryManager::ValidateTrack(Track& track)
{
    // Remove white spaces from text.
    track.artist = TextUtil::Trim(track.artist);
    track.title = TextUtil::Trim(track.title);
    track.label = TextUtil::Trim(track.label);
    track.release = TextUtil::Trim(track.release);
    track.position = TextUtil::Trim(track.position);

    // Ensure artist and title fields are filled.
    if (track.artist.empty())
        return ValidationResult::MissingArtist;
    else if(track.title.empty())
        return ValidationResult::MissingTitle;

    // Don't allow duplicate names (case-insensitive).
    std::string artist = TextUtil::ToLower(track.artist);
    std::string title = TextUtil::ToLower(track.title);

    for (const Track& t : library)
    {
        if (t.id == track.id)
            continue;

        if (TextUtil::ToLower(t.artist) == artist &&
            TextUtil::ToLower(t.title) == title)
        {
            return ValidationResult::DuplicateTrack;
        }
    }

    // Ensure bpm is not negative.
    track.bpm = std::max(0.f, track.bpm);

    // Ensure rating is between 0 & 5.
    track.rating = std::clamp(track.rating, 0, 5);

    // Validate mix.
    std::unordered_set<int> uniqueMixIds;
    for (Mix& mix : track.mix)
    { 
        // Duplicate mix
        if (!uniqueMixIds.insert(mix.id).second)
            return ValidationResult::InvalidMix;

        ValidationResult mixValidationResult = ValidateMix(mix, track.id);

        if (mixValidationResult != ValidationResult::ValidMix)
            return mixValidationResult;
    }
    
    return ValidationResult::ValidTrack;
}