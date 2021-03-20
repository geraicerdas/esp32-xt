//------------ MAIN FUNCTION -------------
//----------------------------------------
void loop() {
  readCamera();
  lv_task_handler(); /* let the GUI do its work */
  esp_camera_fb_return(fb);
}
