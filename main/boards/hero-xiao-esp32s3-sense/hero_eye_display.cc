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
        .flags = {.buff_dma = 1, .buff_spiram = 0, .sw_rotate = 0, .full_refresh = 0,
                  .direct_mode = 0},
    };
    display_ = lvgl_port_add_disp(&display_cfg);
}

HeroEyeDisplay::~HeroEyeDisplay() {
    if (animation_timer_ != nullptr) lv_timer_delete(animation_timer_);
    if (panel_ != nullptr) esp_lcd_panel_del(panel_);
    if (panel_io_ != nullptr) esp_lcd_panel_io_del(panel_io_);
    lvgl_port_deinit();
}

bool HeroEyeDisplay::Lock(int timeout_ms) { return lvgl_port_lock(timeout_ms); }
void HeroEyeDisplay::Unlock() { lvgl_port_unlock(); }

void HeroEyeDisplay::SetupUI() {
    if (setup_ui_called_) return;
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

    state_label_ = lv_label_create(screen);
    lv_obj_set_width(state_label_, 128);
    lv_obj_set_style_text_align(state_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(state_label_, lv_color_white(), 0);
    lv_obj_set_style_text_font(state_label_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_align(state_label_, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_label_set_text(state_label_, "HERO BOOTING");
    animation_timer_ = lv_timer_create(AnimationTimer, 80, this);
}

void HeroEyeDisplay::AnimationTimer(lv_timer_t* timer) {
    static_cast<HeroEyeDisplay*>(lv_timer_get_user_data(timer))->Animate();
}

void HeroEyeDisplay::DrawEyes(int height, int pupil_offset, bool angled) {
    const int y = 63 - height / 2;
    lv_obj_set_y(left_eye_, y + (angled ? 3 : 0));
    lv_obj_set_y(right_eye_, y - (angled ? 3 : 0));
    lv_obj_set_height(left_eye_, height);
    lv_obj_set_height(right_eye_, height);
    if (height > 12) {
        lv_obj_remove_flag(left_pupil_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(right_pupil_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(left_pupil_, LV_ALIGN_CENTER, pupil_offset, 0);
        lv_obj_align(right_pupil_, LV_ALIGN_CENTER, pupil_offset, 0);
    } else {
        lv_obj_add_flag(left_pupil_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(right_pupil_, LV_OBJ_FLAG_HIDDEN);
    }
}

void HeroEyeDisplay::Animate() {
    ++frame_;
    if (state_ == State::kIdle && frame_ >= next_blink_frame_) {
        blink_frames_ = 3;
        next_blink_frame_ = frame_ + 38 + (esp_random() % 50);  // roughly 3-7 seconds
    }
    if (blink_frames_ > 0) {
        --blink_frames_;
        DrawEyes(5, 0, false);
        return;
    }

    switch (state_) {
        case State::kBooting: DrawEyes(18 + ((frame_ / 4) % 2) * 18, 0, false); break;
        case State::kIdle: {
            const int glance = ((frame_ % 150) > 132) ? (((frame_ / 150) & 1) ? -8 : 8) : 0;
            DrawEyes(49, glance, false);
            break;
        }
        case State::kListening: DrawEyes(56, 0, false); break;
        case State::kThinking: DrawEyes(34, static_cast<int>((frame_ / 5) % 3) * 8 - 8, true); break;
        case State::kSpeaking: DrawEyes(36 + ((frame_ / 2) % 3) * 8, 0, false); break;
        case State::kOffline: DrawEyes(12, -8, false); break;
        case State::kError: DrawEyes(10, 0, true); break;
    }
}

void HeroEyeDisplay::SetState(State state) {
    state_ = state;
    if (state_label_ == nullptr) return;
    static const char* labels[] = {"HERO BOOTING", "HERO", "LISTENING", "THINKING",
                                   "SPEAKING", "OFFLINE", "ERROR"};
    DisplayLockGuard lock(this);
    lv_label_set_text(state_label_, labels[static_cast<int>(state)]);
}

void HeroEyeDisplay::SetStatus(const char* status) {
    if (status == nullptr) return;
    if (std::strcmp(status, Lang::Strings::STANDBY) == 0) SetState(State::kIdle);
    else if (std::strcmp(status, Lang::Strings::LISTENING) == 0) SetState(State::kListening);
    else if (std::strcmp(status, Lang::Strings::SPEAKING) == 0) SetState(State::kSpeaking);
    else if (std::strcmp(status, Lang::Strings::CONNECTING) == 0) SetState(State::kThinking);
    else if (std::strcmp(status, Lang::Strings::ERROR) == 0) SetState(State::kError);
}

void HeroEyeDisplay::SetEmotion(const char* emotion) {
    if (emotion != nullptr && (std::strcmp(emotion, "warning") == 0 ||
                               std::strcmp(emotion, "sad") == 0)) SetState(State::kError);
}

void HeroEyeDisplay::SetChatMessage(const char*, const char*) {}
