#define ASIO_DISABLE_THREADS

#include <ef.gy/cli.h>
#include <pokemon-randomiser/character-map.h>

#include <fstream>
#include <iostream>
#include <sstream>

static efgy::cli::flag<std::string> romFile("rom-file", "the ROM to load");

static efgy::cli::flag<std::string> output(
    "output", "the name of the file to write the changed ROM to");

static efgy::cli::flag<bool> clearTitleScreenPokemon(
    "clear-title-screen-pokemon", "clear out the Pokemon on the title screen");

static efgy::cli::flag<std::string> setStarterPokemon(
    "set-starter",
    "set list of starter Pokemon to the given list, separate Pokemon names "
    "with commas");

static efgy::cli::flag<long> minStringLength(
    "min-string-length", 2,
    "strings shorter than this are ignored in --strings");

static efgy::cli::flag<bool> fixChecksum("fix-checksum",
                                         "fix up checksum in output ROM");

static efgy::cli::flag<bool> getStrings("strings",
                                        "like 'strings's for pokemon text");

static efgy::cli::flag<bool> getTitleScreenPokemon(
    "get-title-screen-pokemon",
    "figure out which Pokemon are on the title screen");

static uint8_t getRecodedText(char c) {
  // TODO: this function needs to be modified so that the reverse of this is
  // always the same as the source text - or at the very least the shortest
  // subset, if it starts with P. :)
  for (const auto p : pokemon::randomiser::text::map::gen1) {
    if (p.second.size() > 0) {
      if (c == p.second[0]) {
        return p.first;
      }
    }
  }

  return 0x50;
}

static const std::string listPokemon(
    const std::map<std::string, uint8_t> &ids) {
  std::ostringstream os;
  os.clear();

  os << ids.size() << "\n";
  for (const auto &pk : ids) {
    int16_t v = uint8_t(pk.second);
    os << "0x" << std::hex << std::setw(2) << std::setfill('0') << v << " ["
       << pk.first << "]\n";
  }

  return os.str();
}

static const std::string listPokemon(
    const std::map<uint8_t, std::string> &ids) {
  std::ostringstream os;
  os.clear();

  os << ids.size() << "\n";
  for (const auto &pk : ids) {
    int16_t v = uint8_t(pk.first);
    os << "0x" << std::hex << std::setw(2) << std::setfill('0') << v << " ["
       << pk.second << "]\n";
  }

  return os.str();
}
template <typename B = char>
class ROM {
 public:
  ROM(std::string file = std::string(romFile)) : loadOK(false) {
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

  operator bool(void) { return loadOK; }

  std::string getString(long start, long end) const {
    std::string rv = "";

    for (long i = start; i <= end; i++) {
      const std::string v = pokemon::randomiser::text::map::gen1[image[i]];

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
      const std::string v = pokemon::randomiser::text::map::gen1[image[i]];

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

  const std::string dump(long start, long end, int alignment) const {
    std::ostringstream os;
    os.clear();

    for (long i = start; i <= end; i++) {
      int16_t v = uint8_t(image[i]);

      if ((i - start) % alignment == 0) {
        os << "\n";
      }

      os << " \t0x" << std::hex << std::setw(2) << std::setfill('0') << v;

      if (pokemon::randomiser::text::map::gen1.count(v) == 1) {
        os << " " << std::setw(4) << std::setfill('.')
           << pokemon::randomiser::text::map::gen1[v];
      } else {
        os << " ....";
      }
    }

    os << "\n";

    return os.str();
  }

  const uint16_t word_le(long start) const {
    return uint16_t(uint8_t(image.data()[(start + 1)]) << 8 |
                    uint8_t(image.data()[(start)]));
  }

  const long address(uint8_t bank, uint16_t off) const {
    return bank * 0x4000 + off - (off > 0x4000 ? 0x4000 : 0);
  }

  const std::string getMaps(void) const {
    static const long mapHeadersStart = 0x01ae;
    static const long mapBanksStart = 0xc23d;
    static const long mapHeadersEnd = 0x0390;

    std::ostringstream os;
    os.clear();

    os << "maps\n";

    for (long c = 0, i = mapHeadersStart, b = mapBanksStart; i <= mapHeadersEnd;
         c++, b++, i += 2) {
      const auto off = word_le(i);

      uint16_t bank = uint8_t(image.data()[b]);

      os << std::dec << " [map #" << c << "] 0x" << std::hex << std::setw(2)
         << std::setfill('0') << bank << ":" << std::hex << std::setw(4) << off
         << " = $" << std::setw(5) << address(bank, off) << "\n";
    }

    // os << dump(mapHeadersStart, mapHeadersEnd, 3);

    os << "\n";

    return os.str();
  }

  std::string getPokemonName(long iid) const {
    std::ostringstream os;
    os.clear();

    static long nameBase = 0x1c228;

    os << getString(nameBase + (iid - 2) * 0x0a,
                    nameBase + (iid - 1) * 0x0a - 1);
    return os.str();
  }

  const std::set<uint8_t> getTitleScreenPokemon(void) const {
    std::set<uint8_t> rv{};

    for (long i = 0x4588; i < 0x4597; i++) {
      rv.insert(uint8_t(image[i]));
    }

    return rv;
  }

  const std::set<uint8_t> getStarterPokemon(void) const {
    std::set<uint8_t> rv{};

    for (const auto p : getStarterPointers()) {
      for (const auto pn : p.second) {
        rv.insert(uint8_t(image[pn]));
      }
    }

    return rv;
  }

  std::map<long, std::set<long>> getStarterPointers() const {
    std::map<long, std::set<long>> rv{};

    if (title() == "POKEMON RED") {
      // TODO: research these pointers and figure out what they actually belong
      // to, then derive the addresses "properly" - suspect most of these are
      // encounter data.
      rv[0] = {0x1d126, 0x1cc84, 0x1d10E, 0x39cf8, 0x3a1eb, 0x50fb3, 0x510dd};
      rv[1] = {0x1d104, 0x19591, 0x1cc88, 0x1cdc8, 0x1d11f, 0x3a1e5,
               0x50faf, 0x510d9, 0x51caf, 0x6060e, 0x61450, 0x75f9e};
      rv[2] = {0x1d115, 0x19599, 0x1cdd0, 0x1d130, 0x39cf2, 0x3a1e8,
               0x50fb1, 0x510db, 0x51cb7, 0x60616, 0x61458, 0x75fa6};
    }

    return rv;
  }

  std::map<long, std::set<long>> getStarterTextPointers() const {
    std::map<long, std::set<long>> rv{};

    if (title() == "POKEMON RED") {
      rv[0] = {0x94e23};
      rv[1] = {0x94e4d};
      rv[2] = {0x94e69};
    }

    return rv;
  }

  bool setStarterPokemon(const std::set<std::string> &starter) {
    bool ok = true;

    auto ids = getAllPokemonIds();

    std::cout << listPokemon(ids);

    long n = 0;
    auto sps = getStarterPointers();
    auto spt = getStarterTextPointers();

    for (const auto st : starter) {
      if (sps.count(n) == 1) {
        for (const auto i : sps[n]) {
          image[i] = ids[st];
        }
      }
      if (spt.count(n) == 1) {
        for (auto p : spt[n]) {
          for (long pn = 0; pn <= 0x9; p++, pn++) {
            uint16_t r = getRecodedText(st[pn]);
            image[p] = r;
          }
        }
      }
      n++;
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

  const std::map<uint8_t, std::string> getPokemonNames(
      const std::set<uint8_t> &r) const {
    std::map<uint8_t, std::string> rv{};
    for (uint8_t v : r) {
      rv[v] = getPokemonName(v);
    }
    return rv;
  }

  const std::map<std::string, uint8_t> getPokemonIds(
      const std::set<uint8_t> &r) const {
    std::map<std::string, uint8_t> rv{};
    for (uint8_t v : r) {
      rv[getPokemonName(v)] = v;
    }
    return rv;
  }

  const std::map<std::string, uint8_t> getAllPokemonIds() const {
    std::set<uint8_t> r{};
    for (unsigned i = 1; i < 190; i++) {
      r.insert(i);
    }
    return getPokemonIds(r);
  }

  void clearTitleScreenPokemon() {
    long n = 0;
    for (long i = 0x4588; i <= 0x4597; i++) {
      switch (n) {
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

  bool checksum(void) const { return romChecksum() == headerChecksum(); }

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
      std::cout << "CHECKSUM NOT OK (" << rom.romChecksum() << " vs "
                << rom.headerChecksum() << ")\n";
    }

    std::cout << rom.getString(0x1a58, 0x1a5f) << "\n";

    if (::getStrings) {
      const auto strs = rom.getStrings();

      for (const auto str : strs) {
        std::cout << "0x" << std::hex << std::setw(6) << std::setfill('0')
                  << str.first << " " << str.second << "\n";
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
      std::cout << listPokemon(rom.getPokemonNames(titleScreen));
    }

    std::cout << "starter Pokemon:\n";
    const auto starter = rom.getStarterPokemon();
    std::cout << listPokemon(rom.getPokemonNames(starter));

    std::cout << rom.getMaps();

    if (std::string(output) != "") {
      rom.save(output);
    }
  }

  return 0;
}
