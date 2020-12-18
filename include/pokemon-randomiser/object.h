#if !defined(POKEMON_RANDOMISER_OBJECT_H)
#define POKEMON_RANDOMISER_OBJECT_H

#include <pokemon-randomiser/sprite.h>

namespace pokemon {
namespace object {
template <typename B = uint8_t, typename W = uint16_t>
class bgry : gameboy::rom::view<B, W> {
 public:
  using view = gameboy::rom::view<B, W>;
  using pointer = typename view::pointer;
  using sprite = pokemon::sprite::bgry<B, W>;
  using subviews = typename view::subviews;

  bgry(view v)
      : view{v.asLittleEndian()},
        border_{view::start().asByte()},
        warps_{view::after(border_).asByte()},
        warpData_{view::after(warps_).length(4 * warpc())},
        signs_{view::after(warpData_).asByte()},
        signsData_{view::after(signs_).length(3 * signc())},
        sprites_{view::after(signsData_).asByte()},
        sprites{view::after(sprites_).template repeated<sprite>(sprites_)},
        warpins_{view::from(view::last(sprites) + 1).length(4 * warpc())} {}

  size_t warpc(void) const { return warps_.byte(); }
  size_t signc(void) const { return signs_.byte(); }
  size_t spritec(void) const { return sprites_.byte(); }

  operator bool(void) const {
    bool r = view(*this) && view::check(subs());

    if (!r) {
      std::cerr << "CHECK FAILED: object map invalid:\n"
                << " * vwp " << view(*this).debug() << "\n";
      if (!view(*this)) {
        std::cerr << " ! ERR invalid view\n";
      } else {
        std::cerr << " ! ERR invalid sub view for object map\n";
      }
    }

    return r;
  }

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

  subviews subs(void) const {
    auto s = const_cast<bgry*>(this);

    return subviews{&s->border_,    &s->warps_,   &s->warpData_, &s->signs_,
                    &s->signsData_, &s->sprites_, &s->warpins_};
  }
};
}  // namespace object
}  // namespace pokemon

#endif
