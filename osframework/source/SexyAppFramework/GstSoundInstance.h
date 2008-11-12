#ifndef __GstSOUNDINSTANCE_H__
#define __GstSOUNDINSTANCE_H__

#include "SoundInstance.h"
#include "AutoCrit.h"

#include <gst/gst.h>

namespace Sexy
{

class GstSoundManager;
class GstSoundInstance;

class GstSoundInstance : public SoundInstance
{
	friend class GstSoundManager;

protected:
	GstSoundManager*			mSoundManagerP;
	bool					mAutoRelease;
	bool					mHasPlayed;
	bool					mReleased;
        bool                                    mLoop;

	int					mBasePan;
	double					mBaseVolume;

	int					mPan;
	double					mVolume;

protected:
	void					RehupVolume();
	void					RehupPan();

public:
	GstSoundInstance(GstSoundManager* theSoundManager,
                         const std::string&     theUri);
	virtual ~GstSoundInstance();
	virtual void			Release();

	virtual void			SetBaseVolume(double theBaseVolume);
	virtual void			SetBasePan(int theBasePan);

	virtual void			SetVolume(double theVolume);
	virtual void			SetPan(int thePosition); //-hundredth db to +hundredth db = left to right
	virtual void			AdjustPitch(double theNumSteps);

	virtual bool			Play(bool looping, bool autoRelease);
	virtual void			Stop();
	virtual bool			IsPlaying();
	virtual bool			IsReleased();
	virtual double			GetVolume();

        virtual void                    Pause();
        virtual void                    Resume();

 private:
        static gboolean                 MessageHandler (GstBus * bus, GstMessage * msg,
                                                        gpointer data);

        static gboolean                 TimeoutHandler (gpointer data);

        guint	                        mBusid;
        guint                           mTimeoutid;
	GstBin*                         mBin;
	GstBus*                         mBus;
        gchar*                          mUrl;

        GstClockTime                    mDuration;

        CritSect                        mLock;
        bool                            mEosWorks;
};

}

#endif //__GSTSOUNDINSTANCE_H__
