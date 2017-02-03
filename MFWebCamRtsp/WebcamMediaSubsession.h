#pragma once
#include "liveMedia.hh"
class WebcamMediaSubsession : public OnDemandServerMediaSubsession
{
public:
	static WebcamMediaSubsession *createNew(UsageEnvironment &env, FramedSource *source);
protected:
	WebcamMediaSubsession(UsageEnvironment &env, FramedSource *source);
	~WebcamMediaSubsession();

	virtual const char *getAuxSDPLine(RTPSink *sink, FramedSource *source);
	virtual RTPSink *createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source);
	virtual FramedSource *createNewStreamSource(unsigned sid, unsigned &bitrate);
private:
	FramedSource *mp_source; // ∂‘”¶ WebcamFrameSource  
	char *mp_sdp_line;
	RTPSink *mp_dummy_rtpsink;
	char m_done;

	static void afterPlayingDummy(void *ptr);
	static void chkForAuxSDPLine(void *ptr);
	void chkForAuxSDPLine1();

};

