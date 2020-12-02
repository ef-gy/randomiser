#if !defined(POKEMON_RANDOMISER_MAP_H)
#define POKEMON_RANDOMISER_MAP_H

#include <pokemon-randomiser/rom.h>

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
  bgry(rom::bgry<B> &pROM, uint8_t id) : rom(pROM), id(id), ok(refresh()) {}

  bool refresh(void) {
    ok = true;

    const auto h = header(id);
    const auto base = rom.address(h);

    bank = h.first;
    offset = h.second;

    tileset = rom.byte(base + 0x00);
    height = rom.byte(base + 0x01);
    width = rom.byte(base + 0x01);

    std::cout << std::dec << " [map #" << uint16_t(id) << "] 0x" << std::hex
              << std::setw(2) << std::setfill('0') << int64_t(bank) << ":"
              << std::hex << std::setw(4) << offset << " = $" << std::setw(5)
              << base << "\n";

    std::cout << "   - dimensions: "
              << "0x" << std::setw(2) << uint16_t(width) << " * 0x"
              << std::setw(2) << uint16_t(height) << "\n"
              << "   - tileset: 0x" << std::setw(2) << uint16_t(tileset)
              << "\n";

    return ok;
  }

  static const std::pair<uint8_t, uint16_t> header(const rom::bgry<B> &rom,
                                                   uint8_t id) {
    return {rom.byte(mapBanksStart + id),
            rom.word_le(mapHeadersStart + 2 * id)};
  }

  const std::pair<uint8_t, uint16_t> header(uint8_t id) const {
    return header(rom, id);
  }

  static const std::set<uint8_t> list(const rom::bgry<B> &rom) {
    std::set<uint8_t> rv{};

    for (long c = 0, i = mapHeadersStart, b = mapBanksStart; i <= mapHeadersEnd;
         c++, b++, i += 2) {
      const auto off = rom.word_le(i);
      const auto bank = rom.byte(b);

      rv.insert(c);
    }

    return rv;
  }

  const std::set<uint8_t> list(void) const { return list(rom); }

 protected:
  rom::bgry<B> &rom;

  const uint8_t id;

  bool ok;

  uint8_t bank;
  uint16_t offset;

  uint8_t tileset, width, height;

  static const long mapHeadersStart = 0x01ae;
  static const long mapBanksStart = 0xc23d;
  static const long mapHeadersEnd = 0x0390;
};

}  // namespace map
}  // namespace pokemon

#endif
