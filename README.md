# realsf2

realsf2 converts the music of Nintendo 64 games to MIDI, SoundFont 2 and SFZ. Each sequence gets a MIDI file and a
SoundFont containing its instruments. SFZ output is written beside them.

Drop images onto `realsf2.exe` on Windows or `Realsf2.app` on macOS. Output goes in a folder beside each image. From a
terminal:

```
realsf2 [options] <image> [...]

  --output DIR   write to DIR (one image only)
  --only REGEX   convert matching sequence names (for example --only 0x19)
  --loops N      repeat the main loop N times (default 1)
                 MIDI loop markers are included for players that support them
  --seed N       initial game random state (default 0; Paper Mario only)
  --list         list sequences without converting
  --quiet        print errors only
  --version      show the version
```

An image can be in any of the usual byte orders (`.z64`, `.n64` or `.v64`). The default output folder is
`<image name>_realsf2`, beside the image.

Each sequence gets these files:

- `<name>.mid`: the sequence as a standard MIDI file.
- `<name>.sf2`: a SoundFont that holds the sequence's instruments.
- `sfz/<name>/`: an SFZ file for each MIDI track.
- `sfz/samples/`: the WAV samples that all of the image's SFZ files share.

realsf2 recognises a game by the SHA-1 of its image. For an image that isn't in the table below, it looks for the SDK's
sequence files and takes the largest bank. Such a conversion leaves out what the game code does to its songs. It also
assumes an output rate of 22050 Hz. A game may use another rate.

## Supported games

| Game | Version | Sound engine |
|---|---|---|
| Chameleon Twist | USA, Rev 1 | The SDK's sequence players |
| Chameleon Twist 2 | USA | The SDK's sequence players |
| Cruis'n World | USA | The SDK's sequence players |
| GoldenEye 007 | USA | The SDK's sequence players |
| Hey You, Pikachu! | USA | A player of standard MIDI files, on the SDK's synthesizer |
| Mario Kart 64 | USA | EAD's engine, middle form |
| Mario Tennis | USA | The N64 Music Tools |
| Paper Mario | USA | Intelligent Systems' engine |
| Pokémon Puzzle League | USA | The N64 Music Tools |
| Pokémon Snap | USA | HAL's form of the SDK's compressed sequence player |
| Pokémon Stadium | USA, Rev 2 | The N64 Music Tools, in their early form |
| Pokémon Stadium 2 | USA | EAD's engine, later form |
| Super Mario 64 | USA | EAD's engine, early form |
| The Legend of Zelda: Majora's Mask | USA | EAD's engine, later form |
| The Legend of Zelda: Ocarina of Time | USA, Rev 2 | EAD's engine, later form |
| Wave Race 64 | USA, Rev 1 | EAD's engine, middle form |

## Drag and drop

- **Windows:** drop one or more images onto `realsf2.exe`. The console shows the results and waits for Enter before
  closing.
- **macOS:** drop images onto `Realsf2.app`, or open it to choose files. The app shows the results and can open the
  output folders. The app wraps the command-line program because Finder cannot drop files onto a bare executable. The
  [build](#building) includes the app.

## The conversion

A sequence on the Nintendo 64 is a program as much as a list of notes. realsf2 runs each sequence through a performer
that follows the game's sound engine. The performer records the resulting notes and controllers. Each performer follows
the engine's clock, voice rules and volume arithmetic.

The game's code often changes what a sequence plays. It sets a level for each song, a tempo, a mask of channels or a
choice between arrangements. realsf2 holds a profile for each game with what its code does there. Where a game plays a
song in more than one way, each way gets a track of its own. The track's name says which way it is. Examples:

- Majora's Mask plays some songs differently by the day. Fourteen of its song fanfares have another arrangement for each
  of Link's masked forms.
- Both Zelda games' file selects start their music without the harp's introduction.
- Ocarina of Time plays Ganon's Tower at the level that each room sets, from the entrance to Ganondorf's lair.
- Pokémon Stadium 2 plays three songs faster and two with other channels.
- Paper Mario's songs hold up to four compositions. Toad Town's mix depends on where the player stands, and the areas of
  Bowser's Castle play different tracks of its song.
- Mario Tennis plays one of its trophy songs faster for one character.

A wave's level, tuning, loop and envelope go in the SoundFont. Velocity, CC7 and CC11 act squared in SoundFont and SFZ.
The performers write values that give the engine's level through that law. CC91 holds a channel's reverb send. Where a
game's mixer takes a send's share from the dry level, CC11 holds that share too.

### SoundFont

SoundFont players apply 0.4 of a zone's initialAttenuation. The files take that into account: the generator holds 2.5
times the attenuation that the specification calls for. A player that follows the specification plays such zones too
quiet.

The game's pans are written for the sin/cos law of MIDI RP-036. The specification gives the default CC10 modulator an
amount of 1000. Every player found plays 500. Each global zone therefore holds a CC10 modulator of amount 500 in place
of the default one.

### SFZ

SFZ players ignore MIDI channels, and each file holds one instrument. Load the MIDI file in a DAW and assign an SFZ
player to each track with its matching file. The player tested is sfizz 1.2.3.

sfizz plays mono samples about 3 dB louder than the opcode documentation says. Every sample that realsf2 writes is
mono. The files don't correct for one player. A song therefore plays about 3 dB louder in sfizz than its SoundFont
does in FluidSynth. sfizz has other departures from the documentation. Its decays and releases end sooner than the
documented curve, and its filters are set up for 48 kHz until the sample rate changes after loading.

## Differences from the game

- A SoundFont envelope decays in a straight line of decibels. The engines' envelopes decay in straight lines of
  amplitude. No straight line of decibels meets such a ramp at both ends. For most games, realsf2 fits each release to
  the energy of the game's release. For Chameleon Twist 2, GoldenEye 007 and Hey You, Pikachu!, the fit follows the top
  20 dB of the game's ramp and leaves a tail.
- The game's reverb is left to the player. CC91 holds the send. A game's effect may sound stronger or weaker there.
- Sound effects that a song starts stay out of the song.
- Fades that the game's code starts during play stay out.
- A song whose tracks loop at different lengths runs to their least common multiple, up to a cut at 15 minutes.
- Wave Race 64's sequence 0x0E has an envelope that loops on held notes, which a SoundFont can't do. Its long notes
  hold one level where the game swells them.
- Some engines' finer effects are left out. realsf2 prints each one that a sequence uses.

## Building

Requires C++20 and CMake 3.20 or later. realsf2 supports macOS on Apple Silicon and Windows x64 with clang-cl.

- **Windows:** install Visual Studio 2022 with "Desktop development with C++", including CMake. Add the "C++ Clang tools
  for Windows" component. Use Developer PowerShell or Developer Command Prompt.
- **macOS:** install Xcode command line tools (`xcode-select --install`) and CMake (`brew install cmake` or the
  installer from cmake.org).

realsf2 builds on three libraries: seqlib, xsflib and ntdlib. The repository holds them as Git submodules in
`externals/`. Clone it with its submodules:

```sh
git clone --recursive <repository URL>
```

In a clone made without `--recursive`, run `git submodule update --init` first.

From the repository root:

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release
```

On Windows, the first command is `cmake -S . -B build -A x64 -T ClangCL`. It selects clang-cl and 64-bit output.

The executable is `build/realsf2`, or `build/Release/realsf2.exe` with Visual Studio. macOS builds also produce
`build/Realsf2.app`. The app holds a copy of the program. You can move the app elsewhere.

C++ formatting uses clang-format with the repository's `.clang-format`.

## Tests

`ctest` checks that the program reports its version. The conversion is in ntdlib. ntdlib's tests cover it with small
sequences and banks built by hand, and they require no game data.

## Licence

realsf2 is under the [MIT](LICENSE) licence. The libraries it builds on, seqlib, xsflib and ntdlib, are under the MIT
licence too. `THIRD-PARTY-NOTICES.md` gives their licence texts. Game data is not covered by this licence.
