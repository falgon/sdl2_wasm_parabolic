#ifndef INCLUDED_PBM_TEXTURE_IMAGE_HPP
#define INCLUDED_PBM_TEXTURE_IMAGE_HPP
#include "config.hpp"
#if !defined(__linux) && !defined(__linux__) && !defined(__gnu_linux__) && !defined(linux)
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#else
#include <SDL_image.h>
#include <SDL_video.h>
#include <SDL_render.h>
#endif
#include <string>
#include <optional>
#include <cmath>

namespace pbm::texture {

class image {
public:
    image(const char* path, 
#ifdef __EMSCRIPTEN__
            [[maybe_unused]]
#endif
            const int img_flag = IMG_INIT_PNG)
        : src_{ nullptr, SDL_FreeSurface },
        static_texture_{ nullptr, SDL_DestroyTexture },
        is_ok_(false)
    {
        if (
#ifndef __EMSCRIPTEN__
            ((IMG_Init(img_flag) & img_flag) == img_flag) &&
#endif
            (src_.reset(IMG_Load(path)), src_)) {
            is_ok_ = true;
        } else {
            pbm::debug_console("image", __func__, IMG_GetError);
        }
    }

    unique_texture get_texture(SDL_Renderer* renderer)
    {
        unique_texture img { nullptr, SDL_DestroyTexture };
        if (img.reset(SDL_CreateTextureFromSurface(renderer, src_.get())), !img) {
            pbm::debug_console("image", __func__, SDL_GetError);
        }
        return img;
    }

    shared_texture get_static_texture(SDL_Renderer* renderer)
    {
        auto p = get_texture(renderer);
        if (!static_texture_) static_texture_.reset(p.release(), SDL_DestroyTexture);
        return static_texture_;
    }

    void destroy() noexcept
    {
        src_.reset();
        static_texture_.reset();
    }

    int width() const noexcept { return src_->w; }
    int height() const noexcept { return src_->h; }

    inline void reset_static_texture() noexcept { static_texture_.reset(); }
    inline operator bool() const noexcept { return is_ok_; }
private:
    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> src_;
    shared_texture static_texture_;
    bool is_ok_;
};

} // namespace pbm::texture

#endif
