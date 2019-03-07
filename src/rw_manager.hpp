#ifndef INCLUDED_PBM_RW_MANAGER_HPP
#define INCLUDED_PBM_RW_MANAGER_HPP
#if !defined(__linux) && !defined(__linux__) && !defined(__gnu_linux__) && !defined(linux)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <type_traits>
#include <optional>
#include "utils.hpp"

namespace pbm {

class rw_manager {
    struct winsize {
        std::size_t width, height;

        void update(SDL_Window* win) noexcept
        {
            int w, h;
            SDL_GL_GetDrawableSize(win, &w, &h);
            width = w;
            height = h;
        }
    } wsize_;
public:
    rw_manager(std::size_t w, std::size_t h) 
        : wsize_{ w, h }, initialized_(false)
    {
        if (!SDL_Init(SDL_INIT_VIDEO) && !SDL_CreateWindowAndRenderer(std::move(w), std::move(h), 0, &window_, &renderer_)) {
            SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, true);
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
            initialized_ = true;
            update_drawable_size();
        }
#ifndef NDEBUG
        else {
            pbm::debug_console("sdl_manager", __func__, SDL_GetError);
        }
#endif
    }

    inline operator bool() const noexcept { return initialized_; }

    inline const winsize& drawable_size() const noexcept { return wsize_; }

    inline void update_drawable_size() noexcept { wsize_.update(window_); }
    
    inline void reset_render_size() noexcept
    {
        SDL_RenderSetLogicalSize(renderer(), drawable_size().width, drawable_size().height);
    }

    ~rw_manager()
    {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
    }

    SDL_Renderer* renderer() noexcept { return renderer_; }
    SDL_Window* window() noexcept { return window_; }
private:
    bool initialized_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
};

} // namespace pbm
#endif

