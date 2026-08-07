#include "hero_eye_display.h"

#include "assets/lang_config.h"
#include "lvgl_font.h"
#include "lvgl_theme.h"

#include <esp_lvgl_port.h>
#include <esp_random.h>
#include <cstring>

LV_FONT_DECLARE(BUILTIN_TEXT_FONT);
LV_FONT_DECLARE(BUILTIN_ICON_FONT);

HeroEyeDisplay::HeroEyeDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                               bool mirror_x, bool mirror_y)
    : panel_io_(panel_io), panel_(panel) {
    width_ = 128;
    height_ = 128;

    auto theme = new LvglTheme("hero-monochrome");
    theme->set_text_font(std::make_shared<LvglBuiltInFont>(&BUILTIN_TEXT_FONT));
    theme->set_icon_font(std::make_shared<LvglBuiltInFont>(&BUILTIN_ICON_FONT));
    LvglThemeManager::GetInstance().RegisterTheme("hero-monochrome", theme);
    current_theme_ = theme;

    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 1;
    port_cfg.task_stack = 6144;
#if CONFIG_SOC_CPU_CORES_NUM > 1
    port_cfg.task_affinity = 1;
#endif
    ESP_ERROR_CHECK(lvgl_port_init(&port_cfg));

    const lvgl_port_display_cfg_t display_cfg = {
        .io_handle = panel_io_,
        .panel_handle = panel_,
        .control_handle = nullptr,
        .buffer_size = 128 * 128,
        .double_buffer = false,
        .trans_size = 0,
        .hres = 128,
        .vres = 128,
        .monochrome = true,
        .rotation = {.swap_xy = false, .mirror_x = mirror_x, .mirror_y = mirror_y},
        .flags =
            {.buff_dma = 1, .buff_spiram = 0, .sw_rotate = 0, .full_refresh = 0, .direct_mode = 0},
    };
    display_ = lvgl_port_add_disp(&display_cfg);
}

HeroEyeDisplay::~HeroEyeDisplay() {
    if (animation_timer_ != nullptr)
        lv_timer_delete(animation_timer_);
    if (panel_ != nullptr)
        esp_lcd_panel_del(panel_);
    if (panel_io_ != nullptr)
        esp_lcd_panel_io_del(panel_io_);
    lvgl_port_deinit();
}

bool HeroEyeDisplay::Lock(int timeout_ms) { return lvgl_port_lock(timeout_ms); }
void HeroEyeDisplay::Unlock() { lvgl_port_unlock(); }

void HeroEyeDisplay::SetupUI() {
    if (setup_ui_called_)
        return;
    Display::SetupUI();
    DisplayLockGuard lock(this);
    lv_obj_t* screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    auto make_eye = [screen](int x) {
        lv_obj_t* eye = lv_obj_create(screen);
        lv_obj_remove_style_all(eye);
        lv_obj_set_pos(eye, x, 39);
        lv_obj_set_size(eye, 42, 49);
        lv_obj_set_style_radius(eye, 13, 0);
        lv_obj_set_style_bg_color(eye, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(eye, LV_OPA_COVER, 0);
        return eye;
    };
    left_eye_ = make_eye(17);
    right_eye_ = make_eye(69);

    auto make_pupil = [](lv_obj_t* eye) {
        lv_obj_t* pupil = lv_obj_create(eye);
        lv_obj_remove_style_all(pupil);
        lv_obj_set_size(pupil, 12, 24);
        lv_obj_set_style_radius(pupil, 6, 0);
        lv_obj_set_style_bg_color(pupil, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(pupil, LV_OPA_COVER, 0);
        lv_obj_center(pupil);
        return pupil;
    };
    left_pupil_ = make_pupil(left_eye_);
    right_pupil_ = make_pupil(right_eye_);

    mouth_ = lv_obj_create(screen);
    lv_obj_remove_style_all(mouth_);
    lv_obj_set_size(mouth_, 18, 6);
    lv_obj_set_style_radius(mouth_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(mouth_, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(mouth_, LV_OPA_COVER, 0);
    lv_obj_align(mouth_, LV_ALIGN_TOP_MID, 0, 99);
    animation_timer_ = lv_timer_create(AnimationTimer, 80, this);
}

void HeroEyeDisplay::AnimationTimer(lv_timer_t* timer) {
    static_cast<HeroEyeDisplay*>(lv_timer_get_user_data(timer))->Animate();
}

void HeroEyeDisplay::DrawEyePair(int left_height, int right_height, int left_pupil_x,
                                 int right_pupil_x, int left_pupil_y, int right_pupil_y,
                                 bool angled) {
    lv_obj_set_y(left_eye_, 63 - left_height / 2 + (angled ? 3 : 0));
    lv_obj_set_y(right_eye_, 63 - right_height / 2 - (angled ? 3 : 0));
    lv_obj_set_height(left_eye_, left_height);
    lv_obj_set_height(right_eye_, right_height);

    if (left_height > 12) {
        lv_obj_remove_flag(left_pupil_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(left_pupil_, LV_ALIGN_CENTER, left_pupil_x, left_pupil_y);
    } else {
        lv_obj_add_flag(left_pupil_, LV_OBJ_FLAG_HIDDEN);
    }
    if (right_height > 12) {
        lv_obj_remove_flag(right_pupil_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(right_pupil_, LV_ALIGN_CENTER, right_pupil_x, right_pupil_y);
    } else {
        lv_obj_add_flag(right_pupil_, LV_OBJ_FLAG_HIDDEN);
    }
}

void HeroEyeDisplay::DrawEyes(int height, int pupil_offset, bool angled) {
    DrawEyePair(height, height, pupil_offset, pupil_offset, 0, 0, angled);
}

void HeroEyeDisplay::DrawMouth(int width, int height, int y_offset) {
    lv_obj_set_size(mouth_, width, height);
    lv_obj_align(mouth_, LV_ALIGN_TOP_MID, 0, 99 + y_offset);
}

void HeroEyeDisplay::StartRandomIdleAnimation() {
    // Select one of four expressive sequences; neutral is the resting state between them.
    idle_animation_ = static_cast<IdleAnimation>(1 + (esp_random() % 4));
    idle_animation_start_frame_ = frame_;
    static const uint8_t durations[] = {0, 30, 10, 28, 24};
    idle_animation_end_frame_ = frame_ + durations[static_cast<int>(idle_animation_)];
}

void HeroEyeDisplay::Animate() {
    ++frame_;

    // Restore the neutral pupil shape before applying state-specific expressions.
    lv_obj_set_size(left_pupil_, 12, 24);
    lv_obj_set_size(right_pupil_, 12, 24);

    switch (state_) {
        case State::kBooting:
            DrawMouth(10 + ((frame_ / 4) % 2) * 8, 5);
            break;
        case State::kIdle:
            DrawMouth(18, 6);
            break;
        case State::kListening: {
            const bool listening_pulse = ((frame_ / 3) & 1) != 0;
            DrawMouth(listening_pulse ? 26 : 12, listening_pulse ? 8 : 5, -1);
            break;
        }
        case State::kThinking:
            DrawMouth(12, 5, 1);
            break;
        case State::kSpeaking: {
            static const uint8_t mouth_widths[] = {14, 20, 17, 22};
            static const uint8_t mouth_heights[] = {6, 11, 8, 14};
            const int mouth_frame = (frame_ / 2) % 4;
            DrawMouth(mouth_widths[mouth_frame], mouth_heights[mouth_frame], -2);
            break;
        }
        case State::kOffline:
            DrawMouth(14, 5, 2);
            break;
        case State::kError:
            DrawMouth(10, 5, 2);
            break;
    }

    if (state_ == State::kIdle && idle_animation_ == IdleAnimation::kNeutral &&
        frame_ >= next_idle_animation_frame_) {
        StartRandomIdleAnimation();
    }
    if (state_ == State::kIdle && idle_animation_ != IdleAnimation::kNeutral &&
        frame_ >= idle_animation_end_frame_) {
        idle_animation_ = IdleAnimation::kNeutral;
        next_idle_animation_frame_ = frame_ + 45 + (esp_random() % 75);  // 3.6-9.5 seconds
    }
    if (state_ == State::kIdle && idle_animation_ == IdleAnimation::kNeutral &&
        frame_ >= next_blink_frame_) {
        blink_frames_ = 3;
        next_blink_frame_ = frame_ + 38 + (esp_random() % 50);  // roughly 3-7 seconds
    }
    if (blink_frames_ > 0) {
        --blink_frames_;
        DrawEyes(5, 0, false);
        return;
    }

    switch (state_) {
        case State::kBooting:
            DrawEyes(18 + ((frame_ / 4) % 2) * 18, 0, false);
            break;
        case State::kIdle: {
            const uint32_t phase = frame_ - idle_animation_start_frame_;
            switch (idle_animation_) {
                case IdleAnimation::kNeutral:
                    DrawEyes(49, 0, false);
                    break;
                case IdleAnimation::kLookAround: {
                    const int glance = phase < 8 ? -8 : (phase < 16 ? 8 : 0);
                    DrawEyePair(49, 49, glance, glance, phase < 16 ? -2 : 0, phase < 16 ? -2 : 0);
                    break;
                }
                case IdleAnimation::kWink: {
                    const bool wink_left = ((idle_animation_start_frame_ / 10) & 1) == 0;
                    DrawEyePair(wink_left ? 5 : 49, wink_left ? 49 : 5, 0, 0);
                    break;
                }
                case IdleAnimation::kCurious: {
                    static const int8_t x_path[] = {-7, -3, 3, 7, 3, -3};
                    static const int8_t y_path[] = {0, -4, -4, 0, 4, 4};
                    const int index = (phase / 4) % 6;
                    DrawEyePair(56, 56, x_path[index], x_path[index], y_path[index], y_path[index]);
                    break;
                }
                case IdleAnimation::kSleepy: {
                    const int sleepy_height = 16 + ((phase / 6) & 1) * 6;
                    DrawEyePair(sleepy_height, sleepy_height, 0, 0, 4, 4);
                    break;
                }
            }
            break;
        }
        case State::kListening: {
            // Small pupils and a slow inward orbit form a distinct, attentive expression.
            static const int8_t focus_x[] = {6, 4, 6, 5};
            static const int8_t focus_y[] = {-4, 0, 4, 0};
            static const uint8_t focus_height[] = {58, 54, 58, 54};
            const int focus_frame = (frame_ / 3) % 4;
            lv_obj_set_size(left_pupil_, 9, 14);
            lv_obj_set_size(right_pupil_, 9, 14);
            DrawEyePair(focus_height[focus_frame], focus_height[focus_frame], focus_x[focus_frame],
                        -focus_x[focus_frame], focus_y[focus_frame], focus_y[focus_frame]);
            break;
        }
        case State::kThinking:
            DrawEyes(34, static_cast<int>((frame_ / 5) % 3) * 8 - 8, true);
            break;
        case State::kSpeaking: {
            // Speaking is deliberately much more animated than idle: both gaze direction and
            // individual eye height change every 160 ms to give the face conversational energy.
            static const int8_t speech_x[] = {-8, -4, 4, 8, 4, -4, 0, 0};
            static const int8_t speech_y[] = {0, -5, -3, 0, 5, 3, -4, 4};
            static const uint8_t left_height[] = {42, 50, 58, 48, 56, 44, 54, 46};
            static const uint8_t right_height[] = {56, 44, 50, 58, 42, 54, 46, 56};
            const int speech_frame = (frame_ / 2) % 8;
            DrawEyePair(left_height[speech_frame], right_height[speech_frame],
                        speech_x[speech_frame], speech_x[speech_frame], speech_y[speech_frame],
                        speech_y[speech_frame]);
            break;
        }
        case State::kOffline:
            DrawEyes(12, -8, false);
            break;
        case State::kError:
            DrawEyes(10, 0, true);
            break;
    }
}

void HeroEyeDisplay::SetState(State state) {
    if (state_ != state) {
        idle_animation_ = IdleAnimation::kNeutral;
        idle_animation_start_frame_ = frame_;
        idle_animation_end_frame_ = frame_;
        next_idle_animation_frame_ = frame_ + 25 + (esp_random() % 45);
        next_blink_frame_ = frame_ + 38 + (esp_random() % 50);
        blink_frames_ = 0;
    }
    state_ = state;
}

void HeroEyeDisplay::SetStatus(const char* status) {
    if (status == nullptr)
        return;
    if (std::strcmp(status, Lang::Strings::STANDBY) == 0)
        SetState(State::kIdle);
    else if (std::strcmp(status, Lang::Strings::LISTENING) == 0)
        SetState(State::kListening);
    else if (std::strcmp(status, Lang::Strings::SPEAKING) == 0)
        SetState(State::kSpeaking);
    else if (std::strcmp(status, Lang::Strings::CONNECTING) == 0)
        SetState(State::kThinking);
    else if (std::strcmp(status, Lang::Strings::ERROR) == 0)
        SetState(State::kError);
}

void HeroEyeDisplay::ShowNotification(const char*, int) {}

void HeroEyeDisplay::ShowNotification(const std::string&, int) {}

void HeroEyeDisplay::SetEmotion(const char* emotion) {
    if (emotion != nullptr &&
        (std::strcmp(emotion, "warning") == 0 || std::strcmp(emotion, "sad") == 0))
        SetState(State::kError);
}

void HeroEyeDisplay::SetChatMessage(const char*, const char*) {}

void HeroEyeDisplay::UpdateStatusBar(bool) {}
