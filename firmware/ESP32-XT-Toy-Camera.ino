// ESP32-XT-Toy-Camera
//
// LovyanGFX
// Camera OV2640
// LVGL
// SD-CARD

String file_list[1000];
int file_num = 0;

//-------------- LIBRARIES --------------
//---------------------------------------
#include "bgimage.c"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <esp_camera.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>

//------ ESP32-XT pin definition --------
//---------------------------------------
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

#define LCD_MOSI    SPI_MOSI
#define LCD_MISO    SPI_MISO
#define LCD_SCK     SPI_SCK
#define LCD_CS      15
#define LCD_RST     26
#define LCD_DC      33
#define LCD_BL      -1  

#define LCD_WIDTH   320
#define LCD_HEIGHT  480
#define LCD_SPI_HOST HSPI_HOST

// Camera pins
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    32
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

#define ARRAY_LENGTH 320 * 240 * 3
//#define ARRAY_LENGTH 352 * 288 * 3
#define SCREEN_ROTATION 1


//SPI control for TFT LCD
#define SPI_ON_TFT digitalWrite(LCD_CS, LOW)
#define SPI_OFF_TFT digitalWrite(LCD_CS, HIGH)

//SPI control for SD Card
#define SD_CS 4
#define SPI_ON_SD digitalWrite(SD_CS, LOW)
#define SPI_OFF_SD digitalWrite(SD_CS, HIGH)

boolean isShown = false;


//------------ LGFX STRUCTURE - SPI -------------
//-----------------------------------------------
struct LGFX_Config {
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_sclk = LCD_SCK;
    static constexpr int spi_mosi = LCD_MOSI;
    static constexpr int spi_miso = LCD_MISO;
};

// Create an LGFX_SPI instance with the configuration structure you just created as a template argument.
static lgfx::LGFX_SPI<LGFX_Config> tft;
//static lgfx::LGFX_PARALLEL<LGFX_Config> lcd;
static LGFX_Sprite sprite(&tft);
// Create an instance of the Panel class. Comment out the description of the panel you want to use.
static lgfx::Panel_ILI9488 panel;
// If you want to use a touch panel, create an instance of the Touch class.
static lgfx::Touch_XPT2046 touch;


#define LVGL_TICK_PERIOD 60

//Ticker tick; /* timer for interrupt handler */


//------------ LVGL IMAGES AND OBJECTS -------------
//--------------------------------------------------
lv_obj_t *sw1;
lv_obj_t *imgbtn2;
lv_obj_t *imgbtn;
lv_obj_t *statuslabel;

//LV_IMG_DECLARE(iconcamera);
LV_IMG_DECLARE(shutterofficon);
LV_IMG_DECLARE(backicon);
LV_IMG_DECLARE(playicon);
LV_IMG_DECLARE(cameraicon);
  
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

//lv_obj_t * slider_label;
int screenWidth = 480;
int screenHeight = 320;

const uint8_t img_rgb888_320_240_head[54] = {
    0x42, 0x4d, 0x36, 0x84, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x36, 0x0, 0x0, 0x0, 0x28, 0x0,
    0x0, 0x0, 0x40, 0x1, 0x0, 0x0, 0xf0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x18, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x84, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    
String imgname = "";
int img_index = 0;
int pos[2] = {0, 0};
int stream_flag = 1;

camera_fb_t *fb = NULL;


#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif

//------------ DISPLAY FLUSHING -------------
//-------------------------------------------
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

//------------ TOUCHPANEL FUNCTION -------------
//----------------------------------------------
bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    uint16_t touchX, touchY;

    //bool touched = tft.getTouch(&touchX, &touchY, 600);
bool touched = tft.getTouch(&touchX, &touchY);

    if(!touched) {
      return false;
    }

    if(touchX>screenWidth || touchY > screenHeight) {
      Serial.println("Y or y outside of expected parameters..");
      Serial.print("y:");
      Serial.print(touchX);
      Serial.print(" x:");
      Serial.print(touchY);
    }
    else {
      data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
  
      /*Save the state and save the pressed coordinate*/
      //if(data->state == LV_INDEV_STATE_PR) touchpad_get_xy(&last_x, &last_y);
     
      /*Set the coordinates (if released use the last pressed coordinates)*/
      data->point.x = touchX;
      data->point.y = touchY;
  
      Serial.print("Data x");
      Serial.println(touchX);
      
      Serial.print("Data y");
      Serial.println(touchY);

    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

//------ LIVE VIEW CAMERA FUNCTION  --------
//------------------------------------------
void readCamera() {
  //camera_fb_t *fb = NULL;
  fb = NULL;
  fb = esp_camera_fb_get();
  if (stream_flag == 1) {
      tft.pushImage(11, 50, fb->width, fb->height, (lgfx::swap565_t *)fb->buf);

  }
}

//------------ SD TEST ONLY -------------
//---------------------------------------
void sd_test() {
    //Read SD
    file_num = get_file_list(SD, "/", 0, file_list);
    Serial.print("File count:");
    Serial.println(file_num);
    Serial.println("All File:");
    for (int i = 0; i < file_num; i++) {
        Serial.println(file_list[i]);
    }

    Serial.println("SD Test!");
    //digitalWrite(SD_CS, LOW);
}

//------------ SD TEST ONLY -------------
//---------------------------------------
int get_file_list(fs::FS &fs, const char *dirname, uint8_t levels, String filelist[1000]) {
    Serial.printf("Listing directory: %s\n", dirname);
    int i = 0;

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return i;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return i;
    }

    File file = root.openNextFile();
    while (file) {
        if(i > 20) {
            //i--;
            //break;
        }
        if (file.isDirectory()) {
        }
        else {
            String temp = file.name();

            filelist[i] = temp;
            i++;
        }
        file = root.openNextFile();
    }
    return i;
}
