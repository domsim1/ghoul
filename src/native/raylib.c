#include "common_native.h"

#include <raylib.h>

static ObjKlass *colorKlassRef;
static ObjKlass *vector2KlassRef;
static ObjKlass *imgKlassRef;
static ObjKlass *camera2dRef;

static Value createVector2(double x, double y) {
  ObjInstance *vector2 = newInstance(vector2KlassRef);
  push(OBJ_VAL(vector2));   
  setNativeInstanceField(vector2, vm.string.x, NUMBER_VAL(x));
  setNativeInstanceField(vector2, vm.string.y, NUMBER_VAL(y));
  return pop();
}

static Value createCamera2d(ObjInstance *offset, ObjInstance *target, double rotation, double zoom) {
  ObjInstance *camera2d = newInstance(camera2dRef);
  push(OBJ_VAL(camera2d));
  defineNativeInstanceField(camera2d, "offset", 6, OBJ_VAL(offset));
  defineNativeInstanceField(camera2d, "target", 6, OBJ_VAL(target));
  defineNativeInstanceField(camera2d, "rotation", 8, NUMBER_VAL(rotation));
  defineNativeInstanceField(camera2d, "zoom", 4, NUMBER_VAL(zoom));
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

static Value getClipboardImageRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 1, args, NATIVE_NORMAL, ARG_ANY)) {
    return NIL_VAL;
  }  
  return createImage(GetClipboardImage());
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
  IsCursorHidden();
  return NIL_VAL;
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
  return BOOL_VAL(IsCursorOnScreen);
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

static Value loadImageRLNative(int argCount, Value *args) { 
  if (!checkArgs(argCount, 2, args, NATIVE_NORMAL, ARG_ANY, ARG_STRING)) {
    return NIL_VAL;
  };
  return createImage(LoadImage(AS_CSTRING(args[1])));
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

  ObjKlass *camera2dKlass = newKlass(copyString("Camera2D", 8, &vm.strings), OBJ_INSTANCE);
  push(OBJ_VAL(camera2dKlass));
  camera2dRef = camera2dKlass;
  defineNativeInstanceField(raylibInstance, "Camera2D", 8, pop());

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
  defineNativeInstanceMethod(raylibInstance, "get_clipboard_image", 19, getClipboardImageRLNative);  
  defineNativeInstanceMethod(raylibInstance, "enable_event_waiting", 20, enableEventWaitingRLNative);
  defineNativeInstanceMethod(raylibInstance, "disable_event_waiting", 21, disableEventWaitingRLNative);

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
}

