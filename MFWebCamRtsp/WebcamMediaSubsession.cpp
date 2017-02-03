#include "WebcamMediaSubsession.h"

#include "MFWebCamH264Source.h"

#include "Utilities.h"

WebcamMediaSubsession * WebcamMediaSubsession::createNew(UsageEnvironment & env, FramedSource * source)
{
	return new WebcamMediaSubsession(env,source);
}

WebcamMediaSubsession::WebcamMediaSubsession(UsageEnvironment & env, FramedSource * source)
	: OnDemandServerMediaSubsession(env, True)
{
	mp_source = source;
	mp_sdp_line = 0;
}

WebcamMediaSubsession::~WebcamMediaSubsession()
{
	if (mp_sdp_line) free(mp_sdp_line);
}

const char * WebcamMediaSubsession::getAuxSDPLine(RTPSink * sink, FramedSource * source)
{
	if (mp_sdp_line) return mp_sdp_line;

	mp_dummy_rtpsink = sink;
	mp_dummy_rtpsink->startPlaying(*source, 0, 0);
	//mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);  
	chkForAuxSDPLine(this);
	m_done = 0;
	envir().taskScheduler().doEventLoop(&m_done);
	mp_sdp_line = _strdup(mp_dummy_rtpsink->auxSDPLine());
	mp_dummy_rtpsink->stopPlaying();
	printf("sdp:%s\n", mp_sdp_line);
	debug_log("sdp:%s\n", mp_sdp_line);
	return mp_sdp_line;
}

RTPSink * WebcamMediaSubsession::createNewRTPSink(Groupsock * rtpsock, unsigned char type, FramedSource * source)
{
	return H264VideoRTPSink::createNew(envir(), rtpsock, type);
}

FramedSource * WebcamMediaSubsession::createNewStreamSource(unsigned sid, unsigned & bitrate)
{
	bitrate = 480;
	return H264VideoStreamFramer::createNew(envir(), MFWebCamH264Source::createNew(envir()));
}

void WebcamMediaSubsession::afterPlayingDummy(void * ptr)
{
	WebcamMediaSubsession *This = (WebcamMediaSubsession*)ptr;
	This->m_done = 0xff;
}

void WebcamMediaSubsession::chkForAuxSDPLine(void * ptr)
{
	WebcamMediaSubsession *This = (WebcamMediaSubsession *)ptr;
	This->chkForAuxSDPLine1();
}

void WebcamMediaSubsession::chkForAuxSDPLine1()
{
	if (mp_dummy_rtpsink->auxSDPLine())
		m_done = 0xff;
	else {
		int delay = 100 * 1000; // 100ms  
		nextTask() = envir().taskScheduler().scheduleDelayedTask(delay,
			chkForAuxSDPLine, this);
	}
}

