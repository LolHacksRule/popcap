#include "SelectableWidget.h"
#include "Common.h"

using namespace Sexy;


SelectableWidget::SelectableWidget()
{
	mSelectColor = Color(255, 255, 255, 255);
	mSelectAlpha = 1.0f;
	mSelectMinAlpha = 0.4f;
	mSelectMaxAlpha = 1.0f;
	mSelectAlphaFadeSpeed = 0.01;
}


SelectableWidget::~SelectableWidget()
{
}

int SelectableWidget::GetSelectAlpha()
{
	return int(mSelectAlpha * 255);
}

Color SelectableWidget::GetSelectColor()
{
	Color col = mSelectColor;
	col.mAlpha = AlphaMod(GetSelectAlpha(), col.mAlpha);
	return col;
}

void SelectableWidget::SetSelectAlphaRange(float a1, float a2)
{
	if (a1 > a2)
		return;

	mSelectMinAlpha = a1;
	mSelectMaxAlpha = a2;
}

void SelectableWidget::Update()
{
	Widget::Update();

	if (mIsSelected)
	{
		int oldIntAlpha = int(mSelectAlpha * 255);

		mSelectAlpha += mSelectAlphaFadeSpeed;
		if (mSelectAlpha > mSelectMaxAlpha)
		{
			mSelectAlpha = mSelectMaxAlpha;
			mSelectAlphaFadeSpeed = -mSelectAlphaFadeSpeed;
			MarkDirty();
		}

		if (mSelectAlpha < mSelectMinAlpha)
		{
			mSelectAlpha = mSelectMinAlpha;
			mSelectAlphaFadeSpeed = -mSelectAlphaFadeSpeed;
			MarkDirty();
		}
		if (int(mSelectAlpha * 255) != oldIntAlpha)
			MarkDirty();
	}
	else
	{
		if (mSelectAlpha != 1.0f)
		{
			mSelectAlpha = 1.0f;
			MarkDirty();
		}
	}
}
