#ifndef __WIDGETMANAGER_H__
#define __WIDGETMANAGER_H__

#include "Common.h"
#include "KeyCodes.h"
#include "Event.h"
#include "Widget.h"

namespace Sexy
{

class Widget;
class Image;
class MemoryImage;
class SexyAppBase;
class Graphics;

typedef std::list<Widget*> WidgetList;

enum
{
	WIDGETFLAGS_UPDATE			= 1,
	WIDGETFLAGS_MARK_DIRTY		= 2,
	WIDGETFLAGS_DRAW			= 4,
	WIDGETFLAGS_CLIP			= 8,	
	WIDGETFLAGS_ALLOW_MOUSE		= 16,
	WIDGETFLAGS_ALLOW_FOCUS		= 32,	
};

class PreModalInfo
{
public:
	Widget*					mBaseModalWidget;
	Widget*					mPrevBaseModalWidget;
	Widget*					mPrevFocusWidget;
	FlagsMod				mPrevBelowModalFlagsMod;
};

typedef std::list<PreModalInfo> PreModalInfoList;

typedef std::vector<std::pair<Widget*, int> > DeferredOverlayVector;
typedef std::vector<Event> EventVector;

struct TouchInfo {
	unsigned int timestamp;
	bool         down;
	int          tapCount;
	int          x;
	int          y;

	TouchInfo()
	{
		reset();
	}

	void reset()
	{
		down = false;
		timestamp = 0;
		tapCount = 0;
		x = 0;
		y = 0;
	}
};
typedef std::map<int, TouchInfo> TouchInfoMap;

class WidgetManager : public WidgetContainer
{
public:	
	Widget*					mDefaultTab;
	
	Graphics*				mCurG;
	SexyAppBase*			mApp;
	MemoryImage*			mImage;	
	MemoryImage*			mTransientImage;
	bool					mLastHadTransients;	
	Widget*					mPopupCommandWidget;	
	DeferredOverlayVector	mDeferredOverlayWidgets;
	int						mMinDeferredOverlayPriority;
	
	bool					mHasFocus;
	Widget*					mFocusWidget;
	Widget*					mLastDownWidget;
	Widget*					mOverWidget;
	Widget*					mBaseModalWidget;
	FlagsMod				mLostFocusFlagsMod;
	FlagsMod				mBelowModalFlagsMod;
	FlagsMod				mDefaultBelowModalFlagsMod;
	PreModalInfoList		mPreModalInfoList;
	Rect					mMouseDestRect;
	Rect					mMouseSourceRect;
	bool					mMouseIn;
	int						mLastMouseX;
	int						mLastMouseY;
	int						mDownButtons;
	int						mActualDownButtons;
	int						mLastInputUpdateCnt;
	
	bool					mKeyDown[0xFF];
	int						mLastDownButtonId;	
	
	TouchInfoMap                            mTouchInfoMap;
	TouchVector                             mLastTouch;
	int                                     mActiveTouchId;

	int						mWidgetFlags;

protected:
	int						GetWidgetFlags();
	void					MouseEnter(Widget* theWidget);
	void					MouseLeave(Widget* theWidget);
	void					TouchEnter(Widget* theWidget);
	void					TouchLeave(Widget* theWidget);

protected:
	void					SetBaseModal(Widget* theWidget, const FlagsMod& theBelowFlagsMod);

public:
	WidgetManager(SexyAppBase* theApplet);
	virtual ~WidgetManager();
	
	void					FreeResources();		
	void					AddBaseModal(Widget* theWidget, const FlagsMod& theBelowFlagsMod);
	void					AddBaseModal(Widget* theWidget);
	void					RemoveBaseModal(Widget* theWidget);
	void					Resize(const Rect& theMouseDestRect, const Rect& theMouseSourceRect);
	void					DisableWidget(Widget* theWidget);	
	Widget*					GetAnyWidgetAt(int x, int y, int* theWidgetX, int* theWidgetY);
	Widget*					GetWidgetAt(int x, int y, int* theWidgetX, int* theWidgetY);
	void					SetFocus(Widget* aWidget);
	void					GotFocus();	
	void					LostFocus();	
	void					InitModalFlags(ModalFlags* theModalFlags);
	void					DrawWidgetsTo(Graphics* g);
	void					DoMouseUps(Widget* theWidget, ulong theDownCode);	
	void					DoMouseUps();
	void					DeferOverlay(Widget* theWidget, int thePriority);
	void					FlushDeferredOverlayWidgets(int theMaxPriority);
	
	bool					DrawScreen();
	bool					UpdateFrame();				
	bool					UpdateFrameF(float theFrac);
	void					SetPopupCommandWidget(Widget* theList);
	void					RemovePopupCommandWidget();	
	void					MousePosition(int x, int y);	
	void					RehupMouse();
	void					RemapMouse(int& theX, int& theY);
	void					UnmapMouse(int& theX, int& theY);
	bool					MouseUp(int x, int y, int theClickCount);
	bool					MouseDown(int x, int y, int theClickCount);
	bool					MouseMove(int x, int y);
	bool					MouseDrag(int x, int y);
	bool					MouseExit(int x, int y);
	void					MouseWheel(int theDelta);
	bool					KeyChar(SexyChar theChar);
	bool					KeyDown(KeyCode key);
	bool					KeyUp(KeyCode key);
	bool                                    TouchDown(const EventVector &events);
	bool                                    TouchMove(const EventVector &events);
	bool                                    TouchUp(const EventVector &events);
	bool                                    TouchCancel(const EventVector &events);
	bool                                    UserEvent(const Event event);

	bool					IsLeftButtonDown();
	bool					IsMiddleButtonDown();
	bool					IsRightButtonDown();

private:
	TouchInfo*                              GetTouchInfo(int id);
};

}

#endif // __WIDGETMANAGER_H__
