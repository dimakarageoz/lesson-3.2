#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include "ema.h"

#include "config.h"

#define LED_PIN LED_PIN_DEFAULT

#define TAG "Light checker"

void wait(int ms) {
    vTaskDelay(pdMS_TO_TICKS(ms)); 
}

void adcInputSetup(
    adc_oneshot_unit_handle_t *unitHandler,
    adc_cali_handle_t *calibrationHandle
) {
    adc_oneshot_unit_init_cfg_t unitConfig = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT
    };

    adc_oneshot_new_unit(&unitConfig, unitHandler);

    adc_oneshot_chan_cfg_t channelConfig = {
        .atten = ADC_ATTEN_DB_6,
        .bitwidth = ADC_BITWIDTH_12
    };

    adc_oneshot_config_channel(*unitHandler, PHOTORES_ANALOG_CHANNEL, &channelConfig);

    adc_cali_curve_fitting_config_t calibrationConfig = {
        .unit_id = ADC_UNIT_1,
        .chan = PHOTORES_ANALOG_CHANNEL,
        .atten = ADC_ATTEN_DB_6,
        .bitwidth = ADC_BITWIDTH_12
    };

    adc_cali_create_scheme_curve_fitting(&calibrationConfig, calibrationHandle);
}

void ledSetup() {
    gpio_config_t ledConfig = {
        .pin_bit_mask = (1 << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&ledConfig);
}


void changeLightState(int nextState) {
    static int lightState = 0;

    if (lightState != nextState) {
        lightState = nextState;
        
        ESP_LOGI(TAG, "lightState: %d", lightState);

        gpio_set_level(LED_PIN, lightState);
    }
}

void lightADCReaderHandler(
    adc_oneshot_unit_handle_t *unitHandler,
    adc_cali_handle_t *calibrationHandle,
    EmaFilterCoefficient *emaFilter
) {
    int adc_raw = 0, voltage = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(*unitHandler, PHOTORES_ANALOG_CHANNEL, &adc_raw));

    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(*calibrationHandle, adc_raw, &voltage));

    int filteredVoltage = exponentialMovingAverage(voltage, emaFilter);

    changeLightState(filteredVoltage > LED_SWITCH_VOLTAGE ? 1 : 0);
}

void setup(
    adc_oneshot_unit_handle_t *unitHandler,
    adc_cali_handle_t *calibrationHandle
) {
    adcInputSetup(unitHandler, calibrationHandle);

    ledSetup();
}

void app_main() {
    adc_oneshot_unit_handle_t unitHandler = NULL;
    adc_cali_handle_t calibrationHandle = NULL;
    EmaFilterCoefficient *emaFilter = createEmaFilterCoefficient(0.5f);

    setup(&unitHandler, &calibrationHandle);

    while (1) {
        lightADCReaderHandler(&unitHandler, &calibrationHandle, emaFilter);

        wait(20);
    }
}