#if !defined(POKEMON_RANDOMISER_MAP_H)
#define POKEMON_RANDOMISER_MAP_H

#include <pokemon-randomiser/tileset.h>

namespace pokemon {
namespace map {

template <typename B = char>
class bgry {
  /* The BGRY section used Bulbapedia extensively for its research content.
   *
   * See this link for ALL the detail:
   * https://bulbapedia.bulbagarden.net/wiki/User:Tiddlywinks/Map_header_data_structure_in_Generation_I
   */

 public:
  bgry(rom::bgry<B> &pROM, uint8_t pID)
      : rom(pROM),
        id(pID),
        ptr(header(id)),
        addr(rom.address(ptr)),
        tileset{rom, rom.byte(addr + 0x00)},
        height(rom.byte(addr + 0x01)),
        width(rom.byte(addr + 0x02)),
        dataOffset(rom.word_le(addr + 0x03)),
        textScriptsOffset(rom.word_le(addr + 0x05)),
        scriptOffset(rom.word_le(addr + 0x07)),
        ok(tileset) {}

  static const std::pair<uint8_t, uint16_t> header(const rom::bgry<B> &rom,
                                                   uint8_t id) {
    return {rom.byte(banksStart + id), rom.word_le(headersStart + 2 * id)};
  }

  const std::pair<uint8_t, uint16_t> header(uint8_t id) const {
    return header(rom, id);
  }

  static const std::set<uint8_t> list(rom::bgry<B> &rom) {
    std::set<uint8_t> rv{};

    for (long c = 0; c < 0xff; c++) {
      const auto m = bgry(rom, c);

      // try to load the map at the given address
      if (m) {
        // take a 0x0 map to mean we won't find any further maps
        if (m.empty()) {
          break;
        }

        rv.insert(c);
      } else {
        std::cout << " [ map #" << std::dec << c
                  << " omitted because it didn't load right]\n";
      }
    }

    return rv;
  }

  const std::set<uint8_t> list(void) const { return list(rom); }

  const long size(void) const { return ok ? long(height) * long(width) : 0; }

  const bool empty(void) const { return size() == 0; }

  operator bool(void) const { return ok; }

 protected:
  rom::bgry<B> &rom;

  const uint8_t id;

  const std::pair<uint8_t, uint16_t> ptr;
  const long addr;

  const pokemon::tileset::bgry<> tileset;

  uint8_t width, height;
  uint16_t dataOffset, textScriptsOffset, scriptOffset;

  const bool ok;

  static const long headersStart = 0x01ae;
  static const long banksStart = 0xc23d;
};

}  // namespace map
}  // namespace pokemon

#endif
