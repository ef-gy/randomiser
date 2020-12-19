#if !defined(POKEMON_RANDOMISER_SPRITE_H)
#define POKEMON_RANDOMISER_SPRITE_H

#include <pokemon-randomiser/view.h>

namespace pokemon {
namespace sprite {
template <typename B = uint8_t, typename W = uint16_t>
class bgry : public gameboy::rom::view<B, W> {
 public:
  using view = gameboy::rom::view<B, W>;
  using pointer = typename view::pointer;
  using subviews = typename view::subviews;

  bgry(view v)
      : view{v.asLittleEndian().toBankEnd()},
        sprite_{view::start().asByte()},
        positionY_{view::after(sprite_).asByte()},
        positionX_{view::after(positionY_).asByte()},
        mobility_{view::after(positionX_).asByte()},
        movement_{view::after(mobility_).asByte()},
        flags_{view::after(movement_).asByte()},
        item_{view::after(flags_).asByte()},
        opponent_{view::after(flags_).asByte()},
        level_{view::after(opponent_).asByte()},
        team_{view::after(opponent_).asByte()} {}

  bool isNPC(void) const {
    return flags_ && (flags_.byte() & 0x80) == 0 && (flags_.byte() & 0x40) == 0;
  }

  bool isItem(void) const {
    return flags_ && (flags_.byte() & 0x80) != 0 && (flags_.byte() & 0x40) == 0;
  }

  bool isTrainer(void) const {
    return flags_ && (flags_.byte() & 0x40) != 0 && opponent_ &&
           opponent_.byte() >= 0xc8;
  }

  bool isPokemon(void) const {
    return flags_ && (flags_.byte() & 0x40) != 0 && opponent_ &&
           opponent_.byte() < 0xc8;
  }

  operator bool(void) const {
    return view(*this) && view::check(subviews_()) &&
           (isNPC() || isItem() || isTrainer() || isPokemon());
  }

  pointer last(void) const {
    if (isNPC()) {
      return flags_.last();
    } else if (isItem()) {
      return item_.last();
    } else if (isTrainer()) {
      return team_.last();
    } else if (isPokemon()) {
      return level_.last();
    }

    return view::start_ - 1;
  }

  std::size_t size(void) const {
    if (isNPC()) {
      return 5;
    } else if (isItem()) {
      return 6;
    } else if (isTrainer()) {
      return 7;
    } else if (isPokemon()) {
      return 7;
    }

    return 0;
  }

  // protected:
  view sprite_;
  view positionY_;
  view positionX_;
  view mobility_;
  view movement_;
  view flags_;
  view item_;
  view opponent_;
  view level_;
  view team_;

  subviews subviews_(void) const {
    auto s = const_cast<bgry*>(this);
    subviews rv{&s->sprite_,   &s->positionY_, &s->positionX_,
                &s->mobility_, &s->movement_,  &s->flags_};

    if (isItem()) {
      rv.insert(&s->item_);
    } else if (isTrainer()) {
      rv.insert(&s->opponent_);
      rv.insert(&s->team_);
    } else if (isPokemon()) {
      rv.insert(&s->opponent_);
      rv.insert(&s->level_);
    }

    return rv;
  }
};
}  // namespace sprite
}  // namespace pokemon

#endif
