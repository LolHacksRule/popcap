#include "PerfTimer.h"
#include <map>

using namespace Sexy;

#ifndef _MSC_VER
#define mutable
#else
#define int64_t __int64
#endif

inline int QueryCounters(int64_t *lpPerformanceCount)
{
#ifdef WIN32
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	*lpPerformanceCount = now.QuadPart;
#else
	struct timeval now;

	gettimeofday(&now, NULL);
	*lpPerformanceCount = now.tv_sec * 1000 << 32 | now.tv_usec / 1000;
#endif
	return 1;
}

inline int64_t GetFrequence()
{
#ifdef WIN32
	LARGE_INTEGER freq;
	
	freq.QuadPart = 1;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
#else
	return 1;
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PerfTimer::PerfTimer()
{
	mDuration = 0;
#ifdef WIN32
	mStart.QuadPart = 0;
	mFreq.QuadPart = 0;
#else
	mStart.tv_sec = 0;
	mStart.tv_usec = 0;
#endif
	mRunning = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PerfTimer::CalcDuration()
{
#ifdef WIN32
	LARGE_INTEGER anEnd, aFreq;
	QueryPerformanceCounter(&anEnd);
	QueryPerformanceFrequency(&aFreq);
	mDuration = ((anEnd.QuadPart-mStart.QuadPart)*1000)/(double)aFreq.QuadPart;
#else
	struct timeval anEnd;
	gettimeofday(&anEnd, NULL);
	mDuration = (anEnd.tv_sec - mStart.tv_sec) * 1000;
	mDuration += (anEnd.tv_usec - mStart.tv_usec) / 1000;
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PerfTimer::Start()
{
	mRunning = true;
#ifdef WIN32
	QueryPerformanceCounter(&mStart);
#else
	gettimeofday(&mStart, NULL);
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void PerfTimer::Stop()
{
	if(mRunning)
	{
		CalcDuration();
		mRunning = false;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
double PerfTimer::GetDuration()
{
	if(mRunning)
		CalcDuration();

	return mDuration;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct PerfInfo
{
	const char *mPerfName;
	mutable int64_t mStartTime;
	mutable int64_t mDuration;
	mutable double mMillisecondDuration;
	mutable double mLongestCall;
	mutable int mStartCount;
	mutable int mCallCount;

	PerfInfo(const char *theName) : mPerfName(theName), mStartTime(0), mDuration(0), mStartCount(0), mCallCount(0), mLongestCall(0) { }

	bool operator<(const PerfInfo &theInfo) const { return stricmp(mPerfName,theInfo.mPerfName)<0; }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef std::set<PerfInfo> PerfInfoSet;
static PerfInfoSet gPerfInfoSet;
static bool gPerfOn = false;
static int64_t gStartTime;
static int64_t gCollateTime;
double gDuration = 0;
int gStartCount = 0;
int gPerfRecordTop = 0;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct PerfRecord
{
	const char *mName;
	int64_t mTime;
	bool mStart;

	PerfRecord() { }
	PerfRecord(const char *theName, bool start) : mName(theName), mStart(start) { QueryCounters(&mTime); }
};
typedef std::vector<PerfRecord> PerfRecordVector;
PerfRecordVector gPerfRecordVector;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static inline void InsertPerfRecord(PerfRecord &theRecord)
{
	if(theRecord.mStart)
	{
		PerfInfoSet::iterator anItr = gPerfInfoSet.insert(PerfInfo(theRecord.mName)).first;
		PerfInfo * anInfo = const_cast<PerfInfo*>(&(*anItr));

		anInfo->mCallCount++;

		if ( ++anInfo->mStartCount == 1)
			anInfo->mStartTime = theRecord.mTime;
	}
	else
	{
		PerfInfoSet::iterator anItr = gPerfInfoSet.find(theRecord.mName);
		PerfInfo * anInfo = const_cast<PerfInfo*>(&(*anItr));

		if(anItr != gPerfInfoSet.end())
		{
			if( --anInfo->mStartCount == 0)
			{
				int64_t aDuration = theRecord.mTime - anInfo->mStartTime;
				anInfo->mDuration += aDuration;

				if (aDuration > anInfo->mLongestCall)
					anInfo->mLongestCall = (double)aDuration;
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static inline void CollatePerfRecords()
{
	int64_t aTime1,aTime2;
	QueryCounters(&aTime1);

	for(int i=0; i<gPerfRecordTop; i++)
		InsertPerfRecord(gPerfRecordVector[i]);

	gPerfRecordTop = 0;
	QueryCounters(&aTime2);

	gCollateTime += aTime2-aTime1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static inline void PushPerfRecord(const PerfRecord &theRecord)
{
	if(gPerfRecordTop >= (int)gPerfRecordVector.size())
		gPerfRecordVector.push_back(theRecord);
	else
		gPerfRecordVector[gPerfRecordTop] = theRecord;

	++gPerfRecordTop;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SexyPerf::IsPerfOn()
{
	return gPerfOn;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyPerf::BeginPerf(bool measurePerfOverhead)
{
	gPerfInfoSet.clear();
	gPerfRecordTop = 0;
	gStartCount = 0;
	gCollateTime = 0;

	if(!measurePerfOverhead)
		gPerfOn = true;
	
	QueryCounters(&gStartTime);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyPerf::EndPerf()
{
	int64_t anEndTime;
	QueryCounters(&anEndTime);

	CollatePerfRecords();

	gPerfOn = false;

	int64_t aFreq = GetFrequence();

	gDuration = ((double)(anEndTime - gStartTime - gCollateTime))*1000/aFreq;

	for (PerfInfoSet::iterator anItr = gPerfInfoSet.begin(); anItr != gPerfInfoSet.end(); ++anItr)
	{
		PerfInfo &anInfo = const_cast<PerfInfo&>(*anItr);
		anInfo.mMillisecondDuration = (double)anInfo.mDuration*1000/aFreq;
		anInfo.mLongestCall = anInfo.mLongestCall*1000/aFreq;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyPerf::StartTiming(const char *theName)
{
	if(gPerfOn)
	{
		++gStartCount;
		PushPerfRecord(PerfRecord(theName,true));
	}
}

	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyPerf::StopTiming(const char *theName)
{
	if(gPerfOn)
	{
		PushPerfRecord(PerfRecord(theName,false));
		if(--gStartCount==0)
			CollatePerfRecords();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string SexyPerf::GetResults()
{
	std::string aResult;
	char aBuf[512];

	sprintf(aBuf,"Total Time: %.2f\n",gDuration);
	aResult += aBuf;
	for (PerfInfoSet::iterator anItr = gPerfInfoSet.begin(); anItr != gPerfInfoSet.end(); ++anItr)
	{
		const PerfInfo &anInfo = *anItr;
		snprintf(aBuf, sizeof(aBuf), "%s (%d calls, %%%.2f time): %.2f (%.2f avg, %.2f longest)\n",
			 anInfo.mPerfName,anInfo.mCallCount,anInfo.mMillisecondDuration/gDuration*100,
			 anInfo.mMillisecondDuration,anInfo.mMillisecondDuration/anInfo.mCallCount,anInfo.mLongestCall);
		aResult += aBuf;
	}


	return aResult;
}

