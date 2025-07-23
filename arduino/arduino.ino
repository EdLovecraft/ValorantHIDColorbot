#include <Mouse.h>
#include <Wire.h>
#include <SPI.h>
#include <HID-Project.h>
uint8_t rawhidData[RAWHID_SIZE];


#include <usbhub.h>
USB Usb;
USBHub Hub(&Usb);

#include <hidboot.h>
HIDBoot<USB_HID_PROTOCOL_MOUSE> HidMouse(&Usb);

int lmb = 0;
int rmb = 0;
int mmb = 0;
int scroll = 0;
int xb1 = 0;
int xb2 = 0;

int dx;
int dy;
int dz;

// ----- Mouse Report Parser
class MouseRptParser : public MouseReportParser {
protected:
  void OnMouseMove(MOUSEINFO *mi);
  void OnMouseScroll(MOUSEINFO *mi);
  void OnLeftButtonUp(MOUSEINFO *mi);
  void OnLeftButtonDown(MOUSEINFO *mi);
  void OnRightButtonUp(MOUSEINFO *mi);
  void OnRightButtonDown(MOUSEINFO *mi);
  void OnMiddleButtonUp(MOUSEINFO *mi);
  void OnMiddleButtonDown(MOUSEINFO *mi);
  void OnXButton1Up (MOUSEINFO *mi);
  void OnXButton1Down (MOUSEINFO *mi);
  void OnXButton2Up (MOUSEINFO *mi);
  void OnXButton2Down (MOUSEINFO *mi);
};

void MouseRptParser::OnMouseMove(MOUSEINFO *mi) {
  dx = mi->dX;
  dy = mi->dY;
};

void MouseRptParser::OnMouseScroll(MOUSEINFO *mi) {
  dz = mi->dZ;
};

void MouseRptParser::OnLeftButtonUp(MOUSEINFO *mi) {
  lmb = 0;
};

void MouseRptParser::OnLeftButtonDown(MOUSEINFO *mi) {
  lmb = 1;
};

void MouseRptParser::OnRightButtonUp(MOUSEINFO *mi) {
  rmb = 0;
};

void MouseRptParser::OnRightButtonDown(MOUSEINFO *mi) {
  rmb = 1;
};

void MouseRptParser::OnMiddleButtonUp(MOUSEINFO *mi) {
  mmb = 0;
};

void MouseRptParser::OnMiddleButtonDown(MOUSEINFO *mi) {
  mmb = 1;
};

void MouseRptParser::OnXButton1Up (MOUSEINFO *mi) {
  xb1 = 0;
};

void MouseRptParser::OnXButton1Down (MOUSEINFO *mi) {
  xb1 = 1;
};

void MouseRptParser::OnXButton2Up (MOUSEINFO *mi) {
  xb2 = 0;
};

void MouseRptParser::OnXButton2Down (MOUSEINFO *mi) {
  xb2 = 1;
};

MouseRptParser Prs;

void setup() {
  delay(5000);
  Mouse.begin();  // Initialize mouse control

  Usb.Init();
  HidMouse.SetReportParser(0, &Prs);

  RawHID.begin(rawhidData, sizeof(rawhidData));
}

void loop() {
  dx = 0;
  dy = 0;
  dz = 0;

  Usb.Task();

  if (RawHID.available()) { // check if there's data
    int bytesRead = RawHID.read(); // read incoming data
    uint8_t buttons = rawhidData[0];
    int8_t intended_x = rawhidData[1];
    int8_t intended_y = rawhidData[2];
    if (buttons == 1) {
      Mouse.press(MOUSE_LEFT);
      Mouse.release(MOUSE_LEFT);
    }
    Mouse.move(intended_x, intended_y, 0);
    RawHID.enable(); // flush the data from the rawhidData for the next hid reading
  }

  // Clicking
  if (lmb == 0) {
    Mouse.release(MOUSE_LEFT);
  } else if (lmb == 1) {
    Mouse.press(MOUSE_LEFT);
  }

  if (rmb == 0) {
    Mouse.release(MOUSE_RIGHT);
  } else if (rmb == 1) {
    Mouse.press(MOUSE_RIGHT);
  }

  if (mmb == 0) {
    Mouse.release(MOUSE_MIDDLE);
  } else if (mmb == 1) {
    Mouse.press(MOUSE_MIDDLE);
  }

  if (xb1 == 0) {
    Mouse.release(MOUSE_XB1);
  } else if (xb1 == 1) {
    Mouse.press(MOUSE_XB1);
  }

  if (xb2 == 0) {
    Mouse.release(MOUSE_XB2);
  } else if (xb2 == 1) {
    Mouse.press(MOUSE_XB2);
  }

  Mouse.move(dx, dy, dz); // Moving the mouse
}
