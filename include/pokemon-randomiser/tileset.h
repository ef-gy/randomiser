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
      : view{v.asLittleEndian().toBankEnd()},
        bank_{view::start().asROMBank()},
        blocks_{view::after(bank_).asROMOffset()},
        tiles_{view::after(blocks_).asROMOffset()},
        collision_{view::after(tiles_).asROMOffset()},
        talkOver_{view::after(collision_).length(3).is(gameboy::dt_bytes)},
        grass_{view::after(talkOver_).asByte()},
        animation_{view::after(grass_).asByte()},
        blocks{bank_, blocks_},
        tiles{bank_, tiles_},
        collision{bank_, tiles_} {}

  static bgry byID(view v, uint8_t id) {
    return bgry{v.from(start + id * headerSize).length(headerSize)};
  }

  operator bool(void) const {
    return view(*this) && view::check(lazies_()) && view::check(subviews_());
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
  subviews subviews_(void) const {
    auto s = const_cast<bgry*>(this);
    return subviews{&s->bank_,     &s->blocks_, &s->tiles_,    &s->collision_,
                    &s->talkOver_, &s->grass_,  &s->animation_};
  }

  lazies lazies_(void) const {
    auto s = const_cast<bgry*>(this);
    return lazies{&s->blocks, &s->tiles, &s->collision};
  }

  static const long headerSize = 12;

  static const long headers(void) { return (end - start + 1) / headerSize; }
};

}  // namespace tileset
}  // namespace pokemon

#endif
