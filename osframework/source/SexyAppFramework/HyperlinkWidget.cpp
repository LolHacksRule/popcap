#include "HyperlinkWidget.h"
#include "Graphics.h"
#include "ImageFont.h"
#include "NativeFont.h"
#include "WidgetManager.h"
#include "SexyAppBase.h"

using namespace Sexy;

HyperlinkWidget::HyperlinkWidget(int theId, ButtonListener* theButtonListener) :
	ButtonWidget(theId, theButtonListener),
	mColor(255, 255, 255),
	mOverColor(255, 255, 255)
{	
	mDoFinger = true;
	mUnderlineOffset = 3;
	mUnderlineSize = 1;

	mDrawFocusRect = true;
}

void HyperlinkWidget::Draw(Graphics* g)
{
	if (mFont == NULL)
		mFont = mWidgetManager->mApp->mDDInterface->CreateFont
			(mWidgetManager->mApp, "Arial Unicode MS", 10); //baz changed

	int aFontX = (mWidth - mLabelText.GetWidth())/2;
	int aFontY = (mHeight + mFont->GetAscent())/2 - 1;

	if (mIsOver || mHasFocus)
		g->SetColor(mOverColor);
	else
		g->SetColor(mColor);

	if (mHasFocus && mIsSelected)
	{
		Color col = g->GetColor();
		col.mAlpha = AlphaMod(col.mAlpha, mSelectAlpha * 255);
		g->SetColor(col);
	}

	g->SetFont(mFont);
	//g->DrawString(mLabel, aFontX, aFontY);
	mLabelText.Draw(g, aFontX, aFontY, g->GetColor());

	for (int i = 0; i < mUnderlineSize; i++)
		g->FillRect(aFontX, aFontY+mUnderlineOffset+i, mFont->StringWidth(mLabel), 1);
}

void HyperlinkWidget::MouseEnter()
{
	ButtonWidget::MouseEnter();

	MarkDirtyFull();
}

void HyperlinkWidget::MouseLeave()
{
	ButtonWidget::MouseLeave();

	MarkDirtyFull();
}
