#ifndef __NATIVEDISPLAY_H__
#define __NATIVEDISPLAY_H__
#include "Common.h"
#include "Rect.h"
#include "CritSect.h"

namespace Sexy
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class Font;
class Image;
class SexyAppBase;
class Graphics;

enum EventType {
    EVENT_NONE,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_MOUSE_BUTTON_PRESS,
    EVENT_MOUSE_BUTTON_RELEASE,
    EVENT_MOUSE_WHEEL_UP,
    EVENT_MOUSE_WHELL_DOWN,
    EVENT_MOUSE_MOTION,
    EVENT_ACTIVE,
    EVENT_EXPOSE,
    EVENT_QUIT
};

#define EVENT_FLAGS_AXIS     (1U << 0)
#define EVENT_FLAGS_REL_AXIS (1U << 1)
#define EVENT_FLAGS_BUTTON   (1U << 2)
#define EVENT_FLAGS_KEY_CODE (1U << 3)
#define EVENT_FLAGS_KEY_CHAR (1U << 4)

struct Event {
    enum EventType type;
    unsigned int   flags;
    int            keyCode;
    int            keyChar;
    int            x;
    int            y;
    int            button;
    bool           active;
};

class NativeDisplay
{
public:
	CritSect				mCritSect;

	int						mRGBBits;
	ulong					mRedMask;
	ulong					mGreenMask;
	ulong					mBlueMask;
	int						mRedBits;
	int						mGreenBits;
	int						mBlueBits;
	int						mRedShift;
	int						mGreenShift;
	int						mBlueShift;

        int                                        mCursorX;
        int                                        mCursorY;

public:
	NativeDisplay();
	virtual ~NativeDisplay();

 public:
        virtual int                                Init();
        virtual Font *                             CreateFont(SexyAppBase * theApp,
                                                              const std::string theFace,
                                                              int thePointSize,
                                                              bool bold = false,
                                                              bool italics = false,
                                                              bool underline = false);
        virtual Image *                             CreateImage(SexyAppBase * theApp,
                                                                int width, int height);

	virtual Image*			            GetScreenImage() = 0;
	virtual bool				    Redraw(Rect* theClipRect = 0) = 0;
	virtual void				    RemapMouse(int& theX, int& theY) = 0;

        virtual bool                                UpdateCursor(int theCursorX, int theCursorY);
        virtual bool                                DrawCursor(Graphics* g);

        virtual bool                                EnableCursor(bool enable);
	virtual bool				    SetCursorImage(Image* theImage, int theHotX = 0, int theHotY = 0);
	virtual void				    SetCursorPos(int theCursorX, int theCursorY);

 public:
        virtual bool                                HasEvent();
        virtual bool                                GetEvent(struct Event & event);
};

};


#endif
