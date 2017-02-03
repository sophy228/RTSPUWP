// MFWebCamRtsp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MFWebCamH264Source.h"
#include "CaptureDevice.h"
#include "H264Encoder.h"
#include "WebcamMediaSubsession.h"


void CaptureTest()
{
	CaptureDevice * device = CaptureDevice::GetDeviceInstance(0);
	IMFMediaType * type;
	device->ChooseMediaType(0, &type);

	H264Encoder encoder(type);

	HRESULT hr = S_OK;

	IMFSample *pSample = NULL;
	IMFSample *pH264Sample = NULL;

	size_t  cSamples = 0;



	_LARGE_INTEGER time_start;    /*begin time */

	_LARGE_INTEGER time_over;        /*end time*/

	double dqFreq;                /*timer frequence*/

	LARGE_INTEGER f;            /*timer frequence*/

	QueryPerformanceFrequency(&f);

	dqFreq = (double)f.QuadPart;



	QueryPerformanceCounter(&time_start);
	bool quit = false;
	bool encode = true;

	while (!quit)

	{
		ReadStatus status = device->ReadSample(&pSample);
		if (status != READOK)
			quit = true;
		

		if (pSample)
		{
			BYTE* data;

			IMFMediaBuffer* buffer;

			DWORD max, current;

			if (encode)
			{
				hr = encoder.ProcessSample(pSample, &pH264Sample);
				if (FAILED(hr))
					break;
				if (pH264Sample != NULL)

				{
					++cSamples;

					pH264Sample->GetBufferByIndex(0, &buffer);

					buffer->Lock(&data, &max, &current);

					buffer->Unlock();

					SafeRelease(&buffer);

					QueryPerformanceCounter(&time_over);          //In order to find the frames per second of the camera.

					double usedtime = ((time_over.QuadPart - time_start.QuadPart) / dqFreq);

					if (usedtime > 1)

					{

						printf(" cSamples = %d\n", cSamples);

						cSamples = 0;

						QueryPerformanceCounter(&time_start);

					}
					SafeRelease(&pH264Sample);
				}
			}
			else
			{
				++cSamples;

				pSample->GetBufferByIndex(0, &buffer);

				buffer->Lock(&data, &max, &current);

				buffer->Unlock();

				SafeRelease(&buffer);

				QueryPerformanceCounter(&time_over);          //In order to find the frames per second of the camera.

				double usedtime = ((time_over.QuadPart - time_start.QuadPart) / dqFreq);

				if (usedtime > 1)

				{

					printf(" cSamples = %d\n", cSamples);

					cSamples = 0;

					QueryPerformanceCounter(&time_start);

				}
			}
			
		}
		SafeRelease(&pSample);
		
	}
}


void RTPTest()
{
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	MFWebCamH264Source * mediaFoundationH264Source = MFWebCamH264Source::createNew(*env);

	in_addr dstAddr = { 192, 168, 1, 2 };
	Groupsock rtpGroupsock(*env, dstAddr, 1233, 255);
	rtpGroupsock.addDestination(dstAddr, 1234, 0);
	RTPSink * rtpSink = H264VideoRTPSink::createNew(*env, &rtpGroupsock, 96);
	rtpSink->startPlaying(*mediaFoundationH264Source, NULL, NULL);
	//printf("%s", rtpSink->auxSDPLine());

	env->taskScheduler().doEventLoop();
}

void RTSPTest()
{
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	MFWebCamH264Source * mediaFoundationH264Source = MFWebCamH264Source::createNew(*env);

	RTSPServer *rtspServer = RTSPServer::createNew(*env, 8554);
	if (!rtspServer) {
		fprintf(stderr, "ERR: create RTSPServer err\n");
		::exit(-1);
	}

	do {
		ServerMediaSession *sms = ServerMediaSession::createNew(*env, "webcam", 0, "Session from /dev/video0");
		sms->addSubsession(WebcamMediaSubsession::createNew(*env, mediaFoundationH264Source));
		rtspServer->addServerMediaSession(sms);
		char *url = rtspServer->rtspURL(sms);
		*env << "using url \"" << url << "\"\n";
		delete[] url;
	} while (0);

	env->taskScheduler().doEventLoop();
}
int main()
{
	//CaptureTest();
	//RTPTest();
	RTSPTest();
	return 0;
}



