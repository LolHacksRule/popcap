#include "GstSoundInstance.h"
#include "GstSoundManager.h"
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

	GstElement * fakesink;

	mBin = (GstBin *)gst_element_factory_make ("playbin", 0);
	if (mBin)
	{
		fakesink = gst_element_factory_make ("fakesink", 0);
		g_object_set (G_OBJECT (mBin), "video-sink", fakesink, NULL);

		mBus = gst_pipeline_get_bus (GST_PIPELINE (mBin));
		mBusid = gst_bus_add_watch (mBus, MessageHandler, this);

		const gchar* uri = theUri.c_str ();
		if (gst_uri_is_valid (uri)) {
			g_object_set (G_OBJECT (mBin), "uri", uri, NULL);
			mUrl = g_strdup (uri);
		} else {
			gchar * url;

			if (g_path_is_absolute  (uri)) {
				url = g_strdup_printf ("%s%s", "file://", uri);
			} else {
				gchar * current_dir = g_get_current_dir ();

				url = g_strdup_printf ("%s%s/%s", "file://", current_dir, uri);
				g_free (current_dir);
			}

			gchar* filename = strrchr (url + 7, '/');
			if (!filename)
				filename = url + 7;
			if (!strrchr (filename, '.'))
			{
				static const gchar* extensions[]  = {
					".ogg", ".OGG", ".mp3", ".MP3", ".wav", NULL
				};

				for (int i = 0; extensions[i]; i++)
				{
					gchar * newurl;

					newurl = g_strdup_printf ("%s%s", url, extensions[i]);
					if (!access (newurl + 7, F_OK))
					{
						g_free (url);
						url = newurl;
						break;
					}
					g_free (newurl);
				}
			}

			if (0)
				g_print ("uri = %s\n", url);
			g_object_set (G_OBJECT (mBin), "uri", url, NULL);
			mUrl = url;
		}
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
		GstState state, pending;

		gst_element_get_state (GST_ELEMENT (mBin), &state, &pending, 0);
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
		g_source_remove (mBusid);

		gst_object_unref (G_OBJECT (mBin));
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

	gst_element_set_state (GST_ELEMENT (mBin), GST_STATE_READY);
}

void GstSoundInstance::Pause()
{
	if (!mBin || !IsPlaying ())
		return;

	gst_element_set_state (GST_ELEMENT (mBin), GST_STATE_PAUSED);
}

void GstSoundInstance::Resume()
{
	if (!mBin)
		return;

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
			g_print ("%s: received EOS.\n", GST_OBJECT_NAME (GST_MESSAGE_SRC (msg)));
		if (player->mLoop)
		{
			if (0)
				g_print ("restarting player.\n");
			gst_element_set_state (GST_ELEMENT (player->mBin), GST_STATE_READY);
			gst_element_set_state (GST_ELEMENT (player->mBin), GST_STATE_PLAYING);
		}
		else if (player->mAutoRelease)
		{
			player->mReleased = true;
		}
		break;
	default:
		break;
	}

	return TRUE;
}
