#ifndef __SELECTABLE_WIDGET_H__
#define __SELECTABLE_WIDGET_H__

#include "SexyAppFramework/Widget.h"

namespace Sexy
{
	class SelectableWidget : public Widget
	{
	 public:
		SelectableWidget();
		~SelectableWidget();

	 public:
		virtual void                           Update();
		virtual int                            GetSelectAlpha();
		virtual Color                          GetSelectColor();

		void                                   SetSelectAlphaRange(float a1, float a2);

	 public:
		Color                                   mSelectColor;
		float                                   mSelectAlpha;
		float                                   mSelectMinAlpha;
		float                                   mSelectMaxAlpha;
		float                                   mSelectAlphaFadeSpeed;
	};

}

#endif
