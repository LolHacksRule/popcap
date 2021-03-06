#ifndef __SLIDER_H__
#define __SLIDER_H__

#include "SelectableWidget.h"

namespace Sexy
{

class SliderListener;

class Slider : public SelectableWidget
{
public:		
	SliderListener*			mListener;
	double					mVal;
	int						mId;
	Image*					mTrackImage;
	Image*					mThumbImage;

	bool					mDragging;
	int						mRelX;
	int						mRelY;

	bool					mHorizontal;

public:
	Slider(Image* theTrackImage, Image* theThumbImage, int theId, SliderListener* theListener);

	virtual void			SetValue(double theValue);

	virtual bool			HasTransparencies();
	virtual void			Draw(Graphics* g);	

	virtual void			MouseMove(int x, int y);
	virtual void			MouseDown(int x, int y, int theClickCount);
	virtual void			MouseDrag(int x, int y);
	virtual void			MouseUp(int x, int y);
	virtual void			MouseLeave();
	virtual void                    TouchDown(int id, int x, int y, int tapCount);
	virtual void                    TouchUp(int id, int x, int y , int tapCount);
	virtual void                    TouchMove(int id, int x, int y);
	virtual void                    TouchCancel(const TouchVector &touches);

	virtual bool                    KeyDown(KeyCode theKey);
};

}

#endif //__SLIDER_H__
