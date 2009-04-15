/* Original AudiereLoader by Rheenen 2005 */
#include "AudiereLoader.h"
#include "audiere.h"

#include <cstdlib>

audiere::AudioDevicePtr gAudiereDevice;
audiere::MIDIDevicePtr gAudiereMIDIDevice;

audiere::AudioDevicePtr getAudiereDevice(void)
{
	const char * audiereDevice = getenv("SEXY_AUDIERE_DEVICE");
	const char * audiereParameters = getenv("SEXY_AUDIERE_PARAMETERS");
#ifdef WIN32
	if (!audiereDevice)
		audiereDevice = "winmm";
#endif
	if (!gAudiereDevice)
		gAudiereDevice = audiere::OpenDevice(audiereDevice, audiereParameters);
	return gAudiereDevice;
}

audiere::MIDIDevicePtr getAudiereMIDIDevice(void)
{
	if (!gAudiereMIDIDevice)
		gAudiereMIDIDevice = audiere::OpenMIDIDevice(NULL);
	return gAudiereMIDIDevice;
}
