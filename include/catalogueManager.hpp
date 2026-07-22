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
    enum ValidationResult // remove?
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

    enum class TrackSort // remove?
    {
        Artist,
        Title,
        Label,
        BPM,
        Rating
    };

    void load();
    void save() const;
    void refresh(); //old, used?

    void addTrack(Track& inputTrack); //New
    void editTrack(Track& inputTrack); //New
    void removeTrack(int id); //Old
    void addMix(Mix& newMix, int parentTrackId); //new
    void editMix(Mix mix, int parentTrackId); //Old
    void removeMix(int trackId, int mixId); //Old

    int64_t addArtist(std::string name); //New
    int64_t addLabel(std::string name); //New
    int64_t addRelease(std::string name); //New

    void removeRelease(int64_t id); //New

    const Track* getTrackForDisplay(int id); //Old

    const std::vector<Release>& getReleases(); //Not used, will use in release mananger
    const std::vector<Track>& getTracks(); //New? In use

    std::vector<const Track*> getTracksByArtist(int64_t id) const; //New - good
    std::vector<const Track*> getTracksByLabel(int64_t id) const; //New - good
    std::vector<const Track*> getTracksByRelease(int64_t id) const; //New - good



    const std::vector<Track>& getCatalogueForDisplay() const; //Old remove
    std::vector<int> getIdLibrary() const;  //Old, remove?

    std::vector<const Track*> searchAndSort(const std::string& search, TrackSort sort); //Old, needs update?


    Track* findTrackById(int64_t id);                           //Old/new

    const Artist* findArtistById(int64_t id) const;             //New
    const Artist* findArtistByName(std::string name) const;     //New
    const Label* findLabelById(int64_t id) const;               //New
    const Label* findLabelByName(std::string name) const;       //New
    const Release* findReleaseById(int64_t id) const;           //New
    const Release* findReleaseByName(std::string name) const;   //New

protected:
    int64_t lastId = 0;
    std::vector<Artist> artists;
    std::vector<Label> labels;
    std::vector<Release> releases;
    std::vector<Track> tracks;

    ValidationResult ValidateMix(Mix& m, int parentTrackId); //Old news update

    ValidationResult ValidateTrack(Track& t);   //Old might be useless now



    inline int64_t generateId() { return lastId++; }

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


