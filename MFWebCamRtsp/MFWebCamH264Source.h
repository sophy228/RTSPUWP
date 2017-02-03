#pragma once
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "CaptureDevice.h"
#include "H264Encoder.h"
class MFWebCamH264Source :
	public FramedSource

{

public :
	virtual Boolean isH264VideoStreamFramer() const;
	static MFWebCamH264Source * createNew(UsageEnvironment& env);
protected:
	MFWebCamH264Source(UsageEnvironment& env);
	~MFWebCamH264Source();
private:
	CaptureDevice * device;
	H264Encoder * encoder;
	bool  endofstream;
	
	EventTriggerId eventTriggerId;
	
	virtual void doGetNextFrame();
	static void deliverFrame0(void* clientData);

	void deliverFrame();
	bool firstsample;
	_LARGE_INTEGER time_start;    /*begin time */
	_LARGE_INTEGER time_over;        /*end time*/
	double dqFreq;                /*timer frequence*/
	size_t  cSamples = 0;

	long                m_nRefCount;        // Reference count.
	CRITICAL_SECTION    m_critsec;

	bool				sampleAvalible;
};

