#if !defined(POKEMON_RANDOMISER_MAP_H)
#define POKEMON_RANDOMISER_MAP_H

#include <pokemon-randomiser/object.h>
#include <pokemon-randomiser/tileset.h>

namespace pokemon {
namespace map {
template <typename B = uint8_t, typename W = uint16_t>
class bgry : public gameboy::rom::view<B, W> {
 public:
  using view = gameboy::rom::view<B, W>;
  using pointer = typename view::pointer;
  using lazy = typename view::lazy;
  using subviews = typename view::subviews;
  using lazies = typename view::lazies;

  using tileset = pokemon::tileset::bgry<B, W>;
  using objectData = pokemon::object::bgry<B, W>;

  /* The BGRY section used Bulbapedia extensively for its research content.
   *
   * See this link for ALL the detail:
   * https://bulbapedia.bulbagarden.net/wiki/User:Tiddlywinks/Map_header_data_structure_in_Generation_I
   */

 protected:
  static constexpr pointer headers{0x01ae};
  static constexpr pointer banks{0xc23d};

 public:
  bgry(view v, uint8_t pID)
      : view{v.asLittleEndian()},
        id(pID),
        bank_{view::from(banks + pID).asROMBank()},
        offset_{view::from(headers + 2 * pID).asROMOffset()},
        start_{bank_, offset_},
        tileset_{view::from(start_).asByte()},
        height_{view::after(tileset_).asByte()},
        width_{view::after(height_).asByte()},
        data_{view::after(width_).asROMOffset()},
        text_{view::after(data_).asROMOffset()},
        script_{view::after(text_).asROMOffset()},
        connections_{view::after(script_).asByte()},
        north_{view::after(connections_)
                   .is(gameboy::dt_bytes)
                   .length(haveNorth() ? 11 : 0)},
        south_{view::after(north_)
                   .is(gameboy::dt_bytes)
                   .length(haveSouth() ? 11 : 0)},
        west_{view::after(south_)
                  .is(gameboy::dt_bytes)
                  .length(haveWest() ? 11 : 0)},
        east_{view::after(west_)
                  .is(gameboy::dt_bytes)
                  .length(haveEast() ? 11 : 0)},
        object_{view::after(east_).asROMOffset()},
        text{bank_, text_},
        object{bank_, object_} {}

  bool haveNorth(void) const {
    return bool(connections_) && (connections_.byte() & 0x8);
  }

  bool haveSouth(void) const {
    return bool(connections_) && (connections_.byte() & 0x4);
  }

  bool haveWest(void) const {
    return bool(connections_) && (connections_.byte() & 0x2);
  }

  bool haveEast(void) const {
    return bool(connections_) && (connections_.byte() & 0x1);
  }

  const size_t size(void) const { return width() * height(); }

  const bool empty(void) const { return size() == 0; }

  operator bool(void) const {
    return view(*this) && view::check(lazies_()) && view::check(subviews_()) &&
           bool(tileset(*this));
  }

  operator tileset(void) const { return tileset::byID(*this, tileset_.byte()); }

  objectData objects(void) const {
    return objectData{view::from(pointer(object))};
  }

  B width(void) const { return bool(width_) ? width_.byte() : 0; }

  B height(void) const { return bool(height_) ? height_.byte() : 0; }

  std::optional<pointer> script(uint8_t n) const {
    if (text) {
      view v{view::from(text + 2 * n).asROMOffset()};
      lazy p{bank_, v};

      if (p) {
        return p;
      }
    }

    return {};
  }

 public:
  const uint8_t id;

 protected:
  view bank_;
  view offset_;

  lazy start_;

  view tileset_;
  view height_;
  view width_;
  view data_;
  view text_;
  view script_;
  view connections_;
  view north_;
  view south_;
  view west_;
  view east_;
  view object_;

 public:
  lazy text;
  lazy object;

 protected:
  subviews subviews_(void) const {
    auto s = const_cast<bgry*>(this);
    return subviews{&s->bank_,        &s->offset_, &s->tileset_, &s->height_,
                    &s->width_,       &s->data_,   &s->text_,    &s->script_,
                    &s->connections_, &s->object_};
  }

  lazies lazies_(void) const {
    auto s = const_cast<bgry*>(this);
    return lazies{&s->start_, &s->text, &s->object};
  }
};

}  // namespace map
}  // namespace pokemon

#endif
