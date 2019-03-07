#ifndef INCLUDED_PBM_TEXTURE_CONFIG_HPP
#define INCLUDED_PBM_TEXTURE_CONFIG_HPP
#include "../utils.hpp"
#if !defined(__linux) && !defined(__linux__) && !defined(__gnu_linux__) && !defined(linux)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <type_traits>
#include <utility>
#include <memory>

namespace pbm::texture {

typedef std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> unique_texture;
typedef std::shared_ptr<SDL_Texture> shared_texture;

} // namespace pbm::texure
#endif
