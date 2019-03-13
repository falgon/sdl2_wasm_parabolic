#if __cplusplus >= 201703L
#if (__has_include(<SDL2/SDL.h>) && __has_include(<SDL2/SDL_ttf.h>))
#ifdef __EMSCRIPTEN__
#   include <emscripten.h>
#endif
#include <optional>
#include "scene.hpp"
#include "texture.hpp"
#include "rw_manager.hpp"

#ifdef __EMSCRIPTEN__
typedef std::tuple<
    pbm::scene::pre_drawer&, 
    pbm::rw_manager&, 
    pbm::texture::font&, 
    pbm::texture::font&, 
    pbm::texture::font&, 
    pbm::texture::image&, 
    std::unique_ptr<pbm::scene::drawer>
>* arg_type;

void main_loop(void* pds)
{
    static bool f = true;
    auto ps = static_cast<arg_type>(pds);

    if (f) {
        if (!std::get<0>(*ps)().first) {
            f = false;
            std::get<2>(*ps).reset_static_texture();
        }
    } else {
        if (!std::get<6>(*ps)) 
            std::get<6>(*ps) = std::make_unique<pbm::scene::drawer>(
                std::get<0>(*ps).angle(), 
                std::get<0>(*ps).line_distance(), 
                0.1,
                std::get<1>(*ps),
                std::get<3>(*ps),
                std::get<2>(*ps), 
                std::get<5>(*ps)
            );
        if (!(*std::get<6>(*ps))().first) {
            f = true;
            std::get<6>(*ps).reset();
        }
    }
}
#endif

struct assets {
    static inline constexpr const char* const font = "assets/cmunti.ttf", *const scb_src = "assets/scb.png";
};

int main()
{
    if (pbm::rw_manager rw{ 512, 288 }; !rw) {
        return EXIT_FAILURE;
    } else {
        constexpr std::size_t pixel_size = 23;

        pbm::texture::font ftxr{ assets::font, pixel_size }, frqtxr{ assets::font, pixel_size }, fdxr{ assets::font, pixel_size };
        pbm::texture::image scb{ assets::scb_src, IMG_INIT_PNG };
        
        if (!ftxr || !frqtxr || !fdxr || !scb) return EXIT_FAILURE;
            
        pbm::scene::pre_drawer pd { 45.0, rw, ftxr, frqtxr, fdxr, scb };

#ifndef __EMSCRIPTEN__
        while (true) {
            rw.reset_render_size();
            std::pair<bool, std::optional<int>> res;
        
            while ((res = pd()).first);
            if (res.second && res.second.value() == SDL_QUIT) return EXIT_SUCCESS;
        
            pbm::scene::drawer d{ pd.angle(), pd.line_distance(), 1.0 / 60.0, rw, ftxr, scb };
            
            constexpr Uint32 wait = 1;
            for (Uint32 next_frame = SDL_GetTicks();;) {
                auto p = d(wait, next_frame);
                if (!p.first.first) {
                    if (p.first.second && p.first.second.value() == SDL_QUIT) return EXIT_SUCCESS;
                    break;
                }
                next_frame = p.second;
            }
        }
#else
        auto t = std::remove_pointer_t<arg_type>{ pd, rw, ftxr, frqtxr, fdxr, scb, nullptr };
        emscripten_set_main_loop_arg(main_loop, &t, -1, true);
#endif
    }
}

#else
#   error This code uses the SDL2 lib
#endif
#else
#   error This code uses C++17 features
#endif
