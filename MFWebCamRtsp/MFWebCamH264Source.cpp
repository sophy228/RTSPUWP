#include "MFWebCamH264Source.h"

#include "stdafx.h"
#include "Utilities.h"
#include <mfapi.h>
MFWebCamH264Source * MFWebCamH264Source::createNew(UsageEnvironment & env)
{
	return  new MFWebCamH264Source(env);
}

MFWebCamH264Source::MFWebCamH264Source(UsageEnvironment& env)
	:FramedSource(env)
	,m_nRefCount(1)
{
	
	device =  CaptureDevice::GetDeviceInstance(0,VIDCAP,true);
	IMFMediaType * type;
	device->ChooseMediaType(1, &type);
	UINT32 width, height, framerate, ratio;
	//MFSetAttributeSize(type, MF_MT_FRAME_RATE,8, 1);
	MFGetAttributeSize(type, MF_MT_FRAME_SIZE, &width, &height);
	MFGetAttributeRatio(type, MF_MT_FRAME_RATE, &framerate, &ratio);
	encoder = new  H264Encoder(type, framerate, width, height);

	eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);

	endofstream = false;
	firstsample = true;

	LARGE_INTEGER f;            /*timer frequence*/
	QueryPerformanceFrequency(&f);
	dqFreq = (double)f.QuadPart;

	InitializeCriticalSection(&m_critsec);
	sampleAvalible = false;
}


MFWebCamH264Source::~MFWebCamH264Source()
{
	delete encoder;
	
	envir().taskScheduler().deleteEventTrigger(eventTriggerId);
	eventTriggerId = 0;
}

bool MFWebCamH264Source::isH264VideoStreamFramer() const {
	return true;
}

void MFWebCamH264Source::doGetNextFrame()
{
	// This function is called (by our 'downstream' object) when it asks for new data.

	// Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
	if (endofstream) {
		firstsample = true;
		handleClosure();
		return;
	}

	// If a new frame of data is immediately available to be delivered, then do this now:
	if (!endofstream) {
		deliverFrame();
	}
}

void MFWebCamH264Source::deliverFrame0(void * clientData)
{
	((MFWebCamH264Source*)clientData)->deliverFrame();
}

void MFWebCamH264Source::deliverFrame()
{
	IMFSample *pSample = NULL;
	IMFSample *pH264Sample = NULL;
	HRESULT hr;
	bool framesent = false;

	if (!isCurrentlyAwaitingData()) 
		return;
	ReadStatus status = device->ReadSample(&pSample);
	if (status == READOK)
	{
		if (pSample) {
			hr = encoder->ProcessSample(pSample, &pH264Sample);
			if (SUCCEEDED(hr))
			{
				if (pH264Sample != NULL)
				{
					BYTE* data;
					IMFMediaBuffer* buffer;
					DWORD max, current;
					LONGLONG llSampleDuration;
					pH264Sample->GetSampleDuration(&llSampleDuration);
					//pH264Sample->SetSampleDuration(llSampleDuration);
					fDurationInMicroseconds = llSampleDuration /10;

					pH264Sample->GetBufferByIndex(0, &buffer);
					buffer->Lock(&data, &max, &current);
					if (current > fMaxSize) {
						fFrameSize = fMaxSize;
						fNumTruncatedBytes = current - fMaxSize;
					}
					else {
						fFrameSize = current;
					}
					memmove(fTo, data, fFrameSize);
					//printf("send %d\n", fFrameSize);
					buffer->Unlock();

					gettimeofday(&fPresentationTime, NULL);
					FramedSource::afterGetting(this);

					framesent = true;
					SafeRelease(&buffer);
				}
				SafeRelease(&pH264Sample);
			}
		}
		SafeRelease(&pSample);
	}
	else if(status == READEND)
		endofstream = true;

	if (!framesent && endofstream != true)
	{
		envir().taskScheduler().triggerEvent(eventTriggerId, this);
	}
	else
	{
		++cSamples;
		if (firstsample)
		{
			QueryPerformanceCounter(&time_start);
			firstsample = false;
		}
		else
			QueryPerformanceCounter(&time_over);
		double usedtime = ((time_over.QuadPart - time_start.QuadPart) / dqFreq);

		if (usedtime > 1)

		{

			printf(" cSamples = %d\n", cSamples);
			debug_log(" cSamples = %d\n", cSamples);
			cSamples = 0;

			QueryPerformanceCounter(&time_start);

		}
	}

}
