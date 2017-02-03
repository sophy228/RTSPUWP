#include "stdafx.h"
#include "CaptureDevice.h"
#include "Utilities.h"
#include <mfapi.h>
#include <strmif.h>
#include <atlcomcli.h>
#pragma comment(lib, "mfplat.lib")

#pragma comment(lib, "mf.lib")

#pragma comment(lib, "mfreadwrite.lib")

#pragma comment(lib, "Mfuuid.lib")

CaptureDevice * CaptureDevice::deviceinstance;
CaptureDevice * CaptureDevice::GetDeviceInstance(DWORD index, DeviceType type, bool asyncmode)
{
	if (deviceinstance == NULL)
		deviceinstance = new CaptureDevice(index, type, asyncmode);
	return deviceinstance;
}

CaptureDevice::CaptureDevice(DWORD index, DeviceType type, bool asyncmode)
	:devIndex(index)
	,devType(type)
	,m_nRefCount(1), m_bEOS(FALSE), m_asyncmode(asyncmode)

{
	initialized = false;
	latestDirty = false;
	InitializeCriticalSection(&m_critsec);
	latestSample = NULL;
	HRESULT hr = initialize();
	if (SUCCEEDED(hr))
	{
		initialized = true;
	}
}


CaptureDevice::~CaptureDevice()
{
	SafeRelease(&pSource);
	for (DWORD i = 0; i < supportedTypeCount; i++)
	{
		SafeRelease(&supportedTypes[i]);
	}
	MFShutdown();
}

ReadStatus CaptureDevice::ReadSample(IMFSample ** sample, StreamType type)
{
	HRESULT hr = E_FAIL;
	if (initialized)
	{
		DWORD index;
		if (type == AUDStream)
			index = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
		else if (type = VIDStream)
			index = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
		else if (type = AUDVIDStream)
			index = MF_SOURCE_READER_ANY_STREAM;
		if (!m_asyncmode)
		{
			DWORD streamIndex, flags;

			LONGLONG llTimeStamp;
			
			hr = pReader->ReadSample(

				//	MF_SOURCE_READER_ANY_STREAM,    // Stream index.
				index,
				0,                              // Flags.

				&streamIndex,                   // Receives the actual stream index. 

				&flags,                         // Receives status flags.

				&llTimeStamp,                   // Receives the time stamp.

				sample                        // Receives the sample or NULL.

			);
			if (flags & MF_SOURCE_READERF_ENDOFSTREAM)

			{
				return READEND;
			}
		}
		else
		{
			if (m_bEOS)
			{
				return READEND;
			}
			hr = pReader->ReadSample(

				//	MF_SOURCE_READER_ANY_STREAM,    // Stream index.
				index,
				0,                              // Flags.

				NULL,                   // Receives the actual stream index. 

				NULL,                         // Receives status flags.

				NULL,                   // Receives the time stamp.

				NULL                        // Receives the sample or NULL.

			);
			EnterCriticalSection(&m_critsec);
			*sample = latestSample;
			if (latestSample)
			{
				latestSample->AddRef();
				if (!latestDirty)
				{
					LONGLONG llVideoTimeStamp, llSampleDuration;
					latestSample->GetSampleTime(&llVideoTimeStamp);
					latestSample->GetSampleDuration(&llSampleDuration);
					latestSample->SetSampleTime(llVideoTimeStamp + llSampleDuration);
				}
				else
					latestDirty = false;
			}
			LeaveCriticalSection(&m_critsec);
		}
	}

	if (FAILED(hr))
		return READERROR;
	return READOK;
}

HRESULT CaptureDevice::GetMediaType(IMFMediaType ** type, DWORD * count)
{
	if (initialized)
	{
		*type = (IMFMediaType *)malloc(supportedTypeCount * sizeof(IMFMediaType *));
		for (DWORD i = 0; i < supportedTypeCount; i++)
		{
			type [i] = supportedTypes[i];
		}
		*count = supportedTypeCount;
		return S_OK;
	}
	return E_NOT_VALID_STATE;
}

HRESULT CaptureDevice::ChooseMediaType(DWORD index, IMFMediaType ** outtype)
{
	if (initialized)
	{
		IMFPresentationDescriptor *pPD = NULL;

		IMFStreamDescriptor *pSD = NULL;

		IMFMediaTypeHandler *pHandler = NULL;

		IMFMediaType *pType = NULL;



		HRESULT hr = pSource->CreatePresentationDescriptor(&pPD);

		if (FAILED(hr))

			goto done;

		BOOL fSelected;

		hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);

		if (FAILED(hr))

			goto done;



		hr = pSD->GetMediaTypeHandler(&pHandler);

		if (FAILED(hr))

			goto done;

		if (index >= 0 && index < supportedTypeCount) {
			pType = supportedTypes[index];
			*outtype = pType;
			pType->AddRef();
		}
		else
		{
			hr = E_FAIL;
			goto done;
		}

		//MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 15, 1);
		//MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, 15, 1);
		//	pType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, 0);
		hr = pHandler->SetCurrentMediaType(pType);

		LogMediaType(pType);

	done:

		SafeRelease(&pPD);

		SafeRelease(&pSD);

		SafeRelease(&pHandler);

		return hr;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CaptureDevice::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample * pSample)
{
	EnterCriticalSection(&m_critsec);

	if (SUCCEEDED(hrStatus))
	{
		if (pSample)
		{
			// Do something with the sample.
		//	printf("%p: Frame @ %I64d\n", pSample,llTimestamp);
			SafeRelease(&latestSample);
			latestSample = pSample;
			latestSample->AddRef();
			latestDirty = true;
		}
	}
	else
	{
		// Streaming error.
		//NotifyError(hrStatus);
	}

	if (MF_SOURCE_READERF_ENDOFSTREAM & dwStreamFlags)
	{
		// Reached the end of the stream.
		m_bEOS = TRUE;
	}

	LeaveCriticalSection(&m_critsec);
	return S_OK;

}

HRESULT CaptureDevice::initialize()
{
	IMFAttributes *pAttributes = NULL;
	HRESULT hr = CreateVideoDeviceSource(&pSource);
	if (SUCCEEDED(hr))
	{
		hr = GetSupportedCaptureFormats(pSource);
		if (SUCCEEDED(hr))
		{
			if (m_asyncmode)
			{
				hr = MFCreateAttributes(&pAttributes, 1);
				if (SUCCEEDED(hr))
				{
					hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
				}
			}
			if (SUCCEEDED(hr))
			{
				hr = MFCreateSourceReaderFromMediaSource(pSource, pAttributes, &pReader);
				SetupCamera(pSource);
			}
		}
	}
	SafeRelease(&pAttributes);
	return hr;
}

HRESULT CaptureDevice::CreateVideoDeviceSource(IMFMediaSource **ppSource)

{

	HRESULT hr;

	//hr = CoInitialize(NULL);
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr))

		abort();

	hr = MFStartup(MF_VERSION);

	if (FAILED(hr))

		abort();



	*ppSource = NULL;



	IMFMediaSource *pSource = NULL;

	IMFAttributes *pAttributes = NULL;

	IMFActivate **ppDevices = NULL;



	// Create an attribute store to specify the enumeration parameters.

	/*HRESULT*/ hr = MFCreateAttributes(&pAttributes, 1);

	if (FAILED(hr))

		abort();



	// Source type: video capture devices

	GUID value;
	if (devType == VIDCAP)
		value = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;
	else if (devType == AUDCAP)
		value = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID;
	else
	{
		abort();
	}

	hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, value);

	if (FAILED(hr))

		abort();



	// Enumerate devices.

	UINT32 count;

	hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);

	if (FAILED(hr))

		abort();

	if (count == 0)

	{

		hr = E_FAIL;

		return hr;

	}



	// Create the media source object.
	DWORD index = devIndex > count - 1 ? 0 : devIndex;

	hr = ppDevices[index]->ActivateObject(IID_PPV_ARGS(&pSource));

	if (FAILED(hr))

		abort();



	*ppSource = pSource;

	(*ppSource)->AddRef();



	// release part

	SafeRelease(&pAttributes);



	for (DWORD i = 0; i < count; i++)

	{

		SafeRelease(&ppDevices[i]);

	}

	CoTaskMemFree(ppDevices);

	SafeRelease(&pSource);    //此处不确定，是否需要SafeRelease。

	return hr;

}

HRESULT CaptureDevice::GetSupportedCaptureFormats(IMFMediaSource *pSource)

{

	IMFPresentationDescriptor *pPD = NULL;

	IMFStreamDescriptor *pSD = NULL;

	IMFMediaTypeHandler *pHandler = NULL;

	IMFMediaType *pType = NULL;



	HRESULT hr = pSource->CreatePresentationDescriptor(&pPD);

	if (FAILED(hr))

		goto done;



	BOOL fSelected;

	hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);

	if (FAILED(hr))

		goto done;



	hr = pSD->GetMediaTypeHandler(&pHandler);

	if (FAILED(hr))

		goto done;



	DWORD cTypes = 0;

	supportedTypeCount = 0;

	hr = pHandler->GetMediaTypeCount(&cTypes);

	if (FAILED(hr))

		goto done;

	//supportedTypes = (IMFMediaType **)malloc(cTypes*sizeof(IMFMediaType *));

	for (DWORD i = 0; i < cTypes && i < 100; i++)

	{

		hr = pHandler->GetMediaTypeByIndex(i, &pType);

		if (FAILED(hr))

			goto done;

	//	char s[1024];
	//	sprintf(s, "%d:\n", i);
	//	OutputDebugStringA(s);
		LogMediaType(pType);
		OutputDebugString(L"\n");
		supportedTypes[i] = pType;
		supportedTypes[i]->AddRef();
		supportedTypeCount ++;
		SafeRelease(&pType);
	}



done:

	SafeRelease(&pPD);

	SafeRelease(&pSD);

	SafeRelease(&pHandler);

	SafeRelease(&pType);

	return hr;

}

HRESULT CaptureDevice::SetupCamera(IMFMediaSource * pSource)
{
	CComQIPtr<IAMCameraControl> spCameraControl(pSource);
	HRESULT hr = S_OK;
	if (spCameraControl) {
		long min, max, step, def, control;
		hr = spCameraControl->GetRange(CameraControl_Exposure, &min, &max, &step, &def, &control);
		if (SUCCEEDED(hr))
			hr = spCameraControl->Set(CameraControl_Exposure, -7, CameraControl_Flags_Auto);
	}

	CComQIPtr<IAMVideoProcAmp> spVideo(pSource);
	if (spVideo)
	{
		long min, max, step, def, control;
		hr = spVideo->GetRange(VideoProcAmp_BacklightCompensation, &min, &max, &step, &def, &control);
		if (SUCCEEDED(hr)) {
			long value, flags;
			hr = spVideo->Get(VideoProcAmp_BacklightCompensation, &value, &flags);
			hr = spVideo->Set(VideoProcAmp_BacklightCompensation, 1, CameraControl_Flags_Manual);
			hr = spVideo->Get(VideoProcAmp_BacklightCompensation, &value, &flags);
		}
	}
	return hr;
}

