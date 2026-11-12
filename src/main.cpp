// SPDX-License-Identifier: MIT

// realsf2: convert the music of Nintendo 64 games to MIDI, SoundFont and SFZ.
//
//     realsf2 [options] <image.z64 | set.zip | set folder> [...]
//
// Output goes beside each image or set. Windows accepts files dropped on the executable. macOS uses Realsf2.app.

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "ntdlib/sequence_export/frame.h"
#include "ntdlib/sequence_export/n64/input.h"
#include "ntdlib/sequence_export/n64/instruments.h"
#include "seqlib/sfz/track_file.h"

namespace
{

namespace fs = std::filesystem;
namespace frame = ntdlib::sequence_export;
using ntdlib::sequence_export::n64::Conversion;
using ntdlib::sequence_export::n64::Input;
using ntdlib::sequence_export::n64::PerformOptions;

constexpr char kProgram[] = "realsf2";

void Usage()
{
    std::string text =
        "realsf2: convert the music of Nintendo 64 games to MIDI, SoundFont 2 and SFZ.\n"
        "\n"
        "usage: realsf2 [options] <image or set> [...]\n"
        "\n"
        "Give it a game's image (.z64, .n64 or .v64), which may be in a ZIP archive, or\n"
        "an Aleck64 game's ROM set as a ZIP archive or a folder of its chips. Each\n"
        "sequence gets <name>.mid, <name>.sf2, and per-track SFZ files in sfz/<name>.\n"
        "Shared samples go in sfz/samples. Output is written to\n"
        "<image or set name>_realsf2 beside it. You can also drop files onto\n"
        "realsf2.exe on Windows or Realsf2.app on macOS.\n"
        "\n"
        "options:\n"
        "  --output DIR   write to DIR (one image or set only)\n"
        "  --only REGEX   convert matching sequence names (for example --only 0x19)\n";
    text += frame::kLoopsHelp;
    text += "  --seed N       initial game random state (default 0; Paper Mario only)\n";
    text += frame::kListQuietVersionHelp;

    std::puts(text.c_str());
}

// Reads the image or set at `input` (Input::Open). Returns nothing when no file or folder is there. The frame then
// reports it.
std::optional<Input> Open(const fs::path& input, const std::string&)
{
    std::error_code error;
    if (!fs::exists(input, error))
    {
        return std::nullopt;
    }

    return Input::Open(input, kProgram);
}

} // namespace

int main(int argc, char** argv)
{
    frame::Converter<PerformOptions, Input, Conversion> converter;
    converter.program = kProgram;
    converter.version_line = "realsf2 " REALSF2_VERSION;
    converter.input_word = "input";
    converter.reads_folders = true;
    converter.samples_folder = seqlib::sfz::kSamplesFolder;
    converter.usage = Usage;
    converter.open = Open;
    converter.sequences = std::mem_fn(&Input::Sequences);
    converter.convert = [](Input& input, std::size_t index, const PerformOptions& options)
    {
        PerformOptions named = options;
        named.program = kProgram;
        return input.Convert(index, named);
    };

    const auto run = [&converter](const std::vector<std::string>& args)
    {
        return frame::Run(args, converter);
    };

    return frame::RunMain(argc, argv, run);
}
