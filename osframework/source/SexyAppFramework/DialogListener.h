#ifndef __DIALOGLISTENER_H__
#define __DIALOGLISTENER_H__

namespace Sexy
{

class SEXY_EXPORT DialogListener
{
public:
        virtual ~DialogListener() {}

	virtual void			DialogButtonPress(int theDialogId, int theButtonId) {}
	virtual void			DialogButtonDepress(int theDialogId, int theButtonId) {}
};

}

#endif // __DIALOGLISTENER_H__
