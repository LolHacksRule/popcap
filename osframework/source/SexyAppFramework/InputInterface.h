#ifndef __INPUTINTERFACE_H__
#define __INPUTINTERFACE_H__

#include "Common.h"
#include "NativeDisplay.h"

namespace Sexy
{

class InputManager;
class InputDriver;

struct InputInfo {
	std::string mName;
	bool        mHasPointer;
	bool        mHasKey;
	bool        mHasAcc;
	bool        mHasGyro;
	int         mId;

	void Reset()
	{
		mName = "Unknown";
		mHasPointer = false;
		mHasKey = false;
		mHasAcc = false;
		mHasGyro = false;
		mId = -1;
	}
};

class InputInterface {
public:
	InputInterface(InputManager* theManager);

	virtual ~InputInterface();

public:
	virtual bool	      Init();
	virtual void	      Cleanup();

	virtual bool	      HasEvent ();
	virtual bool	      GetEvent (Event &event);
	virtual void	      Connect ();
	virtual void	      Reconnect();
	virtual void	      Update ();
	virtual bool          GetInfo(InputInfo &theInfo);

	virtual bool          GetProperty (const std::string& name,
					   void* retval);

	void		      PostEvent(Event & event, int subid = 0);
protected:
	InputManager*	      mManager;

public:
	int		      mId;
	InputDriver	     *mInputDriver;
};

}

#endif
