#pragma once
#include <mfidl.h>
#include <strmif.h>
class H264Encoder
{
public:
	H264Encoder(IMFMediaType *srcType, UINT32 outFrameRate = 30, UINT32 outWidth = 640, UINT32 outHeight = 480);
	~H264Encoder();

	HRESULT ProcessSample(IMFSample * inputsample, IMFSample **outputSample);

private:
	IMFTransform * pTransform;
	ICodecAPI *    pCodec;
	IMFMediaType * pInputType;
	IMFMediaType * pOutputType;
	MFT_OUTPUT_DATA_BUFFER outputDataBuffer;

	UINT32 uOutWidth;
	UINT32 uOutHeight;
	UINT32 uOutFrameRate;

	bool initialized;
	HRESULT initialize();
};

