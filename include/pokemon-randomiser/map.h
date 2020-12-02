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
  bgry(rom::bgry<B> &pROM, uint8_t id) : rom(pROM), id(id) {}

  static const std::set<uint8_t> list(const rom::bgry<B> &rom) {
    std::set<uint8_t> rv{};

    for (long c = 0, i = mapHeadersStart, b = mapBanksStart; i <= mapHeadersEnd;
         c++, b++, i += 2) {
      const auto off = rom.word_le(i);
      const auto bank = rom.byte(b);

      std::cout << std::dec << " [map #" << c << "] 0x" << std::hex
                << std::setw(2) << std::setfill('0') << int64_t(bank) << ":"
                << std::hex << std::setw(4) << off << " = $" << std::setw(5)
                << rom.address(bank, off) << "\n";

      rv.insert(c);
    }

    return rv;
  }

  const std::set<uint8_t> list(void) const { return list(rom); }

 protected:
  rom::bgry<B> &rom;

  const uint8_t id;

  static const long mapHeadersStart = 0x01ae;
  static const long mapBanksStart = 0xc23d;
  static const long mapHeadersEnd = 0x0390;
};

}  // namespace map
}  // namespace pokemon

#endif
