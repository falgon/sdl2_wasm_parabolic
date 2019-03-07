#ifndef INCLUDED_PBM_SCENE_BASIC_DRAWER_HPP
#define INCLUDED_PBM_SCENE_BASIC_DRAWER_HPP
#include "../rw_manager.hpp"
#include <cmath>

namespace pbm::scene {

class basic_drawer {
public:
    basic_drawer(pbm::rw_manager& rw)
        : rw_{ rw },
        is_ok_{ true }
    {}

    //! The loop continuation flag (if true, continue looping, if loop is interrupted if false) and 
    //! SDL event codes to be wrapped in std::optional.
    //! If there is no event code, std :: nullopt is set.
    typedef std::pair<bool, std::optional<int>> loop_result;
protected:
    inline bool font_render_copy(SDL_Texture* texture, const SDL_Rect* rect) const noexcept
    {
        return !SDL_RenderCopy(rw_.renderer(), texture, nullptr, rect);
    }

    inline std::optional<SDL_Rect> make_texture_rect(SDL_Texture* txr, int x, int y) const noexcept
    {
        if (const auto twh = query_texture(txr); !twh) {
            return {};
        } else {
            return SDL_Rect{ x, y, twh->first, twh->second };
        }
    }

    inline bool set_black_text(SDL_Texture* txr) const noexcept
    {
        return !SDL_SetTextureColorMod(txr, 0xff, 0xff, 0xff);
    }

    std::optional<std::pair<int, int>> query_texture(SDL_Texture* txr) const noexcept
    {
        int m, h;
        if (SDL_QueryTexture(txr, nullptr, nullptr, &m, &h)) {
            pbm::debug_console("pre_drawer", __func__, SDL_GetError);
            return {};
        }
        return { std::make_pair(m, h) };
    }
public:
    template <class F,
        std::enable_if_t<
            std::conjunction_v<
                std::is_invocable<std::decay_t<F>, const SDL_Event&>,
                std::is_same<std::invoke_result_t<std::decay_t<F>, const SDL_Event&>, loop_result>
            >,
            std::nullptr_t
        > = nullptr
    >
    inline loop_result poll_event(F&& fn) const noexcept
    {
        return event(std::forward<F>(fn), SDL_PollEvent);
    }
    
    template <class F,
        std::enable_if_t<
            std::conjunction_v<
                std::is_invocable<std::decay_t<F>, const SDL_Event&>,
                std::is_same<std::invoke_result_t<std::decay_t<F>, const SDL_Event&>, loop_result>
            >,
            std::nullptr_t
        > = nullptr
    >
    inline loop_result wait_event(F&& fn) const noexcept
    {
        return event(std::forward<F>(fn), SDL_WaitEvent);
    }
private:
    template <class F1, class F2,
        std::enable_if_t<
            std::conjunction_v<
                std::is_invocable<std::decay_t<F1>, const SDL_Event&>,
                std::is_same<std::invoke_result_t<std::decay_t<F1>, const SDL_Event&>, loop_result>,
                std::is_invocable<std::decay_t<F2>, SDL_Event*>,
                std::is_same<std::invoke_result_t<std::decay_t<F2>, SDL_Event*>, int>
            >,
            std::nullptr_t
        > = nullptr
    >
    loop_result event(F1&& fn, F2&& event_detector) const noexcept
    {
        for (SDL_Event e; event_detector(&e);) {
#ifndef __EMSCRIPTEN__
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
                return { false, std::make_optional(SDL_QUIT) };
            else if (e.type == SDL_KEYDOWN) {
                if (const Uint8* keystate = SDL_GetKeyboardState(nullptr);
                    (keystate[SDL_SCANCODE_LGUI] || keystate[SDL_SCANCODE_RGUI]) && keystate[SDL_SCANCODE_W])
                    return { false, SDL_QUIT };
            }
#endif
            loop_result res = fn(e);
            if (!res.first) return res;
        }
        return { true, std::nullopt };
    }
protected:
    pbm::rw_manager& rw_;
    bool is_ok_;
};

} // namespace pbm::scene

#endif
