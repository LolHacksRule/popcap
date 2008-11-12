#include "GstSoundInstance.h"
#include "GstSoundManager.h"
#include "GstPakSrc.h"

#include <cstring>

using namespace Sexy;

static inline gboolean
object_has_property (GObject * object, const char * name)
{
	GObjectClass * klass;

	g_return_val_if_fail (object && name, FALSE);

	klass = G_OBJECT_GET_CLASS (object);
	return g_object_class_find_property (klass, name) != NULL;
}

#define GOBJECT_HAS_PROPERTY(o, n) \
	object_has_property (G_OBJECT (o), (n))

static gchar * pak_probe (const gchar * origuri)
{
	gchar * uri;

	const gchar* filename = strrchr (origuri, '/');
	if (!filename)
		filename = origuri;
	if (!strrchr (filename, '.'))
	{
		static const gchar* extensions[]  = {
			".ogg", ".OGG", ".mp3", ".MP3", ".wav", NULL
		};

		for (int i = 0; extensions[i]; i++)
		{
			PFILE * fp;

			uri = g_strdup_printf ("pak://%s%s", origuri, extensions[i]);
			fp = p_fopen (uri + 6, "rb");
			if (fp)
			{
				p_fclose (fp);
				return uri;
			}
			g_free (uri);
		}
	}

	PFILE * fp;

	uri = g_strdup_printf ("pak://%s", origuri);
	fp = p_fopen (uri + 6, "rb");
	if (fp)
	{
		p_fclose (fp);
		return uri;
	}
	g_free (uri);

	return NULL;
}

static gchar* file_probe (const gchar * origuri)
{
	gchar * uri;

	if (g_path_is_absolute  (origuri)) {
		uri = g_strdup_printf ("%s%s", "file://", origuri);
	} else {
		gchar * current_dir = g_get_current_dir ();

		uri = g_strdup_printf ("%s%s/%s", "file://", current_dir, origuri);
		g_free (current_dir);
	}

	gchar* filename = strrchr (uri + 7, '/');
	if (!filename)
		filename = uri + 7;
	if (!strrchr (filename, '.'))
	{
		static const gchar* extensions[]  = {
			".ogg", ".OGG", ".mp3", ".MP3", ".wav", NULL
		};

		for (int i = 0; extensions[i]; i++)
		{
			gchar * newuri;

			newuri = g_strdup_printf ("%s%s", uri, extensions[i]);
			if (!access (newuri + 7, F_OK))
			{
				g_free (uri);
				return newuri;
			}
			g_free (newuri);
		}
	}

	return uri;
}

GstSoundInstance::GstSoundInstance(GstSoundManager* theSoundManager,
				   const std::string&     theUri)
	: SoundInstance ()
{
	mSoundManagerP = theSoundManager;
	mReleased = false;
	mAutoRelease = false;
	mHasPlayed = false;
	mLoop = false;
	mBaseVolume = 1.0;
	mBasePan = 0;

	mVolume = 1.0;
	mPan = 0;
	mDuration = 0;
	mBusid = 0;
	mTimeoutid = 0;
	mEosWorks = true;

#ifdef SEXY_INTEL_CANMORE
	mEosWorks = false;
#endif

	GstElement * fakesink;

	mBin = (GstBin *)gst_element_factory_make ("playbin2", 0);
	if (!mBin)
		mBin = (GstBin *)gst_element_factory_make ("playbin", 0);
	if (mBin)
	{
		if (GOBJECT_HAS_PROPERTY (mBin, "flags"))
		{
			g_object_set (G_OBJECT (mBin), "flags",
				      0x00000002 | 0x00000010, NULL);
		}
		else
		{
			fakesink = gst_element_factory_make ("fakesink", 0);
			g_object_set (G_OBJECT (mBin), "video-sink", fakesink, NULL);
		}

		mBus = gst_pipeline_get_bus (GST_PIPELINE (mBin));
		mBusid = gst_bus_add_watch (mBus, MessageHandler, this);

		const gchar* uri = theUri.c_str ();
		if (gst_uri_is_valid (uri)) {
			g_object_set (G_OBJECT (mBin), "uri", uri, NULL);
			mUrl = g_strdup (uri);
		} else {
			gchar * url;

			url = pak_probe (uri);
			if (!uri)
				url = file_probe (uri);
			if (0)
				g_print ("uri = %s\n", url);
			g_object_set (G_OBJECT (mBin), "uri", url, NULL);
			mUrl = url;
		}

		mTimeoutid = g_timeout_add (1500, (GSourceFunc)TimeoutHandler, this);
	}
	else
	{
		mUrl = 0;
	}

	RehupVolume();
}

GstSoundInstance::~GstSoundInstance()
{
	if (mBin)
	{
		{
			AutoCrit aAutoCrit (mLock);

			if (mBusid > 0)
				g_source_remove (mBusid);
			if (mTimeoutid > 0)
				g_source_remove (mTimeoutid);
			mBusid = 0;
			mTimeoutid = 0;
		}
		AutoCrit aAutoCrit (mLock);

		GstState state, pending;

		gst_element_get_state (GST_ELEMENT (mBin), &state, &pending,
				       GST_CLOCK_TIME_NONE);
		for (int i = 0; i < 3; i++)
		{
			if (gst_element_set_state (GST_ELEMENT (mBin),
						   GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE)
				g_usleep (50);
			else
				break;
		}
		gst_element_get_state (GST_ELEMENT (mBin), &state, NULL, 0);
		g_assert (state == GST_STATE_NULL);

		gst_object_unref (G_OBJECT (mBin));

		mBin = 0;
	}

	g_free (mUrl);
}

void GstSoundInstance::RehupVolume()
{
	double volume =
		mBaseVolume * mVolume;

	if (!mBin)
		return;

	if (mSoundManagerP)
		volume *= mSoundManagerP->mMasterVolume;

	g_object_set (G_OBJECT (mBin), "volume", volume, NULL);
}

void GstSoundInstance::RehupPan()
{
}

void GstSoundInstance::Release()
{
	Stop ();
	mReleased = true;
}

void GstSoundInstance::SetVolume(double theVolume) // 0 = max
{
	mVolume = theVolume;
	RehupVolume();
}

void GstSoundInstance::SetPan(int thePosition) //-db to =db = left to right
{
	mPan = thePosition;
	RehupPan();
}

void GstSoundInstance::SetBaseVolume(double theBaseVolume)
{
	mBaseVolume = theBaseVolume;
	RehupVolume();
}

void GstSoundInstance::SetBasePan(int theBasePan)
{
	mBasePan = theBasePan;
	RehupPan();
}

bool GstSoundInstance::Play(bool looping, bool autoRelease)
{
	if (!mBin)
		return false;

	Stop ();

	mHasPlayed = true;
	mAutoRelease = autoRelease;
	mLoop = looping;

	if (0)
	    g_print ("%s: playing.(%s auto release %d loop %d).\n",
		     GST_OBJECT_NAME (GST_OBJECT (mBin)), mUrl,
		     autoRelease, looping);

	gst_element_set_state (GST_ELEMENT (mBin), GST_STATE_PLAYING);
	return true;
}

void GstSoundInstance::Stop()
{
	if (!mBin)
		return;

	GstState state;
	gst_element_get_state (GST_ELEMENT (mBin), &state, NULL,
			       GST_CLOCK_TIME_NONE);
	gst_element_set_state (GST_ELEMENT (mBin), GST_STATE_READY);
	mAutoRelease = false;
}

void GstSoundInstance::Pause()
{
	if (!mBin || !IsPlaying ())
		return;

	GstState state;
	gst_element_get_state (GST_ELEMENT (mBin), &state, NULL,
			       GST_CLOCK_TIME_NONE);
	gst_element_set_state (GST_ELEMENT (mBin), GST_STATE_PAUSED);
}

void GstSoundInstance::Resume()
{
	if (!mBin)
		return;

	GstState state;
	gst_element_get_state (GST_ELEMENT (mBin), &state, NULL,
			       GST_CLOCK_TIME_NONE);
	gst_element_set_state (GST_ELEMENT (mBin), GST_STATE_PLAYING);
}

void GstSoundInstance::AdjustPitch(double theNumSteps)
{
}

bool GstSoundInstance::IsPlaying()
{
	if (!mHasPlayed)
		return false;

	GstState state;

	if (!mBin)
		return false;
	gst_element_get_state (GST_ELEMENT (mBin), &state, NULL, 0);
	if (state == GST_STATE_PLAYING)
		return true;
	return false;
}

bool GstSoundInstance::IsReleased()
{
	return mReleased;
}

double GstSoundInstance::GetVolume()
{
	return mVolume;
}

gboolean
GstSoundInstance::MessageHandler (GstBus * bus, GstMessage * msg, gpointer data)
{
	GstSoundInstance * player =
		reinterpret_cast<GstSoundInstance*> (data);
	GError * err;
	gchar * debug;
	AutoCrit aAutoCrit (player->mLock);

	if (!player->mBin)
		return TRUE;

	switch (GST_MESSAGE_TYPE (msg)) {
	case GST_MESSAGE_WARNING:
	case GST_MESSAGE_ERROR:
		gst_message_parse_error (msg, &err, &debug);
		g_print ("uri: %s\n", player->mUrl);
		g_print ("Source: %s\n", GST_OBJECT_NAME (GST_MESSAGE_SRC (msg)));
		g_print ("Error quark - %s\n", g_quark_to_string (err->domain));
		g_print ("Error code - %d\n", err->code);
		g_print ("Error: %s\n", err->message);
		g_error_free (err);
		g_free (debug);
		return TRUE;
	case GST_MESSAGE_STATE_CHANGED:
	{
		GstState oldstate, newstate, pending;

		gst_message_parse_state_changed (msg, &oldstate, &newstate,
						 &pending);

		if (GST_OBJECT (player->mBin) == GST_MESSAGE_SRC (msg))
		{
			if (0)
			{
				g_print ("%s.\n%s: %s -> %s (pending: %s).\n",
					 player->mUrl,
					 GST_OBJECT_NAME (GST_MESSAGE_SRC (msg)),
					 gst_element_state_get_name (oldstate),
					 gst_element_state_get_name (newstate),
					 gst_element_state_get_name (pending));
			}
			if (0 && newstate == GST_STATE_READY && oldstate == GST_STATE_PAUSED &&
			    player->mHasPlayed && player->mAutoRelease)
				player->mReleased = true;
		}
		break;
	}
	case GST_MESSAGE_EOS:
		if (0)
			g_print ("%s<%s>: received EOS.\n",
				 GST_OBJECT_NAME (GST_MESSAGE_SRC (msg)), player->mUrl);
		if (player->mLoop && player->mEosWorks)
		{
			if (0)
				g_print ("restarting player.\n");
			gst_element_set_state (GST_ELEMENT (player->mBin), GST_STATE_READY);

			GstState state;
			gst_element_get_state (GST_ELEMENT (player->mBin), &state, NULL,
					       GST_CLOCK_TIME_NONE);
			gst_element_set_state (GST_ELEMENT (player->mBin), GST_STATE_PLAYING);
		}
		else if (player->mAutoRelease)
		{
			player->mReleased = true;
		}
		break;
	case GST_MESSAGE_DURATION:
	{
		GstClockTime duration;
		GstFormat format = GST_FORMAT_TIME;

		gst_message_parse_duration (msg, &format, (gint64*) &duration);
		if (format == GST_FORMAT_TIME)
			player->mDuration = duration;
		break;
	}
	default:
		break;
	}

	return TRUE;
}

gboolean GstSoundInstance::TimeoutHandler (gpointer data)
{
	GstSoundInstance * player =
		reinterpret_cast<GstSoundInstance*> (data);

	AutoCrit aAutoCrit (player->mLock);

	if (player->mBin && 0)
		g_print ("<%s>uri: %s autorelease %d released %d loop %d\n",
			 GST_OBJECT_NAME (player->mBin), player->mUrl, player->mAutoRelease,
			 player->mReleased, player->mLoop);

 	if (player->mBin && !player->mEosWorks)
	{
		gint64 position;
		gint64 duration;
		GstFormat format;

		position = 0;
		duration = 0;
		format = GST_FORMAT_TIME;
		gst_element_query_position (GST_ELEMENT (player->mBin), &format, &position);

		format = GST_FORMAT_TIME;
		gst_element_query_duration (GST_ELEMENT (player->mBin), &format, &duration);

		if (0)
			g_print ("<%s>uri: %s autorelease %d released %d loop %d\n"
				 " time: %" GST_TIME_FORMAT "/%" GST_TIME_FORMAT "\n",
				 GST_OBJECT_NAME (player->mBin), player->mUrl, player->mAutoRelease,
				 player->mReleased, player->mLoop, GST_TIME_ARGS (position),
				 GST_TIME_ARGS (duration));

 		if (duration == 0 || ABS (GST_CLOCK_DIFF (position,
							  duration)) > GST_NSECOND * 10)
			return TRUE;

		GstState state;
		gst_element_get_state (GST_ELEMENT (player->mBin), &state, NULL,
				       GST_CLOCK_TIME_NONE);
		if (state != GST_STATE_PLAYING)
			return TRUE;

		if (player->mLoop)
		{
			if (0)
				g_print ("restarting player.\n");
			GstState state;
			gst_element_get_state (GST_ELEMENT (player->mBin), &state, NULL,
					       GST_CLOCK_TIME_NONE);
			gst_element_set_state (GST_ELEMENT (player->mBin), GST_STATE_READY);

			gst_element_get_state (GST_ELEMENT (player->mBin), &state, NULL,
					       GST_CLOCK_TIME_NONE);
			gst_element_set_state (GST_ELEMENT (player->mBin), GST_STATE_PLAYING);
		}
		else if (player->mAutoRelease)
		{
			player->mReleased = true;
		}
	}

	return TRUE;
}
