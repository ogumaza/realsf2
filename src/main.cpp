// SPDX-License-Identifier: MIT

// realsf2: convert the music of Nintendo 64 games to MIDI, SoundFont and SFZ.
//
//     realsf2 [options] <image.z64> [...]
//
// Output goes beside each image. Windows accepts files dropped on the executable. macOS uses Realsf2.app.

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
    std::puts(
        "realsf2: convert the music of Nintendo 64 games to MIDI, SoundFont 2 and SFZ.\n"
        "\n"
        "usage: realsf2 [options] <image> [...]\n"
        "\n"
        "Give it a game's image (.z64, .n64 or .v64). Each sequence gets <name>.mid,\n"
        "<name>.sf2, and per-track SFZ files in sfz/<name>. Shared samples go in\n"
        "sfz/samples. Output is written to <image name>_realsf2 beside the image. You\n"
        "can also drop files onto realsf2.exe on Windows or Realsf2.app on macOS.\n"
        "\n"
        "options:\n"
        "  --output DIR   write to DIR (one image only)\n"
        "  --only REGEX   convert matching sequence names (for example --only 0x19)\n"
        "  --loops N      repeat the main loop N times (default 1)\n"
        "                 MIDI loop markers are included for players that support them\n"
        "  --seed N       initial game random state (default 0; Paper Mario only)\n"
        "  --list         list sequences without converting\n"
        "  --quiet        print errors only\n"
        "  --version      show the version\n");
}

std::optional<Input> Open(const fs::path& input, const std::string&)
{
    std::error_code error;
    if (!fs::exists(input, error))
    {
        return std::nullopt;
    }

    return Input::Open(input);
}

} // namespace

int main(int argc, char** argv)
{
    frame::Converter<PerformOptions, Input, Conversion> converter;
    converter.program = kProgram;
    converter.version_line = "realsf2 " REALSF2_VERSION;
    converter.input_word = "image";
    converter.folder_message = " is a folder: give an image of a game";
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

    auto run = [&](const std::vector<std::string>& args)
    {
        return frame::Run(args, converter);
    };
    return frame::RunMain(argc, argv, run);
}
