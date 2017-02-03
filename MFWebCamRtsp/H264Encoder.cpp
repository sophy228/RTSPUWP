#include "stdafx.h"
#include "H264Encoder.h"
#include <mfapi.h>
#include <wmcodecdsp.h>
#include <mferror.h>
#include <codecapi.h>
#include "Utilities.h"
#pragma comment(lib, "wmcodecdspuuid.lib")
H264Encoder::H264Encoder(IMFMediaType *srcType, UINT32 outFrameRate, UINT32 outWidth , UINT32 outHeight)
	:pInputType(srcType)
	,uOutWidth(outWidth)
	,uOutHeight(outHeight)
	,uOutFrameRate(outFrameRate)
{
	pTransform = NULL;
	pOutputType = NULL;
	pCodec = NULL;
	initialized = false;
	HRESULT hr = initialize();
	if (SUCCEEDED(hr))
		initialized = true;
}


H264Encoder::~H264Encoder()
{
	SafeRelease(&pTransform);
	SafeRelease(&pOutputType);
	SafeRelease(&pCodec);
}

HRESULT H264Encoder::ProcessSample(IMFSample * inputsample, IMFSample ** outputSample)
{
	HRESULT hr;
	DWORD mftOutFlags;
	hr = pTransform->ProcessInput(0, inputsample, 0);
	if (SUCCEEDED(hr))
	{
		hr = pTransform->GetOutputStatus(&mftOutFlags);
		if (SUCCEEDED(hr))
		{
			if (mftOutFlags == MFT_OUTPUT_STATUS_SAMPLE_READY)
			{
				MFT_OUTPUT_STREAM_INFO StreamInfo;
				pTransform->GetOutputStreamInfo(0, &StreamInfo);
				MFCreateSample(outputSample);
				IMFMediaBuffer *pBuffer = NULL;
				MFCreateMemoryBuffer(StreamInfo.cbSize, &pBuffer);
				(*outputSample)->AddBuffer(pBuffer);
				outputDataBuffer.dwStreamID = 0;
				outputDataBuffer.dwStatus = 0;
				outputDataBuffer.pEvents = NULL;
				outputDataBuffer.pSample = *outputSample;
				HRESULT mftProcessOutput = S_OK;
				DWORD processOutputStatus;
				mftProcessOutput = pTransform->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);
				if (mftProcessOutput == MF_E_TRANSFORM_NEED_MORE_INPUT)
				{
					hr = E_FAIL;
				}
				SafeRelease(&pBuffer);
			}
			else {
				//printf("Encoder: No sample.%d\n", mftOutFlags);
				debug_log("Encoder: No sample.%d\n", mftOutFlags);
			}
		}
		
	}
	return hr;
}

HRESULT H264Encoder::initialize()
{
	IUnknown * spTransformUnk;
	HRESULT hr;
	hr = CoCreateInstance(CLSID_CMSH264EncoderMFT, NULL, CLSCTX_INPROC_SERVER,
		IID_IUnknown, (void**)&spTransformUnk);

	if (SUCCEEDED(hr))
	{
		hr = spTransformUnk->QueryInterface(IID_PPV_ARGS(&pTransform));
		if (SUCCEEDED(hr))
		{
			hr = spTransformUnk->QueryInterface(IID_PPV_ARGS(&pCodec));
			VARIANT value;
			value.vt = VT_UI4;
			value.ulVal = 0;
			hr = pCodec->SetValue(&CODECAPI_AVEncMPVDefaultBPictureCount, &value);
			value.vt = VT_UI4;
			value.ulVal = eAVEncAdaptiveMode_FrameRate;
			hr = pCodec->SetValue(&CODECAPI_AVEncAdaptiveMode, &value);
			if (SUCCEEDED(hr))
			{
				MFCreateMediaType(&pOutputType);
				pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
				pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
				pOutputType->SetUINT32(MF_MT_AVG_BITRATE, 480000);
				MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, uOutWidth, uOutHeight);
				MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, uOutFrameRate, 1);
				MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
				pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, 2);	// 2 = Progressive scan, i.e. non-interlaced.
				pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);

				pTransform->SetOutputType(0, pOutputType, 0);
				pTransform->SetInputType(0, pInputType, 0);

				DWORD mftStatus;
				hr = pTransform->GetInputStatus(0, &mftStatus);
				if (SUCCEEDED(hr))
				{
					if (MFT_INPUT_STATUS_ACCEPT_DATA == mftStatus) {
						pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
						pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
						pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
						memset(&outputDataBuffer, 0, sizeof outputDataBuffer);
					}
					else
						hr = E_FAIL;
				}
			}
		}
	}
	return hr;
}
