#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "xwiimote.h"

#define MAX_BUTTON 9
#define WIDTH  1920
#define HEIGHT 1080

static struct xwii_iface *iface;

int sendData(int is_pointingDevice, unsigned char *data){
  if(is_pointingDevice){
    FILE *f_pointing;
    f_pointing = fopen("/dev/hidg1", "wb");
    fwrite(data, sizeof(unsigned char), 10, f_pointing);
    fclose(f_pointing);
  }else{
    FILE *f_keyboard;
    f_keyboard = fopen("/dev/hidg0", "wb");
    fwrite(data, sizeof(unsigned char), 8, f_keyboard);
    fclose(f_keyboard);
  }
  return 0;
}

int removeAndSlide(unsigned int *Array, unsigned int button){
  int removed = 0;
  if((Array[0] & button) == 0){
    return 1;
  }
  for(int i=1; i < MAX_BUTTON; i++){
    if(Array[i] == button){
      removed = 1;
    }
    if(removed){
      Array[i] = Array[i+1];
    }
  }
  Array[MAX_BUTTON] = 0;
  Array[0] = Array[0] & (~button);
  return 0;
}

int addButton(unsigned int *Array, unsigned int button){
  int counter = 0;
  for(int i=0; i<11; i++){
    if(Array[0] & (int)pow(2,i)){
      counter = counter + 1;
    }
  }
  if(8<=counter){
    return 1;
  }
  if((Array[0] & button) != 0){
    return 1;
  }
  Array[counter+1] = button;
  Array[0] = Array[0] | button;
  return 0;
}

static int wiimoteIR(const struct xwii_event *event, unsigned char *buf_pointing, int is_tabletMode, int *currentX, int *currentY, int lastX, int lastY){
  if (xwii_event_ir_is_valid(&event->v.abs[0])){
    if(is_tabletMode == 0){
      *currentX = (1024 - event->v.abs[0].x) * WIDTH / 1024;
      *currentY = (event->v.abs[0].y + 1) * HEIGHT / 768;
      if(*currentX >= lastX){
      buf_pointing[2] = (*currentX - lastX) % 0x100;
      buf_pointing[3] = (*currentX - lastX) / 0x100;
      }else{
      buf_pointing[2] = (0x7fff - ((*currentX - lastX) * -1)) % 0x100;
      buf_pointing[3] = (0x7fff - ((*currentX - lastX) * -1)) / 0x100;
      buf_pointing[3] = buf_pointing[3] | 0b10000000;
      }
      if(*currentY >= lastY){
      buf_pointing[4] = (*currentY - lastY) % 0x100;
      buf_pointing[5] = (*currentY - lastY) / 0x100;
      }else{
      buf_pointing[4] = (0x7fff - ((*currentY - lastY) * -1)) % 0x100;
      buf_pointing[5] = (0x7fff - ((*currentY - lastY) * -1)) / 0x100;
      buf_pointing[5] = buf_pointing[5] | 0b10000000;
      }
    }else if(is_tabletMode){
      *currentX = (1023 - event->v.abs[0].x) * WIDTH / 1024;
      *currentY = event->v.abs[0].y * HEIGHT / 768;
      buf_pointing[2] = *currentX % 0x100;
      buf_pointing[3] = *currentX / 0x100;
      buf_pointing[4] = *currentY % 0x100;
      buf_pointing[5] = *currentY / 0x100;

      buf_pointing[1] = buf_pointing[1] | 0x10;
    }
    return 1;
  }
  if(is_tabletMode == 0){
    buf_pointing[2] = 0;
    buf_pointing[3] = 0;
    buf_pointing[4] = 0;
    buf_pointing[5] = 0;
  }
  return 0;
}

static unsigned int wiimoteButton(const struct xwii_event *event, unsigned char *buf_pointing, unsigned int *pressedButtonsArray){
  unsigned int code = event->v.key.code;
  bool pressed = event->v.key.state;
  unsigned int button = 0;
  if (code == XWII_KEY_UP) {
    button = 0b1;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if (code == XWII_KEY_RIGHT) {
    button = 0b10;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if (code == XWII_KEY_DOWN) {
    button = 0b100;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if (code == XWII_KEY_LEFT) {
    button = 0b1000;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if(code == XWII_KEY_A){
    button = 0b10000;
    if(pressed){
      addButton(pressedButtonsArray, button);
      buf_pointing[1] = buf_pointing[1] | 0b00000001;
    }else{
      removeAndSlide(pressedButtonsArray, button);
      buf_pointing[1] = buf_pointing[1] & (~0b00000001);
    }
  }else if(code == XWII_KEY_B){
    button = 0b100000;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if(code == XWII_KEY_MINUS){
    button = 0b1000000;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if(code == XWII_KEY_HOME){
    button = 0b10000000;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if(code == XWII_KEY_PLUS){
    button = 0b100000000;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if(code == XWII_KEY_ONE){
    button = 0b1000000000;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }else if(code == XWII_KEY_TWO){
    button = 0b10000000000;
    if(pressed){
      addButton(pressedButtonsArray, button);
    }else{
      removeAndSlide(pressedButtonsArray, button);
    }
  }
  return button;
}

static char *get_dev(int num){
  struct xwii_monitor *mon;
  char *ent;
  int i = 0;

  mon = xwii_monitor_new(false, false);
  if (!mon) {
    printf("Cannot create monitor\n");
    return NULL;
  }

  while ((ent = xwii_monitor_poll(mon))) {
    if (++i == num)
      break;
    free(ent);
  }

  xwii_monitor_unref(mon);

  if (!ent)
    printf("Cannot find device with number #%d\n", num);

  return ent;
}

static void led_toggle(int n, bool *led_state)
{
  int ret;

  led_state[n] = !led_state[n];
  ret = xwii_iface_set_led(iface, XWII_LED(n+1), led_state[n]);
  if (ret) {
    printf("Error: Cannot toggle LED %i: %d", n+1, ret);
    led_state[n] = !led_state[n];
  }
}

static int battery_refresh(void)
{
	int ret;
	uint8_t capacity;

	ret = xwii_iface_get_battery(iface, &capacity);
	if (ret){
		printf("Error: Cannot read battery capacity");
    return -1;
  }
	
  return capacity;
}

int main(){
  struct xwii_event event;
  bool led_state[4] = {true, true, true, true};
  int ret = 0;
  int is_IR_valid = 0;
  int is_TabletMode = 0;
  int is_SwitchingMode = 0;
  unsigned int pressedButtonsArray[MAX_BUTTON + 1] = {0};
  unsigned int lastPressedButtons;
  char *path;
  unsigned char buf_keyboard[8] = {0};
  unsigned char buf_pointing[10] = {0};
  buf_pointing[0] =0x01;
  unsigned char empty_keyboard[8] = {0};
/* mouse
  buf_pointing[0] = 0x01; //0000 0001
  buf_pointing[1] = 0x00; //0000 0000
  buf_pointing[2] = 0x00; //move X 0-255
  buf_pointing[3] = 0x00; //move X 256*n
  buf_pointing[4] = 0x00; //move Y 0-255
  buf_pointing[5] = 0x00; //move Y 256*n
  buf_pointing[6] = 0x00; //scroll 0-255
  buf_pointing[7] = 0x00; //scroll 256*n
  buf_pointing[8] = 0x00; //scroll 0-255
  buf_pointing[9] = 0x00; //scroll 256*n
*/
/* tablet
  buf_pointing[0] = 0x02; //0000 0002
  buf_pointing[1] = 0x10; //0001 0000
  buf_pointing[2] = 0x00; //X 0-255
  buf_pointing[3] = 0x00; //X 256*n
  buf_pointing[4] = 0x00; //Y 0-255
  buf_pointing[5] = 0x00; //Y 256*n
  buf_pointing[6] = 0x00; //padding
  buf_pointing[7] = 0x00; //padding
  buf_pointing[8] = 0x00; //padding
  buf_pointing[9] = 0x00; //padding
*/
  time_t lastDetectedTime = 0;
  time_t lastTime = 0;
  time_t currentTime = 0;
  int currentX=0,currentY=0;
  int lastX=0,lastY=0;
  path = get_dev(atoi("1"));
  xwii_iface_new(&iface, path ? path : "1");
  ret = xwii_iface_open(iface, xwii_iface_available(iface) | XWII_IFACE_WRITABLE);
  if (ret){
    printf("Error: Cannot open interface: %d\n", ret);
    return 1;
  }

  while(true){
    currentTime = time(NULL);
    ret = xwii_iface_dispatch(iface, &event, sizeof(event));

    //wiimote
    switch (event.type) {
    case XWII_EVENT_KEY:
      wiimoteButton(&event, buf_pointing, pressedButtonsArray);
      if( (lastPressedButtons ^ pressedButtonsArray[0]) & (0b10000000 & pressedButtonsArray[0]) ){
        is_SwitchingMode = 1 - is_SwitchingMode;
      }
      break;
    case XWII_EVENT_IR:
      is_IR_valid = wiimoteIR(&event, buf_pointing, is_TabletMode, &currentX, &currentY, lastX, lastY);
      if(is_IR_valid == 1){
        lastDetectedTime = time(NULL);
      }
      sendData(1, buf_pointing);
      lastX = currentX;
      lastY = currentY;
      break;
    }
    if((is_TabletMode == 1) && ((currentTime - lastDetectedTime) < 5)){
      sendData(1, buf_pointing);
    }

    //Key
    if(pressedButtonsArray[0] != 0 && is_SwitchingMode == 0){
      int counter = 0;
      for(int i=1; i <= MAX_BUTTON; i++){
        if(6 <= counter){
          break;
        }
        switch (pressedButtonsArray[i]){
        case 0b1:
          buf_keyboard[counter+2] = 0x52; //up
          counter = counter + 1;
          break;
        case 0b10:
          buf_keyboard[counter+2] = 0x4F; //right
          counter = counter + 1;
          break;
        case 0b100:
          buf_keyboard[counter+2] = 0x51; //down
          counter = counter + 1;
          break;
        case 0b1000:
          buf_keyboard[counter+2] = 0x50; //left
          counter = counter + 1;
          break;
        case 0b100000:
          buf_keyboard[counter+2] = 0x2A; //Backspace
          counter = counter + 1;
          break;
        case 0b1000000:
          buf_keyboard[counter+2] = 0x81; //-
          counter = counter + 1;
          break;
        case 0b100000000:
          buf_keyboard[counter+2] = 0x80; //+
          counter = counter + 1;
          break;
        case 0b1000000000:
          buf_keyboard[counter+2] = 0x59; //1
          counter = counter + 1;
          break;
        case 0b10000000000:
          buf_keyboard[counter+2] = 0x5A; //2
          counter = counter + 1;
          break;
        }
      }
      for(counter; counter<6; counter++){
        buf_keyboard[counter+2] = 0x00;
      }
      sendData(0, buf_keyboard);
    }else if(is_SwitchingMode){
      if(pressedButtonsArray[0] & 0b10){
        is_TabletMode = 1;
        buf_pointing[0] = 0x02;
        buf_pointing[1] = 0x10;
        buf_pointing[6] = 0;
        buf_pointing[7] = 0;
        buf_pointing[8] = 0;
        buf_pointing[9] = 0;
      }else if(pressedButtonsArray[0] & 0b1000){
        is_TabletMode = 0;
        buf_pointing[0] = 0x01;
        buf_pointing[1] = 0;
      }
      sendData(0, empty_keyboard);
    }else{
      sendData(0, empty_keyboard);
    }

    //LED
    if(currentTime != lastTime){
      if(is_SwitchingMode){
        led_state[1] = true;
        led_state[3] = !led_state[3];
        if(is_TabletMode){
          led_state[0] = false;
          led_state[2] = true;
        }else{
          led_state[0] = true;
          led_state[2] = false;
        }
      }else{
        int battery = battery_refresh();
        if(75 < battery){
          led_state[0] = true;
          led_state[1] = true;
          led_state[2] = true;
          led_state[3] = true;
        }else if(50 < battery){
          led_state[0] = true;
          led_state[1] = true;
          led_state[2] = true;
          led_state[3] = false;
        }else if(25 < battery){
          led_state[0] = true;
          led_state[1] = true;
          led_state[2] = false;
          led_state[3] = false;
        }else if(10 < battery){
          led_state[0] = true;
          led_state[1] = false;
          led_state[2] = false;
          led_state[3] = false;
        }else{
          led_state[0] = !led_state[0];
          led_state[1] = false;
          led_state[2] = false;
          led_state[3] = false;
        }
      }
      for(int n=0; n<4; n++){
        int ret;
        ret = xwii_iface_set_led(iface, XWII_LED(n+1), led_state[n]);
        if (ret) {
          printf("Error: Cannot toggle LED %i: %d", n+1, ret);
        }
      }
    }
    lastPressedButtons = pressedButtonsArray[0];
    lastTime = currentTime;
  }

  return 0;
}
