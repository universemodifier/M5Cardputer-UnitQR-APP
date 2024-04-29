/**
 * @file app_hello.h
 * @author lexarion
 * @brief Small App for M5Stack QRCode in I2C mode
 * @version 0.1
 * @date 2024-04-29
 * 
 * @copyright Copyright (c) 2024
 */

#include <mooncake.h>
#include "../../hal/hal.h"
#include "../utils/theme/theme_define.h"
#include "../utils/anim/anim_define.h"
#include "../utils/icon/icon_define.h"

#include "assets/qr_big.h"
#include "assets/qr_small.h"

namespace MOONCAKE
{
    namespace APPS
    {
        class AppQrcode : public APP_BASE
        {
            private:
                enum State_t
                {
                    state_init = 0,
                    state_wait_input,
                    state_switch_trigger,
                    state_get_fw_ver,
                };
                struct Data_t
                {
                    HAL::Hal* hal = nullptr;
                    int last_key_num = 0;
                    State_t current_state = state_init;
                    bool scan_mode = false;
                    char string_buffer[620];
                };
                typedef enum {
                AUTO_SCAN_MODE   = 0,
                MANUAL_SCAN_MODE = 1

                } qrcode_scan_mode_t;
                Data_t _data;
                
                void _display_fw_ver();
                void _init_i2c();
                uint8_t _get_decoder_ready();
                uint16_t _get_decoder_length();
                void _get_decoder_data(uint8_t *data, uint16_t len);
                void _set_trigger_mode(uint8_t mode);
                void _update_input();
                void _update_state();


                #define UNIT_QRCODE_ADDR             0x21
                #define UNIT_QRCODE_TRIGGER_REG      0x0000
                #define UNIT_QRCODE_READY_REG        0x0010
                #define UNIT_QRCODE_LENGTH_REG       0x0020
                #define UNIT_QRCODE_TRIGGER_MODE_REG 0x0030
                #define UNIT_QRCODE_TRIGGER_KEY_REG  0x0040
                #define UNIT_QRCODE_DATA_REG         0x1000
                #define JUMP_TO_BOOTLOADER_REG 0x00FD
                #define FIRMWARE_VERSION_REG   0x00FE
                #define I2C_ADDRESS_REG        0x00FF

            public:
                void onCreate() override;
                void onResume() override;
                void onRunning() override;
                void onDestroy() override;
        };

        class AppQrcode_Packer : public APP_PACKER_BASE
        {
            std::string getAppName() override { return "QRCODE"; }
            void* getAppIcon() override { return (void*)(new AppIcon_t(image_data_qr_big, image_data_qr_small)); }
            void* newApp() override { return new AppQrcode; }
            void deleteApp(void *app) override { delete (AppQrcode*)app; }
        };
    }
}