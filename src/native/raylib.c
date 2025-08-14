#include "common_native.h"

#include <raylib.h>

static ObjKlass *colorKlassRef;
static ObjKlass *vector2KlassRef;
static ObjKlass *imgKlassRef;
static ObjKlass *camera2dRef;
static ObjKlass *vector3KlassRef;
static ObjKlass *camera3dRef;
static ObjKlass *soundKlassRef;
static ObjKlass *waveKlassRef;
static ObjKlass *musicKlassRef;
static ObjKlass *audioStreamKlassRef;

static Value createVector2(double x, double y) {
  ObjInstance *vector2 = newInstance(vector2KlassRef);
  push(OBJ_VAL(vector2));   
  setNativeInstanceField(vector2, vm.string.x, NUMBER_VAL(x));
  setNativeInstanceField(vector2, vm.string.y, NUMBER_VAL(y));
  return pop();
}


static Value createImage(Image img) {
  ObjInstance *imgInst = newInstance(imgKlassRef); 
  push(OBJ_VAL(imgInst));
  defineNativeInstanceField(imgInst, "*data", 4, OBJ_VAL(img.data));
  defineNativeInstanceField(imgInst, "width", 5, NUMBER_VAL(img.width));
  defineNativeInstanceField(imgInst, "height", 6, NUMBER_VAL(img.height));
  defineNativeInstanceField(imgInst, "mipmaps", 7, NUMBER_VAL(img.mipmaps));
  defineNativeInstanceField(imgInst, "format", 6, NUMBER_VAL(img.format));
  return pop();
}

// window-releated natives
static Value initWindowRLNative(int argCount, Value *args) {	
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_STRING)) {
    return NIL_VAL;
  }
  double width = AS_NUMBER(args[1]);
  double height = AS_NUMBER(args[2]);
  const char *title = AS_CSTRING(args[3]);
  InitWindow((int)width, (int)height, title);
  return NIL_VAL;
}

static Value closeWindowRLNative(int argCount, Value *args) {	
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  CloseWindow();
  return NIL_VAL;
}

static Value windowShouldCloseRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(WindowShouldClose());
}

static Value isWindowReadyRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsWindowReady());
}

static Value isWindowFullscreenRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsWindowFullscreen());
}

static Value isWindowHiddenRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsWindowHidden());
}

static Value isWindowMinimizedRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsWindowMinimized());
}

static Value isWindowMaximizedRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsWindowMaximized());
}

static Value isWindowFocusedRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsWindowFocused());
}

static Value isWindowResizedRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsWindowResized());
}

static Value isWindowStateRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsWindowState(AS_NUMBER(args[1])));
}

static Value setWindowStateRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetWindowState(AS_NUMBER(args[1]));
  return NIL_VAL;
}

static Value clearWindowStateRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ClearWindowState(AS_NUMBER(args[1]));
  return NIL_VAL;
}

static Value toggleFullscreenRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  ToggleFullscreen();
  return NIL_VAL;
}


static Value toggleBorderlessWindowedRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  ToggleBorderlessWindowed();
  return NIL_VAL;
}

static Value maximizeWindowRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  MaximizeWindow();
  return NIL_VAL;
}

static Value minimizeWindowRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  MinimizeWindow();
  return NIL_VAL;
}

static Value restoreWindowRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  RestoreWindow();
  return NIL_VAL;
}

static Value setWindowIconRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *imgInst = AS_INSTANCE(args[1]);
  Image img = {
    (void *)AS_OBJ(readNativeInstanceField(imgInst, "*data", 4)),
    AS_NUMBER(readNativeInstanceField(imgInst, "width", 5)),
    AS_NUMBER(readNativeInstanceField(imgInst, "height", 6)),
    AS_NUMBER(readNativeInstanceField(imgInst, "mipmaps", 7)),
    AS_NUMBER(readNativeInstanceField(imgInst, "format", 6)),
  };
  SetWindowIcon(img);
  return NIL_VAL;
}

static Value setWindowTitleRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  }
  SetWindowTitle(AS_CSTRING(args[1]));
  return NIL_VAL;
}

static Value setWindowPositionRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetWindowPosition(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setWindowMonitorRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetWindowMonitor(AS_NUMBER(args[1]));
  return NIL_VAL;
}

static Value setWindowMinSizeRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetWindowMinSize(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setWindowMaxSizeRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetWindowMaxSize(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setWindowSizeRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetWindowSize(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setWindowOpacityRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetWindowOpacity(AS_NUMBER(args[1]));
  return NIL_VAL;
}

static Value setWindowFocusedRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  SetWindowFocused();
  return NIL_VAL;
}

static Value getWindowHandleRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  // Return handle as a number (pointer converted to number)
  void *handle = GetWindowHandle();
  return NUMBER_VAL((long)(uintptr_t)handle);
}

static Value getScreenWidthRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetScreenWidth());
}

static Value getScreenHeightRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetScreenHeight());
}

static Value getRenderWidthRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetRenderWidth());
}

static Value getRenderHeightRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetRenderHeight());
}

static Value getMonitorCountRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMonitorCount());
}

static Value getCurrentMonitorRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetCurrentMonitor());
}

static Value getMonitorPositionRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  Vector2 pos = GetMonitorPosition(AS_NUMBER(args[1]));
  return createVector2(pos.x, pos.y);
}

static Value getMonitorWidthRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMonitorWidth(AS_NUMBER(args[1])));
}

static Value getMonitorHeightRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMonitorHeight(AS_NUMBER(args[1])));
}

static Value getMonitorPhysicalWidthRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMonitorPhysicalWidth(AS_NUMBER(args[1])));
}

static Value getMonitorPhysicalHeightRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMonitorPhysicalHeight(AS_NUMBER(args[1])));
}

static Value getMonitorRefreshRateRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMonitorRefreshRate(AS_NUMBER(args[1])));
}

static Value getWindowPositionRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  Vector2 pos = GetWindowPosition();
  return createVector2(pos.x, pos.y);
}

static Value getWindowScaleDpiRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  Vector2 pos = GetWindowScaleDPI();
  return createVector2(pos.x, pos.y);
}

static Value getMonitorNameRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }  
  const char *name = GetMonitorName(AS_NUMBER(args[1]));
  return OBJ_VAL(copyString(name, strlen(name), &vm.strings));
}

static Value setClipboardTextRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  }  
  SetClipboardText(AS_CSTRING(args[1]));
  return NIL_VAL;
}

static Value getClipboardTextRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }  
  const char *text = GetClipboardText();
  return OBJ_VAL(copyString(text, strlen(text), &vm.strings));
}


static Value enableEventWaitingRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  EnableEventWaiting();
  return NIL_VAL;
}

static Value disableEventWaitingRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  DisableEventWaiting();
  return NIL_VAL;
}

// Additional window functions  
static Value takeScreenshotRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  }
  TakeScreenshot(AS_CSTRING(args[1]));
  return NIL_VAL;
}

static Value setConfigFlagsRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetConfigFlags(AS_NUMBER(args[1]));
  return NIL_VAL;
}

static Value setExitKeyRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetExitKey(AS_NUMBER(args[1]));
  return NIL_VAL;
}

// cursor-related natives
static Value showCursorRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  ShowCursor();
  return NIL_VAL;
}

static Value hideCursorRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  HideCursor();
  return NIL_VAL;
}

static Value isCursorHiddenRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsCursorHidden());
}

static Value enableCursorRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  EnableCursor();
  return NIL_VAL;
}

static Value disableCursorRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  DisableCursor();
  return NIL_VAL;
}

static Value isCursorOnScreenRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsCursorOnScreen());
}

// drawing-related natives
static Value clearBackgroundRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  };
  ObjInstance *color = AS_INSTANCE(args[1]);
  Value r;
  Value g;
  Value b;
  Value a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);  
  tableGet(&color->fields, vm.string.a, &a);

  ClearBackground((Color){
    AS_NUMBER(r), 
    AS_NUMBER(g),
    AS_NUMBER(b),
    AS_NUMBER(a)
  });

  return NIL_VAL;
}

static Value beginDrawingRLNative(int argCount, Value *args) {	
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  };
  BeginDrawing();
  return NIL_VAL;
}

static Value endDrawingRLNative(int argCount, Value *args) {	
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  };
  EndDrawing();
  return NIL_VAL;
}

static Value beginMode2dRLNative(int argCount, Value *args) {  
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  };
  ObjInstance *cam = AS_INSTANCE(args[1]);
  ObjInstance *offset = AS_INSTANCE(readNativeInstanceField(cam, "offset", 6));
  Value offsetx;
  Value offsety;
  tableGet(&offset->fields, vm.string.x, &offsetx);
  tableGet(&offset->fields, vm.string.y, &offsety);  
  ObjInstance *target = AS_INSTANCE(readNativeInstanceField(cam, "target", 6));
  Value targetx;
  Value targety;
  tableGet(&target->fields, vm.string.x, &targetx);
  tableGet(&target->fields, vm.string.y, &targety);  

  Camera2D cam2d = {
    (Vector2){AS_NUMBER(offsetx), AS_NUMBER(offsety)},
    (Vector2){AS_NUMBER(targetx), AS_NUMBER(targety)},
    AS_NUMBER(readNativeInstanceField(cam, "rotation", 8)),
    AS_NUMBER(readNativeInstanceField(cam, "zoom", 4)),
  };

  BeginMode2D(cam2d);
  return NIL_VAL;
}

static Value endMode2dRLNative(int argCount, Value *args) {  
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  };
  EndMode2D();
  return NIL_VAL;
}

static Value beginMode3dRLNative(int argCount, Value *args) {  
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  };
  
  ObjInstance *cam = AS_INSTANCE(args[1]);
  if (cam == NULL) {
    runtimeError("Camera3D object is null");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Get position field
  Value positionValue = readNativeInstanceField(cam, "position", 8);
  if (!IS_INSTANCE(positionValue)) {
    runtimeError("Camera3D position field is not a Vector3 instance");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjInstance *position = AS_INSTANCE(positionValue);
  
  Value positionx, positiony, positionz;
  if (!tableGet(&position->fields, vm.string.x, &positionx) || !IS_NUMBER(positionx) ||
      !tableGet(&position->fields, vm.string.y, &positiony) || !IS_NUMBER(positiony) ||
      !tableGet(&position->fields, vm.string.z, &positionz) || !IS_NUMBER(positionz)) {
    runtimeError("Camera3D position Vector3 has invalid x, y, or z fields");
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  // Get target field
  Value targetValue = readNativeInstanceField(cam, "target", 6);
  if (!IS_INSTANCE(targetValue)) {
    runtimeError("Camera3D target field is not a Vector3 instance");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjInstance *target = AS_INSTANCE(targetValue);
  
  Value targetx, targety, targetz;
  if (!tableGet(&target->fields, vm.string.x, &targetx) || !IS_NUMBER(targetx) ||
      !tableGet(&target->fields, vm.string.y, &targety) || !IS_NUMBER(targety) ||
      !tableGet(&target->fields, vm.string.z, &targetz) || !IS_NUMBER(targetz)) {
    runtimeError("Camera3D target Vector3 has invalid x, y, or z fields");
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  // Get up field
  Value upValue = readNativeInstanceField(cam, "up", 2);
  if (!IS_INSTANCE(upValue)) {
    runtimeError("Camera3D up field is not a Vector3 instance");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  ObjInstance *up = AS_INSTANCE(upValue);
  
  Value upx, upy, upz;
  if (!tableGet(&up->fields, vm.string.x, &upx) || !IS_NUMBER(upx) ||
      !tableGet(&up->fields, vm.string.y, &upy) || !IS_NUMBER(upy) ||
      !tableGet(&up->fields, vm.string.z, &upz) || !IS_NUMBER(upz)) {
    runtimeError("Camera3D up Vector3 has invalid x, y, or z fields");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Get fovy and projection
  Value fovyValue = readNativeInstanceField(cam, "fovy", 4);
  Value projectionValue = readNativeInstanceField(cam, "projection", 10);
  
  if (!IS_NUMBER(fovyValue) || !IS_NUMBER(projectionValue)) {
    runtimeError("Camera3D fovy or projection field is not a number");
    vm.shouldPanic = true;
    return NIL_VAL;
  }

  Camera3D cam3d = {
    (Vector3){AS_NUMBER(positionx), AS_NUMBER(positiony), AS_NUMBER(positionz)},
    (Vector3){AS_NUMBER(targetx), AS_NUMBER(targety), AS_NUMBER(targetz)},
    (Vector3){AS_NUMBER(upx), AS_NUMBER(upy), AS_NUMBER(upz)},
    AS_NUMBER(fovyValue),
    AS_NUMBER(projectionValue),
  };

  BeginMode3D(cam3d);
  return NIL_VAL;
}

static Value endMode3dRLNative(int argCount, Value *args) {  
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  };
  EndMode3D();
  return NIL_VAL;
}

static Value setTargetFpsRLNative(int argCount, Value *args) {  
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  };
  SetTargetFPS(AS_NUMBER(args[1]));
  return NIL_VAL;
}

static Value getFrameTimeRLNative(int argCount, Value *args) {  
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(GetFrameTime());
}

static Value getTimeRLNative(int argCount, Value *args) {  
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(GetTime());
}

static Value getFpsRLNative(int argCount, Value *args) {  
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  };
  return NUMBER_VAL(GetFPS());
}

static Value loadImageRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  };
  return createImage(LoadImage(AS_CSTRING(args[1])));
}

// input-related natives
static Value isKeyPressedRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsKeyPressed(AS_NUMBER(args[1])));
}

static Value isKeyPressedRepeatRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsKeyPressedRepeat(AS_NUMBER(args[1])));
}

static Value isKeyDownRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsKeyDown(AS_NUMBER(args[1])));
}

static Value isKeyReleasedRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsKeyReleased(AS_NUMBER(args[1])));
}

static Value isKeyUpRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsKeyUp(AS_NUMBER(args[1])));
}

static Value getKeyPressedRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetKeyPressed());
}

static Value getCharPressedRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetCharPressed());
}

// mouse-related natives
static Value isMouseButtonPressedRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsMouseButtonPressed(AS_NUMBER(args[1])));
}

static Value isMouseButtonDownRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsMouseButtonDown(AS_NUMBER(args[1])));
}

static Value isMouseButtonReleasedRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsMouseButtonReleased(AS_NUMBER(args[1])));
}

static Value isMouseButtonUpRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsMouseButtonUp(AS_NUMBER(args[1])));
}

static Value getMouseXRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMouseX());
}

static Value getMouseYRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMouseY());
}

static Value getMousePositionRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  Vector2 pos = GetMousePosition();
  return createVector2(pos.x, pos.y);
}

static Value getMouseDeltaRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  Vector2 delta = GetMouseDelta();
  return createVector2(delta.x, delta.y);
}

static Value setMousePositionRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetMousePosition(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setMouseOffsetRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetMouseOffset(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setMouseScaleRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetMouseScale(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value getMouseWheelMoveRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMouseWheelMove());
}

static Value getMouseWheelMoveVRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  Vector2 move = GetMouseWheelMoveV();
  return createVector2(move.x, move.y);
}

static Value setMouseCursorRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetMouseCursor(AS_NUMBER(args[1]));
  return NIL_VAL;
}

// touch-related natives
static Value getTouchXRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetTouchX());
}

static Value getTouchYRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetTouchY());
}

static Value getTouchPositionRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  Vector2 pos = GetTouchPosition(AS_NUMBER(args[1]));
  return createVector2(pos.x, pos.y);
}

static Value getTouchPointCountRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetTouchPointCount());
}

// drawing primitives
static Value drawPixelRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *color = AS_INSTANCE(args[3]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawPixel(AS_NUMBER(args[1]), AS_NUMBER(args[2]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawPixelVRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *pos = AS_INSTANCE(args[1]);
  Value x, y;
  tableGet(&pos->fields, vm.string.x, &x);
  tableGet(&pos->fields, vm.string.y, &y);
  
  ObjInstance *color = AS_INSTANCE(args[2]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawPixelV((Vector2){AS_NUMBER(x), AS_NUMBER(y)}, (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawLineRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *color = AS_INSTANCE(args[5]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawLine(AS_NUMBER(args[1]), AS_NUMBER(args[2]), AS_NUMBER(args[3]), AS_NUMBER(args[4]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawLineVRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *start = AS_INSTANCE(args[1]);
  Value startX, startY;
  tableGet(&start->fields, vm.string.x, &startX);
  tableGet(&start->fields, vm.string.y, &startY);
  
  ObjInstance *end = AS_INSTANCE(args[2]);
  Value endX, endY;
  tableGet(&end->fields, vm.string.x, &endX);
  tableGet(&end->fields, vm.string.y, &endY);
  
  ObjInstance *color = AS_INSTANCE(args[3]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawLineV((Vector2){AS_NUMBER(startX), AS_NUMBER(startY)}, (Vector2){AS_NUMBER(endX), AS_NUMBER(endY)}, (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawLineExRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 5, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *start = AS_INSTANCE(args[1]);
  Value startX, startY;
  tableGet(&start->fields, vm.string.x, &startX);
  tableGet(&start->fields, vm.string.y, &startY);
  
  ObjInstance *end = AS_INSTANCE(args[2]);
  Value endX, endY;
  tableGet(&end->fields, vm.string.x, &endX);
  tableGet(&end->fields, vm.string.y, &endY);
  
  ObjInstance *color = AS_INSTANCE(args[4]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawLineEx((Vector2){AS_NUMBER(startX), AS_NUMBER(startY)}, (Vector2){AS_NUMBER(endX), AS_NUMBER(endY)}, AS_NUMBER(args[3]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawLine3DRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  
  // Get start position Vector3
  ObjInstance *startPos = AS_INSTANCE(args[1]);
  Value startX, startY, startZ;
  tableGet(&startPos->fields, vm.string.x, &startX);
  tableGet(&startPos->fields, vm.string.y, &startY);
  tableGet(&startPos->fields, vm.string.z, &startZ);
  
  // Get end position Vector3
  ObjInstance *endPos = AS_INSTANCE(args[2]);
  Value endX, endY, endZ;
  tableGet(&endPos->fields, vm.string.x, &endX);
  tableGet(&endPos->fields, vm.string.y, &endY);
  tableGet(&endPos->fields, vm.string.z, &endZ);
  
  // Get color
  ObjInstance *color = AS_INSTANCE(args[3]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawLine3D((Vector3){AS_NUMBER(startX), AS_NUMBER(startY), AS_NUMBER(startZ)}, 
             (Vector3){AS_NUMBER(endX), AS_NUMBER(endY), AS_NUMBER(endZ)}, 
             (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawCircleRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 5, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *color = AS_INSTANCE(args[4]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawCircle(AS_NUMBER(args[1]), AS_NUMBER(args[2]), AS_NUMBER(args[3]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawCircleVRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *center = AS_INSTANCE(args[1]);
  Value x, y;
  tableGet(&center->fields, vm.string.x, &x);
  tableGet(&center->fields, vm.string.y, &y);
  
  ObjInstance *color = AS_INSTANCE(args[3]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawCircleV((Vector2){AS_NUMBER(x), AS_NUMBER(y)}, AS_NUMBER(args[2]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawRectangleRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *color = AS_INSTANCE(args[5]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawRectangle(AS_NUMBER(args[1]), AS_NUMBER(args[2]), AS_NUMBER(args[3]), AS_NUMBER(args[4]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawRectangleVRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *position = AS_INSTANCE(args[1]);
  Value posX, posY;
  tableGet(&position->fields, vm.string.x, &posX);
  tableGet(&position->fields, vm.string.y, &posY);
  
  ObjInstance *size = AS_INSTANCE(args[2]);
  Value sizeX, sizeY;
  tableGet(&size->fields, vm.string.x, &sizeX);
  tableGet(&size->fields, vm.string.y, &sizeY);
  
  ObjInstance *color = AS_INSTANCE(args[3]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawRectangleV((Vector2){AS_NUMBER(posX), AS_NUMBER(posY)}, (Vector2){AS_NUMBER(sizeX), AS_NUMBER(sizeY)}, (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

// Additional 2D drawing functions
static Value drawTriangleRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 5, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE, ARG_INSTANCE, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *v1 = AS_INSTANCE(args[1]);
  Value v1x, v1y;
  tableGet(&v1->fields, vm.string.x, &v1x);
  tableGet(&v1->fields, vm.string.y, &v1y);
  
  ObjInstance *v2 = AS_INSTANCE(args[2]);
  Value v2x, v2y;
  tableGet(&v2->fields, vm.string.x, &v2x);
  tableGet(&v2->fields, vm.string.y, &v2y);
  
  ObjInstance *v3 = AS_INSTANCE(args[3]);
  Value v3x, v3y;
  tableGet(&v3->fields, vm.string.x, &v3x);
  tableGet(&v3->fields, vm.string.y, &v3y);
  
  ObjInstance *color = AS_INSTANCE(args[4]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawTriangle((Vector2){AS_NUMBER(v1x), AS_NUMBER(v1y)}, (Vector2){AS_NUMBER(v2x), AS_NUMBER(v2y)}, (Vector2){AS_NUMBER(v3x), AS_NUMBER(v3y)}, (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawCircleLinesRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 5, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *color = AS_INSTANCE(args[4]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawCircleLines(AS_NUMBER(args[1]), AS_NUMBER(args[2]), AS_NUMBER(args[3]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawRectangleLinesRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *color = AS_INSTANCE(args[5]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawRectangleLines(AS_NUMBER(args[1]), AS_NUMBER(args[2]), AS_NUMBER(args[3]), AS_NUMBER(args[4]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawTextRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *color = AS_INSTANCE(args[5]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawText(AS_CSTRING(args[1]), AS_NUMBER(args[2]), AS_NUMBER(args[3]), AS_NUMBER(args[4]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawFpsRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  DrawFPS(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

// More 3D drawing functions
static Value drawCubeRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *position = AS_INSTANCE(args[1]);
  Value x, y, z;
  tableGet(&position->fields, vm.string.x, &x);
  tableGet(&position->fields, vm.string.y, &y);
  tableGet(&position->fields, vm.string.z, &z);
  
  ObjInstance *color = AS_INSTANCE(args[5]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawCube((Vector3){AS_NUMBER(x), AS_NUMBER(y), AS_NUMBER(z)}, AS_NUMBER(args[2]), AS_NUMBER(args[3]), AS_NUMBER(args[4]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawCubeWiresRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *position = AS_INSTANCE(args[1]);
  Value x, y, z;
  tableGet(&position->fields, vm.string.x, &x);
  tableGet(&position->fields, vm.string.y, &y);
  tableGet(&position->fields, vm.string.z, &z);
  
  ObjInstance *color = AS_INSTANCE(args[5]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawCubeWires((Vector3){AS_NUMBER(x), AS_NUMBER(y), AS_NUMBER(z)}, AS_NUMBER(args[2]), AS_NUMBER(args[3]), AS_NUMBER(args[4]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawSphereRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *center = AS_INSTANCE(args[1]);
  Value x, y, z;
  tableGet(&center->fields, vm.string.x, &x);
  tableGet(&center->fields, vm.string.y, &y);
  tableGet(&center->fields, vm.string.z, &z);
  
  ObjInstance *color = AS_INSTANCE(args[3]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawSphere((Vector3){AS_NUMBER(x), AS_NUMBER(y), AS_NUMBER(z)}, AS_NUMBER(args[2]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawSphereWiresRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *center = AS_INSTANCE(args[1]);
  Value x, y, z;
  tableGet(&center->fields, vm.string.x, &x);
  tableGet(&center->fields, vm.string.y, &y);
  tableGet(&center->fields, vm.string.z, &z);
  
  ObjInstance *color = AS_INSTANCE(args[5]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawSphereWires((Vector3){AS_NUMBER(x), AS_NUMBER(y), AS_NUMBER(z)}, AS_NUMBER(args[2]), AS_NUMBER(args[3]), AS_NUMBER(args[4]), (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

static Value drawGridRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  DrawGrid(AS_NUMBER(args[1]), AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value drawPoint3DRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *position = AS_INSTANCE(args[1]);
  Value x, y, z;
  tableGet(&position->fields, vm.string.x, &x);
  tableGet(&position->fields, vm.string.y, &y);
  tableGet(&position->fields, vm.string.z, &z);
  
  ObjInstance *color = AS_INSTANCE(args[2]);
  Value r, g, b, a;
  tableGet(&color->fields, vm.string.r, &r);
  tableGet(&color->fields, vm.string.g, &g);
  tableGet(&color->fields, vm.string.b, &b);
  tableGet(&color->fields, vm.string.a, &a);
  
  DrawPoint3D((Vector3){AS_NUMBER(x), AS_NUMBER(y), AS_NUMBER(z)}, (Color){AS_NUMBER(r), AS_NUMBER(g), AS_NUMBER(b), AS_NUMBER(a)});
  return NIL_VAL;
}

// basic audio natives
static Value initAudioDeviceRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  InitAudioDevice();
  return NIL_VAL;
}

static Value closeAudioDeviceRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  CloseAudioDevice();
  return NIL_VAL;
}

static Value isAudioDeviceReadyRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return BOOL_VAL(IsAudioDeviceReady());
}

static Value setMasterVolumeRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetMasterVolume(AS_NUMBER(args[1]));
  return NIL_VAL;
}

static Value getMasterVolumeRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }
  return NUMBER_VAL(GetMasterVolume());
}

static Value createSound(Sound sound) {
  // Validate sound data
  if (sound.frameCount <= 0) {
    runtimeError("Invalid sound data loaded");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Allocate memory for Sound struct
  Sound *soundPtr = (Sound*)malloc(sizeof(Sound));
  if (!soundPtr) {
    runtimeError("Failed to allocate memory for sound");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Copy the entire Sound struct
  *soundPtr = sound;
  
  ObjInstance *soundInst = newInstance(soundKlassRef);
  push(OBJ_VAL(soundInst));
  
  // Store the Sound pointer - let Raylib manage the sound data
  defineNativeInstanceField(soundInst, "*sound_ptr", 10, NUMBER_VAL((uintptr_t)soundPtr));
  
  return pop();
}

static Sound extractSound(ObjInstance *soundInst) {
  Sound emptySound = {0};
  
  // Extract the Sound pointer
  Value soundPtrVal = readNativeInstanceField(soundInst, "*sound_ptr", 10);
  if (!IS_NUMBER(soundPtrVal)) {
    runtimeError("Invalid sound instance: missing sound pointer");
    vm.shouldPanic = true;
    return emptySound;
  }
  
  Sound *soundPtr = (Sound*)(uintptr_t)AS_NUMBER(soundPtrVal);
  if (!soundPtr) {
    runtimeError("Invalid sound pointer");
    vm.shouldPanic = true;
    return emptySound;
  }
  
  // Return the sound struct (Raylib manages the actual data)
  return *soundPtr;
}

static Value createWave(Wave wave) {
  // Validate wave data
  if (wave.frameCount <= 0 || wave.data == NULL) {
    runtimeError("Invalid wave data loaded");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Allocate memory for Wave struct and let Raylib manage it
  Wave *wavePtr = (Wave*)malloc(sizeof(Wave));
  if (!wavePtr) {
    runtimeError("Failed to allocate memory for wave");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Copy the entire Wave struct
  *wavePtr = wave;
  
  ObjInstance *waveInst = newInstance(waveKlassRef);
  push(OBJ_VAL(waveInst));
  
  // Store the Wave pointer directly - let Raylib manage the wave data
  defineNativeInstanceField(waveInst, "*wave_ptr", 9, NUMBER_VAL((uintptr_t)wavePtr));
  
  return pop();
}

static Wave extractWave(ObjInstance *waveInst) {
  Wave emptyWave = {0};
  
  // Extract the Wave pointer
  Value wavePtrVal = readNativeInstanceField(waveInst, "*wave_ptr", 9);
  if (!IS_NUMBER(wavePtrVal)) {
    runtimeError("Invalid wave instance: missing wave pointer");
    vm.shouldPanic = true;
    return emptyWave;
  }
  
  Wave *wavePtr = (Wave*)(uintptr_t)AS_NUMBER(wavePtrVal);
  if (!wavePtr) {
    runtimeError("Invalid wave pointer");
    vm.shouldPanic = true;
    return emptyWave;
  }
  
  // Return the wave struct (Raylib manages the actual data)
  return *wavePtr;
}

static Value createMusic(Music music) {
  // Validate music data
  if (music.frameCount <= 0) {
    runtimeError("Invalid music data loaded");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Allocate memory for Music struct
  Music *musicPtr = (Music*)malloc(sizeof(Music));
  if (!musicPtr) {
    runtimeError("Failed to allocate memory for music");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Copy the entire Music struct
  *musicPtr = music;
  
  ObjInstance *musicInst = newInstance(musicKlassRef);
  push(OBJ_VAL(musicInst));
  
  // Store the Music pointer
  defineNativeInstanceField(musicInst, "*music_ptr", 10, NUMBER_VAL((uintptr_t)musicPtr));
  
  return pop();
}

static Music extractMusic(ObjInstance *musicInst) {
  Music emptyMusic = {0};
  
  // Extract the Music pointer
  Value musicPtrVal = readNativeInstanceField(musicInst, "*music_ptr", 10);
  if (!IS_NUMBER(musicPtrVal)) {
    runtimeError("Invalid music instance: missing music pointer");
    vm.shouldPanic = true;
    return emptyMusic;
  }
  
  Music *musicPtr = (Music*)(uintptr_t)AS_NUMBER(musicPtrVal);
  if (!musicPtr) {
    runtimeError("Invalid music pointer");
    vm.shouldPanic = true;
    return emptyMusic;
  }
  
  return *musicPtr;
}

static Value createAudioStream(AudioStream stream) {
  // Allocate memory for AudioStream struct
  AudioStream *streamPtr = (AudioStream*)malloc(sizeof(AudioStream));
  if (!streamPtr) {
    runtimeError("Failed to allocate memory for audio stream");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  // Copy the entire AudioStream struct
  *streamPtr = stream;
  
  ObjInstance *streamInst = newInstance(audioStreamKlassRef);
  push(OBJ_VAL(streamInst));
  
  // Store the AudioStream pointer
  defineNativeInstanceField(streamInst, "*stream_ptr", 11, NUMBER_VAL((uintptr_t)streamPtr));
  
  return pop();
}

static AudioStream extractAudioStream(ObjInstance *streamInst) {
  AudioStream emptyStream = {0};
  
  // Extract the AudioStream pointer
  Value streamPtrVal = readNativeInstanceField(streamInst, "*stream_ptr", 11);
  if (!IS_NUMBER(streamPtrVal)) {
    runtimeError("Invalid audio stream instance: missing stream pointer");
    vm.shouldPanic = true;
    return emptyStream;
  }
  
  AudioStream *streamPtr = (AudioStream*)(uintptr_t)AS_NUMBER(streamPtrVal);
  if (!streamPtr) {
    runtimeError("Invalid audio stream pointer");
    vm.shouldPanic = true;
    return emptyStream;
  }
  
  return *streamPtr;
}

// Extended audio functions
static Value loadSoundRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  }
  
  const char *filename = AS_CSTRING(args[1]);
  if (!filename || strlen(filename) == 0) {
    runtimeError("Invalid filename for sound loading");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  Sound sound = LoadSound(filename);
  
  // Check if sound loaded successfully
  if (sound.frameCount == 0) {
    runtimeError("Failed to load sound file: %s", filename);
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  Value result = createSound(sound);
  return result;
}

static Value playSoundRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound sound = extractSound(soundInst);
  PlaySound(sound);
  return NIL_VAL;
}

static Value stopSoundRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound sound = extractSound(soundInst);
  StopSound(sound);
  return NIL_VAL;
}

static Value unloadSoundRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  push(OBJ_VAL(soundInst));
  
  // Get the Sound pointer
  Value soundPtrVal = readNativeInstanceField(soundInst, "*sound_ptr", 10);
  if (IS_NUMBER(soundPtrVal)) {
    Sound *soundPtr = (Sound*)(uintptr_t)AS_NUMBER(soundPtrVal);
    if (soundPtr) {
      // Let Raylib unload the sound data
      UnloadSound(*soundPtr);
      // Free our allocated struct
      free(soundPtr);
      // Clear the pointer to prevent double-free
      defineNativeInstanceField(soundInst, "*sound_ptr", 10, NUMBER_VAL(0));
    }
  }
  
  pop();
  return NIL_VAL;
}

static Value isSoundPlayingRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound sound = extractSound(soundInst);
  return BOOL_VAL(IsSoundPlaying(sound));
}

// Wave/Sound loading/unloading functions
static Value loadWaveRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  }
  
  const char *filename = AS_CSTRING(args[1]);
  if (!filename || strlen(filename) == 0) {
    runtimeError("Invalid filename for wave loading");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  Wave wave = LoadWave(filename);
  
  // Check if wave loaded successfully
  if (wave.data == NULL || wave.frameCount == 0) {
    runtimeError("Failed to load wave file: %s", filename);
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  Value result = createWave(wave);
  return result;
}


static Value loadSoundFromWaveRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  
  ObjInstance *waveInst = AS_INSTANCE(args[1]);
  push(OBJ_VAL(waveInst)); // GC protection
  
  Wave wave = extractWave(waveInst);
  if (vm.shouldPanic) {
    pop(); // Clean up GC stack
    return NIL_VAL;
  }
  
  Sound sound = LoadSoundFromWave(wave);
  pop(); // Clean up GC stack
  
  if (sound.frameCount == 0) {
    runtimeError("Failed to create sound from wave data");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  return createSound(sound);
}

static Value loadSoundAliasRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound source = extractSound(soundInst);
  Sound alias = LoadSoundAlias(source);
  return createSound(alias);
}


static Value unloadWaveRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  
  ObjInstance *waveInst = AS_INSTANCE(args[1]);
  push(OBJ_VAL(waveInst));
  
  // Get the Wave pointer
  Value wavePtrVal = readNativeInstanceField(waveInst, "*wave_ptr", 9);
  if (IS_NUMBER(wavePtrVal)) {
    Wave *wavePtr = (Wave*)(uintptr_t)AS_NUMBER(wavePtrVal);
    if (wavePtr) {
      // Let Raylib unload the wave data
      UnloadWave(*wavePtr);
      // Free our allocated struct
      free(wavePtr);
      // Clear the pointer to prevent double-free
      defineNativeInstanceField(waveInst, "*wave_ptr", 9, NUMBER_VAL(0));
    }
  }
  
  pop();
  return NIL_VAL;
}

static Value unloadSoundAliasRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound alias = extractSound(soundInst);
  UnloadSoundAlias(alias);
  return NIL_VAL;
}

static Value exportWaveRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_STRING)) {
    return NIL_VAL;
  }
  
  ObjInstance *waveInst = AS_INSTANCE(args[1]);
  push(OBJ_VAL(waveInst)); // GC protection
  
  Wave wave = extractWave(waveInst);
  if (vm.shouldPanic) {
    pop();
    return NIL_VAL;
  }
  
  // Safety checks
  if (wave.data == NULL || wave.frameCount == 0) {
    pop();
    runtimeError("Invalid wave data for export");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  const char *filename = AS_CSTRING(args[2]);
  if (!filename || strlen(filename) == 0) {
    pop();
    runtimeError("Invalid filename for wave export");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  bool result = ExportWave(wave, filename);
  pop(); // Clean up GC stack
  
  return BOOL_VAL(result);
}


// Wave/Sound management functions
static Value pauseSoundRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound sound = extractSound(soundInst);
  PauseSound(sound);
  return NIL_VAL;
}

static Value resumeSoundRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound sound = extractSound(soundInst);
  ResumeSound(sound);
  return NIL_VAL;
}

static Value setSoundVolumeRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound sound = extractSound(soundInst);
  SetSoundVolume(sound, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setSoundPitchRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound sound = extractSound(soundInst);
  SetSoundPitch(sound, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setSoundPanRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *soundInst = AS_INSTANCE(args[1]);
  Sound sound = extractSound(soundInst);
  SetSoundPan(sound, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value waveCopyRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  
  ObjInstance *waveInst = AS_INSTANCE(args[1]);
  push(OBJ_VAL(waveInst)); // GC protection
  
  Wave wave = extractWave(waveInst);
  if (vm.shouldPanic) {
    pop();
    return NIL_VAL;
  }
  
  Wave copy = WaveCopy(wave);
  pop(); // Clean up GC stack
  
  if (copy.data == NULL || copy.frameCount == 0) {
    runtimeError("Failed to copy wave data");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  
  return createWave(copy);
}

static Value waveCropRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *waveInst = AS_INSTANCE(args[1]);
  Wave wave = extractWave(waveInst);
  WaveCrop(&wave, AS_NUMBER(args[2]), AS_NUMBER(args[3]));
  return NIL_VAL;
}

static Value waveFormatRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 5, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *waveInst = AS_INSTANCE(args[1]);
  Wave wave = extractWave(waveInst);
  WaveFormat(&wave, AS_NUMBER(args[2]), AS_NUMBER(args[3]), AS_NUMBER(args[4]));
  return NIL_VAL;
}

// Music management functions
static Value loadMusicStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  }
  Music music = LoadMusicStream(AS_CSTRING(args[1]));
  return createMusic(music);
}


static Value unloadMusicStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  push(OBJ_VAL(musicInst));
  
  // Get the Music pointer
  Value musicPtrVal = readNativeInstanceField(musicInst, "*music_ptr", 10);
  if (IS_NUMBER(musicPtrVal)) {
    Music *musicPtr = (Music*)(uintptr_t)AS_NUMBER(musicPtrVal);
    if (musicPtr) {
      // Let Raylib unload the music data
      UnloadMusicStream(*musicPtr);
      // Free our allocated struct
      free(musicPtr);
      // Clear the pointer
      defineNativeInstanceField(musicInst, "*music_ptr", 10, NUMBER_VAL(0));
    }
  }
  
  pop();
  return NIL_VAL;
}

static Value playMusicStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  PlayMusicStream(music);
  return NIL_VAL;
}

static Value isMusicStreamPlayingRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  return BOOL_VAL(IsMusicStreamPlaying(music));
}

static Value updateMusicStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  UpdateMusicStream(music);
  return NIL_VAL;
}

static Value stopMusicStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  StopMusicStream(music);
  return NIL_VAL;
}

static Value pauseMusicStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  PauseMusicStream(music);
  return NIL_VAL;
}

static Value resumeMusicStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  ResumeMusicStream(music);
  return NIL_VAL;
}

static Value seekMusicStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  SeekMusicStream(music, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setMusicVolumeRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  SetMusicVolume(music, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setMusicPitchRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  SetMusicPitch(music, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setMusicPanRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  SetMusicPan(music, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value getMusicTimeLengthRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  return NUMBER_VAL(GetMusicTimeLength(music));
}

static Value getMusicTimePlayedRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *musicInst = AS_INSTANCE(args[1]);
  Music music = extractMusic(musicInst);
  return NUMBER_VAL(GetMusicTimePlayed(music));
}

// AudioStream management functions
static Value loadAudioStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  }
  AudioStream stream = LoadAudioStream(AS_NUMBER(args[1]), AS_NUMBER(args[2]), AS_NUMBER(args[3]));
  return createAudioStream(stream);
}


static Value unloadAudioStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  push(OBJ_VAL(streamInst));
  
  // Get the AudioStream pointer
  Value streamPtrVal = readNativeInstanceField(streamInst, "*stream_ptr", 11);
  if (IS_NUMBER(streamPtrVal)) {
    AudioStream *streamPtr = (AudioStream*)(uintptr_t)AS_NUMBER(streamPtrVal);
    if (streamPtr) {
      // Let Raylib unload the stream data
      UnloadAudioStream(*streamPtr);
      // Free our allocated struct
      free(streamPtr);
      // Clear the pointer
      defineNativeInstanceField(streamInst, "*stream_ptr", 11, NUMBER_VAL(0));
    }
  }
  
  pop();
  return NIL_VAL;
}

static Value isAudioStreamProcessedRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  return BOOL_VAL(IsAudioStreamProcessed(stream));
}

static Value playAudioStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  PlayAudioStream(stream);
  return NIL_VAL;
}

static Value pauseAudioStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  PauseAudioStream(stream);
  return NIL_VAL;
}

static Value resumeAudioStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  ResumeAudioStream(stream);
  return NIL_VAL;
}

static Value isAudioStreamPlayingRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  return BOOL_VAL(IsAudioStreamPlaying(stream));
}

static Value stopAudioStreamRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  StopAudioStream(stream);
  return NIL_VAL;
}

static Value setAudioStreamVolumeRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  SetAudioStreamVolume(stream, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setAudioStreamPitchRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  SetAudioStreamPitch(stream, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setAudioStreamPanRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_NUMBER)) {
    return NIL_VAL;
  }
  ObjInstance *streamInst = AS_INSTANCE(args[1]);
  AudioStream stream = extractAudioStream(streamInst);
  SetAudioStreamPan(stream, AS_NUMBER(args[2]));
  return NIL_VAL;
}

static Value setAudioStreamBufferSizeDefaultRLNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER)) {
    return NIL_VAL;
  }
  SetAudioStreamBufferSizeDefault(AS_NUMBER(args[1]));
  return NIL_VAL;
}

static Value createColor(double r, double g, double b, double a) {
  ObjInstance *color = newInstance(colorKlassRef);
  push(OBJ_VAL(color));   
  setNativeInstanceField(color, vm.string.r, NUMBER_VAL(r));
  setNativeInstanceField(color, vm.string.g, NUMBER_VAL(g));
  setNativeInstanceField(color, vm.string.b, NUMBER_VAL(b));
  setNativeInstanceField(color, vm.string.a, NUMBER_VAL(a));
  return pop();
}

static Value initColorNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 5, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  ObjInstance *color = NULL;
  if (IS_KLASS(args[0])) {
    color = newInstance(AS_KLASS(args[0]));
  } else if (IS_INSTANCE(args[0])) {
    color = AS_INSTANCE(args[0]);
  } else {
    runtimeError("Could not init Color.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(color));
  setNativeInstanceField(color, vm.string.r, args[1]);
  setNativeInstanceField(color, vm.string.g, args[2]);
  setNativeInstanceField(color, vm.string.b, args[3]);
  setNativeInstanceField(color, vm.string.a, args[4]);
  return pop();
}

static Value initCamera2dNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 5, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  ObjInstance *cam = NULL;
  if (IS_KLASS(args[0])) {
    cam = newInstance(AS_KLASS(args[0]));
  } else if (IS_INSTANCE(args[0])) {
    cam = AS_INSTANCE(args[0]);
  } else {
    runtimeError("Could not init Camera2D.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(cam));
  defineNativeInstanceField(cam, "offset", 6, args[1]);
  defineNativeInstanceField(cam, "target", 6, args[2]);
  defineNativeInstanceField(cam, "rotation", 8, args[3]);
  defineNativeInstanceField(cam, "zoom", 4, args[4]);
  return pop();
}

static Value initCamera3dNative(int argCount, Value *args) {
  if (!checkArgs(argCount, 6, args, NATIVE_NORMAL, ARG_ANY, ARG_INSTANCE, ARG_INSTANCE, ARG_INSTANCE, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  ObjInstance *cam = NULL;
  if (IS_KLASS(args[0])) {
    cam = newInstance(AS_KLASS(args[0]));
  } else if (IS_INSTANCE(args[0])) {
    cam = AS_INSTANCE(args[0]);
  } else {
    runtimeError("Could not init Camera3D.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(cam));
  defineNativeInstanceField(cam, "position", 8, args[1]);
  defineNativeInstanceField(cam, "target", 6, args[2]);
  defineNativeInstanceField(cam, "up", 2, args[3]);
  defineNativeInstanceField(cam, "fovy", 4, args[4]);
  defineNativeInstanceField(cam, "projection", 10, args[5]);
  return pop();
}

static Value initVector2Native(int argCount, Value *args) {
  if (!checkArgs(argCount, 3, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  ObjInstance *vector2 = NULL;
  if (IS_KLASS(args[0])) {
    vector2 = newInstance(AS_KLASS(args[0]));
  } else if (IS_INSTANCE(args[0])) {
    vector2 = AS_INSTANCE(args[0]);
  } else {
    runtimeError("Could not init Vector2.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(vector2));
  setNativeInstanceField(vector2, vm.string.x, args[1]);
  setNativeInstanceField(vector2, vm.string.y, args[2]);
  return pop();
}

static Value initVector3Native(int argCount, Value *args) {
  if (!checkArgs(argCount, 4, args, NATIVE_NORMAL, ARG_ANY, ARG_NUMBER, ARG_NUMBER, ARG_NUMBER)) {
    return NIL_VAL;
  };
  ObjInstance *vector3 = NULL;
  if (IS_KLASS(args[0])) {
    vector3 = newInstance(AS_KLASS(args[0]));
  } else if (IS_INSTANCE(args[0])) {
    vector3 = AS_INSTANCE(args[0]);
  } else {
    runtimeError("Could not init Vector3.");
    vm.shouldPanic = true;
    return NIL_VAL;
  }
  push(OBJ_VAL(vector3));
  setNativeInstanceField(vector3, vm.string.x, args[1]);
  setNativeInstanceField(vector3, vm.string.y, args[2]);
  setNativeInstanceField(vector3, vm.string.z, args[3]);
  return pop();
}

void registerRaylibNatives() {
  static bool isRegistered = false;
  if (isRegistered)
    return;
  isRegistered = true;
  ObjInstance *raylibInstance =
    defineInstance(defineKlass("RL", 2, OBJ_INSTANCE), "RL", 2);

  ObjKlass *colorKlass = newKlass(copyString("Color", 5, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(colorKlass));
  colorKlassRef = colorKlass;
  defineNativeKlassMethod(colorKlass, "init", 4, initColorNative); 
  defineNativeInstanceField(raylibInstance, "Color", 5, pop());

  ObjKlass *vector2Klass = newKlass(copyString("Vector2", 7, &vm.strings), OBJ_INSTANCE); 
  push(OBJ_VAL(vector2Klass));
  vector2KlassRef = vector2Klass;
  defineNativeKlassMethod(vector2Klass, "init", 4, initVector2Native);
  defineNativeInstanceField(raylibInstance, "Vector2", 7, pop());

  ObjKlass *vector3Klass = newKlass(copyString("Vector3", 7, &vm.strings), OBJ_INSTANCE); 
  push(OBJ_VAL(vector3Klass));
  vector3KlassRef = vector3Klass;
  defineNativeKlassMethod(vector3Klass, "init", 4, initVector3Native);
  defineNativeInstanceField(raylibInstance, "Vector3", 7, pop());

  ObjKlass *camera2dKlass = newKlass(copyString("Camera2D", 8, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(camera2dKlass));
  camera2dRef = camera2dKlass;
  defineNativeKlassMethod(camera2dKlass, "init", 4, initCamera2dNative);
  defineNativeInstanceField(raylibInstance, "Camera2D", 8, pop());

  ObjKlass *camera3dKlass = newKlass(copyString("Camera3D", 8, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(camera3dKlass));
  camera3dRef = camera3dKlass;
  defineNativeKlassMethod(camera3dKlass, "init", 4, initCamera3dNative);
  defineNativeInstanceField(raylibInstance, "Camera3D", 8, pop());

  ObjKlass *soundKlass = newKlass(copyString("Sound", 5, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(soundKlass));
  soundKlassRef = soundKlass;
  defineNativeInstanceField(raylibInstance, "Sound", 5, pop());

  ObjKlass *waveKlass = newKlass(copyString("Wave", 4, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(waveKlass));
  waveKlassRef = waveKlass;
  defineNativeInstanceField(raylibInstance, "Wave", 4, pop());

  ObjKlass *musicKlass = newKlass(copyString("Music", 5, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(musicKlass));
  musicKlassRef = musicKlass;
  defineNativeInstanceField(raylibInstance, "Music", 5, pop());

  ObjKlass *audioStreamKlass = newKlass(copyString("AudioStream", 11, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(audioStreamKlass));
  audioStreamKlassRef = audioStreamKlass;
  defineNativeInstanceField(raylibInstance, "AudioStream", 11, pop());

  ObjKlass *imgKlass = newKlass(copyString("Image", 5, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(imgKlass));
  imgKlassRef = imgKlass;
  defineNativeInstanceField(raylibInstance, "Image", 5,pop());

  // rcore
  // window-related methods
  defineNativeInstanceMethod(raylibInstance, "init_window", 11, initWindowRLNative);
  defineNativeInstanceMethod(raylibInstance, "close_window", 12, closeWindowRLNative);
  defineNativeInstanceMethod(raylibInstance, "window_should_close", 19, windowShouldCloseRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_window_ready", 15, isWindowReadyRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_window_fullscreen", 20, isWindowFullscreenRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_window_hidden", 16, isWindowHiddenRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_window_minimized", 19, isWindowMinimizedRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_window_maximized", 19, isWindowMaximizedRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_window_focused", 17, isWindowFocusedRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_window_resized", 17, isWindowResizedRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_window_state", 15, isWindowStateRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_window_state", 16, setWindowStateRLNative);
  defineNativeInstanceMethod(raylibInstance, "clear_window_state", 18, clearWindowStateRLNative);
  defineNativeInstanceMethod(raylibInstance, "toggle_fullscreen", 17, toggleFullscreenRLNative);
  defineNativeInstanceMethod(raylibInstance, "toggle_borderless_windowed", 26, toggleBorderlessWindowedRLNative);
  defineNativeInstanceMethod(raylibInstance, "maximize_window", 15, maximizeWindowRLNative);
  defineNativeInstanceMethod(raylibInstance, "minimize_window", 15, minimizeWindowRLNative);
  defineNativeInstanceMethod(raylibInstance, "restore_window", 14, restoreWindowRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_window_icon", 15, setWindowIconRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_window_title", 16, setWindowTitleRLNative);  
  defineNativeInstanceMethod(raylibInstance, "set_window_position", 19, setWindowPositionRLNative);  
  defineNativeInstanceMethod(raylibInstance, "set_window_monitor", 18, setWindowMonitorRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_window_min_size", 19, setWindowMinSizeRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_window_max_size", 19, setWindowMaxSizeRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_window_size", 15, setWindowSizeRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_window_opacity", 18, setWindowOpacityRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_window_focused", 18, setWindowFocusedRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_window_handle", 17, getWindowHandleRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_screen_width", 16, getScreenWidthRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_screen_height", 17, getScreenHeightRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_render_width", 16, getRenderWidthRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_render_height", 17, getRenderHeightRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_monitor_count", 17, getMonitorCountRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_current_monitor", 19, getCurrentMonitorRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_monitor_position", 20, getMonitorPositionRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_monitor_width", 17, getMonitorWidthRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_monitor_height", 18, getMonitorHeightRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_monitor_physical_width", 26, getMonitorPhysicalWidthRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_monitor_physical_height", 27, getMonitorPhysicalHeightRLNative);  
  defineNativeInstanceMethod(raylibInstance, "get_monitor_refresh_rate", 24, getMonitorRefreshRateRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_window_position", 19, getWindowPositionRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_window_scale_dpi", 20, getWindowScaleDpiRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_monitor_name", 16, getMonitorNameRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_clipboard_text", 18, setClipboardTextRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_clipboard_text", 18, getClipboardTextRLNative);  
  
  defineNativeInstanceMethod(raylibInstance, "enable_event_waiting", 20, enableEventWaitingRLNative);
  defineNativeInstanceMethod(raylibInstance, "disable_event_waiting", 21, disableEventWaitingRLNative);
  defineNativeInstanceMethod(raylibInstance, "take_screenshot", 15, takeScreenshotRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_config_flags", 16, setConfigFlagsRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_exit_key", 12, setExitKeyRLNative);

  // cursor-related methods
  defineNativeInstanceMethod(raylibInstance, "show_cursor", 11, showCursorRLNative);
  defineNativeInstanceMethod(raylibInstance, "hide_cursor", 11, hideCursorRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_cursor_hidden", 16, isCursorHiddenRLNative);
  defineNativeInstanceMethod(raylibInstance, "enable_cursor", 13, enableCursorRLNative);
  defineNativeInstanceMethod(raylibInstance, "disable_cursor", 14, disableCursorRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_cursor_on_screen", 19, isCursorOnScreenRLNative);

  // drawing-related methods
  defineNativeInstanceMethod(raylibInstance, "clear_background", 16, clearBackgroundRLNative);
  defineNativeInstanceMethod(raylibInstance, "begin_drawing", 13, beginDrawingRLNative);
  defineNativeInstanceMethod(raylibInstance, "end_drawing", 11, endDrawingRLNative);
  defineNativeInstanceMethod(raylibInstance, "begin_mode_2d", 13, beginMode2dRLNative);
  defineNativeInstanceMethod(raylibInstance, "end_mode_2d", 11, endMode2dRLNative);
  defineNativeInstanceMethod(raylibInstance, "begin_mode_3d", 13, beginMode3dRLNative);
  defineNativeInstanceMethod(raylibInstance, "end_mode_3d", 11, endMode3dRLNative);

  // timing-related functions
  defineNativeInstanceMethod(raylibInstance, "set_target_fps", 14, setTargetFpsRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_frame_time", 14, getFrameTimeRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_time", 8, getTimeRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_fps", 7, getFpsRLNative);

  // input-related methods
  defineNativeInstanceMethod(raylibInstance, "is_key_pressed", 14, isKeyPressedRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_key_pressed_repeat", 21, isKeyPressedRepeatRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_key_down", 11, isKeyDownRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_key_released", 15, isKeyReleasedRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_key_up", 9, isKeyUpRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_key_pressed", 15, getKeyPressedRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_char_pressed", 16, getCharPressedRLNative);
  
  // mouse-related methods
  defineNativeInstanceMethod(raylibInstance, "is_mouse_button_pressed", 23, isMouseButtonPressedRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_mouse_button_down", 20, isMouseButtonDownRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_mouse_button_released", 24, isMouseButtonReleasedRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_mouse_button_up", 18, isMouseButtonUpRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_mouse_x", 11, getMouseXRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_mouse_y", 11, getMouseYRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_mouse_position", 18, getMousePositionRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_mouse_delta", 15, getMouseDeltaRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_mouse_position", 18, setMousePositionRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_mouse_offset", 16, setMouseOffsetRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_mouse_scale", 15, setMouseScaleRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_mouse_wheel_move", 20, getMouseWheelMoveRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_mouse_wheel_move_v", 22, getMouseWheelMoveVRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_mouse_cursor", 16, setMouseCursorRLNative);
  
  // touch-related methods
  defineNativeInstanceMethod(raylibInstance, "get_touch_x", 11, getTouchXRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_touch_y", 11, getTouchYRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_touch_position", 18, getTouchPositionRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_touch_point_count", 21, getTouchPointCountRLNative);
  
  // drawing primitives
  defineNativeInstanceMethod(raylibInstance, "draw_pixel", 10, drawPixelRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_pixel_v", 12, drawPixelVRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_line", 9, drawLineRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_line_v", 11, drawLineVRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_line_ex", 12, drawLineExRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_line_3d", 12, drawLine3DRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_circle", 11, drawCircleRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_circle_v", 13, drawCircleVRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_rectangle", 14, drawRectangleRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_rectangle_v", 16, drawRectangleVRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_rectangle_lines", 19, drawRectangleLinesRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_triangle", 13, drawTriangleRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_circle_lines", 17, drawCircleLinesRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_text", 9, drawTextRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_fps", 8, drawFpsRLNative);
  
  // 3D drawing functions
  defineNativeInstanceMethod(raylibInstance, "draw_cube", 9, drawCubeRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_cube_wires", 15, drawCubeWiresRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_sphere", 11, drawSphereRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_sphere_wires", 17, drawSphereWiresRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_grid", 9, drawGridRLNative);
  defineNativeInstanceMethod(raylibInstance, "draw_point_3d", 13, drawPoint3DRLNative);
  
  // audio-related methods
  defineNativeInstanceMethod(raylibInstance, "init_audio_device", 17, initAudioDeviceRLNative);
  defineNativeInstanceMethod(raylibInstance, "close_audio_device", 18, closeAudioDeviceRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_audio_device_ready", 20, isAudioDeviceReadyRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_master_volume", 17, setMasterVolumeRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_master_volume", 17, getMasterVolumeRLNative);
  defineNativeInstanceMethod(raylibInstance, "load_sound", 10, loadSoundRLNative);
  defineNativeInstanceMethod(raylibInstance, "play_sound", 10, playSoundRLNative);
  defineNativeInstanceMethod(raylibInstance, "stop_sound", 10, stopSoundRLNative);
  defineNativeInstanceMethod(raylibInstance, "unload_sound", 12, unloadSoundRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_sound_playing", 16, isSoundPlayingRLNative);

  // Wave/Sound loading/unloading methods
  defineNativeInstanceMethod(raylibInstance, "load_wave", 9, loadWaveRLNative);
  defineNativeInstanceMethod(raylibInstance, "load_sound_from_wave", 20, loadSoundFromWaveRLNative);
  defineNativeInstanceMethod(raylibInstance, "load_sound_alias", 16, loadSoundAliasRLNative);
  defineNativeInstanceMethod(raylibInstance, "unload_wave", 11, unloadWaveRLNative);
  defineNativeInstanceMethod(raylibInstance, "unload_sound_alias", 18, unloadSoundAliasRLNative);
  defineNativeInstanceMethod(raylibInstance, "export_wave", 11, exportWaveRLNative);

  // Wave/Sound management methods
  defineNativeInstanceMethod(raylibInstance, "pause_sound", 11, pauseSoundRLNative);
  defineNativeInstanceMethod(raylibInstance, "resume_sound", 12, resumeSoundRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_sound_volume", 16, setSoundVolumeRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_sound_pitch", 15, setSoundPitchRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_sound_pan", 13, setSoundPanRLNative);
  defineNativeInstanceMethod(raylibInstance, "wave_copy", 9, waveCopyRLNative);
  defineNativeInstanceMethod(raylibInstance, "wave_crop", 9, waveCropRLNative);
  defineNativeInstanceMethod(raylibInstance, "wave_format", 11, waveFormatRLNative);

  // Music management methods
  defineNativeInstanceMethod(raylibInstance, "load_music_stream", 17, loadMusicStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "unload_music_stream", 19, unloadMusicStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "play_music_stream", 17, playMusicStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_music_stream_playing", 23, isMusicStreamPlayingRLNative);
  defineNativeInstanceMethod(raylibInstance, "update_music_stream", 19, updateMusicStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "stop_music_stream", 17, stopMusicStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "pause_music_stream", 18, pauseMusicStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "resume_music_stream", 19, resumeMusicStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "seek_music_stream", 17, seekMusicStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_music_volume", 16, setMusicVolumeRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_music_pitch", 15, setMusicPitchRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_music_pan", 13, setMusicPanRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_music_time_length", 21, getMusicTimeLengthRLNative);
  defineNativeInstanceMethod(raylibInstance, "get_music_time_played", 21, getMusicTimePlayedRLNative);

  // AudioStream management methods
  defineNativeInstanceMethod(raylibInstance, "load_audio_stream", 17, loadAudioStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "unload_audio_stream", 19, unloadAudioStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_audio_stream_processed", 25, isAudioStreamProcessedRLNative);
  defineNativeInstanceMethod(raylibInstance, "play_audio_stream", 17, playAudioStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "pause_audio_stream", 18, pauseAudioStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "resume_audio_stream", 19, resumeAudioStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "is_audio_stream_playing", 23, isAudioStreamPlayingRLNative);
  defineNativeInstanceMethod(raylibInstance, "stop_audio_stream", 17, stopAudioStreamRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_audio_stream_volume", 23, setAudioStreamVolumeRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_audio_stream_pitch", 22, setAudioStreamPitchRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_audio_stream_pan", 20, setAudioStreamPanRLNative);
  defineNativeInstanceMethod(raylibInstance, "set_audio_stream_buffer_size_default", 36, setAudioStreamBufferSizeDefaultRLNative);

  // rtextures
  defineNativeInstanceMethod(raylibInstance, "load_image", 10, loadImageRLNative);

  // colors
  defineNativeInstanceField(raylibInstance, "LIGHTGRAY", 9, createColor(200, 200, 200, 255)); 
  defineNativeInstanceField(raylibInstance, "GRAY", 4, createColor(130, 130, 130, 255));
  defineNativeInstanceField(raylibInstance, "DARKGRAY", 8, createColor(80, 80, 80, 255));
  defineNativeInstanceField(raylibInstance, "YELLOW", 6, createColor(253, 249, 0, 255));
  defineNativeInstanceField(raylibInstance, "GOLD", 4, createColor(255, 203, 0, 255));
  defineNativeInstanceField(raylibInstance, "ORANGE", 6, createColor(255, 161, 0, 255));
  defineNativeInstanceField(raylibInstance, "PINK", 4, createColor(255, 109, 194, 255));
  defineNativeInstanceField(raylibInstance, "RED", 3, createColor(230, 41, 55, 255));
  defineNativeInstanceField(raylibInstance, "MAROON", 6, createColor(190, 33, 55, 255));
  defineNativeInstanceField(raylibInstance, "GREEN", 5, createColor(0, 228, 48, 255));
  defineNativeInstanceField(raylibInstance, "LIME", 4, createColor(0, 158, 47, 255));
  defineNativeInstanceField(raylibInstance, "DARKGREEN", 9, createColor(0, 117, 44, 255));
  defineNativeInstanceField(raylibInstance, "SKYBLUE", 7, createColor(102, 191, 255, 255));
  defineNativeInstanceField(raylibInstance, "BLUE", 4, createColor(0, 121, 241, 255));
  defineNativeInstanceField(raylibInstance, "DARKBLUE", 8, createColor(0, 82, 172, 255));
  defineNativeInstanceField(raylibInstance, "PURPLE", 6, createColor(200, 122, 255, 255));
  defineNativeInstanceField(raylibInstance, "VIOLET", 6, createColor(135, 60, 190, 255));
  defineNativeInstanceField(raylibInstance, "DARKPURPLE", 10, createColor(112, 31, 126, 255));
  defineNativeInstanceField(raylibInstance, "BEIGE", 5, createColor(211, 176, 131, 255));
  defineNativeInstanceField(raylibInstance, "BROWN", 5, createColor(127, 106, 79, 255));
  defineNativeInstanceField(raylibInstance, "DARKBROWN", 9, createColor(76, 63, 47, 255));
  defineNativeInstanceField(raylibInstance, "WHITE", 5, createColor(255, 255, 255, 255));
  defineNativeInstanceField(raylibInstance, "BLACK", 5, createColor(0, 0, 0, 255));
  defineNativeInstanceField(raylibInstance, "BLANK", 5, createColor(0, 0, 0, 0));
  defineNativeInstanceField(raylibInstance, "MAGENTA", 7, createColor(255, 0, 255, 255));
  defineNativeInstanceField(raylibInstance, "RAYWHITE", 8, createColor(245, 245, 245, 255));

  // FLAGS
  defineNativeInstanceField(raylibInstance, "FLAG_VSYNC_HINT", 15, NUMBER_VAL(FLAG_VSYNC_HINT)); 
  defineNativeInstanceField(raylibInstance, "FLAG_FULLSCREEN_MODE", 20, NUMBER_VAL(FLAG_FULLSCREEN_MODE)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_RESIZABLE", 21, NUMBER_VAL(FLAG_WINDOW_RESIZABLE)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_UNDECORATED", 23, NUMBER_VAL(FLAG_WINDOW_UNDECORATED)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_HIDDEN", 18, NUMBER_VAL(FLAG_WINDOW_HIDDEN)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_MINIMIZED", 21, NUMBER_VAL(FLAG_WINDOW_MINIMIZED)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_MAXIMIZED", 21, NUMBER_VAL(FLAG_WINDOW_MAXIMIZED)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_UNFOCUSED", 21, NUMBER_VAL(FLAG_WINDOW_UNFOCUSED)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_TOPMOST", 19, NUMBER_VAL(FLAG_WINDOW_TOPMOST)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_ALWAYS_RUN", 22, NUMBER_VAL(FLAG_WINDOW_ALWAYS_RUN)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_TRANSPARENT", 23, NUMBER_VAL(FLAG_WINDOW_TRANSPARENT)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_HIGHDPI", 19, NUMBER_VAL(FLAG_WINDOW_HIGHDPI)); 
  defineNativeInstanceField(raylibInstance, "FLAG_WINDOW_MOUSE_PASSTHROUGH", 29, NUMBER_VAL(FLAG_WINDOW_MOUSE_PASSTHROUGH)); 
  defineNativeInstanceField(raylibInstance, "FLAG_BORDERLESS_WINDOWED_MODE", 29, NUMBER_VAL(FLAG_BORDERLESS_WINDOWED_MODE)); 
  defineNativeInstanceField(raylibInstance, "FLAG_MSAA_4X_HINT", 17, NUMBER_VAL(FLAG_MSAA_4X_HINT)); 
  defineNativeInstanceField(raylibInstance, "FLAG_INTERLACED_HINT", 20, NUMBER_VAL(FLAG_INTERLACED_HINT)); 

  // camera projection
  defineNativeInstanceField(raylibInstance, "CAMERA_PERSPECTIVE", 18, NUMBER_VAL(CAMERA_PERSPECTIVE));
  defineNativeInstanceField(raylibInstance, "CAMERA_ORTHOGRAPHIC", 19, NUMBER_VAL(CAMERA_ORTHOGRAPHIC));
  
  // keyboard keys
  defineNativeInstanceField(raylibInstance, "KEY_NULL", 8, NUMBER_VAL(KEY_NULL));
  defineNativeInstanceField(raylibInstance, "KEY_APOSTROPHE", 14, NUMBER_VAL(KEY_APOSTROPHE));
  defineNativeInstanceField(raylibInstance, "KEY_COMMA", 9, NUMBER_VAL(KEY_COMMA));
  defineNativeInstanceField(raylibInstance, "KEY_MINUS", 9, NUMBER_VAL(KEY_MINUS));
  defineNativeInstanceField(raylibInstance, "KEY_PERIOD", 10, NUMBER_VAL(KEY_PERIOD));
  defineNativeInstanceField(raylibInstance, "KEY_SLASH", 9, NUMBER_VAL(KEY_SLASH));
  defineNativeInstanceField(raylibInstance, "KEY_ZERO", 8, NUMBER_VAL(KEY_ZERO));
  defineNativeInstanceField(raylibInstance, "KEY_ONE", 7, NUMBER_VAL(KEY_ONE));
  defineNativeInstanceField(raylibInstance, "KEY_TWO", 7, NUMBER_VAL(KEY_TWO));
  defineNativeInstanceField(raylibInstance, "KEY_THREE", 9, NUMBER_VAL(KEY_THREE));
  defineNativeInstanceField(raylibInstance, "KEY_FOUR", 8, NUMBER_VAL(KEY_FOUR));
  defineNativeInstanceField(raylibInstance, "KEY_FIVE", 8, NUMBER_VAL(KEY_FIVE));
  defineNativeInstanceField(raylibInstance, "KEY_SIX", 7, NUMBER_VAL(KEY_SIX));
  defineNativeInstanceField(raylibInstance, "KEY_SEVEN", 9, NUMBER_VAL(KEY_SEVEN));
  defineNativeInstanceField(raylibInstance, "KEY_EIGHT", 9, NUMBER_VAL(KEY_EIGHT));
  defineNativeInstanceField(raylibInstance, "KEY_NINE", 8, NUMBER_VAL(KEY_NINE));
  defineNativeInstanceField(raylibInstance, "KEY_SEMICOLON", 13, NUMBER_VAL(KEY_SEMICOLON));
  defineNativeInstanceField(raylibInstance, "KEY_EQUAL", 9, NUMBER_VAL(KEY_EQUAL));
  defineNativeInstanceField(raylibInstance, "KEY_A", 5, NUMBER_VAL(KEY_A));
  defineNativeInstanceField(raylibInstance, "KEY_B", 5, NUMBER_VAL(KEY_B));
  defineNativeInstanceField(raylibInstance, "KEY_C", 5, NUMBER_VAL(KEY_C));
  defineNativeInstanceField(raylibInstance, "KEY_D", 5, NUMBER_VAL(KEY_D));
  defineNativeInstanceField(raylibInstance, "KEY_E", 5, NUMBER_VAL(KEY_E));
  defineNativeInstanceField(raylibInstance, "KEY_F", 5, NUMBER_VAL(KEY_F));
  defineNativeInstanceField(raylibInstance, "KEY_G", 5, NUMBER_VAL(KEY_G));
  defineNativeInstanceField(raylibInstance, "KEY_H", 5, NUMBER_VAL(KEY_H));
  defineNativeInstanceField(raylibInstance, "KEY_I", 5, NUMBER_VAL(KEY_I));
  defineNativeInstanceField(raylibInstance, "KEY_J", 5, NUMBER_VAL(KEY_J));
  defineNativeInstanceField(raylibInstance, "KEY_K", 5, NUMBER_VAL(KEY_K));
  defineNativeInstanceField(raylibInstance, "KEY_L", 5, NUMBER_VAL(KEY_L));
  defineNativeInstanceField(raylibInstance, "KEY_M", 5, NUMBER_VAL(KEY_M));
  defineNativeInstanceField(raylibInstance, "KEY_N", 5, NUMBER_VAL(KEY_N));
  defineNativeInstanceField(raylibInstance, "KEY_O", 5, NUMBER_VAL(KEY_O));
  defineNativeInstanceField(raylibInstance, "KEY_P", 5, NUMBER_VAL(KEY_P));
  defineNativeInstanceField(raylibInstance, "KEY_Q", 5, NUMBER_VAL(KEY_Q));
  defineNativeInstanceField(raylibInstance, "KEY_R", 5, NUMBER_VAL(KEY_R));
  defineNativeInstanceField(raylibInstance, "KEY_S", 5, NUMBER_VAL(KEY_S));
  defineNativeInstanceField(raylibInstance, "KEY_T", 5, NUMBER_VAL(KEY_T));
  defineNativeInstanceField(raylibInstance, "KEY_U", 5, NUMBER_VAL(KEY_U));
  defineNativeInstanceField(raylibInstance, "KEY_V", 5, NUMBER_VAL(KEY_V));
  defineNativeInstanceField(raylibInstance, "KEY_W", 5, NUMBER_VAL(KEY_W));
  defineNativeInstanceField(raylibInstance, "KEY_X", 5, NUMBER_VAL(KEY_X));
  defineNativeInstanceField(raylibInstance, "KEY_Y", 5, NUMBER_VAL(KEY_Y));
  defineNativeInstanceField(raylibInstance, "KEY_Z", 5, NUMBER_VAL(KEY_Z));
  defineNativeInstanceField(raylibInstance, "KEY_LEFT_BRACKET", 16, NUMBER_VAL(KEY_LEFT_BRACKET));
  defineNativeInstanceField(raylibInstance, "KEY_BACKSLASH", 13, NUMBER_VAL(KEY_BACKSLASH));
  defineNativeInstanceField(raylibInstance, "KEY_RIGHT_BRACKET", 17, NUMBER_VAL(KEY_RIGHT_BRACKET));
  defineNativeInstanceField(raylibInstance, "KEY_GRAVE", 9, NUMBER_VAL(KEY_GRAVE));
  defineNativeInstanceField(raylibInstance, "KEY_SPACE", 9, NUMBER_VAL(KEY_SPACE));
  defineNativeInstanceField(raylibInstance, "KEY_ESCAPE", 10, NUMBER_VAL(KEY_ESCAPE));
  defineNativeInstanceField(raylibInstance, "KEY_ENTER", 9, NUMBER_VAL(KEY_ENTER));
  defineNativeInstanceField(raylibInstance, "KEY_TAB", 7, NUMBER_VAL(KEY_TAB));
  defineNativeInstanceField(raylibInstance, "KEY_BACKSPACE", 13, NUMBER_VAL(KEY_BACKSPACE));
  defineNativeInstanceField(raylibInstance, "KEY_INSERT", 10, NUMBER_VAL(KEY_INSERT));
  defineNativeInstanceField(raylibInstance, "KEY_DELETE", 10, NUMBER_VAL(KEY_DELETE));
  defineNativeInstanceField(raylibInstance, "KEY_RIGHT", 9, NUMBER_VAL(KEY_RIGHT));
  defineNativeInstanceField(raylibInstance, "KEY_LEFT", 8, NUMBER_VAL(KEY_LEFT));
  defineNativeInstanceField(raylibInstance, "KEY_DOWN", 8, NUMBER_VAL(KEY_DOWN));
  defineNativeInstanceField(raylibInstance, "KEY_UP", 6, NUMBER_VAL(KEY_UP));
  
  // mouse buttons
  defineNativeInstanceField(raylibInstance, "MOUSE_BUTTON_LEFT", 17, NUMBER_VAL(MOUSE_BUTTON_LEFT));
  defineNativeInstanceField(raylibInstance, "MOUSE_BUTTON_RIGHT", 18, NUMBER_VAL(MOUSE_BUTTON_RIGHT));
  defineNativeInstanceField(raylibInstance, "MOUSE_BUTTON_MIDDLE", 19, NUMBER_VAL(MOUSE_BUTTON_MIDDLE));
  defineNativeInstanceField(raylibInstance, "MOUSE_BUTTON_SIDE", 17, NUMBER_VAL(MOUSE_BUTTON_SIDE));
  defineNativeInstanceField(raylibInstance, "MOUSE_BUTTON_EXTRA", 18, NUMBER_VAL(MOUSE_BUTTON_EXTRA));
  defineNativeInstanceField(raylibInstance, "MOUSE_BUTTON_FORWARD", 20, NUMBER_VAL(MOUSE_BUTTON_FORWARD));
  defineNativeInstanceField(raylibInstance, "MOUSE_BUTTON_BACK", 17, NUMBER_VAL(MOUSE_BUTTON_BACK));
  
  // mouse cursor
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_DEFAULT", 20, NUMBER_VAL(MOUSE_CURSOR_DEFAULT));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_ARROW", 18, NUMBER_VAL(MOUSE_CURSOR_ARROW));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_IBEAM", 18, NUMBER_VAL(MOUSE_CURSOR_IBEAM));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_CROSSHAIR", 22, NUMBER_VAL(MOUSE_CURSOR_CROSSHAIR));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_POINTING_HAND", 26, NUMBER_VAL(MOUSE_CURSOR_POINTING_HAND));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_RESIZE_EW", 22, NUMBER_VAL(MOUSE_CURSOR_RESIZE_EW));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_RESIZE_NS", 22, NUMBER_VAL(MOUSE_CURSOR_RESIZE_NS));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_RESIZE_NWSE", 24, NUMBER_VAL(MOUSE_CURSOR_RESIZE_NWSE));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_RESIZE_NESW", 24, NUMBER_VAL(MOUSE_CURSOR_RESIZE_NESW));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_RESIZE_ALL", 23, NUMBER_VAL(MOUSE_CURSOR_RESIZE_ALL));
  defineNativeInstanceField(raylibInstance, "MOUSE_CURSOR_NOT_ALLOWED", 24, NUMBER_VAL(MOUSE_CURSOR_NOT_ALLOWED));
}

