#if !defined(POKEMON_RANDOMISER_OBJECT_H)
#define POKEMON_RANDOMISER_OBJECT_H

#include <pokemon-randomiser/view.h>

namespace pokemon {
namespace object {
template <typename B = uint8_t, typename W = uint16_t>
class bgry : gameboy::rom::view<B, W> {
 public:
  using view = gameboy::rom::view<B, W>;
  using pointer = typename view::pointer;

  bgry(view v)
      : view{v},
        border_{view::start().is(gameboy::rom::dt_byte)},
        warps_(view::after(border_).is(gameboy::rom::dt_byte)) {}

 protected:
  view border_;
  view warps_;
};
}  // namespace object
}  // namespace pokemon

#endif
