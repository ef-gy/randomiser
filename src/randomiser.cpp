#define ASIO_DISABLE_THREADS

#include <ef.gy/cli.h>

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>

static efgy::cli::flag<std::string> romFile(
  "rom-file", "the ROM to load");

static efgy::cli::flag<std::string> output(
  "output", "the name of the file to write the changed ROM to");

static efgy::cli::flag<bool> getStrings(
  "strings", "like 'strings's for pokemon text");

static efgy::cli::flag<bool> getTitleScreenPokemon(
  "get-title-screen-pokemon", "figure out which Pokemon are on the title screen");

static efgy::cli::flag<bool> clearTitleScreenPokemon(
  "clear-title-screen-pokemon", "clear out the Pokemon on the title screen");

static efgy::cli::flag<std::string> setStarterPokemon(
   "set-starter", "set list of starter Pokemon to the given list, separate Pokemon names with commas");

static efgy::cli::flag<long> minStringLength(
  "min-string-length", 2, "strings shorter than this are ignored in --strings");

static efgy::cli::flag<bool> fixChecksum(
  "fix-checksum", "fix up checksum in output ROM");

static std::map<char,std::string> pokemonTextMap({
  {0x4F, " "},
  {0x57, "#"},
  {0x51, "*"},
  {0x52, "A1"},
  {0x53, "A2"},
  {0x54, "POKé"},
  {0x55, "+"},
  {0x58, "$"},
  {0x75, "…"},
  {0x7F, ""},
  {0x80, "A"},
  {0x81, "B"},
  {0x82, "C"},
  {0x83, "D"},
  {0x84, "E"},
  {0x85, "F"},
  {0x86, "G"},
  {0x87, "H"},
  {0x88, "I"},
  {0x89, "J"},
  {0x8A, "K"},
  {0x8B, "L"},
  {0x8C, "M"},
  {0x8D, "N"},
  {0x8E, "O"},
  {0x8F, "P"},
  {0x90, "Q"},
  {0x91, "R"},
  {0x92, "S"},
  {0x93, "T"},
  {0x94, "U"},
  {0x95, "V"},
  {0x96, "W"},
  {0x97, "X"},
  {0x98, "Y"},
  {0x99, "Z"},
  {0x9A, "("},
  {0x9B, ")"},
  {0x9C, ":"},
  {0x9D, ";"},
  {0x9E, "["},
  {0x9F, "]"},
  {0xA0, "a"},
  {0xA1, "b"},
  {0xA2, "c"},
  {0xA3, "d"},
  {0xA4, "e"},
  {0xA5, "f"},
  {0xA6, "g"},
  {0xA7, "h"},
  {0xA8, "i"},
  {0xA9, "j"},
  {0xAA, "k"},
  {0xAB, "l"},
  {0xAC, "m"},
  {0xAD, "n"},
  {0xAE, "o"},
  {0xAF, "p"},
  {0xB0, "q"},
  {0xB1, "r"},
  {0xB2, "s"},
  {0xB3, "t"},
  {0xB4, "u"},
  {0xB5, "v"},
  {0xB6, "w"},
  {0xB7, "x"},
  {0xB8, "y"},
  {0xB9, "z"},
  {0xBA, "é"},
  {0xBB, "'d"},
  {0xBC, "'l"},
  {0xBD, "'s"},
  {0xBE, "'t"},
  {0xBF, "'v"},
  {0xE0, "'"},
  {0xE1, "PK"},
  {0xE2, "MN"},
  {0xE3, "-"},
  {0xE4, "'r"},
  {0xE5, "'m"},
  {0xE6, "?"},
  {0xE7, "!"},
  {0xE8, "."},
  {0xED, "→"},
  {0xEE, "↓"},
  {0xEF, "♂"},
  {0xF0, "¥"},
  {0xF1, "×"},
  {0xF3, "/"},
  {0xF4, ","},
  {0xF5, "♀"},
  {0xF6, "0"},
  {0xF7, "1"},
  {0xF8, "2"},
  {0xF9, "3"},
  {0xFA, "4"},
  {0xFB, "5"},
  {0xFC, "6"},
  {0xFD, "7"},
  {0xFE, "8"},
  {0xFF, "9"},
});

static uint8_t getRecodedText(char c) {
  for (const auto p : pokemonTextMap) {
    if (p.second.size() > 0) {
      if (c == p.second[0]) {
        return p.first;
      }
    }
  }

  return 0x50;
}

template<typename B = char>
class ROM {
  public:
    ROM(std::string file = std::string(romFile))
      : loadOK(false) {
      if (file != "") {
        load(file);
      }
    }

    bool load(std::string file) {
      loadOK = false;

      std::ifstream rom(std::string(romFile), std::ios::binary | std::ios::ate);
      std::streamsize size = rom.tellg();
      rom.seekg(0, std::ios::beg);

      image.resize(size);

      if (rom.read(image.data(), size)) {
        std::cerr << "read ROM, size=" << size << "\n";

        loadOK = true;
      }

      return loadOK;
    }

    bool save(std::string file) {
      std::ofstream rom(file, std::ios::binary | std::ios::ate);
      std::streamsize size = image.size();

      if (rom.write(image.data(), size)) {
        std::cerr << "write ROM, size=" << size << "\n";
        return true;
      }

      return false;
    }

    operator bool(void) {
      return loadOK;
    }

    std::string getString(long start, long end) const {
      std::string rv = "";

      for (long i = start; i <= end; i++) {
        const std::string v = pokemonTextMap[image[i]];

        if (v != "") {
          rv += v;
        }
      }

      return rv;
    }

    std::map<long, std::string> getStrings(void) const {
      std::map<long, std::string> rv;
      std::string line = "";
      unsigned long start = 0;

      for (unsigned long i = 0; i <= image.size(); i++) {
        const std::string v = pokemonTextMap[image[i]];

        if (v != "") {
          if (line.empty()) {
            start = i;
          }
          line += v;
        } else {
          if (line.size() > long(minStringLength)) {
            rv[start] = line;
          }
          line.clear();
        }
      }

      return rv;
    }

    std::string getPokemonName(long iid) const {
      std::ostringstream os;
      os.clear();

      static long nameBase = 0x1c228;

      os << getString(nameBase + (iid-2)*0x0a, nameBase + (iid-1)*0x0a - 1);
      return os.str();
    }

    const std::vector<uint8_t> getTitleScreenPokemon() const {
      std::vector<uint8_t> rv;
      rv.clear();

      for (long i = 0x4588; i < 0x4597; i++) {
        rv.push_back(uint8_t(image[i]));
      }

      return rv;
    }

    const std::vector<uint8_t> getStarterPokemon(void) const {
      std::vector<uint8_t> rv;
      rv.clear();

      for (long i = 0x3a1e3; i <= 0x3a1eb /*0x3A1F4*/; i+=3) {
        rv.push_back(uint8_t(image[i+2]));
      }

      return rv;
    }

    bool setStarterPokemon(const std::vector<std::string> &starter) {
      bool ok = true;

      auto ids = getAllPokemonIds();

      std::cerr << ids.size() << "\n";

      long n = 0;
      for (const auto st : starter) {
        long p = 0x0;
          switch(n) {
            case 0:
              for (auto n : std::vector<long>{0x1D126, 0x1CC84, 0x1D10E, 0x39CF8, 0x50FB3, 0x510DD, 0x3a1eb}) {
                image[n] = ids[st];
              }
              p = 0x94e23;
              for (long pn=0; pn <= 0x9; p++, pn++) {
                uint16_t r = getRecodedText(st[pn]);
                image[p] = r;
              }
              
              break;
            case 1:
              for (auto n : std::vector<long>{0x1D104, 0x19591, 0x1CC88, 0x1CDC8, 0x1D11F, 0x50FAF, 0x510D9, 0x51CAF, 0x6060E, 0x61450, 0x75F9E, 0x3a1e5}) {
                image[n] = ids[st];
              }
              p = 0x94e4d;
              for (long pn=0; pn <= 0x9; p++, pn++) {
                uint16_t r = getRecodedText(st[pn]);
                image[p] = r;
              }
              break;
            case 2:
              for (auto n : std::vector<long>{0x1D115, 0x19599, 0x1CDD0, 0x1D130, 0x39CF2, 0x50FB1, 0x510DB, 0x51CB7, 0x60616, 0x61458, 0x75FA6, 0x3a1e8}) {
                image[n] = ids[st];
              }
              p = 0x94e69;
              for (long pn=0; pn <= 0x9; p++, pn++) {
                uint16_t r = getRecodedText(st[pn]);

                image[p] = r;
              }
              break;
          }
          n++;

          if (n >= 3) {
            break;
          }
      }

      long i = 0x4588;
      for (const auto &st : starter) {
        if (i <= 0x458a) {
          image[i] = ids[st];
          i += 1;
        }
      }

      return ok;
    }

    const std::map<uint8_t, std::string> getPokemonNames(const std::vector<uint8_t> &r) const {
      std::map<uint8_t, std::string> rv;
      rv.clear();
      for (uint8_t v : r) {
        rv[v] = getPokemonName(v);
      }
      return rv;
    }

    const std::map<std::string, uint8_t> getPokemonIds(const std::vector<uint8_t> &r) const {
      std::map<std::string, uint8_t> rv;
      rv.clear();
      for (uint8_t v : r) {
        rv[getPokemonName(v)] = v;
      }
      return rv;
    }

    const std::map<std::string, uint8_t> getAllPokemonIds() const {
      std::map<std::string, uint8_t> rv;
      rv.clear();
      std::string s = "";
      for (unsigned int i = 1; i < 190; i++) {
        s = getPokemonName(i);
        rv[s] = i;
      }

      return rv;
    }

    void clearTitleScreenPokemon() {
      long n = 0;
      for (long i = 0x4588; i <= 0x4597; i++) {
        switch(n) {
          case 0:
            image[i] = 0xb1;
            break;
          case 1:
            image[i] = 0x99;
            break;
          case 2:
            image[i] = 0xb0;
            break;
          default:
            image[i] = 0x00;
            break;
        } 
        n++;
      }

      image[0x4399] = 0xb1;
    }

    std::string title(void) const {
      std::string t = "";

      static const long start = 0x134;
      static const long end = 0x143;

      for (long i = start; i <= end; i++) {
        if (image[i] != 0) {
          t += image[i];
        }
      }

      return t == "" ? "(NOT SET)" : t;
    }

    long romChecksum(void) const {
      uint32_t checksum = 0;

      static const long high = 0x14e;
      static const long low = 0x14f;

      for (long i = 0; i < image.size(); i++) {
        if (i != high && i != low) {
          checksum += uint8_t(image.data()[i]);
        }
      }

      return uint16_t(checksum);
    }

    long headerChecksum(void) const {
      uint16_t checksum = 0;

      static const long high = 0x14e;
      static const long low = 0x14f;

      checksum = uint8_t(image.data()[high]) << 8;
      checksum |= uint8_t(image.data()[low]);

      return checksum;
    }

    bool checksum(void) const {
      return romChecksum() == headerChecksum();
    }

    bool fixChecksum(void) {
      uint16_t checksum = romChecksum();

      static const long high = 0x14e;
      static const long low = 0x14f;

      image.data()[high] = uint8_t(checksum >> 8);
      image.data()[low] = uint8_t(checksum & 0xff);

      return this->checksum();
    }

    std::vector<B> image;

  protected:
    bool loadOK;
};

int main(int argc, char *argv[]) {
  efgy::cli::options opts(argc, argv);

  ROM<> rom(romFile);

  if (rom) {
    std::cout << rom.title() << "\n";
  
    if (rom.checksum()) {
      std::cout << "CHECKSUM OK\n";
    } else {
      std::cout << "CHECKSUM NOT OK (" << rom.romChecksum() <<  " vs " << rom.headerChecksum() << ")\n";
    }

    std::cout << rom.getString(0x1A58, 0x1A5F) << "\n";

    if (::getStrings) {
      const auto strs = rom.getStrings();

      for (const auto str : strs) {
        std::cout << "0x" << std::hex << std::setw(6) << std::setfill('0') << str.first << " " << str.second << "\n";
      }
    }

    if (::clearTitleScreenPokemon) {
      rom.clearTitleScreenPokemon();
    }

    if (::getTitleScreenPokemon) {
      std::cout << "title screen Pokemon:\n";
      const auto titleScreen = rom.getTitleScreenPokemon();
      const auto titleScreenNames = rom.getPokemonNames(titleScreen);

      for (const auto pk : titleScreenNames) {
        int16_t v = uint8_t(pk.first);
        std::cout << std::hex << v << " [" << pk.second << "]\n";
      }
    }

    if (::fixChecksum) {
      rom.fixChecksum();
    }

    if (std::string(::setStarterPokemon) != "") {
      std::string input = std::string(::setStarterPokemon);
      std::vector<std::string> pokemon;
      pokemon.clear();

      std::size_t pos = 0, prev = 0;
      static const std::string dlim = ",";
      while ((pos = input.find(dlim, prev)) != std::string::npos) {
        pokemon.push_back(input.substr(prev, pos - prev));
        prev = pos + 1;
      }

      pokemon.push_back(input.substr(prev));

      rom.setStarterPokemon(pokemon);
    }

    std::cout << "starter Pokemon:\n";
    const auto starter = rom.getStarterPokemon();
    const auto starterNames = rom.getPokemonNames(starter);
    for (const auto pk : starterNames) {
      int16_t v = uint8_t(pk.first);
      std::cout << std::hex << v << " [" << pk.second << "]\n";
    }

    if (std::string(output) != "") {
      rom.save(output);
    }
  }

  return 0;
}
