#pragma once

#include "../external/json/json.hpp"
#include "UiConstants.hpp"
#include "StringUtils.hpp"
//#include "tags.h"

#include <string>
#include <array>
#include <vector>
#include <unordered_set>
#include <algorithm>


// TODO: rename library manager
// TODO: used unsorted set to make a index lookup table

using json = nlohmann::json;

enum class MixDirection
{
    In,
    Out,
    InAndOut,
    NumDirections
};

struct Mix
{
    int id = -1;
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
    int id = -1;
    std::string artist;
    std::string title;
    std::string label;
    std::string release;
    std::string position;
    float bpm = 0;
    int rating = 0;
    //Format format = Format::None;
    //std::string note;
    std::array<uint8_t, 3> colour = Ui::Colour::RGB_DEFAULT;
    //std::vector<std::string> tag{}
    std::vector<Mix> mix;
};

void to_json(json& j, const Mix& m);
void from_json(const json& j, Mix& m);

void to_json(json& j, const Track& t);
void from_json(const json& j, Track& t);

class LibraryManager
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

    enum TrackSort
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
    ValidationResult addTrack(const Track& newTrack);
    ValidationResult editTrack(const Track& editedTrack);
    void removeTrack(int id);
    void addMix(int mixId, int parentTrackId);
    void editMix(Mix mix, int parentTrackId);
    void removeMix(int trackId, int mixId);

    const Track* getTrackForDisplay(int id);

    std::vector<const Track*> getLabel(const std::string& labelString) const;
    std::vector<const Track*> getRelease(const std::string& releaseString) const;
    std::vector<const Track*> getTag(const std::string& tagString) const;

    Track* getTrack(int id);
    const std::vector<Track>& getCatalogueForDisplay() const;
    std::vector<int> getIdLibrary() const;

    std::vector<Track> searchAndSort(const std::string& search, TrackSort sort);

protected:
    std::vector<Track> library;
    int lastTrackId = 0;

    ValidationResult ValidateMix(Mix& m, int parentTrackId);

    ValidationResult ValidateTrack(Track& t);

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


