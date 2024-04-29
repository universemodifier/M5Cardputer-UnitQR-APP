/**
 * @file app_qrcode.cpp
 * @author Thomas Lengyel
 * @brief 
 * @version 0.1
 * @date 2024-04-26
 * 
 * @copyright Copyright (c) 2024
 */

#include "app_qrcode.h"

#include "../utils/theme/theme_define.h"

#include "driver/i2c.h"

using namespace MOONCAKE::APPS;

#define _keyboard _data.hal->keyboard()
#define _canvas _data.hal->canvas()
#define _canvas_update _data.hal->canvas_update
#define _canvas_clear() _canvas->fillScreen(THEME_COLOR_BG)



void AppQrcode::_update_input()
{
    // If changed 
    if (_keyboard->keyList().size() != _data.last_key_num)
    {
        // If key pressed 
        if (_keyboard->keyList().size() != 0)
        {
            // If enter 
            if (_keyboard->keysState().enter)
            {
               
            }
            // If delete 
            else if (_keyboard->keysState().del)
            {
                
            }
            // Normal chars 
            else 
            {
                for (auto& i : _keyboard->keysState().values)
                {
                    if(i == 'm')
                    {
                        _data.current_state = state_switch_trigger;
                    }
                    else if(i == 'f')
                    {
                        _data.current_state = state_get_fw_ver;
                    }

                }
            }
            // Update last key num 
            _data.last_key_num = _keyboard->keyList().size();
        }
        else 
        {
            // Reset last key num 
            _data.last_key_num = 0;
        }
    }
}


uint8_t AppQrcode::_get_decoder_ready()
{
    uint8_t rxBuf = 0;
        uint8_t temp[2] = {0};
        temp[0] = (UNIT_QRCODE_READY_REG & 0x00ff);
        temp[1] = ((UNIT_QRCODE_READY_REG >> 8) & 0x00ff);

        i2c_master_write_read_device(I2C_NUM_0, UNIT_QRCODE_ADDR, temp, 2, &rxBuf, 1, pdMS_TO_TICKS(1000));
    return rxBuf;
}
uint16_t AppQrcode::_get_decoder_length()
{

    uint16_t value = 0;
    uint8_t temp[2] = {0};
        temp[0] = (UNIT_QRCODE_LENGTH_REG & 0x00ff);
        temp[1] = ((UNIT_QRCODE_LENGTH_REG >> 8) & 0x00ff);

        i2c_master_write_read_device(I2C_NUM_0, UNIT_QRCODE_ADDR, temp, 2, (uint8_t *)&value, 2, pdMS_TO_TICKS(1000));
    return value;

}
void AppQrcode::_get_decoder_data(uint8_t *data, uint16_t len) {
    uint8_t temp[2] = {0};
        temp[0] = (UNIT_QRCODE_DATA_REG & 0x00ff);
        temp[1] = ((UNIT_QRCODE_DATA_REG >> 8) & 0x00ff);

        i2c_master_write_read_device(I2C_NUM_0, UNIT_QRCODE_ADDR, temp, 2, data,  len, pdMS_TO_TICKS(1000));
    
}
void AppQrcode::_init_i2c()
{

    i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = 2,
		.scl_io_num = 1,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
        .clk_flags = 0,
	};
    conf.master.clk_speed = 400000;
	i2c_param_config(I2C_NUM_0, &conf);

	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));



}

void AppQrcode::_update_state()
{
    if (_data.current_state == state_init)
    {
        _canvas->setTextColor(TFT_YELLOW, THEME_COLOR_BG);
        _canvas->printf("QRCode Scanner\r\nf = firmware version\r\nm = toggle auto scan\r\n");
        _canvas_update();
        
        _data.current_state = state_wait_input;
    }
    if(_data.current_state == state_switch_trigger)
    {

        if(_data.scan_mode)
        {
            _set_trigger_mode(0);
            _canvas->println("Manual Scan");
            _data.scan_mode = false;
        }
        else
        {
            _set_trigger_mode(1);
            _canvas->println("Auto Scan");
            _data.scan_mode = true;
        }

        _data.current_state = state_wait_input;
    }
    if(_data.current_state == state_get_fw_ver)
    {
        _display_fw_ver();
        _data.current_state = state_wait_input;
    }

}

void AppQrcode::_set_trigger_mode(uint8_t mode)
{

 uint8_t temp[3] = {0};
        temp[0] = (UNIT_QRCODE_TRIGGER_MODE_REG & 0x00ff);
        temp[1] = ((UNIT_QRCODE_TRIGGER_MODE_REG >> 8) & 0x00ff);
        temp[2] = mode;
        i2c_master_write_to_device(I2C_NUM_0, UNIT_QRCODE_ADDR, temp, 3, pdMS_TO_TICKS(1000));
  


}

void AppQrcode::_display_fw_ver()
{
   
        uint8_t rxBuf{};
        uint8_t temp[2] = {0};
        temp[0] = (FIRMWARE_VERSION_REG & 0x00ff);
        temp[1] = ((FIRMWARE_VERSION_REG >> 8) & 0x00ff);

        i2c_master_write_read_device(I2C_NUM_0, UNIT_QRCODE_ADDR, temp, 2, &rxBuf, 1, pdMS_TO_TICKS(1000));
        snprintf(_data.string_buffer, sizeof(_data.string_buffer), "SW Ver: %02x", rxBuf);
        _canvas->println(_data.string_buffer);

        _canvas_update();
    



}

void AppQrcode::onCreate()
{
    
    // Get hal
    _data.hal = mcAppGetDatabase()->Get("HAL")->value<HAL::Hal *>();
}

void AppQrcode::onResume()
{
    
    ANIM_APP_OPEN();

    _canvas_clear();
    _canvas->setTextScroll(true);
    _canvas->setBaseColor(THEME_COLOR_BG);
    _canvas->setTextColor(THEME_COLOR_REPL_TEXT, THEME_COLOR_BG);
    _canvas->setFont(FONT_REPL);
    _canvas->setTextSize(FONT_SIZE_REPL);

    // Avoid input panel 
    _canvas->setCursor(0, 0);
    

    
    _init_i2c();
    //run function once
    _set_trigger_mode(AUTO_SCAN_MODE);
}

void AppQrcode::onRunning()
{
    _update_input();
    _update_state();
    if (_get_decoder_ready() == 1) {
        uint8_t buffer[512] = {0};
        uint16_t length     = _get_decoder_length();

        _get_decoder_data(buffer, length);
        snprintf(_data.string_buffer, sizeof(_data.string_buffer), "DATA: %s", buffer);
        _canvas->println(_data.string_buffer);
        _canvas_update();


       
    }

    if (_data.hal->homeButton()->pressed())
    {
        _data.hal->playNextSound();
       
        destroyApp();
    }
}

void AppQrcode::onDestroy() {
    i2c_driver_delete(I2C_NUM_0);
    _canvas->setTextScroll(false);
}