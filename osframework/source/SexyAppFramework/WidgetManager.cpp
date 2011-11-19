#include "WidgetManager.h"
#include "Widget.h"
#include "Graphics.h"
#include "Image.h"
#include "KeyCodes.h"
#include "MemoryImage.h"
#include "SexyAppBase.h"
#include "PerfTimer.h"
#include "Debug.h"

#include <math.h>

using namespace Sexy;
using namespace std;

WidgetManager::WidgetManager(SexyAppBase* theApp)
{
	mApp = theApp;

	mMinDeferredOverlayPriority = 0x7FFFFFFF;
	mWidgetManager = this;
	mMouseIn = false;
	mDefaultTab = NULL;
	mImage = NULL;
	mLastHadTransients = false;
	mPopupCommandWidget = NULL;
	mFocusWidget = NULL;
	mLastDownWidget = NULL;
	mOverWidget = NULL;
	mBaseModalWidget = NULL;
	mDefaultBelowModalFlagsMod.mRemoveFlags = WIDGETFLAGS_ALLOW_MOUSE | WIDGETFLAGS_ALLOW_FOCUS;
	mWidth = 0;
	mHeight = 0;
	mHasFocus = true;
	mUpdateCnt = 0;
	mLastDownButtonId = 0;
	mDownButtons = 0;
	mActualDownButtons = 0;
	mLastMouseX = 0;
	mLastMouseY = 0;
	mActiveTouchId = -1;

	mWidgetFlags = WIDGETFLAGS_UPDATE | WIDGETFLAGS_DRAW | WIDGETFLAGS_CLIP |
		WIDGETFLAGS_ALLOW_MOUSE | WIDGETFLAGS_ALLOW_FOCUS;

	for (int i = 0; i < 0xFF; i++)
		mKeyDown[i] = false;
}

WidgetManager::~WidgetManager()
{
	FreeResources();
}

void WidgetManager::InitGamepadKeyMap()
{
	mGamepadKeyMap[KEYCODE_GAMEPAD_A] = KEYCODE_RETURN;
	mGamepadKeyMap[KEYCODE_GAMEPAD_B] = KEYCODE_ESCAPE;
}

void WidgetManager::FreeResources()
{

}

void WidgetManager::DisableWidget(Widget* theWidget)
{
	if (mOverWidget == theWidget)
	{
		Widget* aOverWidget = mOverWidget;
		mOverWidget = NULL;
		MouseLeave(aOverWidget);
	}

	if (mLastDownWidget == theWidget)
	{
		Widget* aLastDownWidget = mLastDownWidget;
		mLastDownWidget = NULL;
		DoMouseUps(aLastDownWidget, mDownButtons);
		mDownButtons = 0;
	}

	if (mFocusWidget == theWidget)
	{
		Widget* aFocusWidget = mFocusWidget;
		mFocusWidget = NULL;
		aFocusWidget->LostFocus();
	}
	else if (theWidget->mHasFocus)
	{
		theWidget->LostFocus();
	}

	if (mBaseModalWidget == theWidget)
		mBaseModalWidget = NULL;
}

int WidgetManager::GetWidgetFlags()
{
	return mHasFocus ? mWidgetFlags : GetModFlags(mWidgetFlags, mLostFocusFlagsMod);
}

Widget* WidgetManager::GetAnyWidgetAt(int x, int y, int* theWidgetX, int* theWidgetY)
{
	bool found;
	return GetWidgetAtHelper(x, y, GetWidgetFlags(), &found, theWidgetX, theWidgetY);
}

Widget* WidgetManager::GetWidgetAt(int x, int y, int* theWidgetX, int* theWidgetY)
{
	Widget* aWidget = GetAnyWidgetAt(x, y, theWidgetX, theWidgetY);
	if ((aWidget != NULL) && (aWidget->mDisabled))
		aWidget = NULL;
	return aWidget;
}

bool WidgetManager::IsLeftButtonDown()
{
	return (mActualDownButtons&1)?true:false;
}

bool WidgetManager::IsMiddleButtonDown()
{
	return (mActualDownButtons&4)?true:false;
}

bool WidgetManager::IsRightButtonDown()
{
	return (mActualDownButtons&2)?true:false;
}

void WidgetManager::DoMouseUps()
{
	if (mLastDownWidget!=NULL && mDownButtons!=0)
	{
		DoMouseUps(mLastDownWidget, mDownButtons);
		mDownButtons = 0;
		mLastDownWidget = NULL;
	}
}

void WidgetManager::DeferOverlay(Widget* theWidget, int thePriority)
{
	mDeferredOverlayWidgets.push_back(std::pair<Widget*, int>(theWidget, thePriority));
	if (thePriority < mMinDeferredOverlayPriority)
		mMinDeferredOverlayPriority = thePriority;
}

void WidgetManager::FlushDeferredOverlayWidgets(int theMaxPriority)
{
	for (;;)
	{
		int aNextMinPriority = 0x7FFFFFFF;

		for (int i = 0; i < (int) mDeferredOverlayWidgets.size(); i++)
		{
			Widget* aWidget = mDeferredOverlayWidgets[i].first;
			if (aWidget != NULL)
			{
				int aPriority = mDeferredOverlayWidgets[i].second;

				if (aPriority == mMinDeferredOverlayPriority)
				{
					// Overlays don't get clipped
					Graphics g(*mCurG);
					g.Translate(-mMouseDestRect.mX, -mMouseDestRect.mY);
					g.Translate(aWidget->mX, aWidget->mY);
					g.SetFastStretch(!g.Is3D());
					g.SetLinearBlend(g.Is3D());

					aWidget->DrawOverlay(&g, aPriority);
					mDeferredOverlayWidgets[i].first = NULL;
				}
				else
				{
					if (aPriority < aNextMinPriority)
						aNextMinPriority = aPriority;
				}
			}
		}

		mMinDeferredOverlayPriority = aNextMinPriority;
		if (aNextMinPriority == 0x7FFFFFFF)
		{
			// No more widgets lined up for overlays, clear our vector
			mDeferredOverlayWidgets.resize(0);
			break;
		}

		// Lowest overlay priority is higher or equal to our current widget,
		//  so continue deferring
		if (aNextMinPriority >= theMaxPriority)
			break;
	}
}

void WidgetManager::DoMouseUps(Widget* theWidget, ulong theDownCode)
{
	int aClickCountTable[3] = { 1,-1, 3 };
	for (int i = 0; i < 3; i++)
	{
		if ((theDownCode & (1 << i)) != 0)
		{
			theWidget->mIsDown = false;
			theWidget->MouseUp(mLastMouseX - theWidget->mX, mLastMouseY - theWidget->mY, aClickCountTable[i]);
		}
	}
}

void WidgetManager::RemapMouse(int& theX, int& theY)
{
	theX = (theX - mMouseSourceRect.mX) * mMouseDestRect.mWidth / mMouseSourceRect.mWidth + mMouseDestRect.mX;
	theY = (theY - mMouseSourceRect.mY) * mMouseDestRect.mHeight / mMouseSourceRect.mHeight + mMouseDestRect.mY;
}

void WidgetManager::UnmapMouse(int& theX, int& theY)
{
	theX = (theX - mMouseDestRect.mX) * mMouseSourceRect.mWidth / mMouseDestRect.mWidth + mMouseSourceRect.mX;
	theY = (theY - mMouseDestRect.mY) * mMouseSourceRect.mHeight / mMouseDestRect.mHeight + mMouseSourceRect.mY;
}

void WidgetManager::MouseEnter(Widget* theWidget)
{
	theWidget->mIsOver = true;

	theWidget->MouseEnter();
	if (theWidget->mDoFinger)
		theWidget->ShowFinger(true);
}

void WidgetManager::MouseLeave(Widget* theWidget)
{
	theWidget->mIsOver = false;

	theWidget->MouseLeave();
	if (theWidget->mDoFinger)
		theWidget->ShowFinger(false);
}

void WidgetManager::SetBaseModal(Widget* theWidget, const FlagsMod& theBelowFlagsMod)
{
	mBaseModalWidget = theWidget;
	mBelowModalFlagsMod = theBelowFlagsMod;

	if ((mOverWidget != NULL) && (mBelowModalFlagsMod.mRemoveFlags & WIDGETFLAGS_ALLOW_MOUSE) &&
		(IsBelow(mOverWidget, mBaseModalWidget)))
	{
		Widget* aWidget = mOverWidget;
		mOverWidget = NULL;
		MouseLeave(aWidget);
	}

	if ((mLastDownWidget != NULL) && (mBelowModalFlagsMod.mRemoveFlags & WIDGETFLAGS_ALLOW_MOUSE) &&
		(IsBelow(mLastDownWidget, mBaseModalWidget)))
	{
		Widget* aWidget = mLastDownWidget;
		int aDownButtons = mDownButtons;
		mDownButtons = 0;
		mLastDownWidget = NULL;
		DoMouseUps(aWidget, aDownButtons);
	}

	if ((mFocusWidget != NULL) && (mBelowModalFlagsMod.mRemoveFlags & WIDGETFLAGS_ALLOW_FOCUS) &&
		(IsBelow(mFocusWidget, mBaseModalWidget)))
	{
		Widget* aWidget = mFocusWidget;
		mFocusWidget = NULL;
		aWidget->LostFocus();
	}
}

void WidgetManager::AddBaseModal(Widget* theWidget, const FlagsMod& theBelowFlagsMod)
{
	PreModalInfo aPreModalInfo;
	aPreModalInfo.mBaseModalWidget = theWidget;
	aPreModalInfo.mPrevBaseModalWidget = mBaseModalWidget;
	aPreModalInfo.mPrevFocusWidget = mFocusWidget;
	aPreModalInfo.mPrevBelowModalFlagsMod = mBelowModalFlagsMod;
	mPreModalInfoList.push_back(aPreModalInfo);

	SetBaseModal(theWidget, theBelowFlagsMod);
}

void WidgetManager::AddBaseModal(Widget* theWidget)
{
	AddBaseModal(theWidget, mDefaultBelowModalFlagsMod);
}

void WidgetManager::RemoveBaseModal(Widget* theWidget)
{
	DBG_ASSERT(mPreModalInfoList.size() > 0);

	bool first = true;

	while (mPreModalInfoList.size() > 0)
	{
		PreModalInfo* aPreModalInfo = &mPreModalInfoList.back();

		if ((first) && (aPreModalInfo->mBaseModalWidget != theWidget))
		{
			// We don't remove it yet, because we want to restore
			//  its keyboard focused widget and crap later
			return;
		}

		// If we removed a widget's self from pre-modal info before
		//  then that means the dialog got removed out-of-order but we
		//  deferred setting the state back until now
		bool done = (aPreModalInfo->mPrevBaseModalWidget != NULL) ||
			(mPreModalInfoList.size() == 1);

		SetBaseModal(aPreModalInfo->mPrevBaseModalWidget,
			aPreModalInfo->mPrevBelowModalFlagsMod);

		if (mFocusWidget == NULL)
		{
			mFocusWidget = aPreModalInfo->mPrevFocusWidget;
			if (mFocusWidget != NULL)
				mFocusWidget->GotFocus();
		}

		mPreModalInfoList.pop_back();

		if (done)
			break;

		first = false;
	}
}

void WidgetManager::Resize(const Rect& theMouseDestRect, const Rect& theMouseSourceRect)
{
	mWidth = theMouseDestRect.mWidth + 2 * theMouseDestRect.mX;
	mHeight = theMouseDestRect.mHeight + 2 * theMouseDestRect.mY;
	mMouseDestRect = theMouseDestRect;
	mMouseSourceRect = theMouseSourceRect;
}

void WidgetManager::SetFocus(Widget* aWidget)
{
	if (aWidget==mFocusWidget)
		return;

	if ((aWidget != NULL) && (aWidget->mWidgetManager == this))
	{
		Widget* OldFocusWidget = mFocusWidget;
		Widget* aFocusWidget = aWidget;

		// set the focus widget to top level widget
		while (aFocusWidget->mParent != this)
			aFocusWidget = (Widget*)aFocusWidget->mParent;
		mFocusWidget = aFocusWidget;

		if (OldFocusWidget != NULL)
			OldFocusWidget->LostFocus();

		if ((mHasFocus) && (mFocusWidget != NULL))
		{
			// Let the top level widget handle the focus
			if (aFocusWidget == aWidget)
				mFocusWidget->GotFocus();
			else
				mFocusWidget->SetFocus(aWidget);
		}
	}
	else
	{
		if (mFocusWidget != NULL)
			mFocusWidget->LostFocus();
		mFocusWidget = NULL;
	}
}

void WidgetManager::GotFocus()
{
	if (!mHasFocus)
	{
		mHasFocus = true;

		if (mFocusWidget != NULL)
			mFocusWidget->GotFocus();
	}
}

void WidgetManager::LostFocus()
{
	if (mHasFocus)
	{
		mActualDownButtons = 0;
		for (int aKeyNum = 0; aKeyNum < 0xFF; aKeyNum++)
		{
			if (mKeyDown[aKeyNum])
				KeyUp((KeyCode) aKeyNum);
		}

		mHasFocus = false;

		if (mFocusWidget != NULL)
			mFocusWidget->LostFocus();
	}
}

void WidgetManager::InitModalFlags(ModalFlags* theModalFlags)
{
	theModalFlags->mIsOver = mBaseModalWidget == NULL;
	theModalFlags->mOverFlags = GetWidgetFlags();
	theModalFlags->mUnderFlags = GetModFlags(theModalFlags->mOverFlags, mBelowModalFlagsMod);
}

void WidgetManager::DrawWidgetsTo(Graphics* g)
{
	mCurG = g;

	ModalFlags aModalFlags;
	InitModalFlags(&aModalFlags);

	WidgetList::iterator anItr = mWidgets.begin();
	while (anItr != mWidgets.end())
	{
		Widget* aWidget = *anItr;

		if (aWidget->mVisible)
		{
			Graphics aClipG(*g);
			aClipG.SetFastStretch(true);
			aClipG.Translate(aWidget->mX, aWidget->mY);
			aWidget->DrawAll(&aModalFlags, &aClipG);
		}

		++anItr;
	}

	mCurG = NULL;
}

bool WidgetManager::DrawScreen()
{
	SEXY_AUTO_PERF("WidgetManager::DrawScreen");
	//DWORD start = timeGetTime();

	ModalFlags aModalFlags;
	InitModalFlags(&aModalFlags);

	bool drewStuff = false;

	int aDirtyCount = 0;
	bool hasTransients = false;
	bool hasDirtyTransients = false;

	if (mDirty)
		aDirtyCount++;

	// Survey
	WidgetList::iterator anItr = mWidgets.begin();
	while (anItr != mWidgets.end())
	{
		Widget* aWidget = *anItr;
		if (aWidget->mDirty && aWidget->mVisible)
		{
			aDirtyCount++;
			break;
		}
		++anItr;
	}

	mMinDeferredOverlayPriority = 0x7FFFFFFF;
	mDeferredOverlayWidgets.resize(0);

	Graphics aScrG(mImage);
	mCurG = &aScrG;

	Image* anImage = mImage;
	bool surfaceLocked = false;
	//if (anImage != NULL)
	//surfaceLocked = anImage->LockSurface();

	NativeDisplay * aInterface = mApp->mDDInterface;
	int aCursorX = mLastMouseX;
	int aCursorY = mLastMouseY;
	UnmapMouse(aCursorX, aCursorY);

	bool cursorChanged = aInterface->UpdateCursor(aCursorX, aCursorY);
	if (cursorChanged)
		aDirtyCount++;

	if (!aDirtyCount)
		return false;

	Rect aVisibleRect;
	aVisibleRect.mX = -mMouseDestRect.mX;
	aVisibleRect.mY = -mMouseDestRect.mY;
	aVisibleRect.mWidth = mWidth;
	aVisibleRect.mHeight = mHeight;

	Graphics g(aScrG);
	g.ClipRect(aVisibleRect);
	g.Translate(-mMouseDestRect.mX, -mMouseDestRect.mY);
	bool is3D = mApp->Is3DAccelerated();
	if (mWidgets.empty())
	{
		anImage->ClearRect(aVisibleRect);
	}
	else if (aDirtyCount > 0)
	{
		// find a widget that isn't transparent and covers the
		// whole screen.
		WidgetList::reverse_iterator anItr = mWidgets.rbegin();
		while (anItr != mWidgets.rend())
		{
			Widget* aWidget = *anItr;
			if (aWidget->mVisible && !aWidget->mHasAlpha &&
			    aWidget->mX <= 0 && aWidget->mY <= 0 &&
			    aWidget->mX + aWidget->mWidth >= mWidth &&
			    aWidget->mY + aWidget->mHeight >= mHeight)
				break;
			++anItr;
		}

		// if failed to find such a widget, then clear
		// background by ourself.
		if (anItr == mWidgets.rend())
		{
			anImage->ClearRect(aVisibleRect);
			--anItr;
		}

		while (true)
		{
			Widget* aWidget = *anItr;

			if (aWidget == mWidgetManager->mBaseModalWidget)
				aModalFlags.mIsOver = true;

			if (aWidget->mVisible)
			{
				Graphics aClipG(g);
				aClipG.SetFastStretch(!is3D);
				aClipG.SetLinearBlend(is3D);
				aClipG.Translate(aWidget->mX, aWidget->mY);
				aWidget->DrawAll(&aModalFlags, &aClipG);

				aDirtyCount++;
				drewStuff = true;
				aWidget->mDirty = false;
			}

			if (anItr == mWidgets.rbegin())
				break;
			--anItr;
		}
	}

	FlushDeferredOverlayWidgets(0x7FFFFFFF);
	g.Translate(mMouseDestRect.mX, mMouseDestRect.mY);
	if (drewStuff && aVisibleRect.Contains (aCursorX, aCursorY) &&
	    aInterface->DrawCursor (&g))
		drewStuff = true;

	//if (anImage != NULL && surfaceLocked)
	//anImage->UnlockSurface();

	mCurG = NULL;
	mDirty = false;

	return drewStuff;
}

bool WidgetManager::UpdateFrame()
{
	SEXY_AUTO_PERF("WidgetManager::UpdateFrame");

	ModalFlags aModalFlags;
	InitModalFlags(&aModalFlags);

	// Keep us from having mLastWMUpdateCount interfere with our own updating
	mUpdateCnt++;
	mLastWMUpdateCount = mUpdateCnt;
	UpdateAll(&aModalFlags);

	return mDirty;
}

bool WidgetManager::UpdateFrameF(float theFrac)
{
	SEXY_AUTO_PERF("WidgetManager::UpdateFrame");

	ModalFlags aModalFlags;
	InitModalFlags(&aModalFlags);

	UpdateFAll(&aModalFlags, theFrac);
	return mDirty;
}

void WidgetManager::SetPopupCommandWidget(Widget* theList)
{
	mPopupCommandWidget = theList;
	AddWidget(mPopupCommandWidget);
}

void WidgetManager::RemovePopupCommandWidget()
{
	if (mPopupCommandWidget != NULL)
	{
		Widget *aWidget = mPopupCommandWidget;
		mPopupCommandWidget = NULL;
		RemoveWidget(aWidget);
	}
}

void WidgetManager::MousePosition(int x, int y)
{

	int aLastMouseX = mLastMouseX;
	int aLastMouseY = mLastMouseY;

	mLastMouseX = x;
	mLastMouseY = y;

	int aWidgetX;
	int aWidgetY;
	Widget* aWidget = GetWidgetAt(x, y, &aWidgetX, &aWidgetY);

	if (aWidget != mOverWidget)
	{
		Widget* aLastOverWidget = mOverWidget;
		mOverWidget = NULL;

		if (aLastOverWidget != NULL)
			MouseLeave(aLastOverWidget);

		mOverWidget = aWidget;
		if (aWidget != NULL)
		{
			MouseEnter(aWidget);
			aWidget->MouseMove(aWidgetX, aWidgetY);
		}
	}
	else if ((aLastMouseX != x) || (aLastMouseY != y))
	{
		if (aWidget != NULL)
			aWidget->MouseMove(aWidgetX, aWidgetY);
	}
}

void WidgetManager::RehupMouse()
{
	if (mLastDownWidget != NULL)
	{
		if (mOverWidget != NULL)
		{
			Widget* aWidgetOver = GetWidgetAt(mLastMouseX, mLastMouseY, NULL, NULL);

			if (aWidgetOver != mLastDownWidget)
			{
				Widget* anOverWidget = mOverWidget;
				mOverWidget = NULL;
				MouseLeave(anOverWidget);
			}
		}
	}
	else if (mMouseIn)
		MousePosition(mLastMouseX, mLastMouseY);
}

bool WidgetManager::MouseUp(int x, int y, int theClickCount)
{
	mLastInputUpdateCnt = mUpdateCnt;

	int aMask;

	if (theClickCount < 0)
		aMask = 0x02;
	else if (theClickCount == 3)
		aMask = 0x04;
	else
		aMask = 0x01;

	// Make sure that we thought this button was down anyway - possibly not, if we
	//  disabled the widget already or something
	mActualDownButtons &= ~aMask;
	if ((mLastDownWidget != NULL) && ((mDownButtons & aMask) != 0))
	{
		Widget* aLastDownWidget = mLastDownWidget;

		mDownButtons &= ~aMask;
		if (mDownButtons == 0)
			mLastDownWidget = NULL;

		aLastDownWidget->mIsDown = false;
		aLastDownWidget->MouseUp(x - aLastDownWidget->mX, y - aLastDownWidget->mY, theClickCount);
	}
	else
		mDownButtons &= ~aMask;

	MousePosition(x, y);

	return true;
}

bool WidgetManager::MouseDown(int x, int y, int theClickCount)
{
	mLastInputUpdateCnt = mUpdateCnt;

	if (theClickCount < 0)
		mActualDownButtons |= 0x02;
	else if (theClickCount == 3)
		mActualDownButtons |= 0x04;
	else
		mActualDownButtons |= 0x01;

	MousePosition(x, y);

	if ((mPopupCommandWidget != NULL) && (!mPopupCommandWidget->Contains(x, y)))
		RemovePopupCommandWidget();

	int aWidgetX;
	int aWidgetY;
	Widget* aWidget = GetWidgetAt(x, y, &aWidgetX, &aWidgetY);

	// Begin mouse down options
/*
	// Option 1
	//This code sets a new widget as the mouse drag focus widget and lets the old
	//mousedownwidget think the buttons popped up.
	if ((mLastDownWidget != NULL) && (mLastDownWidget != aWidget))
	{
		DoMouseUps(mLastDownWidget, mDownButtons);
		mDownButtons = 0;
	}
*/
	// Option 2
	// This code passes all button downs to the mLastDownWidget
	if (mLastDownWidget != NULL)
		aWidget = mLastDownWidget;

	// End mouse down options

	if (theClickCount < 0)
	{
		mLastDownButtonId = -1;
		mDownButtons |= 0x02;
	}
	else if (theClickCount == 3)
	{
		mLastDownButtonId = 2;
		mDownButtons |= 0x04;
	}
	else
	{
		mLastDownButtonId = 1;
		mDownButtons |= 0x01;
	}

	mLastDownWidget = aWidget;
	if (aWidget != NULL)
	{
		if (aWidget->WantsFocus())
			SetFocus(aWidget);

		aWidget->mIsDown = true;
		aWidget->MouseDown(aWidgetX, aWidgetY, theClickCount);
	}

	return true;
}

bool WidgetManager::MouseMove(int x, int y)
{
	mLastInputUpdateCnt = mUpdateCnt;

	if (mDownButtons)
		return MouseDrag(x,y);

	mMouseIn = true;
	MousePosition(x, y);

	return true;
}

bool WidgetManager::MouseDrag(int x, int y)
{
	mLastInputUpdateCnt = mUpdateCnt;

	mMouseIn = true;
	mLastMouseX = x;
	mLastMouseY = y;

	if ((mOverWidget != NULL) && (mOverWidget != mLastDownWidget))
	{
		Widget* anOverWidget = mOverWidget;
		mOverWidget = NULL;
		MouseLeave(anOverWidget);
	}

	if (mLastDownWidget != NULL)
	{
		Point anAbsPos = mLastDownWidget->GetAbsPos();

		int aWidgetX = x - anAbsPos.mX;
		int aWidgetY = y - anAbsPos.mY;
		mLastDownWidget->MouseDrag(aWidgetX, aWidgetY);

		Widget* aWidgetOver = GetWidgetAt(x, y, NULL, NULL);

		if ((aWidgetOver == mLastDownWidget) && (aWidgetOver != NULL))
		{
			if (mOverWidget == NULL)
			{
				mOverWidget = mLastDownWidget;
				MouseEnter(mOverWidget);
			}
		}
		else
		{
			if (mOverWidget != NULL)
			{
				Widget* anOverWidget = mOverWidget;
				mOverWidget = NULL;
				MouseLeave(anOverWidget);
			}
		}
	}

	return true;
}

bool WidgetManager::MouseExit(int x, int y)
{
	mLastInputUpdateCnt = mUpdateCnt;

	mMouseIn = false;

	if (mOverWidget != NULL)
	{
		MouseLeave(mOverWidget);
		mOverWidget = NULL;
	}

	return true;
}

void WidgetManager::MouseWheel(int theDelta)
{
	mLastInputUpdateCnt = mUpdateCnt;

	if (mFocusWidget != NULL)
		mFocusWidget->MouseWheel(theDelta);
}

bool WidgetManager::KeyChar(SexyChar theChar)
{
	mLastInputUpdateCnt = mUpdateCnt;

	if (theChar == KEYCODE_TAB)
	{
		//TODO: Check thing

		if (mKeyDown[KEYCODE_CONTROL])
		{
			if (mDefaultTab != NULL)
				mDefaultTab->KeyChar(theChar);

			return true;
		}
	}

	if (mFocusWidget != NULL)
		mFocusWidget->KeyChar(theChar);
	else if (mBaseModalWidget != NULL)
		mBaseModalWidget->KeyChar(theChar);
	return true;
}

bool WidgetManager::KeyDown(KeyCode key)
{
	mLastInputUpdateCnt = mUpdateCnt;

	if ((key >= 0) && (key < 0xFF))
		mKeyDown[key] = true;

	bool ret = false;
	if (mFocusWidget != NULL)
		ret = mFocusWidget->KeyDown(key);
	else if (mBaseModalWidget != NULL)
		ret = mBaseModalWidget->KeyDown(key);
	if (!ret && mGamepadKeyMap.find(key) != mGamepadKeyMap.end())
		return KeyDown(mGamepadKeyMap[key]);
	return ret;
}

bool WidgetManager::KeyUp(KeyCode key)
{
	mLastInputUpdateCnt = mUpdateCnt;

	if ((key >= 0) && (key < 0xFF))
		mKeyDown[key] = false;

	if ((key == KEYCODE_TAB) && (mKeyDown[KEYCODE_CONTROL]))
		return true;

	bool ret = false;
	if (mFocusWidget != NULL)
		ret = mFocusWidget->KeyUp(key);
	else if (mBaseModalWidget != NULL)
		ret = mBaseModalWidget->KeyUp(key);
	if (!ret && mGamepadKeyMap.find(key) != mGamepadKeyMap.end())
		return KeyUp(mGamepadKeyMap[key]);
	return ret;
}

TouchInfo* WidgetManager::GetTouchInfo(int id)
{
	if (mTouchInfoMap.find(id) != mTouchInfoMap.end())
		mTouchInfoMap.insert(TouchInfoMap::value_type(id, TouchInfo()));

	return &mTouchInfoMap[id];
}

bool WidgetManager::TouchDown(const EventVector &events)
{
	mLastInputUpdateCnt = mUpdateCnt;

	TouchVector touches;

	for (size_t i = 0; i < events.size(); i++)
	{
		TouchInfo *info = GetTouchInfo(events[i].u.touch.id);

		touches.push_back(Touch());
		touches[i].id = events[i].u.touch.id;
		touches[i].timestamp = events[i].timestamp;
		touches[i].x = events[i].u.touch.x;
		touches[i].y = events[i].u.touch.y;
		touches[i].pressure = events[i].u.touch.pressure;
		if (touches[i].timestamp - info->timestamp >= 300)
		{
			info->tapCount = 1;
			info->timestamp = touches[i].timestamp;
		}
		touches[i].tapCount = info->tapCount;
		info->x = touches[i].x;
		info->y = touches[i].y;
		info->down = true;
	}

	if (mActiveTouchId < 0)
		mActiveTouchId = touches[0].id;

	TouchInfo *info = GetTouchInfo(mActiveTouchId);

	int x = info->x;
	int y = info->y;
	mLastMouseX = x;
	mLastMouseY = y;

	int aWidgetX;
	int aWidgetY;
	Widget* aWidget = GetWidgetAt(x, y, &aWidgetX, &aWidgetY);

	if (aWidget != mOverWidget)
	{
		mOverWidget = aWidget;
		if (aWidget != NULL)
			TouchEnter(aWidget);
	}

	TouchDown(mActiveTouchId, x, y, info->tapCount);
	if (aWidget)
	{
		aWidget->TouchDown(touches);
		aWidget->TouchMove(touches);
	}

	mLastTouch = touches;
	return true;
}

bool WidgetManager::TouchMove(const EventVector &events)
{
	mLastInputUpdateCnt = mUpdateCnt;

	TouchVector touches;

	for (size_t i = 0; i < events.size(); i++)
	{
		TouchInfo *info = GetTouchInfo(events[i].u.touch.id);

		touches.push_back(Touch());
		touches[i].id = events[i].u.touch.id;
		touches[i].timestamp = events[i].timestamp;
		touches[i].x = events[i].u.touch.x;
		touches[i].y = events[i].u.touch.y;
		touches[i].pressure = events[i].u.touch.pressure;
		if (touches[i].timestamp - info->timestamp >= 300)
		{
			info->tapCount = 1;
			info->timestamp = touches[i].timestamp;
		}
		else
		{
			info->tapCount++;
		}
		touches[i].tapCount = info->tapCount;
		info->x = touches[i].x;
		info->y = touches[i].y;

		if (mActiveTouchId == touches[i].id)
		{
			mLastMouseX = touches[i].x;
			mLastMouseY = touches[i].y;
		}
	}

	if (mOverWidget)
		mOverWidget->TouchMove(touches);

	mLastTouch = touches;
	return true;
}

bool WidgetManager::TouchUp(const EventVector &events)
{
	mLastInputUpdateCnt = mUpdateCnt;

	TouchVector touches;

	for (size_t i = 0; i < events.size(); i++)
	{
		TouchInfo *info = GetTouchInfo(events[i].u.touch.id);

		touches.push_back(Touch());
		touches[i].id = events[i].u.touch.id;
		touches[i].timestamp = events[i].timestamp;
		touches[i].x = events[i].u.touch.x;
		touches[i].y = events[i].u.touch.y;
		touches[i].pressure = events[i].u.touch.pressure;
		touches[i].tapCount = info->tapCount;

		info->timestamp = touches[i].timestamp;
		info->x = touches[i].x;
		info->y = touches[i].y;
		info->down = false;
	}

	// if the active touch is up, find another one that is still down
	TouchInfo *info = GetTouchInfo(mActiveTouchId);
	if (info && !info->down)
	{
		TouchInfoMap::iterator it;
		int anId = mActiveTouchId;

		for (it = mTouchInfoMap.begin(); it != mTouchInfoMap.end(); ++it)
		{
			if (it->second.down)
			{
				mActiveTouchId = it->first;
				mLastMouseX = info->x;
				mLastMouseY = info->y;
				break;
			}
		}

		if (anId == mActiveTouchId)
			mActiveTouchId = -1;
	}

	Widget *aWidget = mOverWidget;
	if (aWidget)
		aWidget->TouchUp(touches);

	if (mActiveTouchId < 0 && aWidget)
	{
		TouchLeave(aWidget);
		mOverWidget = 0;
	}

	mLastTouch = touches;
	return true;
}

bool WidgetManager::TouchCancel(const EventVector &events)
{
	TouchVector touches;

	for (size_t i = 0; i < events.size(); i++)
	{
		TouchInfo *info = GetTouchInfo(events[i].u.touch.id);

		touches.push_back(Touch());
		touches[i].id = events[i].u.touch.id;
		touches[i].timestamp = events[i].timestamp;
		touches[i].x = events[i].u.touch.x;
		touches[i].y = events[i].u.touch.y;
		touches[i].pressure = events[i].u.touch.pressure;
		touches[i].tapCount = info->tapCount;

		info->timestamp = touches[i].timestamp;
		info->tapCount = 0;
		info->down = false;
	}

	// if the active touch is up, find another one that is still down
	TouchInfo *info = GetTouchInfo(mActiveTouchId);
	if (info && !info->down)
	{
		TouchInfoMap::iterator it;
		int anId = mActiveTouchId;

	        for (it = mTouchInfoMap.begin(); it != mTouchInfoMap.end(); ++it)
		{
			if (it->second.down)
			{
				mActiveTouchId = it->first;
				mLastMouseX = info->x;
				mLastMouseY = info->y;
				break;
			}
		}

		if (anId == mActiveTouchId)
			mActiveTouchId = -1;
	}

	if (mOverWidget)
	{
		Widget *aWidget = mOverWidget;
		aWidget->TouchCancel(touches);
		TouchLeave(aWidget);
		mOverWidget = 0;
	}

	Widget *aWidget = mOverWidget;
	if (aWidget)
		aWidget->TouchUp(touches);

	if (mActiveTouchId < 0 && aWidget)
	{
		TouchLeave(aWidget);
		mOverWidget = 0;
	}

	mLastTouch = touches;
	return true;
}

bool WidgetManager::UserEvent(const Event event)
{
    	mLastInputUpdateCnt = mUpdateCnt;

	if (mFocusWidget != NULL)
		mFocusWidget->UserEvent(event);

	return true;
}

void WidgetManager::TouchEnter(Widget *theWidget)
{
	theWidget->mIsOver = true;

	theWidget->TouchEnter();
}

void WidgetManager::TouchLeave(Widget *theWidget)
{
	theWidget->mIsOver = false;

	theWidget->TouchLeave();
}

bool WidgetManager::TouchDown(int id, int x, int y, int tapCount)
{
	if ((mPopupCommandWidget != NULL) && (!mPopupCommandWidget->Contains(x, y)))
		RemovePopupCommandWidget();

	if (mOverWidget && mOverWidget->mHasFocus)
		return true;

	int aWidgetX;
	int aWidgetY;
	Widget* aWidget = GetWidgetAt(x, y, &aWidgetX, &aWidgetY);
	if (aWidget != NULL)
		SetFocus(aWidget);

	return true;
}

bool WidgetManager::AxisMoved(const Event& event)
{
	mLastInputUpdateCnt = mUpdateCnt;

	bool ret = false;
	if (mFocusWidget != NULL)
		ret = mFocusWidget->AxisMoved(event);
	else if (mBaseModalWidget != NULL)
		ret = mBaseModalWidget->AxisMoved(event);
	if (!ret)
		return DefaultAxisMoved(event);
	return true;
}

AxisInfo* WidgetManager::GetAxisInfo(int id, int subid)
{
	int key = id << 16 | subid;

	if (mAxisInfoMap.find(key) != mAxisInfoMap.end())
	{
		mAxisInfoMap.insert(AxisInfoMap::value_type(key, AxisInfo()));

		AxisInfo* info = &mAxisInfoMap[key];
		memset(info, 0, sizeof(AxisInfo));
		return info;
	}
	return &mAxisInfoMap[key];
}

bool WidgetManager::HandleAxisChanged(float lx, float x, float flat,
				      bool vertical)
{
	int ix = 0, ilx = 0;

	if (fabs(x) <= flat)
		ix = 0;
	else if (x > flat)
		ix = 1;
	else
		ix = -1;

	if (fabs(lx) <= flat)
		ilx = 0;
	else if (lx > flat)
		ilx = 1;
	else
		ilx = -1;

	if (ix == ilx)
		return false;

	KeyCode keyCodes[2];
	if (!vertical)
	{
		keyCodes[0] = KEYCODE_LEFT;
		keyCodes[1] = KEYCODE_RIGHT;
	}
	else
	{
		keyCodes[0] = KEYCODE_UP;
		keyCodes[1] = KEYCODE_DOWN;
	}

	KeyCode keyCode = KEYCODE_UNKNOWN;
	if (ilx + ix == 1)
	{
		keyCode = keyCodes[1];
		if (ix == 1)
			KeyDown(keyCode);
		else
			KeyUp(keyCode);
	}
	else if (ilx + ix == -1)
	{
		keyCode = keyCodes[0];
		if (ix == -1)
			KeyDown(keyCode);
		else
			KeyUp(keyCode);
	}
	else if (ix == 1 && ilx == -1)
	{
		keyCode = keyCodes[1];
		KeyUp(keyCode);
		keyCode = keyCodes[0];
		KeyDown(keyCode);
	}
	else if (ix == -1 && ilx == 1)
	{
		keyCode = keyCodes[0];
		KeyUp(keyCode);
		keyCode = keyCodes[1];
		KeyDown(keyCode);
	}

	return true;
}

bool WidgetManager::DefaultAxisMoved(const Event& event)
{
	int axis = event.u.axis.axis;
	if (axis != AXIS_X && axis != AXIS_Y &&
	    axis != AXIS_HAT0X && axis != AXIS_HAT0Y)
		return false;

	AxisInfo* info = GetAxisInfo(event.id, event.subid);
	float x = axis == AXIS_X ? info->x : info->hatx;
	float y = axis == AXIS_Y ? info->y : info->haty;
	float lx = x;
	float ly = y;
	float flat = event.u.axis.flat;

	if (axis == AXIS_X)
	{
		x = event.u.axis.value;
		if (HandleAxisChanged(lx, x, flat, false))
			info->x = x;
	}
	if (axis == AXIS_HAT0X)
	{
		x = event.u.axis.value;
		if (HandleAxisChanged(lx, x, flat, false))
			info->hatx = x;
	}
	else if (axis == AXIS_Y)
	{
		y = event.u.axis.value;
		if (HandleAxisChanged(ly, y, flat, true))
			info->y = y;
	}
	else if (axis == AXIS_HAT0Y)
	{
		y = event.u.axis.value;
		if (HandleAxisChanged(ly, y, flat, true))
			info->haty = y;
	}

	return true;
}

bool WidgetManager::ShowKeyboard(Widget *theWidget,
				 KeyboardMode mode,
				 const std::string &title,
				 const std::string &hint,
				 const std::string &initial)
{
	if (mApp->mDDInterface->ShowKeyboard(theWidget, mode, title,
					     hint, initial))
	    return true;
	return true;
}

void WidgetManager::HideKeyboard()
{
	mApp->mDDInterface->HideKeyboard();
}
