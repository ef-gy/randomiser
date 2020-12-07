#if !defined(POKEMON_RANDOMISER_TILESET_H)
#define POKEMON_RANDOMISER_TILESET_H

#include <pokemon-randomiser/rom.h>
#include <pokemon-randomiser/view.h>

namespace pokemon {
namespace tileset {

template <typename B = uint8_t, typename W = uint16_t>
class bgry : gameboy::rom::view<B, W> {
 public:
  using view = gameboy::rom::view<B, W>;
  using pointer = typename view::pointer;
  using subviews = typename view::subviews;

  /* The BGRY section used Bulbapedia extensively for its research content.
   *
   * See this link for ALL the detail:
   * https://bulbapedia.bulbagarden.net/wiki/User:Tiddlywinks/Map_header_data_structure_in_Generation_I
   */

 public:
  bgry(rom::bgry<> &pROM, uint8_t pID)
      : view{view(pROM)
                 .from(headersStart + id * headerSize)
                 .length(headerSize)},
        id(pID),
        bank{view::start().is(gameboy::rom::dt_byte)},
        blocks{view::after(bank)
                   .is(gameboy::rom::dt_word)
                   .expect(gameboy::rom::e_little_endian)},
        tiles{view::after(blocks)
                  .is(gameboy::rom::dt_word)
                  .expect(gameboy::rom::e_little_endian)},
        collision{view::after(tiles)
                      .is(gameboy::rom::dt_word)
                      .expect(gameboy::rom::e_little_endian)},
        talkOver{view::after(collision).length(3).is(gameboy::rom::dt_bytes)},
        grass{view::after(talkOver).is(gameboy::rom::dt_byte)},
        animation{view::after(grass).is(gameboy::rom::dt_byte)},
        subviews_{&bank,     &blocks, &tiles,    &collision,
                  &talkOver, &grass,  &animation} {}

  const uint8_t id;

  view bank;
  view blocks;
  view tiles;
  view collision;
  view talkOver;
  view grass;
  view animation;

  operator bool(void) const {
    return id < headers() && bool(view(*this)) && view::check(subviews_);
  }

 protected:
  const subviews subviews_;

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
