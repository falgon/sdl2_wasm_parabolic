#ifndef INCLUDED_PBM_TEXTURE_FONT_HPP
#define INCLUDED_PBM_TEXTURE_FONT_HPP
#include "config.hpp"
#if !defined(__linux) && !defined(__linux__) && !defined(__gnu_linux__) && !defined(linux)
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_render.h>
#else
#include <SDL_ttf.h>
#include <SDL_render.h>
#endif

namespace pbm::texture {

class font {
public:
    font(const char* font_path, std::size_t pixel_size) 
        : cmmi_{ nullptr, TTF_CloseFont },
        text_surface_ { nullptr, SDL_FreeSurface },
        static_texture_{ nullptr, SDL_DestroyTexture },
        is_ok_(false),
        font_path_{ font_path }
    {
        if ((TTF_WasInit() ? true : !TTF_Init()) && (cmmi_.reset(TTF_OpenFont(font_path, pixel_size)), cmmi_)) {
            is_ok_ = true;
        }
#ifndef NDEBUG
        else {
            pbm::debug_console("font", __func__, TTF_GetError);
        }
#endif
    }

    template <class SDLCOLOR = SDL_Color, std::enable_if_t<std::is_same_v<std::decay_t<SDLCOLOR>, SDL_Color>, std::nullptr_t> = nullptr>
    font(const char* font_path, std::size_t pixel_size, const char* text, SDLCOLOR&& color = SDL_Color{ 0, 0, 0, 0 })
        : font(font_path, pixel_size)
    {
        if (!set_surface(text, std::forward<SDLCOLOR>(color))) {
            is_ok_ = false;
        } else {
            is_ok_ = true;
        }
    }

    void reset_size(std::size_t size)
    {
        if (cmmi_.reset(TTF_OpenFont(font_path_, size)), cmmi_) {
            is_ok_ = false;
        }
    }

    template <class SDLCOLOR = SDL_Color, std::enable_if_t<std::is_same_v<std::decay_t<SDLCOLOR>, SDL_Color>, std::nullptr_t> = nullptr>
    unique_texture get_texture(SDL_Renderer* renderer, const char* s, SDLCOLOR&& color = SDL_Color{ 0, 0, 0, 0 }) noexcept
    {
        unique_texture texture { nullptr, SDL_DestroyTexture };

        if (!set_surface(s, std::forward<SDLCOLOR>(color))) {
            is_ok_ = false;
        } else {
            texture.reset(SDL_CreateTextureFromSurface(renderer, text_surface_.get()));
        }
        return texture;   
    }

    template <class SDLCOLOR = SDL_Color, std::enable_if_t<std::is_same_v<std::decay_t<SDLCOLOR>, SDL_Color>, std::nullptr_t> = nullptr>
    shared_texture get_static_texture(SDL_Renderer* renderer, const char* s, SDLCOLOR&& color = SDL_Color{ 0, 0, 0, 0}) noexcept
    {
        if (!static_texture_) {
            auto p = get_texture(renderer, s, std::forward<SDLCOLOR>(color));
            static_texture_.reset(p.release(), SDL_DestroyTexture);
        }
        return static_texture_;
    }

    void destroy() noexcept
    {
        cmmi_.reset();
        text_surface_.reset();
        static_texture_.reset();
    }

    inline void reset_static_texture() noexcept { static_texture_.reset(); }    
    inline operator bool() const noexcept { return is_ok_; }
    inline int width() { return text_surface_->w; }
    inline int height() { return text_surface_->h; }
private:
    template <class SDLCOLOR, std::enable_if_t<std::is_same_v<std::decay_t<SDLCOLOR>, SDL_Color>, std::nullptr_t> = nullptr>
    bool set_surface(const char* text, SDLCOLOR&& color) noexcept
    {
        if (!cmmi_ || !(text_surface_.reset(TTF_RenderText_Blended(cmmi_.get(), text, std::forward<SDLCOLOR>(color))), text_surface_)) {
            pbm::debug_console("contexture", __func__, TTF_GetError);
            return false;
        }
        return true;
    }
    
    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> cmmi_;
    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> text_surface_;
    shared_texture static_texture_;
    bool is_ok_;
    const char* font_path_;
};

} // namespace texture::pbm

#endif
