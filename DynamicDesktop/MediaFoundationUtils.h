#pragma once
#include <windows.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <mfidl.h>
#include <functional>
#include <evr.h>
#include "Utils.h"

extern struct ID2D1Factory* D2D1Factory;
extern struct ID2D1HwndRenderTarget* D2D1HwndRenderTarget;
extern struct IWICImagingFactory* WICImagingFactory;
extern HWND DesktopHwnd;
extern int ScreenWidth;
extern int ScreenHeight;
extern IMFMediaSession* MediaSession;
extern IMFSourceResolver* MediaResolver;
extern IMFByteStream* CurMFByteStream;
extern IMFMediaSource* CurMediaSource;
extern IMFTopology* pTopology;

class MediaEventCallback :public IMFAsyncCallback
{
	IMFMediaSession* MFSession = nullptr;
	int MediaWidth, MediaHeight;
	bool m_ShouldLoop, m_FullVideo;
public:
	std::function<void()> OnMediaEnd;

	void SetInfo(IMFMediaSession* _MFSession, int _MediaWidth, int _MediaHeight, bool _ShouldLoop, bool _FullVideo)
	{
		MFSession = _MFSession;
		MediaWidth = _MediaWidth;
		MediaHeight = _MediaHeight;
		m_ShouldLoop = _ShouldLoop;
		m_FullVideo = _FullVideo;
	}

	virtual HRESULT STDMETHODCALLTYPE GetParameters(__RPC__out DWORD* pdwFlags, __RPC__out DWORD* pdwQueue)
	{
		return S_OK;
	}


	virtual HRESULT STDMETHODCALLTYPE Invoke(__RPC__in_opt IMFAsyncResult* pAsyncResult)
	{
		IMFMediaEvent* Mediaevent;
		MFSession->EndGetEvent(pAsyncResult, &Mediaevent);
		MediaEventType met;
		Mediaevent->GetType(&met);
		switch (met)
		{
		case MEEndOfPresentation:
			if (m_ShouldLoop)
			{
				PROPVARIANT varStart;
				PropVariantInit(&varStart);
				varStart.vt = VT_I8;
				varStart.hVal.LowPart = 0;
				HRESULT hr = MFSession->Start(&GUID_NULL, &varStart);
				PropVariantClear(&varStart);
			}
			else
				OnMediaEnd();
			break;
		case MESessionClosed:
			exit(0);
			break;
		case MESessionTopologyStatus:
		{
			UINT32 status;
			HRESULT hr = Mediaevent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &status);
			if (SUCCEEDED(hr) && (status == MF_TOPOSTATUS_READY))
			{

				// Get the IMFVideoDisplayControl interface from EVR. This call is
				// expected to fail if the media file does not have a video stream.
				IMFVideoDisplayControl* m_pVideoDisplay;
				(void)MFGetService(MFSession, MR_VIDEO_RENDER_SERVICE,
					IID_PPV_ARGS(&m_pVideoDisplay));
				float DPIScale = GetDpiForWindow(DesktopHwnd) / 96.f;
				if (m_FullVideo)
				{
					RECT rcDest = { 0, 0, long(ScreenWidth * DPIScale) ,long(ScreenHeight * DPIScale) };
					m_pVideoDisplay->SetVideoPosition(nullptr, &rcDest);
				}
				else
				{
					if ((float)MediaWidth / (float)MediaHeight > (float)ScreenWidth / (float)ScreenHeight)
					{
						int TargetWidth = (int)(ScreenHeight * (float)MediaWidth / (float)MediaHeight * DPIScale);
						int offet = (int)(TargetWidth - ScreenWidth * DPIScale);
						RECT rcDest = { -offet / 2, 0, -offet / 2 + TargetWidth , int(ScreenHeight * DPIScale) };
						m_pVideoDisplay->SetVideoPosition(nullptr, &rcDest);
					}
					else
					{
						int TargetHeight = int(ScreenWidth * (float)MediaHeight / (float)MediaWidth * DPIScale);
						int offet = (int)(TargetHeight - ScreenHeight * DPIScale);
						RECT rcDest = { 0,-offet / 2, (int)(ScreenWidth * DPIScale), -offet / 2 + TargetHeight };
						m_pVideoDisplay->SetVideoPosition(nullptr, &rcDest);
					}
				}
				break;
			}
		}
		default:
			break;
		}
		return MFSession->BeginGetEvent(this, NULL);
	}


	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		return S_OK;
	}


	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return S_OK;
	}


	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		return S_OK;
	}

};
inline void PlayCurrentVideoInternal(HWND TargetHwnd, bool HaveSound, MediaEventCallback* Callback, bool _ShouldLoop, bool _FullVideo)
{
	IMFPresentationDescriptor* PresentationDescriptor;
	CurMediaSource->CreatePresentationDescriptor(&PresentationDescriptor);
	MFCreateTopology(&pTopology);
	DWORD cSourceStreams = 0;
	PresentationDescriptor->GetStreamDescriptorCount(&cSourceStreams);
	UINT32 MediaHeight = 0, MediaWidth = 0;
	for (DWORD i = 0; i < cSourceStreams; i++)
	{
		BOOL Selected = FALSE;
		IMFStreamDescriptor* pSD = NULL;
		PresentationDescriptor->GetStreamDescriptorByIndex(i, &Selected, &pSD);
		if (Selected)
		{
			IMFMediaTypeHandler* MediaTypeHandler;
			pSD->GetMediaTypeHandler(&MediaTypeHandler);
			GUID MajorType;
			MediaTypeHandler->GetMajorType(&MajorType);
			IMFActivate* pActivate = NULL;
			if (MajorType == MFMediaType_Audio)
			{
				if (HaveSound)
					MFCreateAudioRendererActivate(&pActivate);
				else
					continue;
			}
			else if (MajorType == MFMediaType_Video)
			{
				MFCreateVideoRendererActivate(DesktopHwnd, &pActivate);
				IMFMediaType* MediaType;
				MediaTypeHandler->GetCurrentMediaType(&MediaType);

				MFGetAttributeSize(MediaType, MF_MT_FRAME_SIZE, &MediaWidth, &MediaHeight);
			}
			IMFTopologyNode* SourceTopologyNode, * OutputTopologyNode;
			MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &SourceTopologyNode);
			SourceTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, CurMediaSource);
			SourceTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, PresentationDescriptor);
			SourceTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
			pTopology->AddNode(SourceTopologyNode);
			MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &OutputTopologyNode);
			OutputTopologyNode->SetObject(pActivate);
			OutputTopologyNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, TRUE);
			OutputTopologyNode->SetUINT32(MF_TOPONODE_STREAMID, 0);
			SourceTopologyNode->ConnectOutput(0, OutputTopologyNode, 0);
			pTopology->AddNode(OutputTopologyNode);
		}
	}
	MediaSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, pTopology);
	PROPVARIANT varStart;
	PropVariantInit(&varStart);
	varStart.vt = VT_EMPTY;
	HRESULT hr = MediaSession->Start(&GUID_NULL, &varStart);
	PropVariantClear(&varStart);
	Callback->SetInfo(MediaSession, MediaWidth, MediaHeight,_ShouldLoop,_FullVideo);
	MediaSession->BeginGetEvent(Callback, NULL);
}

inline bool InitMediaFoundation()
{
	if (FAILED(MFStartup(MF_VERSION)))
		return false;
	if (FAILED(MFCreateMediaSession(nullptr, &MediaSession)))
		return false;
	if (FAILED(MFCreateSourceResolver(&MediaResolver)))
		return false;
	return true;
}

inline void PlayVideoByURL(
	LPCWSTR VideoPath,
	HWND TargetHwnd, 
	bool HaveSound, 
	MediaEventCallback* Callback, 
	bool _ShouldLoop, 
	bool _FullVideo)
{
	if (CurMediaSource)
	{
		CurMediaSource->Stop();
		CurMediaSource->Release();
		pTopology->Release();
	}
	if (CurMFByteStream)
	{
		CurMFByteStream->Close();
		CurMFByteStream->Release();
	}
	MF_OBJECT_TYPE OBJType;
	IUnknown* MediaSource = NULL;
	MediaResolver->CreateObjectFromURL(VideoPath,
		MF_RESOLUTION_MEDIASOURCE,
		NULL,
		&OBJType,
		&MediaSource);
	MediaSource->QueryInterface(IID_PPV_ARGS(&CurMediaSource));
	PlayCurrentVideoInternal(TargetHwnd, HaveSound, Callback, _ShouldLoop, _FullVideo);
}

inline void PlayVideoByResource(
	LPCWSTR Name,
	int Type,
	HWND TargetHwnd,
	bool HaveSound,
	MediaEventCallback* Callback,
	bool _ShouldLoop,
	bool _FullVideo)
{
	if (CurMediaSource)
	{
		CurMediaSource->Stop();
		CurMediaSource->Release();
		pTopology->Release();
	}
	if (CurMFByteStream)
	{
		CurMFByteStream->Close();
		CurMFByteStream->Release();
	}
	MF_OBJECT_TYPE OBJType;
	IUnknown* MediaSource = NULL;
	int Length = 0;
	auto VideoResourceBuffer = CreateBufferFromResource(Name, Type, &Length);
	HRESULT hr=MFCreateTempFile(
		MF_ACCESSMODE_READWRITE,
		MF_OPENMODE_DELETE_IF_EXIST,
		MF_FILEFLAGS_NONE,
		&CurMFByteStream
	);
	if (FAILED(hr))
		return;
	ULONG wroteBytes = 0;
	CurMFByteStream->Write((BYTE*)VideoResourceBuffer, Length, &wroteBytes);
	MediaResolver->CreateObjectFromByteStream(CurMFByteStream, nullptr,
		MF_RESOLUTION_MEDIASOURCE,
		NULL,
		&OBJType,
		&MediaSource);
	MediaSource->QueryInterface(IID_PPV_ARGS(&CurMediaSource));
	PlayCurrentVideoInternal(TargetHwnd, HaveSound, Callback, _ShouldLoop, _FullVideo);
}