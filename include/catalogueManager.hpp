#pragma once

#include "../external/json/json.hpp"
#include "constants.hpp"
#include "StringUtils.hpp"
//#include "tags.h"

#include <string>
#include <array>
#include <vector>
#include <unordered_set>
#include <algorithm>

// TODO: used unsorted set to make a index lookup table

using json = nlohmann::json;

enum class MixDirection
{
    In,
    Out,
    InAndOut,
    NumDirections
};

struct Artist
{
    int64_t id = -1;
    std::string name;
};

struct Label
{
    int64_t id = -1;
    std::string name;
};

struct Release
{
    int64_t id = -1;
    std::string name;
};


struct Mix
{
    int64_t otherTrackId = -1;
    MixDirection direction = MixDirection::InAndOut;
    int rating = 0;
    int pitch = 0;
    std::string note;
};

enum class Format
{
    None,
    Vinyl,
    CD,
    File
};

struct Track
{
    int64_t id = -1;
    int64_t artistId = -1;
    std::string title;
    int64_t labelId = -1;
    int64_t releaseId = -1;
    std::string position;
    float bpm = 0;
    int rating = 0;
    //Format format = Format::None;
    //std::string note;
    std::array<uint8_t, 3> colour = Colour::RGB_DEFAULT;
    //std::vector<std::string> tag{}
    std::vector<Mix> mix;
};

void to_json(json& j, const Artist& a);
void from_json(const json& j, Artist& a);

void to_json(json& j, const Label& l);
void from_json(const json& j, Label& l);

void to_json(json& j, const Release& r);
void from_json(const json& j, Release& r);

void to_json(json& j, const Mix& m);
void from_json(const json& j, Mix& m);

void to_json(json& j, const Track& t);
void from_json(const json& j, Track& t);

class Library
{
public:
    enum ValidationResult
    {
        None,
        ValidTrack,
        ValidMix,
        MissingArtist,
        MissingTitle,
        TrackNotFound,
        DuplicateTrack,
        InvalidMix
    };

    enum class TrackSort
    {
        Artist,
        Title,
        Label,
        BPM,
        Rating
    };

    void load();
    void save() const;
    void refresh();

    void addTrack(Track& newTrack);
    ValidationResult editTrack(const Track& editedTrack);
    void removeTrack(int id);
    void addMix(int mixId, int parentTrackId);
    void editMix(Mix mix, int parentTrackId);
    void removeMix(int trackId, int mixId);

    int64_t addArtist(std::string name);
    int64_t addLabel(std::string name);
    int64_t addRelease(std::string name);

    const Track* getTrackForDisplay(int id);

    std::vector<const Track*> getLabel(const std::string& labelString) const;
    std::vector<const Track*> getTracksInRelease(int64_t id) const;
    const std::vector<Release>& getReleases();
    const std::vector<Track>& getTracks();

    Track* getTrack(int id); // TODO: rename findTrackById
    const std::vector<Track>& getCatalogueForDisplay() const;
    std::vector<int> getIdLibrary() const;

    std::vector<Track> searchAndSort(const std::string& search, TrackSort sort); // TODO: use imgui text filtering?

    const Artist* findArtistById(int64_t id) const;
    const Artist* findArtistByName(std::string name) const;
    const Label* findLabelByName(std::string name) const;

    const Release* findReleaseById(int64_t id) const;
    const Release* findReleaseByName(std::string name) const;

protected:
    int64_t lastId = 0;
    std::vector<Artist> artists;
    std::vector<Label> labels;
    std::vector<Release> releases;
    std::vector<Track> tracks;

    ValidationResult ValidateMix(Mix& m, int parentTrackId);

    ValidationResult ValidateTrack(Track& t);



    inline int64_t generateId()
    {
        return lastId++;
    }

    inline MixDirection InvertMixDirection(MixDirection direction)
    {
        switch (direction)
        {
        case MixDirection::In:
            return MixDirection::Out;
        case MixDirection::Out:
            return MixDirection::In;
        case MixDirection::InAndOut:
            return MixDirection::InAndOut;
        default:                      
            return MixDirection::InAndOut;
        }
    }

};


