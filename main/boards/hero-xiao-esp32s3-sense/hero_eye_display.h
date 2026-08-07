#ifndef HERO_EYE_DISPLAY_H_
#define HERO_EYE_DISPLAY_H_

#include "lvgl_display.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

class HeroEyeDisplay : public LvglDisplay {
public:
    enum class State { kBooting, kIdle, kListening, kThinking, kSpeaking, kOffline, kError };

    HeroEyeDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, bool mirror_x,
                   bool mirror_y);
    ~HeroEyeDisplay() override;

    void SetupUI() override;
    void SetStatus(const char* status) override;
    void ShowNotification(const char* notification, int duration_ms = 3000) override;
    void ShowNotification(const std::string& notification, int duration_ms = 3000) override;
    void SetEmotion(const char* emotion) override;
    void SetChatMessage(const char* role, const char* content) override;
    void UpdateStatusBar(bool update_all = false) override;
    void SetState(State state);

private:
    enum class IdleAnimation { kNeutral, kLookAround, kWink, kCurious, kSleepy };

    static void AnimationTimer(lv_timer_t* timer);
    void Animate();
    void DrawEyePair(int left_height, int right_height, int left_pupil_x, int right_pupil_x,
                     int left_pupil_y = 0, int right_pupil_y = 0, bool angled = false);
    void DrawEyes(int height, int pupil_offset, bool angled);
    void DrawMouth(int width, int height, int y_offset = 0);
    void StartRandomIdleAnimation();
    bool Lock(int timeout_ms = 0) override;
    void Unlock() override;

    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    lv_obj_t* left_eye_ = nullptr;
    lv_obj_t* right_eye_ = nullptr;
    lv_obj_t* left_pupil_ = nullptr;
    lv_obj_t* right_pupil_ = nullptr;
    lv_obj_t* mouth_ = nullptr;
    lv_timer_t* animation_timer_ = nullptr;
    State state_ = State::kBooting;
    uint32_t frame_ = 0;
    uint32_t next_blink_frame_ = 60;
    uint8_t blink_frames_ = 0;
    IdleAnimation idle_animation_ = IdleAnimation::kNeutral;
    uint32_t idle_animation_start_frame_ = 0;
    uint32_t idle_animation_end_frame_ = 0;
    uint32_t next_idle_animation_frame_ = 30;
};

#endif  // HERO_EYE_DISPLAY_H_
