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
#include <cassert>
#include <iterator>

#include "optimiser.hpp"

TimeConverter::TimeConverter(const SyncTrack& sync_track,
                             const SongHeader& header)
{
    auto last_tick = 0U;
    auto last_bpm = DEFAULT_BPM;
    auto last_time = 0.0;

    for (const auto& bpm : sync_track.bpms()) {
        last_time += ((bpm.position - last_tick) * MS_PER_MINUTE)
            / (static_cast<uint32_t>(header.resolution()) * last_bpm);
        const auto beat
            = static_cast<double>(bpm.position) / header.resolution();
        m_beat_timestamps.push_back({beat, last_time});
        last_bpm = bpm.bpm;
        last_tick = bpm.position;
    }

    m_last_bpm = last_bpm;

    assert(!m_beat_timestamps.empty()); // NOLINT
}

double TimeConverter::beats_to_seconds(double beats) const
{
    const auto pos = std::lower_bound(
        m_beat_timestamps.cbegin(), m_beat_timestamps.cend(), beats,
        [](const auto& x, const auto& y) { return x.beat < y; });
    if (pos == m_beat_timestamps.cend()) {
        const auto& back = m_beat_timestamps.back();
        return back.time + ((beats - back.beat) * MS_PER_MINUTE) / m_last_bpm;
    }
    if (pos == m_beat_timestamps.cbegin()) {
        return pos->time - ((pos->beat - beats) * MS_PER_MINUTE) / DEFAULT_BPM;
    }
    const auto prev = pos - 1;
    return prev->time
        + (pos->time - prev->time) * (beats - prev->beat)
        / (pos->beat - prev->beat);
}

double TimeConverter::seconds_to_beats(double seconds) const
{
    const auto pos = std::lower_bound(
        m_beat_timestamps.cbegin(), m_beat_timestamps.cend(), seconds,
        [](const auto& x, const auto& y) { return x.time < y; });
    if (pos == m_beat_timestamps.cend()) {
        const auto& back = m_beat_timestamps.back();
        return back.beat + ((seconds - back.time) * m_last_bpm) / MS_PER_MINUTE;
    }
    if (pos == m_beat_timestamps.cbegin()) {
        return pos->beat
            - ((pos->time - seconds) * DEFAULT_BPM) / MS_PER_MINUTE;
    }
    const auto prev = pos - 1;
    return prev->beat
        + (pos->beat - prev->beat) * (seconds - prev->time)
        / (pos->time - prev->time);
}

static bool phrase_contains_pos(const StarPower& phrase, uint32_t position)
{
    if (position < phrase.position) {
        return false;
    }
    return position < (phrase.position + phrase.length);
}

template <class InputIt, class OutputIt>
static void append_note_points(InputIt first, InputIt last, OutputIt points,
                               const SongHeader& header, bool is_note_sp_ender)
{
    constexpr auto NOTE_VALUE = 50U;
    const double resolution = header.resolution();
    const auto tick_gap = std::max(header.resolution() / 25, 1);

    const auto chord_size = static_cast<uint32_t>(std::distance(first, last));
    auto chord_length = static_cast<int32_t>(
        std::max_element(first, last, [](const auto& x, const auto& y) {
            return x.length < y.length;
        })->length);
    auto pos = first->position;
    *points++
        = {pos / resolution, NOTE_VALUE * chord_size, false, is_note_sp_ender};
    while (chord_length > 0) {
        pos += static_cast<uint32_t>(tick_gap);
        chord_length -= tick_gap;
        *points++ = {pos / resolution, 1, true, false};
    }
}

std::vector<Point> notes_to_points(const NoteTrack& track,
                                   const SongHeader& header)
{
    std::vector<Point> points;

    const auto& notes = track.notes();

    auto current_phrase = track.sp_phrases().cbegin();
    for (auto p = notes.cbegin(); p != notes.cend();) {
        const auto q = std::find_if_not(p, notes.cend(), [=](const auto& x) {
            return x.position == p->position;
        });
        auto is_note_sp_ender = false;
        if (current_phrase != track.sp_phrases().cend()
            && phrase_contains_pos(*current_phrase, p->position)
            && ((q == notes.cend())
                || !phrase_contains_pos(*current_phrase, q->position))) {
            is_note_sp_ender = true;
            ++current_phrase;
        }
        append_note_points(p, q, std::back_inserter(points), header,
                           is_note_sp_ender);
        p = q;
    }

    std::stable_sort(points.begin(), points.end(),
                     [](const auto& x, const auto& y) {
                         return x.beat_position < y.beat_position;
                     });

    return points;
}

double front_end(const Point& point, const TimeConverter& converter)
{
    constexpr double FRONT_END = 0.07;

    if (point.is_hold_point) {
        return point.beat_position;
    }

    auto time = converter.beats_to_seconds(point.beat_position);
    time -= FRONT_END;
    return converter.seconds_to_beats(time);
}

double back_end(const Point& point, const TimeConverter& converter)
{
    constexpr double BACK_END = 0.07;

    if (point.is_hold_point) {
        return point.beat_position;
    }

    auto time = converter.beats_to_seconds(point.beat_position);
    time += BACK_END;
    return converter.seconds_to_beats(time);
}
