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

	 public:
		float                                   mSelectAlpha;
		float                                   mSelectMinAlpha;
		float                                   mSelectMaxAlpha;
		float                                   mSelectAlphaFadeSpeed;
	};

}

#endif
