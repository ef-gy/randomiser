#if !defined(POKEMON_RANDOMISER_DEBUG_H)
#define POKEMON_RANDOMISER_DEBUG_H

#include <ef.gy/range.h>
#include <pokemon-randomiser/map.h>
#include <pokemon-randomiser/pointer.h>
#include <pokemon-randomiser/sprite.h>
#include <pokemon-randomiser/view.h>

#include <iomanip>
#include <sstream>

namespace debug {
template <typename B, typename W, W bankSize_>
static std::string dump(const gameboy::rom::pointer<B, W, bankSize_> &ptr) {
  std::ostringstream os{};

  os << "*{" << std::hex << std::setfill('0');
  if (ptr.isLinear()) {
    os << "[0x" << std::setw(6) << ptr.linear() << "]";
  } else {
    os << " 0x" << std::setw(6) << ptr.linear() << " ";
  }
  os << "=" << std::hex << std::setfill('0');
  if (ptr.isBanked()) {
    os << "[0x" << std::setw(2) << W(ptr.bank()) << ":" << std::setw(4)
       << ptr.offset() << "]";
  } else {
    os << " 0x" << std::setw(2) << W(ptr.bank()) << ":" << std::setw(4)
       << ptr.offset() << " ";
  }
  os << "}";

  return os.str();
}

template <typename B, typename W>
static std::string dump(const gameboy::rom::view<B, W> &view) {
  std::ostringstream os{};

  os << std::hex << std::setfill('0') << "[view:"
     << " window=[" << dump(view.startPtr()) << " - " << dump(view.endPtr())
     << "]"
     << " length=[0x" << view.size() << "]"
     << " cursor=[" << dump(view.curPtr()) << "]"
     << "]";

  return os.str();
}

template <typename view, typename B, typename W, W bankSize_>
static std::string dump(const gameboy::rom::lazy<view, B, W, bankSize_> &lazy) {
  using pointer = gameboy::rom::pointer<B, W, bankSize_>;

  std::ostringstream os{};

  os << "?{bank@ " << dump(lazy.bank()) << " offset@ " << dump(lazy.offset())
     << "]";
  if (lazy) {
    os << " =OK=> " << dump(pointer(lazy));
  } else {
    os << " =NOK_";
  }
  os << "}";

  return os.str();
}

template <typename B, typename W>
static std::string dump(const gameboy::rom::view<B, W> &view,
                        const std::set<gameboy::rom::view<B, W> *> &subviews) {
  std::ostringstream os{};

  os << "SUBVIEWS\n"
     << " * par " << dump(view) << "\n";

  for (const auto &v : subviews) {
    os << " * sub " << dump(*v) << "\n";
    if (!bool(*v)) {
      os << " ! ERR view not valid\n";
    }
    if (!view.within(*v)) {
      os << " ! ERR view is not contained within given parent\n";
    }
  }

  return os.str();
}

template <typename B, typename W>
static std::string dump(const pokemon::sprite::bgry<B, W> &sprite) {
  std::ostringstream os{};

  os << "SPRITE\n"
     << " * vwp " << dump(gameboy::rom::view<B, W>(sprite)) << "\n";

  if (!bool(sprite)) {
    os << " ! ERR item is not valid\n";
  } else {
    os << std::hex << std::setw(2) << std::setfill('0');

    os << " - spr 0x" << W(sprite.sprite_.byte()) << "\n"
       << " - pos 0x{" << W(sprite.positionX_.byte()) << ","
       << W(sprite.positionY_.byte()) << "}\n"
       << " - mob 0x" << W(sprite.mobility_.byte()) << "}\n"
       << " - mov 0x" << W(sprite.movement_.byte()) << "}\n";

    if (sprite.isNPC()) {
      os << " > NPC []\n";
    }
    if (sprite.isItem()) {
      os << " > ITM [0x" << W(sprite.item_.byte()) << "]\n";
    }
    if (sprite.isTrainer()) {
      os << " > TRN [opponent: 0x" << W(sprite.opponent_.byte())
         << ", team: " << W(sprite.team_.byte()) << "]\n";
    }
    if (sprite.isPokemon()) {
      os << " > PKM [opponent: 0x" << W(sprite.opponent_.byte())
         << ", level: " << W(sprite.level_.byte()) << "]\n";
    }
  }

  return os.str();
}

template <typename B, typename W>
static std::string dump(const pokemon::object::bgry<B, W> &object) {
  std::ostringstream os{};

  os << "OBJECT MAP\n"
     << " * vwp " << dump(gameboy::rom::view<B, W>(object)) << "\n";
  if (!object) {
    if (!gameboy::rom::view<B, W>(object)) {
      os << " ! ERR invalid view\n";
    } else {
      os << " ! ERR invalid sub view for object map\n";
    }
  } else {
    os << " - wrp#" << std::dec << object.warpc() << "\n";
    os << " - sgn#" << std::dec << object.signc() << "\n";
    os << " - spr#" << std::dec << object.spritec() << "/"
       << object.sprites.size() << "\n";
    for (const auto &s : object.sprites) {
      if (s) {
        os << debug::dump(s);
      }
    }
  }

  return os.str();
}

template <typename B, typename W>
static std::string dump(const pokemon::map::bgry<B, W> &map) {
  using view = gameboy::rom::view<B, W>;
  using tileset = pokemon::tileset::bgry<B, W>;

  std::ostringstream os{};

  os << "MAP\n"
     << " * idn " << std::dec << W(map.id) << "\n"
     << " * vwp " << dump(view(map)) << "\n";

  if (!map) {
    os << " ! ERR invalid map data\n";
    if (!view(map)) {
      os << " ! ERR invalid view\n";
    }
    if (!tileset(map)) {
      os << " ! ERR invalid tile set\n";
    }
  } else {
    os << " * CUR " << dump(map.start) << "\n";

    os << " - dim {W,H}@T: {" << W(map.width()) << "," << W(map.height())
       << "}@" << map.size() << "\n";

    os << " - cnc" << (map.haveNorth() ? " [NORTH]" : " [-----]")
       << (map.haveSouth() ? " [SOUTH]" : " [-----]")
       << (map.haveWest() ? " [WEST.]" : " [-----]")
       << (map.haveEast() ? " [EAST.]" : " [-----]") << "\n";

    for (const auto l : efgy::range<B>(0, map.height(), false)) {
      os << (l == 0 ? " > BLK" : "   xxx");

      for (const auto c : efgy::range<B>(0, map.width(), false)) {
        const auto block = map.block(l, c);

        if (block) {
          os << " " << std::hex << std::setw(2) << std::setfill('0')
             << W(*block);
        } else {
          os << " ??";
        }
      }

      os << "\n";
    }

    if (map.text) {
      os << " - txt " << debug::dump(map.text) << "\n";
    }

    auto obj = map.objects();

    if (obj) {
      os << dump(obj);
    }
  }

  return os.str();
}
}  // namespace debug

#endif
