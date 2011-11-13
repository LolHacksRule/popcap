#include "Widget.h"
#include "WidgetManager.h"
#include "Graphics.h"
#include "Font.h"
#include "Image.h"
#include "SexyAppBase.h"
#include "Debug.h"

using namespace Sexy;

bool Widget::mWriteColoredString = true;

Widget::Widget()
{
	mWidgetManager = NULL;
	mVisible = true;
	mDisabled = false;
	mIsDown = false;
	mIsOver = false;
	mDoFinger = false;
	mMouseVisible = true;
	mHasFocus = false;
	mHasTransparencies = false;
	mWantsFocus = true;
	mIsSelected = false;
	mHasSelectedState = true;
	mFocusColor = Color(80, 80, 80, 80);
	mFocusRect = Rect(0, 0, -1, -1);
	mDrawFocusRect = true;
	mAddToManager = false;
	mTabPrev = NULL;
	mTabNext = NULL;
	mFocus = 0;
	mActiveTouchId = -1;
	mTouchHover = false;
}

Widget::~Widget()
{
	mColors.clear();
}

void Widget::WidgetRemovedHelper()
{
	if (mWidgetManager==NULL)
		return;

	// Call RemovedFromManager on all child widgets and disable them and stuff like that
	for (WidgetList::iterator aWidgetItr = mWidgets.begin(); aWidgetItr != mWidgets.end(); ++aWidgetItr)
	{
		Widget *aWidget = *aWidgetItr;
		aWidget->WidgetRemovedHelper();
	}

	mWidgetManager->DisableWidget(this);

	PreModalInfoList::iterator anItr = mWidgetManager->mPreModalInfoList.begin();
	while (anItr != mWidgetManager->mPreModalInfoList.end())
	{
		PreModalInfo* aPreModalInfo = &(*anItr);
		if (aPreModalInfo->mPrevBaseModalWidget == this)
			aPreModalInfo->mPrevBaseModalWidget = NULL;
		if (aPreModalInfo->mPrevFocusWidget == this)
			aPreModalInfo->mPrevFocusWidget = NULL;
		++anItr;
	}

	RemovedFromManager(mWidgetManager);
	MarkDirtyFull(this);

	mWidgetManager = NULL;
}

void Widget::OrderInManagerChanged()
{
}

bool Widget::IsPointVisible(int x, int y)
{
	return true;
}

void Widget::SetVisible(bool isVisible)
{
	if (mVisible == isVisible)
		return;

	mVisible = isVisible;

	if (mVisible)
		MarkDirty();
	else
		MarkDirtyFull();

	if (mWidgetManager != NULL)
		mWidgetManager->RehupMouse();
}

void Widget::Draw(Graphics* g) // Already translated
{
}

void Widget::DrawFocused(Graphics* g) // Already translated
{
#ifndef SEXY_NO_KEYBOARD
	if (mWantsFocus && mHasFocus && mDrawFocusRect)
	{
		if (mParent && mParent == mWidgetManager)
			return;

		g->SetColor(mFocusColor);
		if (mFocusRect.mWidth > 0 && mFocusRect.mWidth)
			g->FillRect(mFocusRect);
		else if (mWidth > 0 && mHeight > 0)
			g->FillRect(0, 0, mWidth, mHeight);
	}
#endif
}

void Widget::DrawPost(Graphics* g) // Already translated
{
	DrawFocused(g);
}

void Widget::DrawOverlay(Graphics* g)
{
}

void Widget::DrawOverlay(Graphics* g, int thePriority)
{
	DrawOverlay(g);
}

void Widget::SetColors(int theColors[][3], int theNumColors)
{
	mColors.clear();

	for (int i = 0; i < theNumColors; i++)
		SetColor(i, Color(theColors[i][0], theColors[i][1], theColors[i][2]));
	MarkDirty();
}

void Widget::SetColors(int theColors[][4], int theNumColors)
{
	mColors.clear();

	for (int i = 0; i < theNumColors; i++)
		SetColor(i, Color(theColors[i][0], theColors[i][1], theColors[i][2], theColors[i][3]));

	MarkDirty();
}

void Widget::SetColor(int theIdx, const Color& theColor)
{
	if (theIdx >= (int)mColors.size())
		mColors.resize(theIdx + 1);

	mColors[theIdx] = theColor;
	MarkDirty();
}

const Color& Widget::GetColor(int theIdx)
{
	static Color aColor;
	if (theIdx < (int) mColors.size())
		return mColors[theIdx];
	return aColor;
}

Color Widget::GetColor(int theIdx, const Color& theDefaultColor)
{
	if (theIdx < (int) mColors.size())
		return mColors[theIdx];
	return theDefaultColor;
}

void Widget::Resize(int theX, int theY, int theWidth, int theHeight)
{
	if ((mX == theX) && (mY == theY) && (mWidth == theWidth) && (mHeight == theHeight))
		return;

	// Mark everything dirty that is over or under the old position
	MarkDirtyFull();

	mX = theX;
	mY = theY;
	mWidth = theWidth;
	mHeight = theHeight;

	// Mark things dirty that are over the new position
	MarkDirty();

	if (mWidgetManager != NULL)
		mWidgetManager->RehupMouse();


	SortWidgets();
}

void Widget::Resize(const Rect& theRect)
{
	Resize(theRect.mX, theRect.mY, theRect.mWidth, theRect.mHeight);
}

void Widget::Move(int theNewX, int theNewY)
{
	Resize(theNewX, theNewY, mWidth, mHeight);
}

bool Widget::WantsFocus()
{
	return mWantsFocus;
}

void Widget::SetDisabled(bool isDisabled)
{
	if (mDisabled == isDisabled)
		return;

	mDisabled = isDisabled;

	if ((isDisabled) && (mWidgetManager != NULL))
		mWidgetManager->DisableWidget(this);

	MarkDirty();

	// Incase a widget is enabled right under our cursor
	if ((!isDisabled) && (mWidgetManager != NULL) && (Contains(mWidgetManager->mLastMouseX, mWidgetManager->mLastMouseY)))
		mWidgetManager->MousePosition(mWidgetManager->mLastMouseX, mWidgetManager->mLastMouseY);
}

void Widget::SetFocus(Widget* theWidget)
{
	Widget* aFocusWidget;
	std::vector<Widget*> aParents;
	std::vector<Widget*> aOldParents;

	if (theWidget && mFocus)
	{
		aFocusWidget = mFocus;
		while (aFocusWidget->mParent != this)
		{
			aFocusWidget = (Widget*)aFocusWidget->mParent;
			aOldParents.push_back(aFocusWidget);
		}
	}

	if (theWidget)
	{
		aFocusWidget = theWidget;
		while (aFocusWidget->mParent != this)
		{
			aFocusWidget = (Widget*)aFocusWidget->mParent;
			aParents.push_back(aFocusWidget);
		}
	}
	else
	{
		aFocusWidget = 0;
	}

	Widget* aParent = 0;
	unsigned i;
	for (i = 0; i < std::min(aParents.size(), aOldParents.size()); i++)
	{
		if (aParents[i] != aOldParents[i])
			break;
		aParent = aParents[i];
	}
	if (theWidget)
	{
		if (theWidget->mParent != this)
		{
			if (aParent)
			{
				// The old focus and new focus has a common ancestor.
				assert (aParent == aParents[i - 1]);
				aOldParents[i]->LostFocus();
				for (i = 0; i < aParents.size(); i++)
				{
					if (i)
						aParents[i]->mFocus = aParents[i - 1];
					else
						aParents[i]->mFocus = theWidget;
					aParents[i]->GotFocus();
					aParents[i]->mIsSelected = true;
				}
			}
			else if (mFocus)
			{
				// No common ancestor.
				mFocus->LostFocus();
			}
			else
			{
				// Doesn't have a focus yet
				for (i = 0; i < aParents.size(); i++)
				{
					if (i)
						aParents[i]->mFocus = aParents[i - 1];
					else
						aParents[i]->mFocus = theWidget;
					aParents[i]->GotFocus();
					aParents[i]->mIsSelected = true;
				}
			}
		}
		else if (mFocus)
		{
			mFocus->LostFocus();
		}

		theWidget->mIsSelected = true;
		theWidget->GotFocus();

		mIsSelected = true;
		mFocus = aFocusWidget;
	}
	else
	{
		LostFocus();
	}
}

void Widget::GotFocus()
{
	if (!mHasFocus)
	{
		mHasFocus = true;
		mIsSelected = true;
		MarkDirty();
	}
}

void Widget::LostFocus()
{
	bool dirty = mHasFocus || mIsSelected;

	mHasFocus = false;
	mIsSelected = false;
	if (mFocus)
		mFocus->LostFocus();
	mFocus = 0;
	if (mParent && mParent != mWidgetManager)
	{
		Widget* aWidget = (Widget*)mParent;
		if (aWidget && aWidget->mFocus == this)
			aWidget->mFocus = 0;
	}
	if (dirty)
		MarkDirty();
}

void Widget::Update()
{
	WidgetContainer::Update();

	if (mActiveTouchId < 0 || mTouchHover)
		return;

	DWORD now = Sexy::GetTickCount();
	if (now - mLastTouch.timestamp < 500)
		return;

	TouchHover(mLastTouch.id, mLastTouch.x, mLastTouch.y,
		   mLastTouch.tapCount);
	mTouchHover = true;
}

void Widget::UpdateF(float theFrac)
{
}

bool Widget::IsFocusable()
{
	return mVisible && mWantsFocus && mMouseVisible && !mDisabled;
}

bool Widget::DoKeyChar(SexyChar theChar)
{
	WidgetVector::iterator it;
	for (it = mSortedWidgets.begin(); it != mSortedWidgets.end(); ++it)
	{
		if ((*it)->mHasFocus && (*it)->IsFocusable())
			break;
	}
	if (it != mSortedWidgets.end())
	{
		Widget* aWidget = *it;
		bool aIsSelected = aWidget->mIsSelected;
		if (!aWidget->KeyChar(theChar))
			return false;
		if (aIsSelected == aWidget->mIsSelected)
			aWidget->mIsSelected = true;
		return true;
	}

	return false;
}

bool Widget::KeyChar(SexyChar theChar)
{
	bool aIsSelected = mIsSelected;
	if (DoKeyChar(theChar))
	{
		if (aIsSelected == mIsSelected)
			mIsSelected = true;
		return true;
	}

	return false;
}

bool Widget::KeyDownUp(KeyCode theKey, bool down)
{
	bool aIsSelected = mIsSelected;
	bool aKeyHandled;

	if (down)
		aKeyHandled = KeyDown(theKey);
	else
		aKeyHandled = KeyUp(theKey);
	if (aKeyHandled)
	{
		if (aIsSelected == mIsSelected)
			mIsSelected = true;
		return true;
	}

	return false;
}

WidgetVector::iterator Widget::FindFocusableWidget(int direct, Widget* current)
{
	Widget* widget = 0;

	if (mSortedWidgets.empty())
		return mSortedWidgets.end();

	WidgetVector::iterator it;
	if (!current)
	{
		// Find the current focus
		for (it = mSortedWidgets.begin(); it != mSortedWidgets.end(); ++it)
		{
			if ((*it)->mHasFocus && (*it)->IsFocusable())
			{
				current = *it;
				break;
			}
		}

		if (!current)
		{
			for (it = mSortedWidgets.begin(); it != mSortedWidgets.end(); ++it)
				if ((*it)->IsFocusable())
					return it;
		}
	}
	else
	{
		it = std::find(mSortedWidgets.begin(), mSortedWidgets.end(), current);
	}
	if (it == mSortedWidgets.end())
		return it;

	Point center = current->GetCenter();
	if (direct == LAY_Below)
	{
		for (it++; it != mSortedWidgets.end(); it++)
		{
			if ((*it)->IsFocusable() && (*it)->Top() > current->Top())
			{
				widget = *it;
				break;
			}
		}

		if (it != mSortedWidgets.end())
		{
			WidgetVector::iterator pit = it;
			WidgetVector::iterator best = it;
			Point diff = center - (*it)->GetCenter();
			int dist = diff.mX * diff.mX + diff.mY * diff.mY;
			int bestDist = dist;

			for (++it; it != mSortedWidgets.end(); pit = it++)
			{
				if ((*it)->Left() < (*pit)->Left())
					break;

				Point c = (*it)->GetCenter();
				diff = center - c;
				dist = diff.mX * diff.mX + diff.mY * diff.mY;
				if (dist < bestDist)
				{
					bestDist = dist;
					best = it;
					widget = *it;
				}
			}
			it = best;
		}
	}
	else if (direct == LAY_Above)
	{
		if (it != mSortedWidgets.begin())
		{
			do
			{
				--it;
				if ((*it)->IsFocusable() && (*it)->Top() < current->Top())
				{
					widget = *it;
					break;
				}
			} while (it != mSortedWidgets.begin());
		}

		if (it != mSortedWidgets.begin())
		{
			WidgetVector::iterator pit = it;
			WidgetVector::iterator best = it;
			Point diff = center - (*it)->GetCenter();
			int dist = diff.mX * diff.mX + diff.mY * diff.mY;
			int bestDist = dist;

			for (--it; it != mSortedWidgets.begin(); pit = it--)
			{
				if ((*it)->Left() >= (*pit)->Left())
					break;

				Point c = (*it)->GetCenter();
				diff = center - c;
				dist = diff.mX * diff.mX + diff.mY * diff.mY;
				if (dist < bestDist)
				{
					bestDist = dist;
					best = it;
					widget = *it;
				}
			}
			it = best;
		}
	}
	else if (direct == LAY_Right)
	{
		WidgetVector::iterator saved = it;

		for (; it != mSortedWidgets.end(); ++it)
		{
			if ((*it)->IsFocusable() && (*it)->Left() > current->Left())
			{
				widget = *it;
				break;
			}
		}

		if (!widget && saved != mSortedWidgets.begin())
		{
			it = saved;
			do
			{
				--it;
				if ((*it)->IsFocusable() && (*it)->Left() > current->Left())
				{
					widget = *it;
					break;
				}
			} while (it != mSortedWidgets.begin());
		}
	}
	else if (direct == LAY_Left)
	{
		WidgetVector::iterator saved = it;

		if (it != mSortedWidgets.begin())
		{
			do
			{
				--it;
				if ((*it)->IsFocusable() &&
				    (*it)->Left() <= current->Left())
				{
					widget = *it;
					break;
				}
			} while (it != mSortedWidgets.begin());
		}

		if (!widget && saved != mSortedWidgets.end())
		{
			for (it = saved + 1; it != mSortedWidgets.end(); ++it)
			{
				if ((*it)->IsFocusable() && (*it)->Left() <= current->Left())
				{
					widget = *it;
					break;
				}
			}
		}
	}

	return widget ? it : mSortedWidgets.end();
}

bool Widget::OnArrowKeys(KeyCode theKey)
{
	if (mSortedWidgets.empty())
		return false;

	int flags;
	switch (theKey)
	{
	case KEYCODE_DOWN:
		flags = LAY_Below;
		break;

	case KEYCODE_UP:
		flags = LAY_Above;
		break;

	case KEYCODE_RIGHT:
		flags = LAY_Right;
		break;

	case KEYCODE_LEFT:
		flags = LAY_Left;
		break;

	default:
		break;
	}


	Widget* cur = 0;
	WidgetVector::iterator it;
	for (it = mSortedWidgets.begin(); it != mSortedWidgets.end(); it++)
	{
		if ((*it)->mHasFocus && (*it)->IsFocusable())
		{
			cur = *it;
			break;
		}
	}

	// Prevents someone try to hijack the focus...
	assert(!cur || (cur && mFocus == cur));

	if (cur && cur->KeyDown(theKey))
		return true;

	while (true)
	{
		WidgetVector::iterator it = FindFocusableWidget(flags, cur);
		if (it == mSortedWidgets.end())
			return false;

		cur = *it;
		if (cur->mWidgets.empty())
		{
			if (cur->mHasFocus)
				return false;

			SetFocus(cur);
			return true;
		}
		cur = *it;
		if (!cur->mHasFocus)
			SetFocus(cur);
		if (cur->KeyDown(theKey))
			return true;
	}

	return false;
}

bool Widget::OnKeyReturn()
{
	if (!mSortedWidgets.size())
		return false;

	WidgetVector::iterator it;
	Widget* cur = 0;

	for (it = mSortedWidgets.begin(); it != mSortedWidgets.end(); it++)
	{
		if ((*it)->mHasFocus && (*it)->IsFocusable())
		{
			cur = *it;
			break;
		}
	}

	if (!cur)
		return false;

	if (cur->KeyDownUp(KEYCODE_RETURN))
		return true;

	if (!cur->mIsSelected)
	{
		cur->mIsSelected = true;
		return true;
	}
	return false;
}

bool Widget::OnKeyEscape()
{
	if (!mSortedWidgets.size())
		return false;

	WidgetVector::iterator it;

	for (it = mSortedWidgets.begin(); it != mSortedWidgets.end(); it++)
		if ((*it)->mHasFocus && (*it)->IsFocusable())
			break;

	Widget* cur = 0;
	if (it == mSortedWidgets.end())
		return false;

	cur = *it;
	if (cur->KeyDownUp(KEYCODE_ESCAPE))
		return true;

	if (cur->mIsSelected && cur->mHasSelectedState)
	{
		cur->mIsSelected = false;
	}
	else
	{
		cur->LostFocus();
		cur->MarkDirty();
		mFocus = 0;
	}

	return true;
}

bool Widget::KeyDown(KeyCode theKey)
{
	if (theKey == KEYCODE_UP || theKey == KEYCODE_DOWN ||
	    theKey == KEYCODE_LEFT || theKey == KEYCODE_RIGHT)
	{
		return OnArrowKeys(theKey);
	}
	else if (theKey == KEYCODE_RETURN)
	{
		return OnKeyReturn();
	}
	else if (theKey == KEYCODE_ESCAPE)
	{
		return OnKeyEscape();
	}

	WidgetVector::iterator it;
	for (it = mSortedWidgets.begin(); it != mSortedWidgets.end(); ++it)
	{
		if ((*it)->mHasFocus && (*it)->IsFocusable())
			break;
	}
	if (it != mSortedWidgets.end())
		return (*it)->KeyDownUp(theKey);

	return false;
}

bool Widget::KeyUp(KeyCode theKey)
{
	WidgetVector::iterator it;
	for (it = mSortedWidgets.begin(); it != mSortedWidgets.end(); ++it)
	{
		if ((*it)->mHasFocus && (*it)->IsFocusable())
			break;
	}
	if (it != mSortedWidgets.end())
	{
		Widget* aWidget = *it;
		if (aWidget->KeyDownUp(theKey, false))
			return true;
	}

	return false;
}

void Widget::TouchEnter()
{
}

void Widget::TouchLeave()
{
}

void Widget::TouchDown(int id, int x, int y, int tapCount)
{
}

void Widget::TouchHover(int id, int x, int y, int tapCount)
{
}

void Widget::TouchMove(int id, int x, int y)
{
}

void Widget::TouchUp(int id, int x, int y, int tapCount)
{
}

void Widget::TouchCancel(int id, int x, int y)
{
}

void Widget::TouchDown(const TouchVector &touches)
{
	if (mActiveTouchId >= 0)
		return;

	mActiveTouchId = touches[0].id;

	Point pos = GetAbsPos();
	int x = touches[0].x - pos.mX;
	int y = touches[0].y - pos.mY;
	mLastTouch = touches[0];
	mTouchHover = false;
	TouchDown(touches[0].id, x, y, touches[0].tapCount);
}

void Widget::TouchMove(const TouchVector &touches)
{
	size_t i;
	for (i = 0; i < touches.size(); i++)
		if (touches[i].id == mActiveTouchId)
			break;
	if (i == touches.size())
		return;

	Point pos = GetAbsPos();
	int x = touches[i].x - pos.mX;
	int y = touches[i].y - pos.mY;
	mLastTouch = touches[0];
	TouchMove(touches[i].id, x, y);
}

void Widget::TouchUp(const TouchVector &touches)
{
	size_t i;
	for (i = 0; i < touches.size(); i++)
		if (touches[i].id == mActiveTouchId)
			break;
	if (i == touches.size())
		return;

	Point pos = GetAbsPos();
	int x = touches[i].x - pos.mX;
	int y = touches[i].y - pos.mY;
	TouchUp(touches[i].id, x, y, touches[i].tapCount);
	mActiveTouchId = -1;

}

void Widget::TouchCancel(const TouchVector &touches)
{
	size_t i;
	for (i = 0; i < touches.size(); i++)
		if (touches[i].id == mActiveTouchId)
			break;
	if (i == touches.size())
		return;

	Point pos = GetAbsPos();
	int x = touches[i].x - pos.mX;
	int y = touches[i].y - pos.mY;
	TouchCancel(touches[i].id, x, y);
	mActiveTouchId = -1;
}

void Widget::ShowFinger(bool on)
{
	if (mWidgetManager == NULL)
		return;

	if (on)
		mWidgetManager->mApp->SetCursor(CURSOR_HAND);
	else
		mWidgetManager->mApp->SetCursor(CURSOR_POINTER);

	/*if (on)
		mWidgetManager->mApplet.setCursor(new Cursor(Cursor.HAND_CURSOR));
	else
		mWidgetManager->mApplet.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));*/
}

void Widget::MouseEnter()
{

}

void Widget::MouseLeave()
{

}

void Widget::MouseMove(int x, int y)
{
}

void Widget::MouseDown(int x, int y, int theClickCount)
{
	if (theClickCount == 3)
		MouseDown(x, y, 2, 1);
	else if (theClickCount >= 0)
		MouseDown(x, y, 0, theClickCount);
	else
		MouseDown(x, y, 1, -theClickCount);
}

void Widget::MouseDown(int x, int y, int theBtnNum, int theClickCount)
{
}

void Widget::MouseUp(int x, int y)
{
}

void Widget::MouseUp(int x, int y, int theLastDownButtonId)
{
	MouseUp(x, y);

	if (theLastDownButtonId == 3)
		MouseUp(x, y, 2, 1);
	else if (theLastDownButtonId >= 0)
		MouseUp(x, y, 0, theLastDownButtonId);
	else
		MouseUp(x, y, 1, -theLastDownButtonId);
}

void Widget::MouseUp(int x, int y, int theBtnNum, int theClickCount)
{
}

void Widget::MouseDrag(int x, int y)
{
}

void Widget::MouseWheel(int theDelta)
{
}

void Widget::UserEvent(const Event event)
{
}

//////// Helper functions

Rect Widget::WriteCenteredLine(Graphics* g, int anOffset, const SexyString& theLine)
{
	Font* aFont = g->GetFont();
	int aWidth = aFont->StringWidth(theLine);
	int aX = (mWidth - aWidth) / 2;

	g->DrawString(theLine, aX, anOffset);

	return Rect(aX, anOffset - aFont->GetAscent(), aWidth, aFont->GetHeight());
}

Rect Widget::WriteCenteredLine(Graphics* g, int anOffset, const SexyString& theLine, Color theColor1, Color theColor2, const Point& theShadowOffset)
{
	Font* aFont = g->GetFont();
	int aWidth = aFont->StringWidth(theLine);
	int aX = (mWidth - aWidth) / 2;

	g->SetColor(theColor2);
	g->DrawString(theLine, (mWidth - aWidth)/2 + theShadowOffset.mX, anOffset + theShadowOffset.mY);

	g->SetColor(theColor1);
	g->DrawString(theLine, (mWidth - aWidth)/2, anOffset);

	// account for shadow in position and size
	// TODO: this may not be necessary.
	return Rect(
		aX + std::min(0,theShadowOffset.mX),
		anOffset - aFont->GetAscent() + std::min(0,theShadowOffset.mY),
		aWidth + abs(theShadowOffset.mX),
		aFont->GetHeight() + abs(theShadowOffset.mY));
}

int Widget::WriteString(Graphics* g, const SexyString& theString, int theX, int theY, int theWidth, int theJustification, bool drawString, int theOffset, int theLength)
{
	bool oldColored = g->mWriteColoredString;
	g->mWriteColoredString = mWriteColoredString;
	int aXOffset = g->WriteString(theString,theX,theY,theWidth,theJustification,drawString,theOffset,theLength);
	g->mWriteColoredString = oldColored;

	return aXOffset;
}

int	Widget::WriteWordWrapped(Graphics* g, const Rect& theRect, const SexyString& theLine, int theLineSpacing, int theJustification)
{
	bool oldColored = g->mWriteColoredString;
	g->mWriteColoredString = mWriteColoredString;
	int aReturn = g->WriteWordWrapped(theRect,theLine,theLineSpacing,theJustification);
	g->mWriteColoredString = oldColored;

	return aReturn;
}

int Widget::GetWordWrappedHeight(Graphics* g, int theWidth, const SexyString& theLine, int aLineSpacing)
{
	return g->GetWordWrappedHeight(theWidth,theLine,aLineSpacing);
}

int Widget::GetNumDigits(int theNumber)
{
	int aDivisor = 10;
	int aNumDigits = 1;
	while (theNumber >= aDivisor)
	{
		aNumDigits++;
		aDivisor *= 10;
	}

	return aNumDigits;
}

void Widget::WriteNumberFromStrip(Graphics* g, int theNumber, int theX, int theY, Image* theNumberStrip, int aSpacing)
{
	int aDivisor = 10;
	int aNumDigits = 1;
	while (theNumber >= aDivisor)
	{
		aNumDigits++;
		aDivisor *= 10;
	}
	if (theNumber == 0)
		aDivisor = 10;

	int aDigitLen = theNumberStrip->GetWidth() / 10;

	for (int aDigitIdx = 0; aDigitIdx < aNumDigits; aDigitIdx++)
	{
		aDivisor /= 10;
		int aDigit = (theNumber / aDivisor) % 10;

		Graphics* aClipG = g->Create();
		aClipG->ClipRect(theX + aDigitIdx*(aDigitLen + aSpacing), theY, aDigitLen, theNumberStrip->GetHeight());
		aClipG->DrawImage(theNumberStrip, theX + aDigitIdx*(aDigitLen + aSpacing) - aDigit*aDigitLen, theY);
		delete aClipG;
	}
}

bool Widget::Contains(int theX, int theY)
{
	return ((theX >= mX) && (theX < mX + mWidth) &&
			(theY >= mY) && (theY < mY + mHeight));
}

Rect Widget::GetInsetRect()
{
	return Rect(mX + mMouseInsets.mLeft, mY + mMouseInsets.mTop,
						 mWidth - mMouseInsets.mLeft - mMouseInsets.mRight,
						 mHeight - mMouseInsets.mTop - mMouseInsets.mBottom);
}

void Widget::DeferOverlay(int thePriority)
{
	mWidgetManager->DeferOverlay(this, thePriority);
}

void Widget::Layout(int theLayoutFlags, Widget *theRelativeWidget, int theLeftPad, int theTopPad, int theWidthPad, int theHeightPad)
{
	int aRelLeft = theRelativeWidget->Left();
	int aRelTop = theRelativeWidget->Top();
	if (theRelativeWidget==mParent)
	{
		aRelLeft = 0;
		aRelTop = 0;
	}

	int aRelWidth = theRelativeWidget->Width();
	int aRelHeight = theRelativeWidget->Height();
	int aRelRight = aRelLeft + aRelWidth;
	int aRelBottom = aRelTop + aRelHeight;

	int aLeft = Left();
	int aTop = Top();
	int aWidth = Width();
	int aHeight = Height();

	int aType = 1;
	while(aType<LAY_Max)
	{
		if(theLayoutFlags&aType)
		{
			switch(aType)
			{
				case LAY_SameWidth: aWidth = aRelWidth+theWidthPad; break;
				case LAY_SameHeight: aHeight = aRelHeight+theHeightPad; break;

				case LAY_Above: aTop = aRelTop-aHeight+theTopPad; break;
				case LAY_Below: aTop = aRelBottom+theTopPad; break;
				case LAY_Right: aLeft = aRelRight+theLeftPad; break;
				case LAY_Left:  aLeft = aRelLeft-aWidth+theLeftPad; break;

				case LAY_SameLeft: aLeft = aRelLeft+theLeftPad; break;
				case LAY_SameRight: aLeft = aRelRight-aWidth+theLeftPad; break;
				case LAY_SameTop: aTop = aRelTop+theTopPad; break;
				case LAY_SameBottom: aTop = aRelBottom-aHeight+theTopPad; break;

				case LAY_GrowToRight: aWidth = aRelRight-aLeft+theWidthPad; break;
				case LAY_GrowToLeft: aWidth = aRelLeft-aLeft+theWidthPad; break;
				case LAY_GrowToTop: aHeight = aRelTop-aTop+theHeightPad; break;
				case LAY_GrowToBottom: aHeight = aRelBottom-aTop+theHeightPad; break;

				case LAY_SetLeft: aLeft = theLeftPad; break;
				case LAY_SetTop: aTop = theTopPad; break;
				case LAY_SetWidth: aWidth = theWidthPad; break;
				case LAY_SetHeight: aHeight = theHeightPad; break;

				case LAY_HCenter: aLeft = aRelLeft+(aRelWidth-aWidth)/2 + theLeftPad; break;
				case LAY_VCenter: aTop = aRelTop+(aRelHeight-aHeight)/2 + theTopPad; break;
			}
		}

		aType<<=1;
	}

	Resize(aLeft,aTop,aWidth,aHeight);
}
