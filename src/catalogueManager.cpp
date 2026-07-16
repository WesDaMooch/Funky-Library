#include "../include/catalogueManager.hpp"
#include <iostream>
#include <fstream>


// ADL serialization

// Artist
void to_json(json& j, const Artist& a)
{
    j = {
        {"id", a.id},
        {"name", a.name}
    };
}

void from_json(const json& j, Artist& a)
{
    a.id = j.at("id").get<int64_t>();
    a.name = j.at("name").get<std::string>();
}

// Label
void to_json(json& j, const Label& l)
{
    j = {
        {"id", l.id},
        {"name", l.name}
    };
}

void from_json(const json& j, Label& l)
{
    l.id = j.at("id").get<int64_t>();
    l.name = j.at("name").get<std::string>();
}

// Release
void to_json(json& j, const Release& r)
{
    j = {
        {"id", r.id},
        {"name", r.name}
    };
}

void from_json(const json& j, Release& r)
{
    r.id = j.at("id").get<int64_t>();
    r.name = j.at("name").get<std::string>();
}

// Mix
void to_json(json& j, const Mix& m)
{
    j = {
        {"id", m.otherTrackId},
        {"direction", static_cast<int>(m.direction)},
        {"rating", m.rating},
        {"pitch", m.pitch },
        {"note", m.note }
    };
}

void from_json(const json& j, Mix& m)
{
    m.otherTrackId = j.at("id").get<int64_t>();

    int Jdirection = j.value("direction", static_cast<int>(MixDirection::InAndOut));
    m.direction = static_cast<MixDirection>(Jdirection);

    m.rating = j.value("rating", 0);
    m.pitch = j.value("pitch", 0);
    m.note = j.value("note", "");
}

// Track
void to_json(json& j, const Track& t)
{
    j = {
        {"id", t.id},
        {"artistId", t.artistId},
        {"title", t.title},
        {"labelId", t.labelId},
        {"releaseId", t.releaseId},
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
        t.id        = j.at("id").get<int64_t>();
        t.artistId  = j.at("artistId").get<int64_t>();
        t.title     = j.at("title").get<std::string>();
        t.labelId   = j.at("labelId").get<int64_t>();
        t.releaseId = j.at("releaseId").get<int64_t>();
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
void Library::load()
{
    std::ifstream file("catalogue.json");

    if (!file.is_open())
        return;

    json j;
    file >> j;

    file.close();

    artists = j.value("artists", std::vector<Artist>{});
    labels = j.value("labels", std::vector<Label>{});
    releases = j.value("releases", std::vector<Release>{});
    tracks = j.value("tracks", std::vector<Track>{});

    // Scan library and update id
    for (const auto& artist : artists)
        lastId = std::max(lastId, artist.id + 1);

    for (const auto& label : labels)
        lastId = std::max(lastId, label.id + 1);

    for (const auto& release : releases)
        lastId = std::max(lastId, release.id + 1);

    for (const auto& track : tracks)
        lastId = std::max(lastId, track.id + 1);
}

void Library::save() const
{
    json j;
    j["artists"] = artists;
    j["labels"] = labels;
    j["releases"] = releases;
    j["tracks"] = tracks;

    std::ofstream file("catalogue.json");
    file << j.dump(4);

    file.close();
}

void Library::refresh()
{
    save();
    load();
}

void Library::addTrack(Track& inputTrack)
{
    inputTrack.id = generateId();
    inputTrack.title = TextUtil::Trim(inputTrack.title);
    inputTrack.position = TextUtil::Trim(inputTrack.position);
    inputTrack.bpm = std::clamp(inputTrack.bpm, 0.0f, 999.0f);
    inputTrack.rating = std::clamp(inputTrack.rating, 0, 5);
    tracks.emplace_back(inputTrack);

    save();
}

void Library::editTrack(Track& inputTrack)
{ 
    auto* trackToEdit = findTrackById(inputTrack.id);

    if (trackToEdit == nullptr)
        return;

    trackToEdit->artistId = inputTrack.artistId;
    trackToEdit->title = TextUtil::Trim(inputTrack.title);
    trackToEdit->labelId = inputTrack.labelId;
    trackToEdit->releaseId = inputTrack.releaseId;
    trackToEdit->position = TextUtil::Trim(inputTrack.position);
    trackToEdit->bpm = std::clamp(inputTrack.bpm, 0.0f, 999.0f);
    trackToEdit->rating = std::clamp(inputTrack.rating, 0, 5);
    trackToEdit->colour = inputTrack.colour;

    save();
    /*
    // TODO: could just use a non const edited track?
    Track track = editedTrack;

    ValidationResult result = ValidateTrack(track);

    if (result != ValidationResult::ValidTrack)
        return result;
    
    Track* foundTrack = findTrackById(editedTrack.id);

    if (foundTrack == nullptr)
        return ValidationResult::TrackNotFound;

    //foundTrack->artist = track.artist;
    foundTrack->title = track.title;
    foundTrack->labelId = track.labelId;
    foundTrack->releaseId = track.releaseId;
    foundTrack->position = track.position;
    foundTrack->bpm = track.bpm;
    foundTrack->rating = track.rating;
    foundTrack->colour = track.colour;
    
    save();
    return result;
    */
}

void Library::addMix(Mix& newMix, int parentTrackId)
{
    // Prevent self mixing
    if (newMix.otherTrackId == parentTrackId)
        return;

    auto* thisTrack = findTrackById(parentTrackId);
    auto* otherTrack = findTrackById(newMix.otherTrackId);

    if (thisTrack == nullptr || otherTrack == nullptr)
        return;

    // Ensure direction
    int direction = std::clamp(static_cast<int>(newMix.direction), 0, static_cast<int>(MixDirection::NumDirections) - 1);
    newMix.direction = static_cast<MixDirection>(direction);

    // Ensure rating is between 0 & 5
    newMix.rating = std::clamp(newMix.rating, 0, 5);

    // Ensure pitch adjust in between -99 & 99
    newMix.pitch = std::clamp(newMix.pitch, -999, 999);

    // Remove space padding from note
    newMix.note = TextUtil::Trim(newMix.note);

    // Add mix to this track
    thisTrack->mix.push_back(newMix);

    // TODO: Prevent duplicate mixes? this happens in main app?
    

    // Add mix to other track
    Mix otherMix;
    otherMix.otherTrackId = thisTrack->id;
    
    if (newMix.direction != MixDirection::InAndOut)
    {
        otherMix.rating = newMix.rating;
        otherMix.pitch = otherMix.pitch;
        otherMix.note = otherMix.note;
    }

    otherMix.direction = InvertMixDirection(newMix.direction);
    otherTrack->mix.push_back(otherMix);

    


    /*
    // Prevent self mixing
    if (parentTrackId = newMix.otherTrackId)
        return;

    auto* parentTrack = findTrackById(parentTrackId);
    auto* otherTrack = findTrackById(newMix.otherTrackId);

    if ((parentTrack == nullptr) || (otherTrack == nullptr))
        return;

    bool changed = false;

    // Add other track to parent mix list
    bool parentFound = false;
    for (const Mix& mix : parentTrack->mix)
    {
        if (mix.otherTrackId == newMix.otherTrackId)
        {
            parentFound = true;
            break;
        }
    }

    if (!parentFound)
    {
        parentTrack->mix.emplace_back(newMix);
        changed = true;
    }

    // Add parent track to child mix list
    bool otherTrackFound = false;
    for (const Mix& m : otherTrack->mix)
    {
        if (m.otherTrackId == parentTrackId)
        {
            otherTrackFound = true;
            break;
        }
    }

    if (!otherTrackFound)
    {
        otherTrack->mix.emplace_back(newMix);
        changed = true;
    }

    if (changed)
        save();

    */

    ///
    /*
    if (mixId == parentTrackId)
        return;

    Track* parentTrack = findTrackById(parentTrackId);
    Track* childTrack = findTrackById(mixId);

    if ((parentTrack == nullptr) || (childTrack == nullptr))
        return;

    bool changed = false;

    // Add child track to parent mix list
    bool parentFound = false;
    for (const Mix& m : parentTrack->mix)
    {
        if (m.otherTrackId == mixId)
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
        if (m.otherTrackId == parentTrackId)
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
        */
}


void Library::editMix(Mix mix, int parentTrackId)
{
    ValidationResult result = ValidateMix(mix, parentTrackId);

    if (result != ValidationResult::ValidMix)
        return;

    Track* parentTrack = findTrackById(parentTrackId);
    Track* childTrack = findTrackById(mix.otherTrackId);

    if ((parentTrack == nullptr) || (childTrack == nullptr))
        return;

    bool changed = false;

    // Edit parent track mix 
    bool parentFound = false;
    for (Mix& m : parentTrack->mix)
    {
        if (m.otherTrackId == mix.otherTrackId)
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
        if (m.otherTrackId == parentTrackId)
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

void Library::removeTrack(int id)
{
    // Remove any refernce of ID.
    for (Track& track : tracks)
    {
        if (track.id == id)
            continue;

        track.mix.erase(
            std::remove_if(
                track.mix.begin(),
                track.mix.end(),
                [&](const Mix& m)
                {
                    return m.otherTrackId == id;
                }),
            track.mix.end()
        );
    }

    // Remove track.
    tracks.erase(
        std::remove_if(tracks.begin(), tracks.end(),
            [&](const Track& track)
            {
                return track.id == id;
            }),
        tracks.end()
    );
    refresh();
}

void Library::removeMix(int trackId, int mixId)
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
                    return m.otherTrackId == idToRemove;
                }),
            track.mix.end()
        );
    };

    bool trackFound = false;
    bool mixFound = false;

    for (Track& track : tracks)
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

int64_t Library::addArtist(std::string name)
{
    artists.push_back({ generateId(), TextUtil::Trim(name) });
    return artists.back().id;
}

int64_t Library::addLabel(std::string name)
{
    labels.push_back({ generateId(), TextUtil::Trim(name) });
    return labels.back().id;
}

int64_t Library::addRelease(std::string name)
{
    releases.push_back({ generateId(), TextUtil::Trim(name) });
    return releases.back().id;
}


const Track* Library::getTrackForDisplay(int id)
{
    for (const Track& t : tracks)
    {
        if (t.id == id)
            return &t;
    }

    return nullptr;
}


Track* Library::findTrackById(int64_t id)
{
    // TODO: Use find or find_if?
    for (Track& t : tracks)
    {
        if (t.id == id)
            return &t;
    }

    return nullptr;
}

std::vector<const Track*> Library::getTracksByArtist(int64_t id) const
{
    std::vector<const Track*> tracksByArtist{};

    for (const auto& track : tracks)
    {
        if (track.artistId == id)
            tracksByArtist.push_back(&track);
    }

    return tracksByArtist;
}


std::vector<const Track*> Library::getTracksByLabel(int64_t id) const
{
    std::vector<const Track*> tracksByLabel{};

    for (const auto& track : tracks)
    {
        if (track.labelId == id)
            tracksByLabel.push_back(&track);
    }

    return tracksByLabel;
}

std::vector<const Track*> Library::getTracksByRelease(int64_t id) const
{
    std::vector<const Track*> tracksByRelease{};

    for (const auto& track : tracks)
    {
        if (track.releaseId == id)
            tracksByRelease.push_back(&track);
    }

    return tracksByRelease;
}


const std::vector<Release>& Library::getReleases() { return releases; }

const std::vector<Track>& Library::getTracks() { return tracks; }




const std::vector<Track>& Library::getCatalogueForDisplay() const
{
    return tracks;
}

std::vector<int> Library::getIdLibrary() const
{
    std::vector<int> idLibrary;
    idLibrary.reserve(tracks.size());

    for (const Track& track : tracks)
        idLibrary.emplace_back(track.id);

    return idLibrary;
}


// Returns a sorted and searched libary
// TODO: use imgui text filtering?
std::vector<const Track*> Library::searchAndSort(const std::string& search, TrackSort sort)
{
    // TODO: Add bpm search
    // Add release and label search

    std::vector<const Track*> outputLibrary;
    outputLibrary.reserve(tracks.size());

    bool doSeach = true;

    if (search.empty())
    {
        for (const auto& t : tracks)
            outputLibrary.push_back(&t);
        
        doSeach = false;
    }

    // Search
    if (doSeach)
    {
        std::vector<std::string> searchWords = TextUtil::Split(TextUtil::ToLower(search));

        for (const Track& t : tracks)
        {
            //std::string artist = TextUtil::ToLower(t.artist);
            std::string artist = "";

            std::string title = TextUtil::ToLower(t.title);

            for (const auto& word : searchWords)
            {
                if (artist.find(word) != std::string::npos ||
                    title.find(word) != std::string::npos)
                {
                    outputLibrary.emplace_back(&t);
                    break;
                }
            }
        }
    }

    // Sort
    switch (sort)
    {
        /*
    case TrackSort::Artist:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track& a, const Track& b)
            {
                return TextUtil::ToLower(a.artist) < TextUtil::ToLower(b.artist);
            });
        break;
        */
    case TrackSort::Title:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track* a, const Track* b)
            {
                return TextUtil::ToLower(a->title) < TextUtil::ToLower(b->title);
            });
        break;

    case TrackSort::BPM: 
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track* a, const Track* b)
            {
                return a->bpm < b->bpm;
            });
        break;

    case TrackSort::Rating:
        std::sort(outputLibrary.begin(), outputLibrary.end(),
            [](const Track* a, const Track* b)
            {
                return a->rating < b->rating;
            });
        break;
    }
    
    return outputLibrary;
}

const Artist* Library::findArtistById(int64_t id) const
{
    for (const auto& artist : artists)
    {
        if (id == artist.id)
            return &artist;
    }

    return nullptr;
}

const Artist* Library::findArtistByName(std::string name) const
{
    name = TextUtil::ToLower(TextUtil::Trim(name));
    // TODO: create map for idx and names
    for (const auto& artist : artists)
    {
        if (name == TextUtil::ToLower(artist.name))
            return &artist;
    }

    return nullptr;
}

const Label* Library::findLabelByName(std::string name) const
{
    name = TextUtil::ToLower(TextUtil::Trim(name));
    // TODO: create map for idx and names
    for (const auto& label : labels)
    {
        if (name == TextUtil::ToLower(label.name))
            return &label;
    }

    return nullptr;
}

const Label* Library::findLabelById(int64_t id) const
{
    for (const auto& label : labels)
    {
        if (id == label.id)
            return &label;
    }

    return nullptr;
}

const Release* Library::findReleaseById(int64_t id) const
{
    for (const auto& release : releases)
    {
        if (id == release.id)
            return &release;
    }

    return nullptr;
}

const Release* Library::findReleaseByName(std::string name) const
{
    name = TextUtil::ToLower(TextUtil::Trim(name));
    // TODO: create map for idx and names
    for (const auto& release : releases)
    {
        if (name == TextUtil::ToLower(release.name))
            return &release;
    }

    return nullptr;
}


Library::ValidationResult Library::ValidateMix(Mix& m, int parentTrackId)
{
    // Mix ID is the same as the parent track.
    if (m.otherTrackId == parentTrackId)
        return ValidationResult::InvalidMix;

    // No track found with given mix ID.
    bool matchingTrackFound = false;

    for (const Track& t : tracks)
    {
        if (m.otherTrackId == t.id)
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


Library::ValidationResult Library::ValidateTrack(Track& track)
{
    // Remove white spaces from text.
    //track.artist = TextUtil::Trim(track.artist);
    track.title = TextUtil::Trim(track.title);
    track.position = TextUtil::Trim(track.position);

    // Ensure artist and title fields are filled.
    //if (track.artist.empty())
    //    return ValidationResult::MissingArtist;
    //else if(track.title.empty())
    //    return ValidationResult::MissingTitle;

    // Don't allow duplicate names (case-insensitive).
    //std::string artist = TextUtil::ToLower(track.artist);
    std::string title = TextUtil::ToLower(track.title);

    for (const Track& t : tracks)
    {
        if (t.id == track.id)
            continue;
        /*
        if (TextUtil::ToLower(t.artist) == artist &&
            TextUtil::ToLower(t.title) == title)
        {
            return ValidationResult::DuplicateTrack;
        }
        */
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
        if (!uniqueMixIds.insert(mix.otherTrackId).second)
            return ValidationResult::InvalidMix;

        ValidationResult mixValidationResult = ValidateMix(mix, track.id);

        if (mixValidationResult != ValidationResult::ValidMix)
            return mixValidationResult;
    }
    
    return ValidationResult::ValidTrack;
}