#include "SelectableWidget.h"

using namespace Sexy;


SelectableWidget::SelectableWidget()
{
	mSelectAlpha = 1.0f;
	mSelectMinAlpha = 0.4f;
	mSelectMaxAlpha = 1.0f;
	mSelectAlphaFadeSpeed = 0.01;
}


SelectableWidget::~SelectableWidget()
{
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
