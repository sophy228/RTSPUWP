#pragma once
#include <stdio.h>
#include <tchar.h>
#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <wmcodecdsp.h>
#include <wmsdkidl.h>

#define CHECK_HR(hr, msg) if (hr != S_OK) { printf(msg); printf("Error: %.2X.\n", hr); goto done; }

#define CHECKHR_GOTO(x, y) if(FAILED(x)) goto y

#define INTERNAL_GUID_TO_STRING( _Attribute, _skip ) \
if (Attr == _Attribute) \
{ \
	pAttrStr = #_Attribute; \
	C_ASSERT((sizeof(#_Attribute) / sizeof(#_Attribute[0])) > _skip); \
	pAttrStr += _skip; \
	goto done; \
} \

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

template <class T> inline void SafeRelease(T*& pT)
{
	if (pT != NULL)
	{
		pT->Release();
		pT = NULL;
	}
}

LPCSTR STRING_FROM_GUID(GUID Attr)
{
	LPCSTR pAttrStr = NULL;

	// Generics
	INTERNAL_GUID_TO_STRING(MF_MT_MAJOR_TYPE, 6);                     // MAJOR_TYPE
	INTERNAL_GUID_TO_STRING(MF_MT_SUBTYPE, 6);                        // SUBTYPE
	INTERNAL_GUID_TO_STRING(MF_MT_ALL_SAMPLES_INDEPENDENT, 6);        // ALL_SAMPLES_INDEPENDENT   
	INTERNAL_GUID_TO_STRING(MF_MT_FIXED_SIZE_SAMPLES, 6);             // FIXED_SIZE_SAMPLES
	INTERNAL_GUID_TO_STRING(MF_MT_COMPRESSED, 6);                     // COMPRESSED
	INTERNAL_GUID_TO_STRING(MF_MT_SAMPLE_SIZE, 6);                    // SAMPLE_SIZE
	INTERNAL_GUID_TO_STRING(MF_MT_USER_DATA, 6);                      // MF_MT_USER_DATA

																	  // Audio
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_NUM_CHANNELS, 12);            // NUM_CHANNELS
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_SAMPLES_PER_SECOND, 12);      // SAMPLES_PER_SECOND
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 12);    // AVG_BYTES_PER_SECOND
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_BLOCK_ALIGNMENT, 12);         // BLOCK_ALIGNMENT
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_BITS_PER_SAMPLE, 12);         // BITS_PER_SAMPLE
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_VALID_BITS_PER_SAMPLE, 12);   // VALID_BITS_PER_SAMPLE
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_SAMPLES_PER_BLOCK, 12);       // SAMPLES_PER_BLOCK
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_CHANNEL_MASK, 12);            // CHANNEL_MASK
	INTERNAL_GUID_TO_STRING(MF_MT_AUDIO_PREFER_WAVEFORMATEX, 12);     // PREFER_WAVEFORMATEX

																	  // Video
	INTERNAL_GUID_TO_STRING(MF_MT_FRAME_SIZE, 6);                     // FRAME_SIZE
	INTERNAL_GUID_TO_STRING(MF_MT_FRAME_RATE, 6);                     // FRAME_RATE
	INTERNAL_GUID_TO_STRING(MF_MT_PIXEL_ASPECT_RATIO, 6);             // PIXEL_ASPECT_RATIO
	INTERNAL_GUID_TO_STRING(MF_MT_INTERLACE_MODE, 6);                 // INTERLACE_MODE
	INTERNAL_GUID_TO_STRING(MF_MT_AVG_BITRATE, 6);                    // AVG_BITRATE
	INTERNAL_GUID_TO_STRING(MF_MT_DEFAULT_STRIDE, 6);				  // STRIDE
	INTERNAL_GUID_TO_STRING(MF_MT_AVG_BIT_ERROR_RATE, 6);
	INTERNAL_GUID_TO_STRING(MF_MT_GEOMETRIC_APERTURE, 6);
	INTERNAL_GUID_TO_STRING(MF_MT_MINIMUM_DISPLAY_APERTURE, 6);
	INTERNAL_GUID_TO_STRING(MF_MT_PAN_SCAN_APERTURE, 6);
	INTERNAL_GUID_TO_STRING(MF_MT_VIDEO_NOMINAL_RANGE, 6);

	// Major type values
	INTERNAL_GUID_TO_STRING(MFMediaType_Default, 12);                 // Default
	INTERNAL_GUID_TO_STRING(MFMediaType_Audio, 12);                   // Audio
	INTERNAL_GUID_TO_STRING(MFMediaType_Video, 12);                   // Video
	INTERNAL_GUID_TO_STRING(MFMediaType_Script, 12);                  // Script
	INTERNAL_GUID_TO_STRING(MFMediaType_Image, 12);                   // Image
	INTERNAL_GUID_TO_STRING(MFMediaType_HTML, 12);                    // HTML
	INTERNAL_GUID_TO_STRING(MFMediaType_Binary, 12);                  // Binary
	INTERNAL_GUID_TO_STRING(MFMediaType_SAMI, 12);                    // SAMI
	INTERNAL_GUID_TO_STRING(MFMediaType_Protected, 12);               // Protected

																	  // Minor video type values
	INTERNAL_GUID_TO_STRING(MFVideoFormat_Base, 14);                  // Base
	INTERNAL_GUID_TO_STRING(MFVideoFormat_MP43, 14);                  // MP43
	INTERNAL_GUID_TO_STRING(MFVideoFormat_WMV1, 14);                  // WMV1
	INTERNAL_GUID_TO_STRING(MFVideoFormat_WMV2, 14);                  // WMV2
	INTERNAL_GUID_TO_STRING(MFVideoFormat_WMV3, 14);                  // WMV3
	INTERNAL_GUID_TO_STRING(MFVideoFormat_MPG1, 14);                  // MPG1
	INTERNAL_GUID_TO_STRING(MFVideoFormat_MPG2, 14);                  // MPG2
	INTERNAL_GUID_TO_STRING(MFVideoFormat_RGB24, 14);				  // RGB24
	INTERNAL_GUID_TO_STRING(MFVideoFormat_RGB32, 14);				  // RGB32
	INTERNAL_GUID_TO_STRING(MFVideoFormat_H264, 14);				  // H264


																	  // Minor audio type values
	INTERNAL_GUID_TO_STRING(MFAudioFormat_Base, 14);                  // Base
	INTERNAL_GUID_TO_STRING(MFAudioFormat_PCM, 14);                   // PCM
	INTERNAL_GUID_TO_STRING(MFAudioFormat_DTS, 14);                   // DTS
	INTERNAL_GUID_TO_STRING(MFAudioFormat_Dolby_AC3_SPDIF, 14);       // Dolby_AC3_SPDIF
	INTERNAL_GUID_TO_STRING(MFAudioFormat_Float, 14);                 // IEEEFloat
	INTERNAL_GUID_TO_STRING(MFAudioFormat_WMAudioV8, 14);             // WMAudioV8
	INTERNAL_GUID_TO_STRING(MFAudioFormat_WMAudioV9, 14);             // WMAudioV9
	INTERNAL_GUID_TO_STRING(MFAudioFormat_WMAudio_Lossless, 14);      // WMAudio_Lossless
	INTERNAL_GUID_TO_STRING(MFAudioFormat_WMASPDIF, 14);              // WMASPDIF
	INTERNAL_GUID_TO_STRING(MFAudioFormat_MP3, 14);                   // MP3
	INTERNAL_GUID_TO_STRING(MFAudioFormat_MPEG, 14);                  // MPEG

																	  // Media sub types
	INTERNAL_GUID_TO_STRING(WMMEDIASUBTYPE_I420, 15);                  // I420
	INTERNAL_GUID_TO_STRING(WMMEDIASUBTYPE_WVC1, 0);
	INTERNAL_GUID_TO_STRING(WMMEDIASUBTYPE_WMAudioV8, 0);

	// MP4 Media Subtypes.
	INTERNAL_GUID_TO_STRING(MF_MT_MPEG4_SAMPLE_DESCRIPTION, 6);
	INTERNAL_GUID_TO_STRING(MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY, 6);
	//INTERNAL_GUID_TO_STRING(MFMPEG4Format_MP4A, 0);

done:
	return pAttrStr;
}
