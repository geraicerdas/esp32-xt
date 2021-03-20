//------------ SETUP FUNCTION -------------
//-----------------------------------------
void setup() {
  int SD_init_flag = 0;
  tft.setSwapBytes(true); // neeeded!!!! for displaying image flash

  Serial.begin(115200); /* prepare for possible serial debug */

  lv_init();

  #if USE_LV_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
  #endif

  pinMode(SD_CS, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  SPI_OFF_SD;
  SPI_OFF_TFT;
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  SPI_ON_SD;
  if (!SD.begin(SD_CS, SPI, 80000000)) {
     Serial.println("Card Mount Failed");
     SD_init_flag = 1;
  }
  else{
     Serial.println("Card Mount Successed");
     sd_test();
  }
  SPI_OFF_SD;

  Serial.println("SD init over.");

    
  SPI_ON_TFT;

  set_tft();
  tft.begin();
  // skip callibration at this moment
    
  tft.calibrateTouch(nullptr, 0xFFFFFFU, 0x000000U, 20);

  tft.clear();
    
  tft.setRotation(1);  /* Landscape orientation */
  tft.fillScreen(TFT_BLACK);
  SPI_OFF_TFT;
    
  //uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
  //tft.setTouch(calData);

  camera_init();

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the input device driver*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

  //Set the theme..
  lv_theme_t * th = lv_theme_material_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY, LV_THEME_DEFAULT_FLAG, LV_THEME_DEFAULT_FONT_SMALL , LV_THEME_DEFAULT_FONT_NORMAL, LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE);     
    
  lv_theme_set_act(th);


  lv_obj_t *bg_top = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_clean_style_list(bg_top, LV_OBJ_PART_MAIN);
  lv_obj_set_style_local_bg_opa(bg_top, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_bg_color(bg_top, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x8a8a8a));
  lv_obj_set_size(bg_top, LV_HOR_RES, LV_VER_RES/4);
    
  lv_obj_t * scr = lv_cont_create(lv_scr_act(), NULL);
  lv_obj_set_auto_realign(scr, true);                    /*Auto realign when the size changes*/
  lv_obj_align_origo(scr, NULL, LV_ALIGN_CENTER, -68, 9);  /*This parametrs will be sued when realigned*/
  lv_cont_set_fit2(scr, LV_FIT_TIGHT, LV_FIT_TIGHT);
  lv_obj_set_click(scr, false);

  /* Create simple label */
  lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
  //lv_label_set_style(label, LV_LABEL_STYLE_MAIN, LV_THEME_DEFAULT_FONT_TITLE);
  lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_TITLE);
  lv_label_set_text(label, "#ff0000 ESP32-XT# #ffffff camera#");
  lv_label_set_recolor(label, true);
  lv_obj_align(label, NULL, LV_ALIGN_CENTER, -70, -140);

  //lv_obj_t *statuslabel = lv_label_create(lv_scr_act(), NULL);
  statuslabel = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_font(statuslabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_THEME_DEFAULT_FONT_SMALL);
  lv_label_set_text(statuslabel, "Status : Live View");
  lv_obj_align(statuslabel, NULL, LV_ALIGN_IN_LEFT_MID, 10, 150);
  
  // create line
  static lv_point_t line_points[] = { {0, 0}, {310, 0}, {310, 233}, {0, 233}, {0, 0} };
  
  static lv_style_t style_line;
  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, LV_STATE_DEFAULT, 0); // 0 imaginary lines
    
  lv_obj_t *line1 = lv_line_create(scr, NULL);
  lv_line_set_points(line1, line_points, 5);
  lv_obj_add_style(line1, LV_LINE_PART_MAIN, &style_line);     /*Set the points*/
  lv_line_set_auto_size(line1, true);
  

  lv_obj_t * cont2 = lv_cont_create(lv_scr_act(), NULL);  
  lv_obj_set_auto_realign(cont2, true);                    /*Auto realign when the size changes*/
  lv_obj_align_origo(cont2, NULL, LV_ALIGN_CENTER, 180, 9); 
  lv_cont_set_fit2(cont2, LV_FIT_TIGHT, LV_FIT_TIGHT);

  lv_obj_set_click(cont2, false);


  static lv_point_t line2_points[] = { {79, 0}, {0, 0}, {0, 210}, {79, 210} };
  static lv_point_t line_separator1[] = { {0,40}, {76,40} };
  static lv_point_t line_separator2[] = { {0,150}, {76,150} };

  static lv_style_t style_line_separator;
  lv_style_init(&style_line_separator);
  lv_style_set_line_width(&style_line_separator, LV_STATE_DEFAULT, 2); // 0 imaginary lines
  lv_style_set_line_color(&style_line_separator, LV_STATE_DEFAULT, lv_color_hex(0xc4c4c4));
  lv_style_set_line_rounded(&style_line_separator, LV_STATE_DEFAULT, true);

  
  lv_obj_t *line2 = lv_line_create(cont2, NULL);
  lv_line_set_points(line2, line2_points, 4);
  lv_obj_add_style(line2, LV_LINE_PART_MAIN, &style_line);   
  lv_line_set_auto_size(line2, true);
  lv_obj_align(line1, NULL, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t *separator1 = lv_line_create(cont2, NULL);
  lv_line_set_points(separator1, line_separator1, 2);
  lv_obj_add_style(separator1, LV_LINE_PART_MAIN, &style_line_separator);
  lv_line_set_auto_size(separator1, true);
  lv_obj_align(separator1, NULL, LV_ALIGN_CENTER, 0, -70);

  lv_obj_t *separator2 = lv_line_create(cont2, NULL);
  lv_line_set_points(separator2, line_separator2, 2);
  lv_obj_add_style(separator2, LV_LINE_PART_MAIN, &style_line_separator);
  lv_line_set_auto_size(separator1, true);
  lv_obj_align(separator2, NULL, LV_ALIGN_CENTER, 0, -30);

  // switch on-off camera
  //Create a switch and apply the styles
  sw1 = lv_switch_create(cont2, NULL);
  lv_obj_align(sw1, NULL, LV_ALIGN_CENTER, 3, -70);
  lv_switch_on(sw1, LV_ANIM_ON);
  lv_obj_set_size(sw1, LV_HOR_RES / 9, LV_VER_RES / 12);
  lv_obj_set_style_local_value_ofs_y(sw1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, - LV_VER_RES/70);
  lv_obj_set_style_local_value_align(sw1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
  lv_obj_set_style_local_value_str(sw1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "CAMERA");
  lv_obj_set_style_local_value_font(sw1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_small());
  lv_obj_set_event_cb(sw1, event_handler_sw1);


  //lv_obj_t * imgbtn = lv_imgbtn_create(cont2, NULL);
  imgbtn = lv_imgbtn_create(cont2, NULL);
  lv_imgbtn_set_src(imgbtn, LV_BTN_STATE_RELEASED, &cameraicon);
  lv_imgbtn_set_src(imgbtn, LV_BTN_STATE_PRESSED, &shutterofficon);
  lv_obj_align(imgbtn, NULL, LV_ALIGN_CENTER, 8, 10);
  lv_obj_set_event_cb(imgbtn, btn_event_takephoto);


  lv_obj_set_style_local_value_ofs_y(imgbtn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, - LV_VER_RES/70);
  lv_obj_set_style_local_value_align(imgbtn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
  lv_obj_set_style_local_value_font(imgbtn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_small());
  lv_obj_set_style_local_value_str(imgbtn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "SHOOT");

  //Play
  //lv_obj_t * imgbtn2 = lv_imgbtn_create(cont2, NULL);
  imgbtn2 = lv_imgbtn_create(cont2, NULL);
  lv_imgbtn_set_src(imgbtn2, LV_BTN_STATE_RELEASED, &playicon);
  lv_imgbtn_set_src(imgbtn2, LV_BTN_STATE_PRESSED, &backicon);
  lv_obj_align(imgbtn2, NULL, LV_ALIGN_CENTER, 8, 110);
  lv_obj_set_event_cb(imgbtn2, btn_event_viewlastphoto);

  lv_obj_set_style_local_value_ofs_y(imgbtn2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, - LV_VER_RES/70);
  lv_obj_set_style_local_value_align(imgbtn2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_MID);
  lv_obj_set_style_local_value_str(imgbtn2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "VIEW");
  lv_obj_set_style_local_value_font(imgbtn2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_theme_get_font_small());


}

//------------ INIT LVGL -------------
//------------------------------------
void set_tft()
{
    // パネルクラスに各種設定値を代入していきます。
    // （LCD一体型製品のパネルクラスを選択した場合は、
    //   製品に合った初期値が設定されているので設定は不要です）

    // 通常動作時のSPIクロックを設定します。
    // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
    // 設定した値に一番近い設定可能な値が使用されます。
    panel.freq_write = 60000000;
    //panel.freq_write = 20000000;

    // 単色の塗り潰し処理時のSPIクロックを設定します。
    // 基本的にはfreq_writeと同じ値を設定しますが、
    // より高い値を設定しても動作する場合があります。
    panel.freq_fill = 60000000;
    //panel.freq_fill  = 27000000;

    // LCDから画素データを読取る際のSPIクロックを設定します。
    panel.freq_read = 16000000;

    // SPI通信モードを0~3から設定します。
    panel.spi_mode = 0;

    // データ読み取り時のSPI通信モードを0~3から設定します。
    panel.spi_mode_read = 0;

    // 画素読出し時のダミービット数を設定します。
    // 画素読出しでビットずれが起きる場合に調整してください。
    panel.len_dummy_read_pixel = 8;

    // データの読取りが可能なパネルの場合はtrueを、不可の場合はfalseを設定します。
    // 省略時はtrueになります。
    panel.spi_read = true;

    // データの読取りMOSIピンで行うパネルの場合はtrueを設定します。
    // 省略時はfalseになります。
    panel.spi_3wire = false;

    // LCDのCSを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.spi_cs = LCD_CS;

    // LCDのDCを接続したピン番号を設定します。
    panel.spi_dc = LCD_DC;

    // LCDのRSTを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.gpio_rst = LCD_RST;

    // LCDのバックライトを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.gpio_bl = LCD_BL;

    // バックライト使用時、輝度制御に使用するPWMチャンネル番号を設定します。
    // PWM輝度制御を使わない場合は省略するか-1を設定します。
    panel.pwm_ch_bl = -1;

    // バックライト点灯時の出力レベルがローかハイかを設定します。
    // 省略時は true。true=HIGHで点灯 / false=LOWで点灯になります。
    panel.backlight_level = true;

    // invertDisplayの初期値を設定します。trueを設定すると反転します。
    // 省略時は false。画面の色が反転している場合は設定を変更してください。
    panel.invert = true                                                                                                                                                                                                                             ;

    // パネルの色順がを設定します。  RGB=true / BGR=false
    // 省略時はfalse。赤と青が入れ替わっている場合は設定を変更してください。
    panel.rgb_order = false;

    // パネルのメモリが持っているピクセル数（幅と高さ）を設定します。
    // 設定が合っていない場合、setRotationを使用した際の座標がずれます。
    // （例：ST7735は 132x162 / 128x160 / 132x132 の３通りが存在します）
    panel.memory_width = LCD_WIDTH;
    panel.memory_height = LCD_HEIGHT;

    // パネルの実際のピクセル数（幅と高さ）を設定します。
    // 省略時はパネルクラスのデフォルト値が使用されます。
    panel.panel_width = LCD_WIDTH;
    panel.panel_height = LCD_HEIGHT;

    // パネルのオフセット量を設定します。
    // 省略時はパネルクラスのデフォルト値が使用されます。
    panel.offset_x = 0;
    panel.offset_y = 0;

    // setRotationの初期化直後の値を設定します。
    panel.rotation = 0;

    // setRotationを使用した時の向きを変更したい場合、offset_rotationを設定します。
    // setRotation(0)での向きを 1の時の向きにしたい場合、 1を設定します。
    panel.offset_rotation = 0;

  
    // 設定を終えたら、LGFXのsetPanel関数でパネルのポインタを渡します。
    tft.setPanel(&panel);
   // If you use a touch panel, you will assign various setting values to the touch class.
  // タッチパネルを使用する場合、タッチクラスに各種設定値を代入していきます。

  // for SPI setting.  (XPT2046 / STMPE610)
  // SPI接続のタッチパネルの場合
  touch.spi_host = VSPI_HOST;  // VSPI_HOST or HSPI_HOST
  touch.spi_sclk = SPI_SCK;
  touch.spi_mosi = SPI_MOSI;
  touch.spi_miso = SPI_MISO;
  touch.spi_cs   =  2;
  //touch.freq = 1000000;  
  touch.freq = 400000;
  touch.bus_shared = true;  // If the LCD and touch share SPI, set to true.

  // for I2C setting. (FT5x06)
  // I2C接続のタッチパネルの場合
  //touch.i2c_port = I2C_NUM_1;
  //touch.i2c_sda  = 21;
  //touch.i2c_scl  = 22;
  //touch.i2c_addr = 0x38;
  //touch.freq = 400000;

  // タッチパネルから得られるレンジを設定
  // (キャリブレーションを実施する場合は省略可)
  // (This can be omitted if calibration is performed.)
  touch.x_min = 0;
  touch.x_max = 319;
  touch.y_min = 0;
  touch.y_max = 479;

  // After setting up, you can pass the touch pointer to the lcd.setTouch function.
  // 設定を終えたら、lcdのsetTouch関数でタッチクラスのポインタを渡します。
  tft.setTouch(&touch);
  
}

//------------ CAMERA SETTING -------------
//-----------------------------------------
void camera_init() {
    //camera config
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


    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_QVGA; // 340x240
    //config.frame_size = FRAMESIZE_SVGA; // 800x600
    config.jpeg_quality = 10;
    config.fb_count = 1;

 // config.pixel_format = PIXFORMAT_RGB565;
  //init with high specs to pre-allocate larger buffers
 // if(psramFound()){
 //   Serial.println("Found PSRAM");
 //   config.frame_size = FRAMESIZE_SVGA;
 //   config.jpeg_quality = 10;
 //   config.fb_count = 2;
 // } else {
 //   Serial.println("PSRAM not found");
 //   config.frame_size = FRAMESIZE_SVGA;
  //  config.jpeg_quality = 12;
  //  config.fb_count = 1;
 // }
  
    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        while (1)
            ;
    }
  Serial.print("Camera Ready!");

    //sensor_t *s = esp_camera_sensor_get();
    //initial sensors are flipped vertically and colors are a bit saturated
    //if (s->id.PID == OV2640_PID)
   // {
    //    s->set_vflip(s, 1);      //flip it back
    //    s->set_brightness(s, 0); //up the blightness just a bit
     //   s->set_saturation(s, 1); //lower the saturation
   // }
    //drop down frame size for higher initial frame rate
    //s->set_framesize(s, FRAMESIZE_QVGA);
  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  //if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_hmirror(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    //s->set_saturation(s, -2);//lower the saturation
  //}
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);
  
//    show_log(2);
}
