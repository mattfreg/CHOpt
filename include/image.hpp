/*
 * chopt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020 Raymond Wright
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CHOPT_IMAGE_HPP
#define CHOPT_IMAGE_HPP

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "chart.hpp"
#include "points.hpp"
#include "processed.hpp"
#include "sp.hpp"

struct DrawnRow {
    double start;
    double end;
};

template <typename T> struct DrawnNote {
    double beat;
    double length;
    T colour;
    bool is_sp_note;
};

class ImageBuilder {
private:
    std::vector<DrawnRow> m_rows;
    std::vector<double> m_half_beat_lines;
    std::vector<double> m_beat_lines;
    std::vector<double> m_measure_lines;
    std::vector<std::tuple<double, double>> m_bpms;
    std::vector<std::tuple<double, int, int>> m_time_sigs;
    std::vector<DrawnNote<NoteColour>> m_notes;
    std::vector<DrawnNote<GHLNoteColour>> m_ghl_notes;
    std::vector<int> m_base_values;
    std::vector<int> m_score_values;
    std::vector<double> m_sp_values;
    std::string m_song_name;
    std::string m_artist;
    std::string m_charter;
    std::vector<std::tuple<double, double>> m_green_ranges;
    std::vector<std::tuple<double, double>> m_blue_ranges;
    std::vector<std::tuple<double, double>> m_red_ranges;
    std::vector<std::tuple<double, double>> m_yellow_ranges;
    std::vector<std::tuple<double, double>> m_solo_ranges;

public:
    ImageBuilder(const NoteTrack<NoteColour>& track, int resolution,
                 const SyncTrack& sync_track);
    ImageBuilder(const NoteTrack<GHLNoteColour>& track, int resolution,
                 const SyncTrack& sync_track);
    void add_bpms(const SyncTrack& sync_track, int resolution);
    void add_measure_values(const PointSet& points, const Path& path);
    void add_solo_sections(const NoteTrack<NoteColour>& track, int resolution);
    void add_song_header(const SongHeader& header);
    void add_sp_acts(const PointSet& points, const Path& path);
    void add_sp_phrases(const NoteTrack<NoteColour>& track, int resolution);
    void add_sp_values(const SpData& sp_data);
    void add_time_sigs(const SyncTrack& sync_track, int resolution);

    [[nodiscard]] const std::string& artist() const { return m_artist; }
    [[nodiscard]] const std::vector<int>& base_values() const
    {
        return m_base_values;
    }
    [[nodiscard]] const std::vector<double>& beat_lines() const
    {
        return m_beat_lines;
    }
    [[nodiscard]] const std::vector<std::tuple<double, double>>&
    blue_ranges() const
    {
        return m_blue_ranges;
    }
    [[nodiscard]] const std::vector<std::tuple<double, double>>& bpms() const
    {
        return m_bpms;
    }
    [[nodiscard]] const std::string& charter() const { return m_charter; }
    [[nodiscard]] const std::vector<std::tuple<double, double>>&
    green_ranges() const
    {
        return m_green_ranges;
    }
    [[nodiscard]] const std::vector<double>& half_beat_lines() const
    {
        return m_half_beat_lines;
    }
    [[nodiscard]] const std::vector<double>& measure_lines() const
    {
        return m_measure_lines;
    }
    [[nodiscard]] const std::vector<DrawnNote<NoteColour>>& notes() const
    {
        return m_notes;
    }
    [[nodiscard]] const std::vector<DrawnNote<GHLNoteColour>>& ghl_notes() const
    {
        return m_ghl_notes;
    }
    [[nodiscard]] const std::vector<std::tuple<double, double>>&
    red_ranges() const
    {
        return m_red_ranges;
    }
    [[nodiscard]] const std::vector<DrawnRow>& rows() const { return m_rows; }
    [[nodiscard]] const std::vector<int>& score_values() const
    {
        return m_score_values;
    }
    [[nodiscard]] const std::vector<std::tuple<double, double>>&
    solo_ranges() const
    {
        return m_solo_ranges;
    }
    [[nodiscard]] const std::string& song_name() const { return m_song_name; }
    [[nodiscard]] const std::vector<double>& sp_values() const
    {
        return m_sp_values;
    }
    [[nodiscard]] const std::vector<std::tuple<double, int, int>>&
    time_sigs() const
    {
        return m_time_sigs;
    }
    [[nodiscard]] const std::vector<std::tuple<double, double>>&
    yellow_ranges() const
    {
        return m_yellow_ranges;
    }
};

class ImageImpl;

class Image {
private:
    std::unique_ptr<ImageImpl> m_impl;

public:
    explicit Image(const ImageBuilder& builder);
    ~Image();
    Image(const Image&) = delete;
    Image(Image&& image) noexcept;
    Image& operator=(const Image&) = delete;
    Image& operator=(Image&& image) noexcept;

    void save(const char* filename) const;
};

#endif
