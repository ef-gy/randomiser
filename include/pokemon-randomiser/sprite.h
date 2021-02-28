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

  constexpr bgry(view v)
      : view{v.asLittleEndian().toBankEnd().limit(8).label("__scope")},
        sprite_{view::start().asByte().label("sprite_reference_id")},
        positionY_{view::after(sprite_).asByte().label("sprite_position_y")},
        positionX_{view::after(positionY_).asByte().label("sprite_position_x")},
        mobility_{view::after(positionX_).asByte().label("sprite_mobility")},
        movement_{view::after(mobility_).asByte().label("sprite_movement")},
        flags_{view::after(movement_).asByte().label("sprite_type_flags")},
        item_{view::after(flags_).asByte().label("sprite_item_code")},
        opponent_{view::after(flags_).asByte().label("sprite_opponent")},
        level_{view::after(opponent_).asByte().label("sprite_opponent_level")},
        team_{view::after(opponent_).asByte().label("sprite_team_code")} {}

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
    return view(*this) && view::check(fields()) &&
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

 public:
  constexpr std::array<view, 8> fields(void) const {
    view ignored{view::start().length(1).label("__ignore")};

    if (isItem()) {
      return {sprite_,   positionY_, positionX_, mobility_,
              movement_, flags_,     item_,      ignored};
    } else if (isTrainer()) {
      return {sprite_,   positionY_, positionX_, mobility_,
              movement_, flags_,     opponent_,  team_};
    } else if (isPokemon()) {
      return {sprite_,   positionY_, positionX_, mobility_,
              movement_, flags_,     opponent_,  level_};
    }

    // guess the most conservative field set - this would normally be an NPC
    // sprite
    return {sprite_,   positionY_, positionX_, mobility_,
            movement_, flags_,     ignored,    ignored};
  }
};
}  // namespace sprite
}  // namespace pokemon

#endif
