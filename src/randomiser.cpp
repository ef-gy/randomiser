#define ASIO_DISABLE_THREADS

#include <ef.gy/cli.h>
#include <pokemon-randomiser/map.h>
#include <pokemon-randomiser/rom.h>

static efgy::cli::flag<std::string> romFile("rom-file", "the ROM to load");

static efgy::cli::flag<std::string> output(
    "output", "the name of the file to write the changed ROM to");

static efgy::cli::flag<bool> clearTitleScreenPokemon(
    "clear-title-screen-pokemon", "clear out the Pokemon on the title screen");

static efgy::cli::flag<std::string> setStarterPokemon(
    "set-starter",
    "set list of starter Pokemon to the given list, separate Pokemon names "
    "with commas");

static efgy::cli::flag<bool> fixChecksum("fix-checksum",
                                         "fix up checksum in output ROM");

static efgy::cli::flag<bool> getStrings("strings",
                                        "like 'strings's for pokemon text");

static efgy::cli::flag<bool> getTitleScreenPokemon(
    "get-title-screen-pokemon",
    "figure out which Pokemon are on the title screen");

static efgy::cli::flag<bool> getAllPokemon(
    "get-all-pokemon", "show all Pokemon in the loaded ROM");

static efgy::cli::flag<long> getMap("get-map", -1,
                                    "show this map in the loaded ROM");

int main(int argc, char *argv[]) {
  efgy::cli::options opts(argc, argv);

  pokemon::rom::bgry<> rom(romFile);

  if (rom) {
    std::cout << rom.title() << "\n";

    if (::getStrings) {
      const auto strs = rom.getStrings();

      for (const auto str : strs) {
        std::cout << "0x" << std::hex << std::setw(6) << std::setfill('0')
                  << str.first.linear() << " " << str.second << "\n";
      }
    }

    if (::clearTitleScreenPokemon) {
      rom.clearTitleScreenPokemon();
    }

    if (::fixChecksum) {
      rom.fixChecksum();
    }

    if (std::string(::setStarterPokemon) != "") {
      std::string input = std::string(::setStarterPokemon);
      std::set<std::string> pokemon{};

      std::size_t pos = 0, prev = 0;
      static const std::string dlim = ",";
      while ((pos = input.find(dlim, prev)) != std::string::npos) {
        pokemon.insert(input.substr(prev, pos - prev));
        prev = pos + 1;
      }

      pokemon.insert(input.substr(prev));
      rom.setStarterPokemon(pokemon);
    }

    if (::getTitleScreenPokemon) {
      std::cout << "title screen Pokemon:\n";
      const auto titleScreen = rom.getTitleScreenPokemon();
      std::cout << pokemon::listPokemon(rom.getPokemonNames(titleScreen));
    }

    std::cout << "starter Pokemon:\n";
    const auto starter = rom.getStarterPokemon();
    std::cout << pokemon::listPokemon(rom.getPokemonNames(starter));

    if (::getAllPokemon) {
      auto ids = rom.getAllPokemonIds();

      std::cout << pokemon::listPokemon(ids);
    }

    if (long(::getMap) >= 0) {
      std::cout << "map #" << long(::getMap) << "\n";

      auto m = pokemon::map::bgry<>(rom, long(::getMap));

      if (m) {
        std::cout << "  - size {W,H}@T: {" << uint16_t(m.width()) << ","
                  << uint16_t(m.height()) << "}@" << m.size() << "\n";

        if (m.text) {
          std::cout << "  - text scripts at: 0x" << std::hex
                    << m.text.resolve().linear() << "\n";
        }
      } else {
        std::cerr << "invalid map data\n";
      }
    }

    if (!std::string(output).empty()) {
      rom.save(output);
    }
  } else {
    std::cerr << "NOT LOADED\n";

    if (rom.checksum()) {
      std::cout << "CHECKSUM OK\n";
    } else {
      std::cout << "CHECKSUM NOT OK (" << rom.romChecksum() << " vs "
                << rom.headerChecksum() << ")\n";
    }
  }

  return 0;
}
