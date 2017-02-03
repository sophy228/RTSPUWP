#pragma once
#include <mfidl.h>
#include <strsafe.h>
#include <mfreadwrite.h>


enum StreamType
{
	AUDStream = 0,
	VIDStream,
	AUDVIDStream,
};

enum DeviceType
{
	AUDCAP = 0,
	VIDCAP
};

enum ReadStatus
{
	READOK = 0,
	READERROR,
	READEND
};

class CaptureDevice : public IMFSourceReaderCallback
{
	
public:
	static CaptureDevice * GetDeviceInstance(DWORD index = 0, DeviceType type = VIDCAP, bool asyncmode = false);
private:
	CaptureDevice(DWORD index, DeviceType type, bool asyncmode);
	~CaptureDevice();
	static CaptureDevice * deviceinstance;
	
public:
	ReadStatus ReadSample(IMFSample ** sample, StreamType type = VIDStream);
	HRESULT GetMediaType(IMFMediaType ** type, DWORD * count);
	HRESULT ChooseMediaType(DWORD index, IMFMediaType ** type);
	
	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
	{
		HRESULT hr = S_OK;
		if (ppv == nullptr)
		{
			return E_POINTER;
		}
		(*ppv) = nullptr;
		if (riid == IID_IMFSourceReaderCallback)
		{
			(*ppv) = static_cast<IMFSourceReaderCallback*>(this);
			AddRef();
		}
		else
		{
			hr = E_NOINTERFACE;
		}

		return hr;
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_nRefCount);
	}
	STDMETHODIMP_(ULONG) Release()
	{
		ULONG uCount = InterlockedDecrement(&m_nRefCount);
		if (uCount == 0)
		{
			delete this;
		}
		return uCount;
	}

	// IMFSourceReaderCallback methods
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,
		DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample);

	STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *)
	{
		return S_OK;
	}

	STDMETHODIMP OnFlush(DWORD)
	{
		return S_OK;
	}

private:
	DWORD devIndex;
	DeviceType devType;
	IMFMediaSource *pSource;
	bool initialized;
	IMFMediaType *supportedTypes[100];
	DWORD supportedTypeCount;
	IMFSourceReader *pReader;

	HRESULT initialize();

	HRESULT CreateVideoDeviceSource(IMFMediaSource ** ppSource);
	HRESULT GetSupportedCaptureFormats(IMFMediaSource * pSource);
	HRESULT SetupCamera(IMFMediaSource * pSource);

	bool				m_asyncmode;
	long                m_nRefCount;        // Reference count.
	CRITICAL_SECTION    m_critsec;
	BOOL                m_bEOS;

	IMFSample *			latestSample;
	bool                latestDirty;
	//static HRESULT LogAttributeValueByIndex(IMFAttributes * pAttr, DWORD index);
};

