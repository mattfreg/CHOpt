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

#include <algorithm>
#include <array>
#include <cmath>

#include "cimg_wrapper.hpp"

#include "image.hpp"

using namespace cimg_library;

constexpr int BEAT_WIDTH = 60;
constexpr int LEFT_MARGIN = 31;
constexpr int MARGIN = 32;
constexpr int MAX_BEATS_PER_LINE = 16;
constexpr int MEASURE_HEIGHT = 61;
constexpr float OPEN_NOTE_OPACITY = 0.5F;
constexpr int DIST_BETWEEN_MEASURES = MEASURE_HEIGHT + MARGIN;

static double get_beat_rate(const SyncTrack& sync_track, int resolution,
                            double beat)
{
    constexpr double BASE_BEAT_RATE = 4.0;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return BASE_BEAT_RATE;
    }
    --ts;
    return BASE_BEAT_RATE * ts->numerator / ts->denominator;
}

static int get_numer(const SyncTrack& sync_track, int resolution, double beat)
{
    constexpr int BASE_NUMERATOR = 4;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return BASE_NUMERATOR;
    }
    --ts;
    return ts->numerator;
}

static double get_denom(const SyncTrack& sync_track, int resolution,
                        double beat)
{
    constexpr double BASE_BEAT_RATE = 4.0;

    auto ts = std::find_if(
        sync_track.time_sigs().cbegin(), sync_track.time_sigs().cend(),
        [=](const auto& x) { return x.position > resolution * beat; });
    if (ts == sync_track.time_sigs().cbegin()) {
        return 1.0;
    }
    --ts;
    return BASE_BEAT_RATE / ts->denominator;
}

static std::vector<DrawnNote> drawn_notes(const NoteTrack& track,
                                          int resolution)
{
    std::vector<DrawnNote> notes;

    for (const auto& note : track.notes()) {
        auto beat = note.position / static_cast<double>(resolution);
        auto length = note.length / static_cast<double>(resolution);
        auto is_sp_note = false;
        for (const auto& phrase : track.sp_phrases()) {
            if (note.position >= phrase.position
                && note.position < phrase.position + phrase.length) {
                is_sp_note = true;
            }
        }
        notes.push_back({beat, length, note.colour, is_sp_note});
    }

    return notes;
}

static std::vector<DrawnRow> drawn_rows(const NoteTrack& track, int resolution,
                                        const SyncTrack& sync_track)
{
    int max_pos = 0;
    for (const auto& note : track.notes()) {
        auto length = note.position + note.length;
        if (length > max_pos) {
            max_pos = length;
        }
    }

    const auto max_beat = max_pos / static_cast<double>(resolution);
    auto current_beat = 0.0;
    std::vector<DrawnRow> rows;

    while (current_beat <= max_beat) {
        auto row_length = 0.0;
        while (true) {
            auto contribution = get_beat_rate(sync_track, resolution,
                                              current_beat + row_length);
            if (contribution > MAX_BEATS_PER_LINE && row_length == 0.0) {
                // Break up a measure that spans more than a full row.
                while (contribution > MAX_BEATS_PER_LINE) {
                    rows.push_back(
                        {current_beat, current_beat + MAX_BEATS_PER_LINE});
                    current_beat += MAX_BEATS_PER_LINE;
                    contribution -= MAX_BEATS_PER_LINE;
                }
            }
            if (contribution + row_length > MAX_BEATS_PER_LINE) {
                break;
            }
            row_length += contribution;
            if (current_beat + row_length > max_beat) {
                break;
            }
        }
        rows.push_back({current_beat, current_beat + row_length});
        current_beat += row_length;
    }

    return rows;
}

DrawingInstructions::DrawingInstructions(const NoteTrack& track, int resolution,
                                         const SyncTrack& sync_track)
    : m_rows {drawn_rows(track, resolution, sync_track)}
    , m_notes {drawn_notes(track, resolution)}
{
    constexpr double HALF_BEAT = 0.5;

    for (const auto& row : m_rows) {
        auto start = row.start;
        while (start < row.end) {
            auto meas_length = get_beat_rate(sync_track, resolution, start);
            auto numer = get_numer(sync_track, resolution, start);
            auto denom = get_denom(sync_track, resolution, start);
            m_measure_lines.push_back(start);
            m_half_beat_lines.push_back(start + HALF_BEAT * denom);
            for (int i = 1; i < numer; ++i) {
                m_beat_lines.push_back(start + i * denom);
                m_half_beat_lines.push_back(start + (i + HALF_BEAT) * denom);
            }
            start += meas_length;
        }
    }
}

void DrawingInstructions::add_solo_sections(const NoteTrack& track,
                                            int resolution)
{
    for (const auto& solo : track.solos()) {
        auto start = solo.start / static_cast<double>(resolution);
        auto end = solo.end / static_cast<double>(resolution);
        m_solo_ranges.emplace_back(start, end);
    }
}

void DrawingInstructions::add_sp_phrases(const NoteTrack& track, int resolution)
{
    for (const auto& phrase : track.sp_phrases()) {
        auto start = phrase.position / static_cast<double>(resolution);
        auto end = (phrase.position + phrase.length)
            / static_cast<double>(resolution);
        m_green_ranges.emplace_back(start, end);
    }
}

void DrawingInstructions::add_sp_acts(const Path& path)
{
    for (const auto& act : path.activations) {
        auto start = act.act_start->position.beat.value();
        auto end = act.act_end->position.beat.value();
        m_blue_ranges.emplace_back(start, end);
    }
}

static std::array<unsigned char, 3> note_colour_to_colour(NoteColour colour)
{
    constexpr std::array<unsigned char, 3> GREEN {0, 255, 0};
    constexpr std::array<unsigned char, 3> RED {255, 0, 0};
    constexpr std::array<unsigned char, 3> YELLOW {255, 255, 0};
    constexpr std::array<unsigned char, 3> BLUE {0, 0, 255};
    constexpr std::array<unsigned char, 3> ORANGE {255, 165, 0};
    constexpr std::array<unsigned char, 3> PURPLE {128, 0, 128};

    switch (colour) {
    case NoteColour::Green:
        return GREEN;
    case NoteColour::Red:
        return RED;
    case NoteColour::Yellow:
        return YELLOW;
    case NoteColour::Blue:
        return BLUE;
    case NoteColour::Orange:
        return ORANGE;
    case NoteColour::Open:
        return PURPLE;
    }

    throw std::invalid_argument("Invalid colour to note_colour_to_colour");
}

static int note_colour_to_offset(NoteColour colour)
{
    constexpr int GREEN_OFFSET = 0;
    constexpr int RED_OFFSET = 15;
    constexpr int YELLOW_OFFSET = 30;
    constexpr int BLUE_OFFSET = 45;
    constexpr int ORANGE_OFFSET = 60;

    switch (colour) {
    case NoteColour::Green:
        return GREEN_OFFSET;
    case NoteColour::Red:
        return RED_OFFSET;
    case NoteColour::Yellow:
        return YELLOW_OFFSET;
    case NoteColour::Blue:
        return BLUE_OFFSET;
    case NoteColour::Orange:
        return ORANGE_OFFSET;
    case NoteColour::Open:
        return YELLOW_OFFSET;
    }

    throw std::invalid_argument("Invalid colour to note_colour_to_offset");
}

class ImageImpl {
private:
    CImg<int> m_image;

    void draw_note_circle(int x, int y, NoteColour note_colour);
    void draw_note_star(int x, int y, NoteColour note_colour);
    void draw_note_sustain(const DrawingInstructions& instructions,
                           const DrawnNote& note);
    void draw_vertical_lines(const DrawingInstructions& instructions,
                             const std::vector<double>& positions,
                             std::array<unsigned char, 3> colour);

public:
    ImageImpl(unsigned int size_x, unsigned int size_y, unsigned int size_z,
              unsigned int size_c, const int& value)
        : m_image {size_x, size_y, size_z, size_c, value}
    {
    }

    void colour_beat_range(const DrawingInstructions& instructions,
                           std::array<unsigned char, 3> colour,
                           std::tuple<double, double> x_range,
                           std::tuple<int, int> y_range, float opacity);
    void draw_measures(const DrawingInstructions& instructions);
    void draw_note(const DrawingInstructions& instructions,
                   const DrawnNote& note);

    void save(const char* filename) const { m_image.save(filename); }
};

void ImageImpl::draw_measures(const DrawingInstructions& instructions)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr std::array<unsigned char, 3> grey {160, 160, 160};
    constexpr std::array<unsigned char, 3> light_grey {224, 224, 224};
    constexpr int COLOUR_DISTANCE = 15;

    draw_vertical_lines(instructions, instructions.beat_lines(), grey);
    draw_vertical_lines(instructions, instructions.half_beat_lines(),
                        light_grey);

    auto current_row = 0;
    for (const auto& row : instructions.rows()) {
        auto y = DIST_BETWEEN_MEASURES * current_row + MARGIN;
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (row.end - row.start));
        for (int i = 1; i < 4; ++i) {
            m_image.draw_line(LEFT_MARGIN, y + COLOUR_DISTANCE * i, x_max,
                              y + COLOUR_DISTANCE * i, grey.data());
        }
        m_image.draw_rectangle(LEFT_MARGIN, y, x_max, y + MEASURE_HEIGHT,
                               black.data(), 1.0, ~0U);
        ++current_row;
    }

    // We do measure lines after the boxes because we want the measure lines to
    // lie over the horizontal grey fretboard lines.
    draw_vertical_lines(instructions, instructions.measure_lines(), black);
}

void ImageImpl::draw_vertical_lines(const DrawingInstructions& instructions,
                                    const std::vector<double>& positions,
                                    std::array<unsigned char, 3> colour)
{
    for (auto pos : positions) {
        auto row = std::find_if(instructions.rows().cbegin(),
                                instructions.rows().cend(),
                                [=](const auto x) { return x.end > pos; });
        auto x
            = LEFT_MARGIN + static_cast<int>(BEAT_WIDTH * (pos - row->start));
        auto y = MARGIN
            + DIST_BETWEEN_MEASURES
                * static_cast<int>(
                    std::distance(instructions.rows().cbegin(), row));
        m_image.draw_line(x, y, x, y + MEASURE_HEIGHT, colour.data());
    }
}

void ImageImpl::draw_note(const DrawingInstructions& instructions,
                          const DrawnNote& note)
{
    if (note.length > 0.0) {
        draw_note_sustain(instructions, note);
    }

    auto row_iter
        = std::find_if(instructions.rows().cbegin(), instructions.rows().cend(),
                       [&](const auto& r) { return r.end > note.beat; });
    auto row = static_cast<int>(
        std::distance(instructions.rows().cbegin(), row_iter));
    auto beats_along_row = note.beat - row_iter->start;
    auto x = LEFT_MARGIN + static_cast<int>(beats_along_row * BEAT_WIDTH);
    auto y = MARGIN + DIST_BETWEEN_MEASURES * row;
    if (note.is_sp_note) {
        draw_note_star(x, y, note.colour);
    } else {
        draw_note_circle(x, y, note.colour);
    }
}

void ImageImpl::draw_note_circle(int x, int y, NoteColour note_colour)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};
    constexpr int RADIUS = 5;

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour);

    if (note_colour == NoteColour::Open) {
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               colour.data(), OPEN_NOTE_OPACITY);
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               black.data(), 1.0, ~0U);
    } else {
        m_image.draw_circle(x, y + offset, RADIUS, colour.data());
        m_image.draw_circle(x, y + offset, RADIUS, black.data(), 1.0, ~0U);
    }
}

void ImageImpl::draw_note_star(int x, int y, NoteColour note_colour)
{
    constexpr std::array<unsigned char, 3> black {0, 0, 0};

    auto colour = note_colour_to_colour(note_colour);
    auto offset = note_colour_to_offset(note_colour);

    constexpr unsigned int POINTS_IN_STAR_POLYGON = 10;
    CImg<int> points {POINTS_IN_STAR_POLYGON, 2};
    constexpr std::array<int, 20> coords {0, -6, 1, -2, 5, -2, 2,  1,  3, 5, 0,
                                          2, -3, 5, -2, 1, -5, -2, -1, -2};
    for (auto i = 0U; i < POINTS_IN_STAR_POLYGON; ++i) {
        points(i, 0) = coords[2 * i] + x; // NOLINT
        points(i, 1) = coords[2 * i + 1] + y + offset; // NOLINT
    }

    if (note_colour == NoteColour::Open) {
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               colour.data(), OPEN_NOTE_OPACITY);
        m_image.draw_rectangle(x - 3, y - 3, x + 3, y + MEASURE_HEIGHT + 3,
                               black.data(), 1.0, ~0U);
    } else {
        m_image.draw_polygon(points, colour.data());
        m_image.draw_polygon(points, black.data(), 1.0, ~0U);
    }
}

void ImageImpl::draw_note_sustain(const DrawingInstructions& instructions,
                                  const DrawnNote& note)
{
    constexpr std::tuple<int, int> OPEN_NOTE_Y_RANGE {7, 53};

    auto colour = note_colour_to_colour(note.colour);
    std::tuple<double, double> x_range {note.beat, note.beat + note.length};
    auto offset = note_colour_to_offset(note.colour);
    std::tuple<int, int> y_range {offset - 3, offset + 3};
    float opacity = 1.0F;
    if (note.colour == NoteColour::Open) {
        y_range = OPEN_NOTE_Y_RANGE;
        opacity = OPEN_NOTE_OPACITY;
    }
    colour_beat_range(instructions, colour, x_range, y_range, opacity);
}

void ImageImpl::colour_beat_range(const DrawingInstructions& instructions,
                                  std::array<unsigned char, 3> colour,
                                  std::tuple<double, double> x_range,
                                  std::tuple<int, int> y_range, float opacity)
{
    auto start = std::get<0>(x_range);
    auto end = std::get<1>(x_range);
    auto row_iter
        = std::find_if(instructions.rows().cbegin(), instructions.rows().cend(),
                       [=](const auto& r) { return r.end > start; });
    auto row = static_cast<int>(
        std::distance(instructions.rows().cbegin(), row_iter));

    auto y_min = std::get<0>(y_range);
    auto y_max = std::get<1>(y_range);

    while (start < end) {
        auto block_end = std::min(row_iter->end, end);
        auto x_min = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (start - row_iter->start));
        // -1 is so regions that cross rows do not go over the ending line of a
        // row.
        auto x_max = LEFT_MARGIN
            + static_cast<int>(BEAT_WIDTH * (block_end - row_iter->start)) - 1;
        if (x_min <= x_max) {
            auto y = MARGIN + DIST_BETWEEN_MEASURES * row;
            m_image.draw_rectangle(x_min, y + y_min, x_max, y + y_max,
                                   colour.data(), opacity);
        }
        start = block_end;
        ++row_iter;
        ++row;
    }
}

Image::Image(const DrawingInstructions& instructions)
{
    constexpr std::array<unsigned char, 3> green {0, 255, 0};
    constexpr std::array<unsigned char, 3> blue {0, 0, 255};
    constexpr std::array<unsigned char, 3> solo_blue {0, 51, 128};

    constexpr unsigned int IMAGE_WIDTH = 1024;
    constexpr float RANGE_OPACITY = 0.25F;
    constexpr int SOLO_HEIGHT = 10;
    constexpr unsigned char WHITE = 255;

    auto height = static_cast<unsigned int>(
        MARGIN + DIST_BETWEEN_MEASURES * instructions.rows().size());

    m_impl = std::make_unique<ImageImpl>(IMAGE_WIDTH, height, 1, 3, WHITE);
    m_impl->draw_measures(instructions);

    for (const auto& range : instructions.solo_ranges()) {
        m_impl->colour_beat_range(instructions, solo_blue, range,
                                  {-SOLO_HEIGHT, MEASURE_HEIGHT + SOLO_HEIGHT},
                                  RANGE_OPACITY / 2);
    }

    for (const auto& note : instructions.notes()) {
        m_impl->draw_note(instructions, note);
    }

    for (const auto& range : instructions.green_ranges()) {
        m_impl->colour_beat_range(instructions, green, range,
                                  {0, MEASURE_HEIGHT}, RANGE_OPACITY);
    }
    for (const auto& range : instructions.blue_ranges()) {
        m_impl->colour_beat_range(instructions, blue, range,
                                  {0, MEASURE_HEIGHT}, RANGE_OPACITY);
    }
}

Image::~Image() = default;

Image::Image(Image&& image) noexcept = default;

Image& Image::operator=(Image&& image) noexcept = default;

void Image::save(const char* filename) const { m_impl->save(filename); }
