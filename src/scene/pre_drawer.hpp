#ifndef INCLUDED_PBM_SCENE_PRE_DRAWER_HPP
#define INCLUDED_PBM_SCENE_PRE_DRAWER_HPP
#include "../texture/font.hpp"
#include "../texture/image.hpp"
#include "../rw_manager.hpp"
#include "basic_drawer.hpp"
#include <cmath>

namespace pbm::scene {

class pre_drawer : public basic_drawer {
public:
    pre_drawer(double angle, pbm::rw_manager& rw, pbm::texture::font& tf, pbm::texture::font& tfr, pbm::texture::font& td, pbm::texture::image& ti)
        : basic_drawer(rw),
        angle_(std::move(angle)), 
        tf_{ tf },
        tfr_{ tfr },
        td_{ td },
        ti_{ ti },
        bp_{ 0, static_cast<double>(rw_.drawable_size().height) - ti_.height() },
        mp_{ 0, 0 }
    {}

    void angle_from_2p(double x, double y) noexcept
    {
        mp_.x = x;
        mp_.y = y;

        double r = std::atan2(mp_.y - (bp_.y + ti_.height() / 2), mp_.x - (bp_.x + ti_.width() / 2));
        if (r < 0) r += 2 * M_PI;
        angle_ = 360 - r * 180 / M_PI;
        
        if (angle_ > 90 && angle_ <= 225) angle_ = 90;
        else if (angle_ > 225 && angle_ <= 360) angle_ = 0;
    }

    inline constexpr double angle() const noexcept { return angle_; }

    inline void draw() noexcept { draw_impl(); }

    inline double line_distance() const noexcept 
    {
        const auto l = mp_.x - (ti_.width() / 2);
        const auto r = mp_.y - (bp_.y + ti_.height() / 2);
        const auto d = std::sqrt(l * l + r * r);
        return d / 10 < 1 ? 1 : d / 7.6; // HACK
    }

    basic_drawer::loop_result operator()()
    {
        static bool clicked = false;
        int x = 0, y = 0;
        SDL_GetMouseState(&x, &y);

        basic_drawer::loop_result res = poll_event([&x, &y, this](const SDL_Event& e) -> basic_drawer::loop_result {
            if (e.type == SDL_MOUSEMOTION) {
                x = e.motion.x;
                y = e.motion.y;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && e.button.state == SDL_PRESSED) {
                clicked = true;
            } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && e.button.state == SDL_RELEASED && clicked) {
                angle_from_2p(e.button.x, e.button.y);
                tf_.reset_static_texture();
                return { false, std::nullopt };
            }
            return { true, std::nullopt };
        });

        if (!res.first) return res;

        angle_from_2p(x, y);
        draw();
        return res;
    }
private:
    void draw_follow_line() noexcept
    {
        Uint8 r, g, b, a;
        if (SDL_GetRenderDrawColor(rw_.renderer(), &r, &g, &b, &a) || 
            SDL_SetRenderDrawColor(rw_.renderer(), 29, 161, 242, SDL_ALPHA_OPAQUE)) {
            debug_console("pre_drawer", __func__, SDL_GetError);
        }
        
        double rq = std::atan2(mp_.y - (bp_.y + ti_.height() / 2), mp_.x - (bp_.x + ti_.width() / 2));
        if (rq < 0) rq += 2 * M_PI;
        rq = 360 - rq * 180 / M_PI;

        if (SDL_RenderDrawLine(
                rw_.renderer(), 
                bp_.x + ti_.width() / 2, 
                bp_.y + ti_.height() / 2, 
                rq > 90 && rq <= 225 ? ti_.width() / 2 : mp_.x, 
                rq > 225 && rq <= 360 ? rw_.drawable_size().height - ti_.height() / 2 : mp_.y
            ) ||
            SDL_SetRenderDrawColor(rw_.renderer(), r, g, b, a)) {
            debug_console("pre_drawer", __func__, SDL_GetError);
        }
    }

    void draw_impl() noexcept
    {
        if(SDL_SetRenderDrawColor(rw_.renderer(), 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE) ||
            SDL_RenderClear(rw_.renderer())) {
            debug_console("drawer", __func__, SDL_GetError);
        }

        draw_follow_line();

        std::string s = "a = " + std::to_string(std::size_t(angle())), 
            ldist = "s = " + std::to_string(std::size_t(line_distance()));
        
        auto tf_txtr = tf_.get_static_texture(rw_.renderer(), "t = 0");
        auto tfr_txtr = tfr_.get_texture(rw_.renderer(), s.c_str());
        auto td_txtr = td_.get_texture(rw_.renderer(), ldist.c_str());
        auto ti_txtr = ti_.get_static_texture(rw_.renderer());
        
        if (!set_black_text(tf_txtr.get()) || !set_black_text(tfr_txtr.get()) || !set_black_text(td_txtr.get())) {
            is_ok_ = false;
            pbm::debug_console("pre_drawer", __func__, SDL_GetError);
            return;
        }
        
        const auto text_rect = make_texture_rect(tf_txtr.get(), 10, 10),
              angle_rect = make_texture_rect(tfr_txtr.get(), 10, 40),
              dis_rect = make_texture_rect(td_txtr.get(), 10, 80),
              image_rect = make_texture_rect(ti_txtr.get(), static_cast<int>(bp_.x), static_cast<int>(bp_.y));
        
        if (!text_rect || !angle_rect || !dis_rect || !image_rect) {
            is_ok_ = false;
            return;
        } else if (!font_render_copy(tf_txtr.get(), &text_rect.value()) ||
            !font_render_copy(tfr_txtr.get(), &angle_rect.value()) ||
            !font_render_copy(td_txtr.get(), &dis_rect.value()) ||
            SDL_RenderCopyEx(rw_.renderer(), ti_txtr.get(), nullptr, &image_rect.value(), -angle(), nullptr, SDL_FLIP_NONE)) {
            is_ok_ = false;
            pbm::debug_console("pre_drawer", __func__, SDL_GetError);
            return;
        }

        SDL_RenderPresent(rw_.renderer());
    }

private:
    double angle_;
    pbm::texture::font& tf_, &tfr_, &td_;
    pbm::texture::image& ti_;
    const struct scb_pos { double x, y; } bp_;
    struct mouse_pos { double x, y; } mp_;
};

} // namespace pbm::scene

#endif
