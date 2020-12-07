#if !defined(POKEMON_RANDOMISER_TILESET_H)
#define POKEMON_RANDOMISER_TILESET_H

#include <pokemon-randomiser/view.h>

namespace pokemon {
namespace tileset {

template <typename B = uint8_t, typename W = uint16_t>
class bgry : gameboy::rom::view<B, W> {
 public:
  using view = gameboy::rom::view<B, W>;
  using pointer = typename view::pointer;
  using lazy = typename view::lazy;
  using subviews = typename view::subviews;
  using lazies = typename view::lazies;

  /* The BGRY section used Bulbapedia extensively for its research content.
   *
   * See this link for ALL the detail:
   * https://bulbapedia.bulbagarden.net/wiki/User:Tiddlywinks/Map_header_data_structure_in_Generation_I
   */

 protected:
  static constexpr pointer start{0xc7be};
  static constexpr pointer end{0xc8dd};

 public:
  bgry(view v)
      : view{v},
        bank_{view::start().is(gameboy::rom::dt_rom_bank)},
        blocks_{view::after(bank_)
                    .is(gameboy::rom::dt_rom_offset)
                    .expect(gameboy::rom::e_little_endian)},
        tiles_{view::after(blocks_)
                   .is(gameboy::rom::dt_rom_offset)
                   .expect(gameboy::rom::e_little_endian)},
        collision_{view::after(tiles_)
                       .is(gameboy::rom::dt_rom_offset)
                       .expect(gameboy::rom::e_little_endian)},
        talkOver_{view::after(collision_).length(3).is(gameboy::rom::dt_bytes)},
        grass_{view::after(talkOver_).is(gameboy::rom::dt_byte)},
        animation_{view::after(grass_).is(gameboy::rom::dt_byte)},
        blocks{bank_, blocks_},
        tiles{bank_, tiles_},
        collision{bank_, tiles_},
        subviews_{&bank_,     &blocks_, &tiles_,    &collision_,
                  &talkOver_, &grass_,  &animation_},
        lazies_{&blocks, &tiles, &collision} {}

  static bgry byID(view v, uint8_t id) {
    return bgry{v.from(start + id * headerSize).length(headerSize)};
  }

  operator bool(void) const {
    return bool(view(*this)) && view::check(lazies_) && view::check(subviews_);
  }

 protected:
  view bank_;
  view blocks_;
  view tiles_;
  view collision_;
  view talkOver_;
  view grass_;
  view animation_;

 public:
  lazy blocks;
  lazy tiles;
  lazy collision;

 protected:
  const subviews subviews_;
  const lazies lazies_;

  static const long headerSize = 12;

  static const long headers(void) { return (end - start + 1) / headerSize; }
};

}  // namespace tileset
}  // namespace pokemon

#endif
