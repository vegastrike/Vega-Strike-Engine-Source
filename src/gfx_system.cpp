void System:: SelectCamera(int cam) {
  if(cam<NUM_CAM&&cam>=0)
    currentcamera = cam;
}	

Camera *System::AccessCamera(int num) {
  if(num<NUM_CAM&&num>=0)
    return &cam[num];
  else
    return NULL;
}	

