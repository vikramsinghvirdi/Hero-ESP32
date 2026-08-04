#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "hero_eye_display.h"

#include <driver/i2c_master.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_sh1107.h>
#include <esp_log.h>
#include <esp_psram.h>
#include <esp_system.h>

#define TAG "HeroBoard"

class HeroXiaoEsp32S3Sense : public WifiBoard {
public:
    HeroXiaoEsp32S3Sense() : boot_button_(BOOT_BUTTON_GPIO) {
        LogHardware();
        InitializeDisplay();
        InitializeButton();
    }

    AudioCodec* GetAudioCodec() override {
        static NoAudioCodecSimplexPdm codec(
            AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT,
            AUDIO_I2S_MIC_GPIO_CLK, AUDIO_I2S_MIC_GPIO_DIN);
        return &codec;
    }

    Display* GetDisplay() override { return display_; }

private:
    void LogHardware() {
        esp_chip_info_t info = {};
        esp_chip_info(&info);
        uint32_t flash_size = 0;
        ESP_ERROR_CHECK(esp_flash_get_size(nullptr, &flash_size));
        ESP_LOGI(TAG, "DIAG chip=ESP32-S3 cores=%d revision=%d reset_reason=%d", info.cores,
                 info.revision, esp_reset_reason());
        ESP_LOGI(TAG, "DIAG flash=%" PRIu32 " bytes psram=%zu bytes", flash_size,
                 esp_psram_get_size());
        ESP_LOGI(TAG, "DIAG %s flash size", flash_size == 8 * 1024 * 1024 ? "PASS" : "FAIL");
        ESP_LOGI(TAG, "DIAG %s PSRAM", esp_psram_get_size() > 0 ? "PASS" : "FAIL");
    }

    void InitializeDisplay() {
        i2c_master_bus_config_t bus_config = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = DISPLAY_SDA_PIN,
            .scl_io_num = DISPLAY_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {.enable_internal_pullup = 1},
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &display_i2c_bus_));

        uint8_t address = 0;
        for (uint8_t candidate : {DISPLAY_I2C_PRIMARY_ADDRESS, DISPLAY_I2C_SECONDARY_ADDRESS}) {
            if (i2c_master_probe(display_i2c_bus_, candidate, 100) == ESP_OK) {
                address = candidate;
                break;
            }
        }
        if (address == 0) {
            ESP_LOGE(TAG, "DIAG FAIL OLED not found at 0x3C or 0x3D");
            display_ = new NoDisplay();
            return;
        }
        ESP_LOGI(TAG, "DIAG PASS OLED detected at 0x%02X", address);

        esp_lcd_panel_io_i2c_config_t io_config = {
            .dev_addr = address,
            .scl_speed_hz = 100000,
            .control_phase_bytes = 1,
            .dc_bit_offset = 0,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .on_color_trans_done = nullptr,
            .user_ctx = nullptr,
            .flags = {.dc_low_on_data = 0, .disable_control_phase = 1},
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(display_i2c_bus_, &io_config, &panel_io_));
        esp_lcd_panel_sh1107_config_t sh1107_config = {.contrast = 128, .offset = 0x60};
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.bits_per_pixel = 1;
        panel_config.vendor_config = &sh1107_config;
        ESP_ERROR_CHECK(esp_lcd_new_panel_sh1107(panel_io_, &panel_config, &panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_, false));
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));
        display_ = new HeroEyeDisplay(panel_io_, panel_, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
    }

    void InitializeButton() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) EnterWifiConfigMode();
            else app.ToggleChatState();
        });
    }

    i2c_master_bus_handle_t display_i2c_bus_ = nullptr;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    Display* display_ = nullptr;
    Button boot_button_;
};

DECLARE_BOARD(HeroXiaoEsp32S3Sense);
