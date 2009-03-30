#ifndef __INPUTINTERFACE_H__
#define __INPUTINTERFACE_H__

#include "Common.h"
#include "NativeDisplay.h"

namespace Sexy
{

class InputManager;
class InputDriver;
class InputInterface {
public:
	InputInterface(InputManager* theManager);

	virtual ~InputInterface();

public:
	virtual bool	      Init();
	virtual void	      Cleanup();

	virtual bool	      HasEvent () = 0;
	virtual bool	      GetEvent (Event & event) = 0;
	virtual void	      Connect ();
	virtual void	      Reconnect();
	virtual void	      Update ();

	void		      PostEvent(Event & event);
protected:
	InputManager*	      mManager;

public:
	int		      mId;
	InputDriver	     *mInputDriver;
};

}

#endif
