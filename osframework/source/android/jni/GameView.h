#ifndef __GAME_VIEW_H__
#define __GAME_VIEW_H__

#ifdef __cplusplus
extern "C" {
#endif

  enum AGKeyCode {
    // key codes
    AG_KEY_UNKNOWN         = 0,
    AG_KEY_SOFT_LEFT       = 1,
    AG_KEY_SOFT_RIGHT      = 2,
    AG_KEY_HOME            = 3,
    AG_KEY_BACK            = 4,
    AG_KEY_CALL            = 5,
    AG_KEY_ENDCALL         = 6,
    AG_KEY_0               = 7,
    AG_KEY_1               = 8,
    AG_KEY_2               = 9,
    AG_KEY_3               = 10,
    AG_KEY_4               = 11,
    AG_KEY_5               = 12,
    AG_KEY_6               = 13,
    AG_KEY_7               = 14,
    AG_KEY_8               = 15,
    AG_KEY_9               = 16,
    AG_KEY_STAR            = 17,
    AG_KEY_POUND           = 18,
    AG_KEY_DPAD_UP         = 19,
    AG_KEY_DPAD_DOWN       = 20,
    AG_KEY_DPAD_LEFT       = 21,
    AG_KEY_DPAD_RIGHT      = 22,
    AG_KEY_DPAD_CENTER     = 23,
    AG_KEY_VOLUME_UP       = 24,
    AG_KEY_VOLUME_DOWN     = 25,
    AG_KEY_POWER           = 26,
    AG_KEY_CAMERA          = 27,
    AG_KEY_CLEAR           = 28,
    AG_KEY_A               = 29,
    AG_KEY_B               = 30,
    AG_KEY_C               = 31,
    AG_KEY_D               = 32,
    AG_KEY_E               = 33,
    AG_KEY_F               = 34,
    AG_KEY_G               = 35,
    AG_KEY_H               = 36,
    AG_KEY_I               = 37,
    AG_KEY_J               = 38,
    AG_KEY_K               = 39,
    AG_KEY_L               = 40,
    AG_KEY_M               = 41,
    AG_KEY_N               = 42,
    AG_KEY_O               = 43,
    AG_KEY_P               = 44,
    AG_KEY_Q               = 45,
    AG_KEY_R               = 46,
    AG_KEY_S               = 47,
    AG_KEY_T               = 48,
    AG_KEY_U               = 49,
    AG_KEY_V               = 50,
    AG_KEY_W               = 51,
    AG_KEY_X               = 52,
    AG_KEY_Y               = 53,
    AG_KEY_Z               = 54,
    AG_KEY_COMMA           = 55,
    AG_KEY_PERIOD          = 56,
    AG_KEY_ALT_LEFT        = 57,
    AG_KEY_ALT_RIGHT       = 58,
    AG_KEY_SHIFT_LEFT      = 59,
    AG_KEY_SHIFT_RIGHT     = 60,
    AG_KEY_TAB             = 61,
    AG_KEY_SPACE           = 62,
    AG_KEY_SYM             = 63,
    AG_KEY_EXPLORER        = 64,
    AG_KEY_ENVELOPE        = 65,
    AG_KEY_ENTER           = 66,
    AG_KEY_DEL             = 67,
    AG_KEY_GRAVE           = 68,
    AG_KEY_MINUS           = 69,
    AG_KEY_EQUALS          = 70,
    AG_KEY_LEFT_BRACKET    = 71,
    AG_KEY_RIGHT_BRACKET   = 72,
    AG_KEY_BACKSLASH       = 73,
    AG_KEY_SEMICOLON       = 74,
    AG_KEY_APOSTROPHE      = 75,
    AG_KEY_SLASH           = 76,
    AG_KEY_AT              = 77,
    AG_KEY_NUM             = 78,
    AG_KEY_HEADSETHOOK     = 79,
    AG_KEY_FOCUS           = 80,   // *Camera* focus
    AG_KEY_PLUS            = 81,
    AG_KEY_MENU            = 82,
    AG_KEY_NOTIFICATION    = 83,
    AG_KEY_SEARCH          = 84,
    AG_KEY_MEDIA_PLAY_PAUSE= 85,
    AG_KEY_MEDIA_STOP      = 86,
    AG_KEY_MEDIA_NEXT      = 87,
    AG_KEY_MEDIA_PREVIOUS  = 88,
    AG_KEY_MEDIA_REWIND    = 89,
    AG_KEY_MEDIA_FAST_FORWARD = 90,
    AG_KEY_MUTE            = 91,
    AG_KEY_PAGE_UP         = 92,
    AG_KEY_PAGE_DOWN       = 93,
    AG_KEY_PICTSYMBOLS     = 94,   // switch symbol-sets (Emoji,Kao-moji)
    AG_KEY_SWITCH_CHARSET  = 95   // switch char-sets (Kanji,Katakana)
  };

  enum AGEventType {
    AG_KEY_DOWN_EVENT        = 0,
    AG_KEY_UP_EVENT          = 1,
    AG_POINTER_DOWN_EVENT    = 2,
    AG_POINTER_MOVE_EVENT    = 3,
    AG_POINTER_UP_EVENT      = 4,
    AG_POINTER_CANCEL_EVENT  = 5
  };

  enum AGEventFlags {
    AG_EVENT_FLAG_NONE       = 0,
    AG_EVENT_FOLLOW          = 1 << 0,
    AG_EVENT_STATE_CHANGED   = 1 << 1
  };

  struct AGKeyEvent {
    int keyCode;
    int keyChar;
  };

  struct AGPointerEvent {
    int id;
    int flags;
    float x;
    float y;
    float pressure;
  };

  struct AGSensorEvent {
    int   id;
    int   nvaluse;
    float values[10];
  };

  struct AGEvent {
    int type;
    int flags;
    int timestamp;
    union {
      struct AGKeyEvent key;
      struct AGPointerEvent pointer;
      struct AGSensorEvent sensor;
    } u;
  };

  typedef void (*AGEventListener)(const struct AGEvent* event,
				  void*                 data);

int
AGViewGetSize(int *width, int *height);

void
AGViewAddEventListener(AGEventListener listener,
		       void* data);

void
AGViewSwapBuffers(void);

void
AGViewUpdate(void);

#ifdef __cplusplus
}
#endif

#endif
