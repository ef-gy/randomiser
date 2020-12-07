#if !defined(POKEMON_RANDOMISER_MAP_H)
#define POKEMON_RANDOMISER_MAP_H

#include <pokemon-randomiser/tileset.h>
#include <pokemon-randomiser/view.h>

namespace pokemon {
namespace map {

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
  static constexpr pointer headersStart{0x01ae};
  static constexpr pointer banksStart{0xc23d};

 public:
  bgry(rom::bgry<> &pROM, uint8_t pID)
      : view{pROM},
        id(pID),
        bank_{view::from(banksStart + pID).is(gameboy::rom::dt_rom_bank)},
        offset_{view::from(headersStart + 2 * pID)
                    .is(gameboy::rom::dt_rom_offset)
                    .expect(gameboy::rom::e_little_endian)},
        start_{bank_, offset_},
        tileset_{view::from(start_).is(gameboy::rom::dt_byte)},
        height_{view::after(tileset_).is(gameboy::rom::dt_byte)},
        width_{view::after(height_).is(gameboy::rom::dt_byte)},
        data_{view::after(width_)
                  .is(gameboy::rom::dt_rom_offset)
                  .expect(gameboy::rom::e_little_endian)},
        text_{view::after(data_)
                  .is(gameboy::rom::dt_rom_offset)
                  .expect(gameboy::rom::e_little_endian)},
        script_{view::after(text_)
                    .is(gameboy::rom::dt_rom_offset)
                    .expect(gameboy::rom::e_little_endian)},
        connections_{view::after(script_).is(gameboy::rom::dt_byte)},
        north_{view::after(connections_)
                   .is(gameboy::rom::dt_bytes)
                   .length(haveNorth() ? 11 : 0)},
        south_{view::after(north_)
                   .is(gameboy::rom::dt_bytes)
                   .length(haveSouth() ? 11 : 0)},
        west_{view::after(south_)
                  .is(gameboy::rom::dt_bytes)
                  .length(haveWest() ? 11 : 0)},
        east_{view::after(west_)
                  .is(gameboy::rom::dt_bytes)
                  .length(haveEast() ? 11 : 0)},
        objects_{view::after(east_)
                     .is(gameboy::rom::dt_rom_offset)
                     .expect(gameboy::rom::e_little_endian)},
        subviews_{&bank_, &offset_, &tileset_, &height_,      &width_,
                  &data_, &text_,   &script_,  &connections_, &objects_},
        lazies_{&start_} {}

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

  static const std::pair<uint8_t, uint16_t> header(const rom::bgry<> &rom,
                                                   uint8_t id) {
    return {rom.byte(banksStart.linear() + id),
            rom.word_le(headersStart.linear() + 2 * id)};
  }

  const size_t size(void) const {
    return bool(*this) ? size_t(height_.byte()) * size_t(width_.byte()) : 0;
  }

  const bool empty(void) const { return size() == 0; }

  operator bool(void) const {
    return view(*this) && view::check(lazies_) && view::check(subviews_) &&
           bool(tileset());
  }

  pokemon::tileset::bgry<> tileset(void) const {
    return pokemon::tileset::bgry<>::byID(*this, tileset_.byte());
  }

 protected:
  const uint8_t id;

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
  view objects_;

  subviews subviews_;
  lazies lazies_;
};

}  // namespace map
}  // namespace pokemon

#endif
