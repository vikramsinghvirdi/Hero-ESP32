#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "application.h"

#ifdef CONFIG_HERO_DIAGNOSTIC_MODE
#include <algorithm>
#include <cmath>
#include <vector>
#include "board.h"
#include "boards/hero-xiao-esp32s3-sense/config.h"
#include "display.h"
#endif

#define TAG "main"

extern "C" void app_main(void) {
    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#ifdef CONFIG_HERO_DIAGNOSTIC_MODE
    auto& board = Board::GetInstance();
    board.GetDisplay()->SetupUI();
    auto* codec = board.GetAudioCodec();
    codec->Start();
    const int saved_output_volume = codec->output_volume();
    // The codec uses a squared volume curve. Keep this conservative but clearly audible;
    // the earlier 8% setting reduced the 16-bit test waveform to roughly 11 counts.
    codec->SetOutputVolume(55);
    codec->EnableOutput(true);

    std::vector<int16_t> tone(AUDIO_OUTPUT_SAMPLE_RATE * 7 / 10);
    for (size_t i = 0; i < tone.size(); ++i) {
        tone[i] = static_cast<int16_t>(12000.0 *
                                       std::sin(2.0 * M_PI * 440.0 * i / AUDIO_OUTPUT_SAMPLE_RATE));
    }
    codec->OutputData(tone);
    codec->EnableOutput(false);
    codec->SetOutputVolume(saved_output_volume);
    ESP_LOGI(TAG, "DIAG PASS speaker I2S write complete (440 Hz, conservative audible level)");

    codec->EnableInput(true);
    int changing_windows = 0;
    int64_t previous_rms = -1;
    for (int window = 0; window < 30; ++window) {
        std::vector<int16_t> samples(512);
        if (!codec->InputData(samples)) {
            ESP_LOGE(TAG, "DIAG FAIL microphone read window=%d", window);
            continue;
        }
        uint64_t square_sum = 0;
        int peak = 0;
        for (int16_t sample : samples) {
            square_sum += static_cast<int64_t>(sample) * sample;
            peak = std::max(peak, std::abs(static_cast<int>(sample)));
        }
        int64_t rms = static_cast<int64_t>(std::sqrt(square_sum / samples.size()));
        if (previous_rms >= 0 && std::abs(rms - previous_rms) > 5)
            ++changing_windows;
        previous_rms = rms;
        ESP_LOGI(TAG, "DIAG MIC window=%d rms=%" PRId64 " peak=%d", window, rms, peak);
    }
    codec->EnableInput(false);
    ESP_LOGI(TAG, "DIAG %s microphone signal changes=%d", changing_windows >= 3 ? "PASS" : "FAIL",
             changing_windows);
    ESP_LOGI(TAG, "DIAG COMPLETE; display eyes continue blinking");
    while (true)
        vTaskDelay(pdMS_TO_TICKS(1000));
#else
    // Initialize and run the application
    auto& app = Application::GetInstance();
    app.Initialize();
    app.Run();  // This function runs the main event loop and never returns
#endif
}
