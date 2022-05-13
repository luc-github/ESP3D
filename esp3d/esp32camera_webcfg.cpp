#include "Arduino.h"

#ifdef ARDUINO_ARCH_ESP32
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"

#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"

#include "config.h"
#include "esp32camera.h"

enum CAMERA_PARAM_ENUM {
    CAMERA_PARAM_FRAMESIZE,
    CAMERA_PARAM_QUALITY,
    CAMERA_PARAM_CONTRAST,
    CAMERA_PARAM_BRIGHTNESS,
    CAMERA_PARAM_SATURATION,
    CAMERA_PARAM_GAINCEILING,
    CAMERA_PARAM_FLASH,
    CAMERA_PARAM_AWB,
    CAMERA_PARAM_AGC,
    CAMERA_PARAM_AEC,
    CAMERA_PARAM_HMIRROR,
    CAMERA_PARAM_VFLIP,
    CAMERA_PARAM_AWB_GAIN,
    CAMERA_PARAM_AGC_GAIN,
    CAMERA_PARAM_AEC_VALUE,
    CAMERA_PARAM_AEC2,
    CAMERA_PARAM_DCW,
    CAMERA_PARAM_BPC,
    CAMERA_PARAM_WPC,
    CAMERA_PARAM_RAW_GMA,
    CAMERA_PARAM_LENC,
    CAMERA_PARAM_SPECIAL_EFFECT,
    CAMERA_PARAM_WB_MODE,
    CAMERA_PARAM_AE_LEVEL,
    CAMERA_PARAM_SHARPNESS,
    CAMERA_PARAMS_COUNT
};

static const char* const camera_param_strings[25] = {
    "framesize",
    "quality",
    "contrast",
    "brightness",
    "saturation",
    "gainceiling",
    "flash",
    "awb",
    "agc",
    "aec",
    "hmirror",
    "vflip",
    "awb_gain",
    "agc_gain",
    "aec_value",
    "aec2",
    "dcw",
    "bpc",
    "wpc",
    "raw_gma",
    "lenc",
    "special_effect",
    "wb_mode",
    "ae_level",
    "sharpness",
};

#define PARAM_STR(x) camera_param_strings[CAMERA_PARAM_ ## x]

static const char* json_param_int_fmt = "\"%s\":%d%c";

String getCameraStatusJSON(void)
{
    sensor_t * s = esp_camera_sensor_get();
    String resp = "{";

    #define PRINT_JSON_PARAM_INT(str, param, value, last) do { \
        char print_buf[32]; \
        snprintf(print_buf, 32, json_param_int_fmt, PARAM_STR(param), value, last ? ' ': ','); \
        str +=  print_buf; \
    } while (0)

    PRINT_JSON_PARAM_INT(resp, FRAMESIZE, s->status.framesize, false);
    PRINT_JSON_PARAM_INT(resp, QUALITY, s->status.quality, false);
    PRINT_JSON_PARAM_INT(resp, BRIGHTNESS, s->status.brightness, false);
    PRINT_JSON_PARAM_INT(resp, CONTRAST, s->status.contrast, false);
    PRINT_JSON_PARAM_INT(resp, SATURATION, s->status.saturation, false);
    PRINT_JSON_PARAM_INT(resp, SHARPNESS, s->status.sharpness, false);
    PRINT_JSON_PARAM_INT(resp, SPECIAL_EFFECT, s->status.special_effect, false);
    PRINT_JSON_PARAM_INT(resp, WB_MODE, s->status.wb_mode, false);
    PRINT_JSON_PARAM_INT(resp, AWB, s->status.awb, false);
    PRINT_JSON_PARAM_INT(resp, AWB_GAIN, s->status.awb_gain, false);
    PRINT_JSON_PARAM_INT(resp, AEC, s->status.aec, false);
    PRINT_JSON_PARAM_INT(resp, AEC2, s->status.aec2, false);
    PRINT_JSON_PARAM_INT(resp, AE_LEVEL, s->status.ae_level, false);
    PRINT_JSON_PARAM_INT(resp, AEC_VALUE, s->status.aec_value, false);
    PRINT_JSON_PARAM_INT(resp, AGC, s->status.agc, false);
    PRINT_JSON_PARAM_INT(resp, AGC_GAIN, s->status.agc_gain, false);
    PRINT_JSON_PARAM_INT(resp, GAINCEILING, s->status.gainceiling, false);
    PRINT_JSON_PARAM_INT(resp, BPC, s->status.bpc, false);
    PRINT_JSON_PARAM_INT(resp, WPC, s->status.wpc, false);
    PRINT_JSON_PARAM_INT(resp, RAW_GMA, s->status.raw_gma, false);
    PRINT_JSON_PARAM_INT(resp, LENC, s->status.lenc, false);
    PRINT_JSON_PARAM_INT(resp, VFLIP, s->status.vflip, false);
    PRINT_JSON_PARAM_INT(resp, HMIRROR, s->status.hmirror, false);
    PRINT_JSON_PARAM_INT(resp, DCW, s->status.dcw, false);
    PRINT_JSON_PARAM_INT(resp, FLASH, s->status.colorbar, true);
    resp += "}";
    return resp;
}

int setCameraStatus(String variable, String value)
{
    int res;
    int val = atoi(value.c_str());
    int param_idx = 0;
    for (; param_idx < CAMERA_PARAMS_COUNT; param_idx++) {
        if (variable.equals(camera_param_strings[param_idx])) {
            break;
        }
    }

    sensor_t * s = esp_camera_sensor_get();
    switch (param_idx) {
        case CAMERA_PARAM_FRAMESIZE: {
            if (s->pixformat == PIXFORMAT_JPEG){
                res = s->set_framesize(s, (framesize_t)val);
            } else {
                res = 0;
            }
        }
        break;
        case CAMERA_PARAM_QUALITY:        res = s->set_quality(s, val); break;
        case CAMERA_PARAM_CONTRAST:       res = s->set_contrast(s, val); break;
        case CAMERA_PARAM_BRIGHTNESS:     res = s->set_brightness(s, val); break;
        case CAMERA_PARAM_SATURATION:     res = s->set_saturation(s, val); break;
        case CAMERA_PARAM_GAINCEILING:    res = s->set_gainceiling(s, (gainceiling_t)val); break;
        case CAMERA_PARAM_FLASH:          digitalWrite(LED_GPIO_NUM, val); res = 0; break;
        case CAMERA_PARAM_AWB:            res = s->set_whitebal(s, val); break;
        case CAMERA_PARAM_AGC:            res = s->set_gain_ctrl(s, val); break;
        case CAMERA_PARAM_AEC:            res = s->set_exposure_ctrl(s, val); break;
        case CAMERA_PARAM_HMIRROR:        res = s->set_hmirror(s, val); break;
        case CAMERA_PARAM_VFLIP:          res = s->set_vflip(s, val); break;
        case CAMERA_PARAM_AWB_GAIN:       res = s->set_awb_gain(s, val); break;
        case CAMERA_PARAM_AGC_GAIN:       res = s->set_agc_gain(s, val); break;
        case CAMERA_PARAM_AEC_VALUE:      res = s->set_aec_value(s, val); break;
        case CAMERA_PARAM_AEC2:           res = s->set_aec2(s, val); break;
        case CAMERA_PARAM_DCW:            res = s->set_dcw(s, val); break;
        case CAMERA_PARAM_BPC:            res = s->set_bpc(s, val); break;
        case CAMERA_PARAM_WPC:            res = s->set_wpc(s, val); break;
        case CAMERA_PARAM_RAW_GMA:        res = s->set_raw_gma(s, val); break;
        case CAMERA_PARAM_LENC:           res = s->set_lenc(s, val); break;
        case CAMERA_PARAM_SPECIAL_EFFECT: res = s->set_special_effect(s, val); break;
        case CAMERA_PARAM_WB_MODE:        res = s->set_wb_mode(s, val); break;
        case CAMERA_PARAM_AE_LEVEL:       res = s->set_ae_level(s, val); break;
        case CAMERA_PARAM_SHARPNESS:      res = s->set_sharpness(s, val); break;
        default: res = -1; break;
    }

    return res;
}

bool cameraInit(void)
{
    pinMode(LED_GPIO_NUM, OUTPUT);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    //init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        LOG("PSRAM found\n");
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        LOG("PSRAM not found\n");
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        LOGF("Camera init failed with error 0x%x", err);
        return false;
    }

    sensor_t * s = esp_camera_sensor_get();
    LOGF("Sensor PID: %d\n", s->id.PID);
    LOGF("Sensor full ID: %08x\n", *(uint32_t*)(&s->id));
    // drop down frame size for higher initial frame rate
    int ret = s->set_framesize(s, FRAMESIZE_VGA);
#if ROTATE_CAMERA_180
    // rotate sensor 180 degrees
    ret |= s->set_hmirror(s, 1);
    ret |= s->set_vflip(s, 1);
#endif
    if (ret != 0) {
        return false;
    }

    return true;
}
#endif // ARDUINO_ARCH_ESP32
