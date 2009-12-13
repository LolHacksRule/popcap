#ifndef __SCROLLLISTENER_H__
#define __SCROLLLISTENER_H__

#include "Common.h"

namespace Sexy 
{

class SEXY_EXPORT ScrollListener 
{
public:
	virtual void ScrollPosition(int theId, double thePosition) {};
};

}

#endif // __SCROLLLISTENER_H__
