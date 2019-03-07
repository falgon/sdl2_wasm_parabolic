#ifndef INCLUDED_PBM_SDL_MANAGER_HPP
#define INCLUDED_PBM_SDL_MANAGER_HPP
#include <SDL2/SDL.h>
#include <type_traits>
#include <optional>
#include "utils.hpp"

namespace pbm {

template <class Contexture>
class sdl_manager {
    static_assert(std::is_invocable_v<decltype(&Contexture::update), Contexture, SDL_Renderer*>);
public:
    sdl_manager(std::size_t width, std::size_t height) 
        : initialized_(false)
    {
        if (!SDL_Init(SDL_INIT_VIDEO) && !SDL_CreateWindowAndRenderer(std::move(width), std::move(height), SDL_WINDOW_ALLOW_HIGHDPI, &window_, &renderer_)) {
            initialized_ = true;
        }
#ifndef NDEBUG
        else {
            pbm::debug_console("sdl_manager", __func__, SDL_GetError);
        }
#endif
    }

    std::pair<int, int> drawable_size() const noexcept
    {
        int w = 0, h = 0;
        SDL_GL_GetDrawableSize(window_, &w, &h);
        return { w, h };
    }
    
    template <class... Ts, std::enable_if_t<std::is_constructible_v<Contexture, Ts&&...>, std::nullptr_t> = nullptr>
    bool set_contexture(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<Contexture, Ts&&...>)
    {
        return initialized_ ? ctx_.emplace(std::forward<Ts>(ts)...), initialized_ : initialized_;
    }

    /*inline std::invoke_result_t<decltype(&Contexture::pre_update()), Contexture, SDL_Window*, SDL_Renderer*>
    pre_update() noexcept(std::is_nothrow_invocable_v<decltype(&Contexture::pre_update()), Contexture, SDL_Window*, SDL_Renderer*>)
    {
        if (initialized_ && ctx_) ctx_.value().pre_update(window_, renderer_);
    }*/

    inline std::invoke_result_t<decltype(&Contexture::operator()), Contexture, SDL_Window*, SDL_Renderer*>
    operator()()
    {
        if (initialized_ && ctx_) ctx_.value()(window_, renderer_);
    }

    inline std::invoke_result_t<decltype(&Contexture::update), Contexture, SDL_Renderer*> 
    update() noexcept(std::is_nothrow_invocable_v<decltype(&Contexture::update), Contexture, SDL_Renderer*>)
    {
        return ctx_.value().update(renderer_); 
    }
    
    template <class... Ts> 
    inline void draw(Ts&&... ts)
    {
        return ctx_.value().draw(window_, renderer_, std::forward<Ts>(ts)...); 
    }

    explicit operator bool() const noexcept
    {
        return initialized_ && static_cast<bool>(ctx_);
    }

    ~sdl_manager()
    {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
    }

    //SDL_Renderer* renderer() noexcept { return renderer_; }
    //SDL_Window* window() noexcept { return window_; }
    //std::optional<Contexture>& contexture() noexcept { return ctx_; }
private:
    std::optional<Contexture> ctx_;
    bool initialized_;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
};

} // namespace pbm
#endif
