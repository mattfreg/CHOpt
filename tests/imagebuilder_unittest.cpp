/*
 * CHOpt - Star Power optimiser for Clone Hero
 * Copyright (C) 2020, 2021, 2022, 2023 Raymond Wright
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

#include <cstdlib>

#include <boost/test/unit_test.hpp>

#include "test_helpers.hpp"

namespace boost::test_tools::tt_detail {
template <> struct print_log_value<std::tuple<double, double>> {
    void operator()(std::ostream& stream,
                    const std::tuple<double, double>& tuple)
    {
        const auto& [first, second] = tuple;
        stream << '{' << first << ", " << second << '}';
    }
};

template <> struct print_log_value<std::tuple<double, int, int>> {
    void operator()(std::ostream& stream,
                    const std::tuple<double, int, int>& tuple)
    {
        const auto& [first, second, third] = tuple;
        stream << '{' << first << ", " << second << ", " << third << '}';
    }
};
}

namespace {
DrawnNote make_drawn_note(double position, double length = 0.0,
                          FiveFretNotes colour = FIVE_FRET_GREEN)
{
    DrawnNote note {};
    note.beat = position;
    note.note_flags = FLAGS_FIVE_FRET_GUITAR;
    note.lengths.fill(-1.0);
    note.lengths.at(colour) = length;
    note.is_sp_note = false;

    return note;
}

DrawnNote make_drawn_sp_note(double position, double length = 0.0,
                             FiveFretNotes colour = FIVE_FRET_GREEN)
{
    DrawnNote note {};
    note.beat = position;
    note.note_flags = FLAGS_FIVE_FRET_GUITAR;
    note.lengths.fill(-1.0);
    note.lengths.at(colour) = length;
    note.is_sp_note = true;

    return note;
}

DrawnNote make_drawn_ghl_note(double position, double length = 0.0,
                              SixFretNotes colour = SIX_FRET_WHITE_LOW)
{
    DrawnNote note {};
    note.beat = position;
    note.note_flags = FLAGS_SIX_FRET_GUITAR;
    note.lengths.fill(-1.0);
    note.lengths.at(colour) = length;
    note.is_sp_note = false;

    return note;
}

DrawnNote make_drawn_drum_note(double position, DrumNotes colour = DRUM_RED,
                               NoteFlags flags = FLAGS_NONE)
{
    DrawnNote note {};
    note.beat = position;
    note.note_flags = static_cast<NoteFlags>(flags | FLAGS_DRUMS);
    note.lengths.fill(-1.0);
    note.lengths.at(colour) = 0.0;
    note.is_sp_note = false;

    return note;
}
}

BOOST_AUTO_TEST_SUITE(track_type_is_stored_correctly)

BOOST_AUTO_TEST_CASE(five_fret_gets_the_right_track_type)
{
    NoteTrack track {{},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};

    BOOST_CHECK_EQUAL(builder.track_type(), TrackType::FiveFret);
}

BOOST_AUTO_TEST_CASE(six_fret_gets_the_right_track_type)
{
    NoteTrack track {{},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::SixFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};

    BOOST_CHECK_EQUAL(builder.track_type(), TrackType::SixFret);
}

BOOST_AUTO_TEST_CASE(drums_gets_the_right_track_type)
{
    NoteTrack track {{},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};

    BOOST_CHECK_EQUAL(builder.track_type(), TrackType::Drums);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(notes_are_handled_correctly)

BOOST_AUTO_TEST_CASE(non_sp_non_sustains_are_handled_correctly)
{
    NoteTrack track {{make_note(0), make_note(768, 0, FIVE_FRET_RED)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnNote> expected_notes {
        make_drawn_note(0), make_drawn_note(4, 0, FIVE_FRET_RED)};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.notes().cbegin(), builder.notes().cend(),
        expected_notes.cbegin(), expected_notes.cend());
}

BOOST_AUTO_TEST_CASE(sustains_are_handled_correctly)
{
    NoteTrack track {{make_note(0, 96)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnNote> expected_notes {make_drawn_note(0, 0.5)};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.notes().cbegin(), builder.notes().cend(),
        expected_notes.cbegin(), expected_notes.cend());
}

BOOST_AUTO_TEST_CASE(sp_notes_are_recorded)
{
    NoteTrack track {{make_note(0), make_note(768)},
                     {{Tick {768}, Tick {100}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnNote> expected_notes {make_drawn_note(0),
                                           make_drawn_sp_note(4)};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.notes().cbegin(), builder.notes().cend(),
        expected_notes.cbegin(), expected_notes.cend());
}

BOOST_AUTO_TEST_CASE(six_fret_notes_are_handled_correctly)
{
    NoteTrack track {
        {make_ghl_note(0), make_ghl_note(768, 0, SIX_FRET_BLACK_HIGH)},
        {},
        {},
        {},
        {},
        {},
        TrackType::SixFret,
        std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnNote> expected_notes {
        make_drawn_ghl_note(0), make_drawn_ghl_note(4, 0, SIX_FRET_BLACK_HIGH)};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.notes().cbegin(), builder.notes().cend(),
        expected_notes.cbegin(), expected_notes.cend());
}

BOOST_AUTO_TEST_CASE(drum_notes_are_handled_correctly)
{
    NoteTrack track {
        {make_drum_note(0), make_drum_note(768, DRUM_YELLOW, FLAGS_CYMBAL)},
        {},
        {},
        {},
        {},
        {},
        TrackType::Drums,
        std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnNote> expected_notes {
        make_drawn_drum_note(0),
        make_drawn_drum_note(4, DRUM_YELLOW, FLAGS_CYMBAL)};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.notes().cbegin(), builder.notes().cend(),
        expected_notes.cbegin(), expected_notes.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(drawn_rows_are_handled_correctly)

BOOST_AUTO_TEST_CASE(simple_four_four_is_handled_correctly)
{
    NoteTrack track {{make_note(2880)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnRow> expected_rows {{0.0, 16.0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(builder.rows().cbegin(),
                                  builder.rows().cend(), expected_rows.cbegin(),
                                  expected_rows.cend());
}

BOOST_AUTO_TEST_CASE(three_x_time_sigs_are_handled)
{
    NoteTrack track {{make_note(2450)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {{{Tick {0}, 4, 4},
                         {Tick {768}, 3, 4},
                         {Tick {1344}, 3, 8},
                         {Tick {1632}, 4, 4}},
                        {},
                        {},
                        192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnRow> expected_rows {{0.0, 12.5}, {12.5, 16.5}};

    BOOST_CHECK_EQUAL_COLLECTIONS(builder.rows().cbegin(),
                                  builder.rows().cend(), expected_rows.cbegin(),
                                  expected_rows.cend());
}

BOOST_AUTO_TEST_CASE(time_signature_changes_off_measure_are_coped_with)
{
    NoteTrack track {{make_note(768)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {
        {{Tick {0}, 4, 4}, {Tick {767}, 3, 4}, {Tick {1344}, 3, 8}},
        {},
        {},
        192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnRow> expected_rows {{0.0, 7.0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(builder.rows().cbegin(),
                                  builder.rows().cend(), expected_rows.cbegin(),
                                  expected_rows.cend());
}

BOOST_AUTO_TEST_CASE(x_four_for_x_gt_16_is_handled)
{
    NoteTrack track {{make_note(0)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {{{Tick {0}, 17, 4}}, {}, {}, 192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<DrawnRow> expected_rows {{0.0, 16.0}, {16.0, 17.0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(builder.rows().cbegin(),
                                  builder.rows().cend(), expected_rows.cbegin(),
                                  expected_rows.cend());
}

BOOST_AUTO_TEST_CASE(enough_rows_are_drawn_for_end_of_song_sustains)
{
    NoteTrack track {{make_note(0, 3840)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};

    BOOST_CHECK_EQUAL(builder.rows().size(), 2);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(beat_lines_are_correct)

BOOST_AUTO_TEST_CASE(four_four_works_fine)
{
    NoteTrack track {{make_note(767)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<double> expected_half_beat_lines {0.5, 1.5, 2.5, 3.5};
    std::vector<double> expected_beat_lines {1.0, 2.0, 3.0};
    std::vector<double> expected_measure_lines {0.0, 4.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.half_beat_lines().cbegin(), builder.half_beat_lines().cend(),
        expected_half_beat_lines.cbegin(), expected_half_beat_lines.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.beat_lines().cbegin(), builder.beat_lines().cend(),
        expected_beat_lines.cbegin(), expected_beat_lines.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.measure_lines().cbegin(), builder.measure_lines().cend(),
        expected_measure_lines.cbegin(), expected_measure_lines.cend());
}

BOOST_AUTO_TEST_CASE(four_eight_works_fine)
{
    NoteTrack track {{make_note(767)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {{{Tick {0}, 4, 8}}, {}, {}, 192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<double> expected_half_beat_lines {0.25, 0.75, 1.25, 1.75,
                                                  2.25, 2.75, 3.25, 3.75};
    std::vector<double> expected_beat_lines {0.5, 1.0, 1.5, 2.5, 3.0, 3.5};
    std::vector<double> expected_measure_lines {0.0, 2.0, 4.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.half_beat_lines().cbegin(), builder.half_beat_lines().cend(),
        expected_half_beat_lines.cbegin(), expected_half_beat_lines.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.beat_lines().cbegin(), builder.beat_lines().cend(),
        expected_beat_lines.cbegin(), expected_beat_lines.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.measure_lines().cbegin(), builder.measure_lines().cend(),
        expected_measure_lines.cbegin(), expected_measure_lines.cend());
}

BOOST_AUTO_TEST_CASE(combination_of_four_four_and_four_eight_works_fine)
{
    NoteTrack track {{make_note(1151)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {{{Tick {0}, 4, 4}, {Tick {768}, 4, 8}}, {}, {}, 192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    std::vector<double> expected_half_beat_lines {0.5,  1.5,  2.5,  3.5,
                                                  4.25, 4.75, 5.25, 5.75};
    std::vector<double> expected_beat_lines {1.0, 2.0, 3.0, 4.5, 5.0, 5.5};
    std::vector<double> expected_measure_lines {0.0, 4.0, 6.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.half_beat_lines().cbegin(), builder.half_beat_lines().cend(),
        expected_half_beat_lines.cbegin(), expected_half_beat_lines.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.beat_lines().cbegin(), builder.beat_lines().cend(),
        expected_beat_lines.cbegin(), expected_beat_lines.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.measure_lines().cbegin(), builder.measure_lines().cend(),
        expected_measure_lines.cbegin(), expected_measure_lines.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(time_signatures_are_handled_correctly)

BOOST_AUTO_TEST_CASE(normal_time_signatures_are_handled_correctly)
{
    NoteTrack track {{make_note(1920)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {{{Tick {0}, 4, 4}, {Tick {768}, 4, 8}}, {}, {}, 192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_time_sigs(tempo_map);
    std::vector<std::tuple<double, int, int>> expected_time_sigs {{0.0, 4, 4},
                                                                  {4.0, 4, 8}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.time_sigs().cbegin(), builder.time_sigs().cend(),
        expected_time_sigs.cbegin(), expected_time_sigs.cend());
}

BOOST_AUTO_TEST_CASE(time_sig_changes_past_the_end_of_the_song_are_removed)
{
    NoteTrack track {{make_note(768)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {{{Tick {0}, 4, 4}, {Tick {1920}, 3, 4}}, {}, {}, 192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_time_sigs(tempo_map);

    BOOST_CHECK_EQUAL(builder.time_sigs().size(), 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(tempos_are_handled_correctly)

BOOST_AUTO_TEST_CASE(normal_tempos_are_handled_correctly)
{
    NoteTrack track {{make_note(1920)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {
        {},
        {{Tick {0}, 150000}, {Tick {384}, 120000}, {Tick {768}, 200000}},
        {},
        192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_bpms(tempo_map);
    std::vector<std::tuple<double, double>> expected_bpms {
        {0.0, 150.0}, {2.0, 120.0}, {4.0, 200.0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(builder.bpms().cbegin(),
                                  builder.bpms().cend(), expected_bpms.cbegin(),
                                  expected_bpms.cend());
}

BOOST_AUTO_TEST_CASE(tempo_changes_past_the_end_of_the_song_are_removed)
{
    NoteTrack track {{make_note(768)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    TempoMap tempo_map {
        {}, {{Tick {0}, 120000}, {Tick {1920}, 200000}}, {}, 192};
    ImageBuilder builder {
        track, tempo_map, Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_bpms(tempo_map);

    BOOST_CHECK_EQUAL(builder.bpms().size(), 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(normal_speed)
{
    NoteTrack track {{make_note(0)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SongGlobalData global_data;
    global_data.name("TestName");
    global_data.artist("GMS");
    global_data.charter("NotGMS");
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};

    builder.add_song_header(global_data);

    BOOST_CHECK_EQUAL(builder.song_name(), "TestName");
    BOOST_CHECK_EQUAL(builder.artist(), "GMS");
    BOOST_CHECK_EQUAL(builder.charter(), "NotGMS");
}

BOOST_AUTO_TEST_SUITE(green_sp_ranges)

BOOST_AUTO_TEST_CASE(green_ranges_for_sp_phrases_are_added_correctly)
{
    NoteTrack track {{make_note(960), make_note(1344, 96)},
                     {{Tick {768}, Tick {384}}, {Tick {1200}, Tick {150}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_phrases(track, {}, Path {});
    std::vector<std::tuple<double, double>> expected_green_ranges {{5.0, 5.1},
                                                                   {7.0, 7.5}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.green_ranges().cbegin(), builder.green_ranges().cend(),
        expected_green_ranges.cbegin(), expected_green_ranges.cend());
}

BOOST_AUTO_TEST_CASE(green_ranges_have_a_minimum_size)
{
    NoteTrack track {
        {make_note(768)},    {{Tick {768}, Tick {384}}},        {}, {}, {}, {},
        TrackType::FiveFret, std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_phrases(track, {}, Path {});

    std::vector<std::tuple<double, double>> expected_green_ranges {{4.0, 4.1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.green_ranges().cbegin(), builder.green_ranges().cend(),
        expected_green_ranges.cbegin(), expected_green_ranges.cend());
}

BOOST_AUTO_TEST_CASE(green_ranges_for_six_fret_sp_phrases_are_added_correctly)
{
    NoteTrack track {{make_ghl_note(960), make_ghl_note(1344, 96)},
                     {{Tick {768}, Tick {384}}, {Tick {1200}, Tick {150}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::SixFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_phrases(track, {}, Path {});
    std::vector<std::tuple<double, double>> expected_green_ranges {{5.0, 5.1},
                                                                   {7.0, 7.5}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.green_ranges().cbegin(), builder.green_ranges().cend(),
        expected_green_ranges.cbegin(), expected_green_ranges.cend());
}

BOOST_AUTO_TEST_CASE(green_ranges_for_drums_sp_phrases_are_added_correctly)
{
    NoteTrack track {{make_drum_note(960), make_drum_note(1344)},
                     {{Tick {768}, Tick {384}}, {Tick {1200}, Tick {150}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_phrases(track, {}, Path {});
    std::vector<std::tuple<double, double>> expected_green_ranges {{5.0, 5.1},
                                                                   {7.0, 7.1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.green_ranges().cbegin(), builder.green_ranges().cend(),
        expected_green_ranges.cbegin(), expected_green_ranges.cend());
}

BOOST_AUTO_TEST_CASE(neutralised_green_ranges_are_ommitted_on_non_overlap_games)
{
    NoteTrack track {{make_note(0), make_note(768), make_note(3840)},
                     {{Tick {3840}, Tick {192}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    ImageBuilder builder {
        track, {},   Difficulty::Expert, DrumSettings::default_settings(),
        false, false};
    Path path {{{points.cbegin() + 1, points.cbegin() + 2, Beat {0.05},
                 Beat {4.01}, Beat {20.01}}},
               100};
    builder.add_sp_phrases(track, {}, path);

    BOOST_TEST(builder.green_ranges().empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(drum_fills_are_drawn_with_add_drum_fills)
{
    NoteTrack track {{{make_drum_note(288)}},
                     {},
                     {},
                     {{Tick {192}, Tick {96}}},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_drum_fills(track);

    std::vector<std::tuple<double, double>> expected_fill_ranges {{1.0, 1.5}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.fill_ranges().cbegin(), builder.fill_ranges().cend(),
        expected_fill_ranges.cbegin(), expected_fill_ranges.cend());
}

BOOST_AUTO_TEST_CASE(drum_fills_cannot_be_cancelled_by_a_kick)
{
    NoteTrack track {{make_drum_note(288, DRUM_KICK)},
                     {},
                     {},
                     {{Tick {192}, Tick {96}}},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_drum_fills(track);

    BOOST_CHECK_EQUAL(builder.fill_ranges().size(), 1);
}

BOOST_AUTO_TEST_CASE(double_kicks_only_drawn_with_enable_double_kick)
{
    NoteTrack track {
        {make_drum_note(0, DRUM_KICK), make_drum_note(192, DRUM_DOUBLE_KICK)},
        {},
        {},
        {},
        {},
        {},
        TrackType::Drums,
        std::make_shared<SongGlobalData>()};
    ImageBuilder no_double_builder {
        track, {},  Difficulty::Expert, {false, false, false, false},
        false, true};
    ImageBuilder double_builder {
        track, {},  Difficulty::Expert, {true, false, false, false},
        false, true};

    BOOST_CHECK_EQUAL(no_double_builder.notes().size(), 1);
    BOOST_CHECK_EQUAL(double_builder.notes().size(), 2);
}

BOOST_AUTO_TEST_CASE(single_kicks_disappear_with_disable_kick)
{
    NoteTrack track {
        {make_drum_note(0, DRUM_KICK), make_drum_note(192, DRUM_DOUBLE_KICK)},
        {},
        {},
        {},
        {},
        {},
        TrackType::Drums,
        std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {}, Difficulty::Expert, {true, true, false, false}, false, true};

    BOOST_CHECK_EQUAL(builder.notes().size(), 1);
}

BOOST_AUTO_TEST_CASE(cymbals_become_toms_with_pro_drums_off)
{
    NoteTrack track {{make_drum_note(0, DRUM_YELLOW, FLAGS_CYMBAL)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, {true, false, false, false},
        false, true};

    BOOST_CHECK_EQUAL(builder.notes().size(), 1);
    BOOST_CHECK_EQUAL(builder.notes().front().note_flags, FLAGS_DRUMS);
}

BOOST_AUTO_TEST_CASE(disco_flip_matters_only_with_pro_drums_on)
{
    NoteTrack track {{make_drum_note(192, DRUM_YELLOW, FLAGS_CYMBAL),
                      make_drum_note(288, DRUM_YELLOW)},
                     {},
                     {},
                     {},
                     {{Tick {192}, Tick {192}}},
                     {},
                     TrackType::Drums,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder normal_builder {
        track, {},  Difficulty::Expert, {true, false, false, false},
        false, true};
    ImageBuilder pro_builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};

    BOOST_CHECK_EQUAL(normal_builder.notes().size(), 2);
    BOOST_CHECK_EQUAL(normal_builder.notes().front().note_flags, FLAGS_DRUMS);
    BOOST_CHECK_EQUAL(pro_builder.notes().size(), 2);
    BOOST_CHECK_EQUAL(pro_builder.notes()[0].lengths[DRUM_RED], 0);
    BOOST_CHECK_EQUAL(pro_builder.notes()[1].lengths[DRUM_YELLOW], 0);
    BOOST_CHECK_EQUAL(pro_builder.notes()[1].note_flags, FLAGS_DRUMS);
}

BOOST_AUTO_TEST_CASE(unison_phrases_are_added_correctly)
{
    NoteTrack track {{make_note(960), make_note(1344, 96)},
                     {{Tick {768}, Tick {384}}, {Tick {1200}, Tick {150}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_phrases(track, {{Tick {768}, Tick {384}}}, Path {});
    std::vector<std::tuple<double, double>> expected_unison_ranges {{5.0, 5.1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.unison_ranges().cbegin(), builder.unison_ranges().cend(),
        expected_unison_ranges.cbegin(), expected_unison_ranges.cend());
}

BOOST_AUTO_TEST_SUITE(add_sp_acts_adds_correct_ranges)

BOOST_AUTO_TEST_CASE(normal_path_is_drawn_correctly)
{
    NoteTrack track {{make_note(0, 96), make_note(192)},
                     {{Tick {0}, Tick {50}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    Path path {{{points.cbegin(), points.cend() - 1, Beat {0.25}, Beat {0.1},
                 Beat {0.9}}},
               0};
    builder.add_sp_phrases(track, {}, path);
    builder.add_sp_acts(points, {}, path);
    std::vector<std::tuple<double, double>> expected_blue_ranges {{0.1, 0.9}};
    std::vector<std::tuple<double, double>> expected_red_ranges {{0.0, 0.1},
                                                                 {0.9, 1.0}};
    std::vector<std::tuple<double, double>> expected_yellow_ranges {
        {0.25, 0.5}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.blue_ranges().cbegin(), builder.blue_ranges().cend(),
        expected_blue_ranges.cbegin(), expected_blue_ranges.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.red_ranges().cbegin(), builder.red_ranges().cend(),
        expected_red_ranges.cbegin(), expected_red_ranges.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.yellow_ranges().cbegin(), builder.yellow_ranges().cend(),
        expected_yellow_ranges.cbegin(), expected_yellow_ranges.cend());
}

BOOST_AUTO_TEST_CASE(squeezes_are_only_drawn_when_required)
{
    NoteTrack track {
        {make_note(0), make_note(192), make_note(384), make_note(576)},
        {},
        {},
        {},
        {},
        {},
        TrackType::FiveFret,
        std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    Path path {{{points.cbegin(), points.cbegin() + 1, Beat {0.25}, Beat {0.1},
                 Beat {1.1}},
                {points.cbegin() + 2, points.cbegin() + 3, Beat {0.25},
                 Beat {2.0}, Beat {2.9}}},
               0};
    builder.add_sp_acts(points, {}, path);
    std::vector<std::tuple<double, double>> expected_red_ranges {{0.0, 0.1},
                                                                 {2.9, 3.0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.red_ranges().cbegin(), builder.red_ranges().cend(),
        expected_red_ranges.cbegin(), expected_red_ranges.cend());
}

BOOST_AUTO_TEST_CASE(blue_ranges_are_cropped_for_reverse_squeezes)
{
    NoteTrack track {
        {make_note(192), make_note(384), make_note(576), make_note(768)},
        {},
        {},
        {},
        {},
        {},
        TrackType::FiveFret,
        std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    Path path {{{points.cbegin() + 1, points.cbegin() + 2, Beat {5.0},
                 Beat {0.0}, Beat {5.0}}},
               0};
    builder.add_sp_acts(points, {}, path);
    std::vector<std::tuple<double, double>> expected_blue_ranges {{1.0, 4.0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.blue_ranges().cbegin(), builder.blue_ranges().cend(),
        expected_blue_ranges.cbegin(), expected_blue_ranges.cend());
}

BOOST_AUTO_TEST_CASE(blue_ranges_are_cropped_by_the_end_of_the_song)
{
    NoteTrack track {{make_note(192)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    Path path {{{points.cbegin(), points.cbegin(), Beat {0.0}, Beat {0.0},
                 Beat {16.0}}},
               0};
    builder.add_sp_acts(points, {}, path);
    std::vector<std::tuple<double, double>> expected_blue_ranges {{0.0, 4.0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.blue_ranges().cbegin(), builder.blue_ranges().cend(),
        expected_blue_ranges.cbegin(), expected_blue_ranges.cend());
}

BOOST_AUTO_TEST_CASE(blue_and_red_ranges_are_shifted_by_video_lag)
{
    NoteTrack track {{make_note(0), make_note(192), make_note(384),
                      make_note(576), make_note(768), make_note(1530)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     {1.0, 1.0, Second(0.0), Second(0.05), Second(0.0)},
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    Path path {{{points.cbegin(), points.cbegin() + 1, Beat {0.25}, Beat {0.1},
                 Beat {1.1}},
                {points.cbegin() + 2, points.cbegin() + 3, Beat {0.25},
                 Beat {2.0}, Beat {2.9}},
                {points.cbegin() + 5, points.cbegin() + 5, Beat {0.25},
                 Beat {7.0}, Beat {23.0}}},
               0};
    std::vector<std::tuple<double, double>> expected_blue_ranges {
        {0.0, 1.0}, {1.9, 2.8}, {6.9, 8.0}};
    std::vector<std::tuple<double, double>> expected_red_ranges {{2.8, 3.0}};

    builder.add_sp_acts(points, {}, path);

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.blue_ranges().cbegin(), builder.blue_ranges().cend(),
        expected_blue_ranges.cbegin(), expected_blue_ranges.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.red_ranges().cbegin(), builder.red_ranges().cend(),
        expected_red_ranges.cbegin(), expected_red_ranges.cend());
}

BOOST_AUTO_TEST_CASE(green_ranges_do_not_overlap_blue_for_no_overlap_engines)
{
    NoteTrack track {{make_note(0, 96), make_note(192)},
                     {{Tick {0}, Tick {50}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    ImageBuilder builder {
        track, {},   Difficulty::Expert, DrumSettings::default_settings(),
        false, false};
    Path path {{{points.cbegin() + 1, points.cend() - 1, Beat {0.05},
                 Beat {0.1}, Beat {0.9}}},
               0};
    builder.add_sp_phrases(track, {}, path);
    builder.add_sp_acts(points, {}, path);
    std::vector<std::tuple<double, double>> expected_green_ranges {{0.0, 0.1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.green_ranges().cbegin(), builder.green_ranges().cend(),
        expected_green_ranges.cbegin(), expected_green_ranges.cend());
}

BOOST_AUTO_TEST_CASE(almost_overlapped_green_ranges_remain)
{
    NoteTrack track {{make_note(0), make_note(768), make_note(3840)},
                     {{Tick {3840}, Tick {192}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    ImageBuilder builder {
        track, {},   Difficulty::Expert, DrumSettings::default_settings(),
        false, false};
    Path path {{{points.cbegin() + 1, points.cbegin() + 1, Beat {0.05},
                 Beat {4.01}, Beat {20.01}}},
               50};
    builder.add_sp_phrases(track, {}, path);
    builder.add_sp_acts(points, {}, path);
    std::vector<std::tuple<double, double>> expected_green_ranges {
        {20.0, 20.1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.green_ranges().cbegin(), builder.green_ranges().cend(),
        expected_green_ranges.cbegin(), expected_green_ranges.cend());
}

BOOST_AUTO_TEST_CASE(
    extra_green_ranges_are_not_discarded_for_no_overlap_engines)
{
    NoteTrack track {{make_note(0, 96), make_note(192), make_note(3840)},
                     {{Tick {0}, Tick {50}}, {Tick {3840}, Tick {192}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    ImageBuilder builder {
        track, {},   Difficulty::Expert, DrumSettings::default_settings(),
        false, false};
    Path path {{{points.cbegin() + 1, points.cend() - 2, Beat {0.05},
                 Beat {0.1}, Beat {0.9}}},
               0};
    builder.add_sp_phrases(track, {}, path);
    builder.add_sp_acts(points, {}, path);
    std::vector<std::tuple<double, double>> expected_green_ranges {
        {0.0, 0.1}, {20.0, 20.1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.green_ranges().cbegin(), builder.green_ranges().cend(),
        expected_green_ranges.cbegin(), expected_green_ranges.cend());
}

BOOST_AUTO_TEST_CASE(yellow_ranges_do_not_overlap_blue_for_no_overlap_engines)
{
    NoteTrack track {{make_note(0, 96), make_note(192)},
                     {{Tick {0}, Tick {50}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     Gh1Engine()};
    ImageBuilder builder {
        track, {},   Difficulty::Expert, DrumSettings::default_settings(),
        false, false};
    Path path {{{points.cbegin() + 1, points.cend() - 1, Beat {0.05},
                 Beat {0.1}, Beat {0.9}}},
               0};
    builder.add_sp_phrases(track, {}, path);
    builder.add_sp_acts(points, {}, path);
    std::vector<std::tuple<double, double>> expected_yellow_ranges {
        {0.05, 0.1}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.yellow_ranges().cbegin(), builder.yellow_ranges().cend(),
        expected_yellow_ranges.cbegin(), expected_yellow_ranges.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(add_solo_sections_add_correct_ranges)
{
    NoteTrack track {{make_note(0)},
                     {},
                     {{Tick {192}, Tick {384}, 0}},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_solo_sections(track.solos(DrumSettings::default_settings()),
                              {});
    std::vector<std::tuple<double, double>> expected_solo_ranges {{1.0, 2.0}};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.solo_ranges().cbegin(), builder.solo_ranges().cend(),
        expected_solo_ranges.cbegin(), expected_solo_ranges.cend());
}

BOOST_AUTO_TEST_SUITE(add_measure_values_gives_correct_values)

BOOST_AUTO_TEST_CASE(notes_with_no_activations_or_solos)
{
    NoteTrack track {{make_note(0), make_note(768)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    Path path;
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_measure_values(points, {}, path);
    std::vector<int> expected_base_values {50, 50};
    std::vector<int> expected_score_values {50, 100};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.base_values().cbegin(), builder.base_values().cend(),
        expected_base_values.cbegin(), expected_base_values.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.score_values().cbegin(), builder.score_values().cend(),
        expected_score_values.cbegin(), expected_score_values.cend());
}

BOOST_AUTO_TEST_CASE(solos_are_added)
{
    NoteTrack track {
        {make_note(768)},
        {},
        {{Tick {0}, Tick {100}, 100}, {Tick {200}, Tick {800}, 100}},
        {},
        {},
        {},
        TrackType::FiveFret,
        std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    Path path;
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_measure_values(points, {}, path);
    std::vector<int> expected_score_values {100, 250};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.score_values().cbegin(), builder.score_values().cend(),
        expected_score_values.cbegin(), expected_score_values.cend());
}

// This bug caused a crash in a few songs, for example Satch Boogie (Live) from
// Guitar Hero X.
BOOST_AUTO_TEST_CASE(solos_ending_past_last_note_are_handled_correctly)
{
    NoteTrack track {{make_note(0)},
                     {},
                     {{Tick {0}, Tick {1600}, 50}},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    Path path;
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_measure_values(points, {}, path);
    std::vector<int> expected_score_values {100};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.score_values().cbegin(), builder.score_values().cend(),
        expected_score_values.cbegin(), expected_score_values.cend());
}

BOOST_AUTO_TEST_CASE(activations_are_added)
{
    NoteTrack track {
        {make_note(0), make_note(192), make_note(384), make_note(768)},
        {},
        {},
        {},
        {},
        {},
        TrackType::FiveFret,
        std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    Path path {
        {{points.cbegin() + 2, points.cbegin() + 3, Beat {0.0}, Beat {0.0}}},
        100};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_measure_values(points, {}, path);
    std::vector<int> expected_score_values {200, 300};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.score_values().cbegin(), builder.score_values().cend(),
        expected_score_values.cbegin(), expected_score_values.cend());
}

BOOST_AUTO_TEST_CASE(video_lag_is_accounted_for)
{
    NoteTrack track {{make_note(0), make_note(768)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     {1.0, 1.0, Second {0.0}, Second {-0.1}, Second {0.0}},
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    Path path {
        {{points.cbegin() + 1, points.cbegin() + 1, Beat {0.0}, Beat {0.0}}},
        50};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_measure_values(points, {}, path);
    std::vector<int> expected_base_values {50, 50};
    std::vector<int> expected_score_values {50, 150};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.base_values().cbegin(), builder.base_values().cend(),
        expected_base_values.cbegin(), expected_base_values.cend());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.score_values().cbegin(), builder.score_values().cend(),
        expected_score_values.cbegin(), expected_score_values.cend());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(add_sp_values_gives_correct_values)
{
    NoteTrack track {{make_note(0), make_note(192, 768)},
                     {{Tick {192}, Tick {50}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_values(sp_data, ChGuitarEngine());
    std::vector<double> expected_sp_values {3.14, 1.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(
        builder.sp_values().cbegin(), builder.sp_values().cend(),
        expected_sp_values.cbegin(), expected_sp_values.cend());
}

BOOST_AUTO_TEST_CASE(set_total_score_sets_the_correct_value)
{
    NoteTrack track {{make_note(0), make_note(192)},
                     {{Tick {0}, Tick {50}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    Path path {{{points.cbegin(), points.cend() - 1, Beat {0.25}, Beat {0.1},
                 Beat {0.9}}},
               50};
    builder.set_total_score(points, {{Tick {0}, Tick {1}, 100}}, path);

    BOOST_CHECK_EQUAL(builder.total_score(), 250);
}

BOOST_AUTO_TEST_CASE(difficulty_is_handled)
{
    NoteTrack track {{make_note(0)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder hard_builder {
        track, {},  Difficulty::Hard, DrumSettings::default_settings(),
        false, true};
    ImageBuilder expert_builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};

    BOOST_CHECK_EQUAL(hard_builder.difficulty(), Difficulty::Hard);
    BOOST_CHECK_EQUAL(expert_builder.difficulty(), Difficulty::Expert);
}

BOOST_AUTO_TEST_CASE(lefty_flip_is_handled)
{
    NoteTrack track {{make_note(0)},
                     {},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    ImageBuilder lefty_builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        true,  true};
    ImageBuilder righty_builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};

    BOOST_TEST(lefty_builder.is_lefty_flip());
    BOOST_TEST(!righty_builder.is_lefty_flip());
}

BOOST_AUTO_TEST_SUITE(add_sp_percent_values_adds_correct_values)

BOOST_AUTO_TEST_CASE(sp_percents_added_with_no_whammy)
{
    NoteTrack track {{make_note(960), make_note(1080), make_note(1920),
                      make_note(3840), make_note(4050), make_note(19200)},
                     {{Tick {960}, Tick {10}},
                      {Tick {1080}, Tick {10}},
                      {Tick {1920}, Tick {10}},
                      {Tick {3840}, Tick {10}},
                      {Tick {4050}, Tick {10}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Path path {{{points.cbegin() + 5, points.cend(), Beat {1000.0}, Beat {70.0},
                 Beat {102.0}}},
               0};

    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_percent_values(sp_data, {}, points, path);
    std::vector<double> expected_percents {
        0.0,    0.5,    0.75,   0.75,   0.75,   1.0,    1.0,    1.0, 1.0,
        1.0,    1.0,    1.0,    1.0,    1.0,    1.0,    1.0,    1.0, 0.9375,
        0.8125, 0.6875, 0.5625, 0.4375, 0.3125, 0.1875, 0.0625, 0.0};

    BOOST_CHECK_EQUAL_COLLECTIONS(builder.sp_percent_values().cbegin(),
                                  builder.sp_percent_values().cend(),
                                  expected_percents.cbegin(),
                                  expected_percents.cend());
}

BOOST_AUTO_TEST_CASE(sp_percents_added_with_no_whammy_and_mid_act_gain)
{
    NoteTrack track {{make_note(960), make_note(1080), make_note(1920),
                      make_note(3840), make_note(4050), make_note(19200)},
                     {{Tick {960}, Tick {10}},
                      {Tick {1080}, Tick {10}},
                      {Tick {1920}, Tick {10}},
                      {Tick {3840}, Tick {10}},
                      {Tick {4050}, Tick {10}},
                      {Tick {19200}, Tick {10}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Path path {{{points.cbegin() + 5, points.cend(), Beat {1000.0}, Beat {98.0},
                 Beat {132.0}}},
               0};

    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_percent_values(sp_data, {}, points, path);
    std::vector<double> expected_percents {
        0.0, 0.5, 0.75, 0.75, 0.75, 1.0, 1.0,    1.0,  1.0,
        1.0, 1.0, 1.0,  1.0,  1.0,  1.0, 1.0,    1.0,  1.0,
        1.0, 1.0, 1.0,  1.0,  1.0,  1.0, 0.9375, 0.875};

    BOOST_CHECK_EQUAL_COLLECTIONS(builder.sp_percent_values().cbegin(),
                                  builder.sp_percent_values().cend(),
                                  expected_percents.cbegin(),
                                  expected_percents.cend());
}

BOOST_AUTO_TEST_CASE(whammy_is_added)
{
    NoteTrack track {{make_note(960), make_note(1632, 1920)},
                     {{Tick {960}, Tick {10}}, {Tick {1632}, Tick {10}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Path path {{{points.cbegin() + 5, points.cend(), Beat {1000.0}, Beat {9.0},
                 Beat {22.0}}},
               0};

    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_percent_values(sp_data, {}, points, path);
    std::vector<double> expected_percents {0.0, 0.25, 0.5275833333,
                                           0.5359166667, 0.49425};

    BOOST_REQUIRE_EQUAL(builder.sp_percent_values().size(),
                        expected_percents.size());
    for (std::size_t i = 0; i < expected_percents.size(); ++i) {
        BOOST_CHECK_CLOSE(builder.sp_percent_values()[i], expected_percents[i],
                          0.0001);
    }
}

BOOST_AUTO_TEST_CASE(forced_no_whammy_is_accounted_for)
{
    NoteTrack track {{make_note(960), make_note(1632, 1920)},
                     {{Tick {960}, Tick {10}}, {Tick {1632}, Tick {10}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Path path {{{points.cbegin() + 5, points.cend(), Beat {12.0}, Beat {9.0},
                 Beat {22.0}}},
               0};

    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_percent_values(sp_data, {}, points, path);
    std::vector<double> expected_percents {0.0, 0.25, 0.5275833333,
                                           0.4025833333, 0.2775833333};

    BOOST_REQUIRE_EQUAL(builder.sp_percent_values().size(),
                        expected_percents.size());
    for (std::size_t i = 0; i < expected_percents.size(); ++i) {
        BOOST_CHECK_CLOSE(builder.sp_percent_values()[i], expected_percents[i],
                          0.0001);
    }
}

BOOST_AUTO_TEST_CASE(forced_no_whammy_with_not_last_act_is_accounted_for)
{
    NoteTrack track {{make_note(960), make_note(1632, 1920), make_note(6336),
                      make_note(6528), make_note(7104)},
                     {{Tick {960}, Tick {10}},
                      {Tick {1632}, Tick {10}},
                      {Tick {6336}, Tick {10}},
                      {Tick {6528}, Tick {10}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Path path {{{points.cbegin() + 5, points.cend() - 3, Beat {12.0},
                 Beat {9.0}, Beat {28.8827}},
                {points.cend() - 1, points.cend(), Beat {1000.0}, Beat {37.0},
                 Beat {53.0}}},
               0};

    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_percent_values(sp_data, {}, points, path);
    std::vector<double> expected_percents {
        0.0,          0.25,         0.5275833333, 0.4025833333, 0.2775833333,
        0.1525833333, 0.0275833333, 0.0,          0.5,          0.40625};

    BOOST_REQUIRE_EQUAL(builder.sp_percent_values().size(),
                        expected_percents.size());
    for (std::size_t i = 0; i < expected_percents.size(); ++i) {
        BOOST_CHECK_CLOSE(builder.sp_percent_values()[i], expected_percents[i],
                          0.0001);
    }
}

// See /issues/4, Triathlon m662 on 100%/100%.
BOOST_AUTO_TEST_CASE(nearly_overlapped_phrases_are_handled_correctly)
{
    NoteTrack track {{make_note(0), make_note(192), make_note(384),
                      make_note(3224), make_note(3456)},
                     {{Tick {0}, Tick {10}},
                      {Tick {192}, Tick {10}},
                      {Tick {3224}, Tick {10}}},
                     {},
                     {},
                     {},
                     {},
                     TrackType::FiveFret,
                     std::make_shared<SongGlobalData>()};
    PointSet points {track,
                     {},
                     SqueezeSettings::default_settings(),
                     DrumSettings::default_settings(),
                     ChGuitarEngine()};
    SpData sp_data {
        track, {}, {}, SqueezeSettings::default_settings(), ChGuitarEngine()};
    Path path {{{points.cbegin() + 2, points.cbegin() + 2, Beat {17.0},
                 Beat {0.8958}, Beat {16.8958}}},
               50};

    ImageBuilder builder {
        track, {},  Difficulty::Expert, DrumSettings::default_settings(),
        false, true};
    builder.add_sp_percent_values(sp_data, {}, points, path);
    std::vector<double> expected_percents {0.40299375, 0.27799375, 0.15299375,
                                           0.02799375, 0.25};

    BOOST_REQUIRE_EQUAL(builder.sp_percent_values().size(),
                        expected_percents.size());
    for (std::size_t i = 0; i < expected_percents.size(); ++i) {
        BOOST_CHECK_CLOSE(builder.sp_percent_values()[i], expected_percents[i],
                          0.0001);
    }
}

BOOST_AUTO_TEST_SUITE_END()
