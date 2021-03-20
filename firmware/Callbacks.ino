//------------ PRINT / DEBUG -------------
//----------------------------------------
void printEvent(String Event, lv_event_t event) {
  
  Serial.print(Event);
  printf(" ");

  switch(event) {
      case LV_EVENT_PRESSED:
          printf("Pressed\n");
          break;

      case LV_EVENT_SHORT_CLICKED:
          printf("Short clicked\n");
          break;

      case LV_EVENT_CLICKED:
          printf("Clicked\n");
          break;

      case LV_EVENT_LONG_PRESSED:
          printf("Long press\n");
          break;

      case LV_EVENT_LONG_PRESSED_REPEAT:
          printf("Long press repeat\n");
          break;

      case LV_EVENT_RELEASED:
          printf("Released\n");
          break;
  }
}

//------------ VIEW LAST PHOTO FUNCTION -------------
//---------------------------------------------------
void btn_event_viewlastphoto(lv_obj_t* btn, lv_event_t event) {
  if(event == LV_EVENT_CLICKED){
    if ((stream_flag == 1) && (img_index > 0) ) {
            Serial.println("go to back");
            lv_switch_off(sw1, LV_ANIM_OFF);
            lv_obj_set_click(sw1, false);
            
            lv_obj_set_style_local_value_str(imgbtn2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "BACK");
              lv_imgbtn_set_src(imgbtn2, LV_BTN_STATE_RELEASED, &backicon);
              lv_imgbtn_set_src(imgbtn, LV_BTN_STATE_RELEASED, &shutterofficon);

            stream_flag = 0;
            isShown = true;
            char msg[50];
            sprintf(msg, "Status : Viewing %s ", imgname);
            lv_label_set_text(statuslabel, msg);
            //tft.fillRect(0, 0, 320, 240, TFT_BLACK);
            print_img(SD, imgname, 320, 240);
    } else {
      if (isShown) { // will return true if the swich is ON
        Serial.println("back to view");
        stream_flag = 1;
        lv_switch_on(sw1, LV_ANIM_ON);
        lv_obj_set_click(sw1, true);
        lv_obj_set_style_local_value_str(imgbtn2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "VIEW");
        lv_imgbtn_set_src(imgbtn2, LV_BTN_STATE_RELEASED, &playicon);
        lv_imgbtn_set_src(imgbtn, LV_BTN_STATE_RELEASED, &cameraicon);
        isShown = false;
        lv_label_set_text(statuslabel, "Status : Live View");
      }
    }

  }
}

//------------ TAKE PHOTO FUNCTION -------------
//----------------------------------------------
void btn_event_takephoto(lv_obj_t* btn, lv_event_t event){
  if(event == LV_EVENT_CLICKED){
    Serial.println("Clicked");

    if (stream_flag == 1) {
       void *ptrVal = NULL;
       ptrVal = heap_caps_malloc(ARRAY_LENGTH, MALLOC_CAP_SPIRAM);
       uint8_t *rgb = (uint8_t *)ptrVal;
       fmt2rgb888(fb->buf, fb->len, PIXFORMAT_RGB565, rgb);
       if (save_image(SD, rgb) == -1) {
       }
       else { 
          char msg[50];
          sprintf(msg, "Status : %s successfully saved", imgname);
          lv_label_set_text(statuslabel, msg);
       }
                   
       heap_caps_free(ptrVal);
       rgb = NULL;
    }
  }
}

//------------ SLIDER EVENT -------------
//---------------------------------------
void event_handler_sw1(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        printf("State: %s\n", lv_switch_get_state(obj) ? "On" : "Off");
        // if true = on, so start stream
        // else, turn off camera
        if (lv_switch_get_state(obj)) {
          stream_flag = 1;
          lv_label_set_text(statuslabel, "Status : Live View");

        }
        else {
          stream_flag = 0;
          tft.pushImage(11, 50, bgimageWidth, bgimageHeight, bgimage);
          lv_label_set_text(statuslabel, "Status : Camera Off");
        }

    }
}


//------------ SAVE IMAGE TO FILE -------------
//---------------------------------------------
int save_image(fs::FS &fs, uint8_t *rgb) {
    SPI_ON_SD;
    imgname = "/img" + String(img_index) + ".bmp";
    img_index++;

    Serial.println("Image name：" + imgname);

    File f = fs.open(imgname, "w");
    if (!f) {
        Serial.println("Failed to open file for writing");
        return -1;
    }

    f.write(img_rgb888_320_240_head, 54);
    f.write(rgb, 230400); // 3 x 320 x 240 

    f.close();

    Serial.println("write successfully");
    SPI_OFF_SD;
    return 0;
}


//------------ DISPLAY IMAGE -------------
//----------------------------------------
int print_img(fs::FS &fs, String filename, int x, int y) {
    SPI_ON_SD;
    //File f = fs.open(filename, "r");
    Serial.println("filename：" + filename);
    File f = fs.open(filename);
    if (!f) {
        Serial.println("Failed to open file for reading");
        f.close();
        return 0;
    }

    f.seek(54);
    int X = x;
    int Y = y;
    uint8_t RGB[3 * X];
    for (int row = 0; row < Y; row++) {
        f.seek(54 + 3 * X * row);
        f.read(RGB, 3 * X);
        SPI_OFF_SD;
        tft.pushImage(11, 50+row, X, 1, (lgfx::rgb888_t *)RGB);
        SPI_ON_SD;
    }

    f.close();
    SPI_OFF_SD;
    return 0;
}
