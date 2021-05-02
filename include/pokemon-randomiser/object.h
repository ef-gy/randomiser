#if !defined(POKEMON_RANDOMISER_OBJECT_H)
#define POKEMON_RANDOMISER_OBJECT_H

#include <pokemon-randomiser/sprite.h>

namespace pokemon {
namespace object {
template <typename B = uint8_t, typename W = uint16_t>
class bgry : public gameboy::rom::view<B, W> {
 public:
  using view = gameboy::rom::view<B, W>;
  using pointer = typename view::pointer;
  using sprite = pokemon::sprite::bgry<B, W>;

  constexpr bgry(view v)
      : view{v.asLittleEndian().label("__scope")},
        border_{view::start().asByte().label("object_map_border")},
        warps_{view::after(border_).asByte().label("object_map_warps")},
        warpData_{view::after(warps_)
                      .length(4 * warpc())
                      .label("object_map_warp_data")},
        signs_{view::after(warpData_).asByte().label("object_map_signs")},
        signsData_{view::after(signs_)
                       .length(3 * signc())
                       .label("object_map_signs_data")},
        sprites_{view::after(signsData_).asByte().label("object_map_sprites")},
        sprites{view::after(sprites_).template repeated<sprite>(sprites_)},
        warpins_{view::from(view::last(sprites) + 1)
                     .length(4 * warpc())
                     .label("object_map_warpins")} {}

  size_t warpc(void) const { return warps_.byte(); }
  size_t signc(void) const { return signs_.byte(); }
  size_t spritec(void) const { return sprites_.byte(); }

  operator bool(void) const { return view(*this) && view::check(fields()); }

 protected:
  view border_;
  view warps_;
  view warpData_;
  view signs_;
  view signsData_;
  view sprites_;

 public:
  std::vector<sprite> sprites;

 protected:
  view warpins_;

 public:
  constexpr std::array<view, 7> fields(void) const {
    return {border_, warps_, warpData_, signs_, signsData_, sprites_, warpins_};
  }
};
}  // namespace object
}  // namespace pokemon

#endif
