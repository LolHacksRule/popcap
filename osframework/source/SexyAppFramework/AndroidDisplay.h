#ifndef __ANDROIDDISPLAY_H__
#define __ANDROIDDISPLAY_H__

#include "Common.h"
#include "CritSect.h"
#include "GLDisplay.h"
#include "Rect.h"
#include "Ratio.h"

#include <GLES/gl.h>

extern "C" {
	struct awEvent;
}

namespace Sexy
{

class SexyAppBase;
class Image;
class MemoryImage;

class AndroidDisplay : public GLDisplay
{
public:
	AndroidDisplay(SexyAppBase* theApp);
	virtual ~AndroidDisplay();

	virtual int				Init();
	virtual void				Cleanup();
	virtual bool				CanReinit();
	virtual bool				Reinit();

	virtual void				SwapBuffers();

	virtual void				RemapMouse(int& theX, int& theY);
	virtual Image*				CreateImage(SexyAppBase * theApp,
							    int width, int height);
	virtual bool				HasEvent();
	virtual bool				GetEvent(struct Event &event);

	void                                    HandleKeyEvent(const awEvent*event);
	void                                    HandlePointerEvent(const awEvent*event);
	static void                             HandleEvents(const awEvent*, void* data);

private:
	std::list<Event>                        mEvents;
};

}

#endif

