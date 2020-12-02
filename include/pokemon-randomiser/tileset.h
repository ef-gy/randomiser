#if !defined(POKEMON_RANDOMISER_TILESET_H)
#define POKEMON_RANDOMISER_TILESET_H

#include <pokemon-randomiser/rom.h>

namespace pokemon {
namespace tileset {

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
        addr(headersStart + id * headerSize),
        bank(rom.byte(addr + 0x00)),
        blocksOffset(rom.word_le(addr + 0x01)),
        tilesOffset(rom.word_le(addr + 0x03)),
        collisionOffset(rom.word_le(addr + 0x05)),
        talkOver{rom.byte(addr + 0x07), rom.byte(addr + 0x08),
                 rom.byte(addr + 0x09)},
        grass(rom.byte(addr + 0x0a)),
        animation(rom.byte(addr + 0x0b)),
        ok(id < headers()) {}

  operator bool(void) const { return ok; }

  const rom::bgry<B> &rom;
  const uint8_t id;

  const int addr;
  const uint8_t bank;

  const uint16_t blocksOffset;
  const uint16_t tilesOffset;
  const uint16_t collisionOffset;
  const uint8_t talkOver[3];
  const uint8_t grass;
  const uint8_t animation;

  const bool ok;

 protected:
  static const long headersStart = 0xc7be;
  static const long headersEnd = 0xc8dd;
  static const long headerSize = 12;

  static const long headers(void) {
    return (headersEnd - headersStart + 1) / headerSize;
  }
};

}  // namespace tileset
}  // namespace pokemon

#endif
