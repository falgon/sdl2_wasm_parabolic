#ifndef INCLUDED_PBM_SCENE_DRAWER_HPP
#define INCLUDED_PBM_SCENE_DRAWER_HPP

#include "basic_drawer.hpp"
#include "../rw_manager.hpp"
#include <cmath>
#include <iostream>

namespace pbm::scene {

class drawer : public basic_drawer {
public:
    drawer(double angle, double speed, double duration, pbm::rw_manager& rw, pbm::texture::font& tf, 
#ifdef __EMSCRIPTEN__
            pbm::texture::font& tl, 
#endif
            pbm::texture::image& ti)
        : basic_drawer(rw),
        angle_(std::move(angle)),
        speed_(speed),
        tf_{ tf },
#ifdef __EMSCRIPTEN__
        tl_{ tl },
#endif
        ti_{ ti },
        bp_{ 0, static_cast<double>(rw_.drawable_size().height) - ti_.height() },
        current_pos_{ bp_ },
        t_{ duration },
        land_time_{ 2 * speed * std::sin(rad_angle()) / g },
        c_{ 0u }
    {}

    inline void draw() { draw_impl(); }

    inline void update() noexcept 
    {
        ++c_;
        const auto p = position(bp_.x, bp_.y, time());

        current_pos_.x = std::get<0>(p); 
        current_pos_.y = rw_.drawable_size().height + 224 - std::get<1>(p); // HACK
    }

    inline operator bool() const noexcept { return is_ok_; }

#ifdef __EMSCRIPTEN__
    basic_drawer::loop_result operator()()
    {
        if (time() < land_time_) {
            loop_result res = poll_event([](const SDL_Event& e) -> basic_drawer::loop_result {
                if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && e.button.state == SDL_RELEASED) {
                    return { false, std::nullopt };
                }
                return { true, std::nullopt };
            });
            if (!res.first || !is_ok_) return res;
            draw();
            update();
            return res;
        } else {
            return wait_again();
        }
    }
#else
    std::pair<basic_drawer::loop_result, double>
    operator()(double wait, double next_frame) noexcept
    {
        if (time() < land_time_) {
            loop_result res = poll_event([](const SDL_Event& e) -> basic_drawer::loop_result {
                if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && e.button.state == SDL_RELEASED) {
                    return { false, std::nullopt };
                }
                return { true, std::nullopt };
            });
            if (!res.first || !is_ok_) return { res, next_frame + wait };
            draw();

            if (SDL_GetTicks() > next_frame + wait) {
                update();
                return { res, SDL_GetTicks() };
            } else {
                return { res, next_frame };
            }
        } else {
            return { wait_again(), 0 };
        }
    }
#endif
private:
    inline double time() const noexcept { return c_ * t_; }
    
    inline double max_height() const noexcept { return bp_.y + std::pow(speed_ * std::sin(rad_angle()), 2) / (2 * g); }

    inline double reached_distance() const noexcept { return std::pow(speed_, 2) * std::sin(2 * rad_angle()) / g; }

    double rad_angle() const noexcept { return angle_ * (M_PI / 180); }

    basic_drawer::loop_result wait_again() noexcept
    {
#ifdef __EMSCRIPTEN__
        if (SDL_SetRenderDrawColor(rw_.renderer(), 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE) ||
            SDL_RenderClear(rw_.renderer())) {
            debug_console("drawer", __func__, SDL_GetError);
            is_ok_ = false;
            return { false, std::nullopt };
        }

        const std::string t = "t = " + std::to_string(time());

        auto tf_txtr = tf_.get_texture(rw_.renderer(), t.c_str());
        auto ti_txtr = ti_.get_static_texture(rw_.renderer());

        if (!set_black_text(tf_txtr.get())) {
            is_ok_ = false;
            pbm::debug_console("drawer", __func__, SDL_GetError);
            return { false, std::nullopt };
        }

        const auto text_rect = make_texture_rect(tf_txtr.get(), 10, 10),
              image_rect = make_texture_rect(ti_txtr.get(), static_cast<int>(current_pos_.x), static_cast<int>(current_pos_.y));

        if (!text_rect || !image_rect) {
            is_ok_ = false;
            return { false, std::nullopt };
        } else if (!font_render_copy(tf_txtr.get(), &text_rect.value()) || 
                SDL_RenderCopyEx(rw_.renderer(), ti_txtr.get(), nullptr, &image_rect.value(), -angle_, nullptr, SDL_FLIP_NONE)) {
            is_ok_ = false;
            pbm::debug_console("drawer", __func__, SDL_GetError);
            return { false, std::nullopt };
        }
#endif
        const std::string h = "h = " + std::to_string(max_height()), l = "l = " + std::to_string(reached_distance());
        auto hf_txtr = tf_.get_static_texture(rw_.renderer(), h.c_str());
        const auto lf_txtr = 
#ifdef __EMSCRIPTEN__
            tl_.get_static_texture(rw_.renderer(), l.c_str())
#else
            tf_.get_texture(rw_.renderer(), l.c_str())
#endif
            ;

        if (!set_black_text(hf_txtr.get()) || !set_black_text(lf_txtr.get())) {
            is_ok_ = false;
            pbm::debug_console("drawer", __func__, SDL_GetError);
            return { false, std::nullopt };
        }
        const auto h_rect = make_texture_rect(hf_txtr.get(), 10, 40), l_rect = make_texture_rect(lf_txtr.get(), 10, 70);
        if (!h_rect || !l_rect) {
            is_ok_ = false;
            return { false, std::nullopt };
        } else if (!font_render_copy(hf_txtr.get(), &h_rect.value()) || !font_render_copy(lf_txtr.get(), &l_rect.value())) {
            is_ok_ = false;
            pbm::debug_console("drawer", __func__, SDL_GetError);
        }
        SDL_RenderPresent(rw_.renderer());

        return  
#ifdef __EMSCRIPTEN__
            poll_event
#else
            wait_event
#endif
            ([/*&clicked,*/ this](const SDL_Event& e) -> basic_drawer::loop_result {
            /*if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && e.button.state == SDL_PRESSED) {
                clicked = true;
            } else*/ if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && e.button.state == SDL_RELEASED/* && clicked*/) {
                tf_.reset_static_texture();
#ifdef __EMSCRIPTEN__
                tl_.reset_static_texture();
#endif
                return { false, std::nullopt };
            }
            return { true, std::nullopt };
        });
    }

    void draw_impl()
    {
        if (SDL_SetRenderDrawColor(rw_.renderer(), 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE) ||
            SDL_RenderClear(rw_.renderer())) {
            debug_console("drawer", __func__, SDL_GetError);
        }

        const std::string t = "t = " + std::to_string(time());

        auto tf_txtr = tf_.get_texture(rw_.renderer(), t.c_str());
        auto ti_txtr = ti_.get_static_texture(rw_.renderer());

        if (!set_black_text(tf_txtr.get())) {
            is_ok_ = false;
            pbm::debug_console("drawer", __func__, SDL_GetError);
            return;
        }

        const auto text_rect = make_texture_rect(tf_txtr.get(), 10, 10),
              image_rect = make_texture_rect(ti_txtr.get(), static_cast<int>(current_pos_.x), static_cast<int>(current_pos_.y));

        if (!text_rect || !image_rect) {
            is_ok_ = false;
            return;
        } else if (!font_render_copy(tf_txtr.get(), &text_rect.value()) ||
                SDL_RenderCopyEx(rw_.renderer(), ti_txtr.get(), nullptr, &image_rect.value(), -angle_, nullptr, SDL_FLIP_NONE)) {
            is_ok_ = false;
            pbm::debug_console("drawer", __func__, SDL_GetError);
            return;
        }
        SDL_RenderPresent(rw_.renderer());
    }

    std::tuple<double, double> position(std::size_t x, std::size_t y, double tt) const noexcept
    {
        return { x + (speed_ * std::cos(rad_angle())) * tt, y + (speed_ * std::sin(rad_angle()) * tt) - ((g * tt * tt) / 2) };
    }
private:
    inline static constexpr double g = 9.80665;

    double angle_;
    const double speed_;
    pbm::texture::font& tf_;
#ifdef __EMSCRIPTEN__
    pbm::texture::font& tl_;
#endif
    pbm::texture::image& ti_;
    struct scb_pos { double x, y; } bp_, current_pos_;
    const double t_;
    const double land_time_;
    std::size_t c_;
};

} // namespace pbm

#endif
