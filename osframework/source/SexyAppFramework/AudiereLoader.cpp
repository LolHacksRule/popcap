/* Original AudiereLoader by Rheenen 2005 */
#include "AudiereLoader.h"
#include "audiere.h"

#include <cstdlib>

audiere::AudioDevicePtr gAudiereDevice;
audiere::MIDIDevicePtr gAudiereMIDIDevice;

audiere::AudioDevicePtr getAudiereDevice(void)
{
	const char * audiereDevice = getenv("SEXY_AUDIERE_DEVICE");
#ifdef WIN32
	if (!audiereDevice)
		audiereDevice = "winmm";
#endif
	if (!gAudiereDevice)
		gAudiereDevice = audiere::OpenDevice(audiereDevice);
	return gAudiereDevice;
}

audiere::MIDIDevicePtr getAudiereMIDIDevice(void)
{
	if (!gAudiereMIDIDevice)
		gAudiereMIDIDevice = audiere::OpenMIDIDevice(NULL);
	return gAudiereMIDIDevice;
}
