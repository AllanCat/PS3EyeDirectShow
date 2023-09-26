#include <streams.h>
#include <strsafe.h>

#include "PS3EyeSourceFilter.h"

PS3EyeMicPushPin::PS3EyeMicPushPin(HRESULT *phr, CSource *pFilter, std::shared_ptr<ps3eye::camera> device) :
	CSourceStream(NAME("PS3 Eye Mic Source"), phr, pFilter, L"Out"),
	_device(device)
{
	OutputDebugString(L"PS3Mic: PS3EyeMicPushPin\n");
	LPVOID refClock;
	HRESULT r = CoCreateInstance(CLSID_SystemClock, NULL, CLSCTX_INPROC_SERVER, IID_IReferenceClock, &refClock);
	if ((SUCCEEDED(r))) {
		_refClock = (IReferenceClock *)refClock;
	}
	else {
		OutputDebugString(L"PS3EyeMicPushPin: failed to create reference clock\n");
		_refClock = NULL;
	}
	OutputDebugString(L"PS3Mic: 1\n");
	alloc_prop.cBuffers = alloc_prop.cbBuffer = -1;
	alloc_prop.cbAlign = alloc_prop.cbPrefix = -1;
	OutputDebugString(L"PS3Mic: 2\n");
	GetMediaType(&m_mt);
	OutputDebugString(L"PS3Mic: 3\n");
}

PS3EyeMicPushPin::~PS3EyeMicPushPin() {
	if(_refClock != NULL) _refClock->Release();
}

HRESULT PS3EyeMicPushPin::ChangeMediaType(int nMediatype)
{
	OutputDebugString(L"PS3Mic: ChangeMediaType\n");
	GetMediaType(&m_mt);
	return S_OK;
}

HRESULT PS3EyeMicPushPin::CheckMediaType(const CMediaType *pMediaType)
{
	OutputDebugString(L"PS3Mic: CheckMediaType\n");
	CheckPointer(pMediaType, E_POINTER);

	if (pMediaType->IsValid() && *pMediaType->Type() == MEDIATYPE_Audio &&
		pMediaType->Subtype() != NULL && *pMediaType->Subtype() == MEDIASUBTYPE_PCM) {
		if (*pMediaType->FormatType() == FORMAT_WaveFormatEx &&
			pMediaType->Format() != NULL && pMediaType->FormatLength() > 0) {
			WAVEFORMATEX *pwf = (WAVEFORMATEX*)pMediaType->Format();
			if (pwf->nChannels == 2 
				&& pwf->nSamplesPerSec == ps3eye::PS3EYEMic::kSampleRate
				&& pwf->nAvgBytesPerSec == ps3eye::PS3EYEMic::kSampleRate * 4
				&& pwf->nBlockAlign == 4
				&& pwf->wBitsPerSample == 16
				&& pwf->cbSize == 0
				&& pwf->wFormatTag == WAVE_FORMAT_PCM
				) {
				return S_OK;
			}
		}
	}

	return E_FAIL;
}

HRESULT PS3EyeMicPushPin::Stop()
{
	return S_OK;
}

HRESULT PS3EyeMicPushPin::Pause()
{
	return S_OK;
}

HRESULT PS3EyeMicPushPin::SetMediaType(const CMediaType *pMediaType)
{
	OutputDebugString(L"PS3Mic: SetMediaType\n");
	WAVEFORMATEX *pwf = (WAVEFORMATEX*)pMediaType->Format();

	HRESULT hr = CSourceStream::SetMediaType(pMediaType);
	return hr;
}

HRESULT PS3EyeMicPushPin::GetMediaType(CMediaType *pMediaType) {
	OutputDebugString(L"PS3Mic: WAVEFORMATEX\n");
	WAVEFORMATEX *pwf = (WAVEFORMATEX*)pMediaType->AllocFormatBuffer(sizeof(WAVEFORMATEX));
	if (pwf == 0)
		return(E_OUTOFMEMORY);
	ZeroMemory(pwf, sizeof(WAVEFORMATEX));

	pwf->nChannels = 2;
	pwf->nSamplesPerSec = ps3eye::PS3EYEMic::kSampleRate;
	pwf->nAvgBytesPerSec = ps3eye::PS3EYEMic::kSampleRate * 4;
	pwf->nBlockAlign = 4;
	pwf->wBitsPerSample = 16;
	pwf->cbSize = 0;
	pwf->wFormatTag = WAVE_FORMAT_PCM;

	HRESULT hr = ::CreateAudioMediaType(pwf, pMediaType, FALSE);

	char msgbuf[64];
	wchar_t wmsgbuf[64];
	size_t outSize;
	snprintf(msgbuf, 64, "HRESULT: %lu\n", hr);
	mbstowcs_s(&outSize, wmsgbuf, strlen(msgbuf) + 1, msgbuf, strlen(msgbuf));
	OutputDebugString(wmsgbuf);

	return hr;
}

HRESULT PS3EyeMicPushPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
	OutputDebugString(L"PS3Mic: DecideBufferSize\n");
	HRESULT hr;
	CAutoLock cAutoLock(m_pFilter->pStateLock());

	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pRequest, E_POINTER);

	WAVEFORMATEX* pwfex = (WAVEFORMATEX*)m_mt.Format();
	if (pwfex) {

		pRequest->cBuffers = 1;
		pRequest->cbBuffer = 4096;

		if (0 < alloc_prop.cBuffers)
			pRequest->cBuffers = alloc_prop.cBuffers;

		if (0 < alloc_prop.cbBuffer)
			pRequest->cbBuffer = alloc_prop.cbBuffer;

		if (0 < alloc_prop.cbAlign)
			pRequest->cbAlign = alloc_prop.cbAlign;

		if (0 < alloc_prop.cbPrefix)
			pRequest->cbPrefix = alloc_prop.cbPrefix;

		ALLOCATOR_PROPERTIES Actual;
		hr = pAlloc->SetProperties(pRequest, &Actual);
		if (SUCCEEDED(hr))
			if (Actual.cbBuffer < pRequest->cbBuffer)
				hr = E_FAIL;
	}
	else {
		hr = E_POINTER;
	}
	return hr;
}

HRESULT PS3EyeMicPushPin::OnThreadStartPlay()
{
	OutputDebugString(L"PS3Mic: OnThreadStartPlay\n");
	if (_refClock != NULL) _refClock->GetTime(&_startTime);
	return S_OK;
}

HRESULT PS3EyeMicPushPin::OnThreadCreate()
{
	OutputDebugString(L"PS3Mic: initing device\n");

	if (_device.use_count() > 0) {
		if (!_device->is_initialized())
		{
			_device->init(ps3eye::res_VGA, 60, ps3eye::format::BGR);
		}
		if (!_device->is_open())
		{
			_device->start();
		}

		_audioCallback = new MyAudioCallback();
		bool didInit = _mic.init(_device->device(), _audioCallback);
		if (didInit) {
			OutputDebugString(L"done\n");
			return S_OK;
		}
		else
		{
			OutputDebugString(L"failed to init device\n");
			return E_FAIL;
		}
	}
	else {
		// no device found but we'll render a blank frame so press on
		return S_OK;
	}
}

HRESULT PS3EyeMicPushPin::OnThreadDestroy()
{
	if (_device.use_count() > 0) {
		_mic.shut();
		_device->stop();
		delete(_audioCallback);
	}
	return S_OK;
}

HRESULT PS3EyeMicPushPin::FillBuffer(IMediaSample *pSample)
{
	BYTE *pData;
	long cbData;

	CheckPointer(pSample, E_POINTER);

	pSample->GetPointer(&pData);
	cbData = pSample->GetSize();

	long numSamples = cbData / 4;

	// Check that we're still using audio
	ASSERT(m_mt.formattype == FORMAT_WaveFormatEx);

	if (_device.use_count() > 0) {
		/*
		char msgbuf[64];
		wchar_t wmsgbuf[64];
		size_t outSize;
		snprintf(msgbuf, 64, "cbData: %lu\n", cbData);
		mbstowcs_s(&outSize, wmsgbuf, strlen(msgbuf) + 1, msgbuf, strlen(msgbuf));
		OutputDebugString(wmsgbuf);
		*/
		uint16_t* pAudioData = new uint16_t[numSamples];
		_audioCallback->dequeue(pAudioData, numSamples);

		for (long i = 0; i < numSamples; i++)
		{
			pData[i * 4] = static_cast<BYTE>(pAudioData[i] & 0xFF);            // Channel 1, Low byte
			pData[i * 4 + 1] = static_cast<BYTE>((pAudioData[i] >> 8) & 0xFF); // Channel 1, High byte
			pData[i * 4 + 2] = static_cast<BYTE>(pAudioData[i] & 0xFF);            // Channel 2, Low byte
			pData[i * 4 + 3] = static_cast<BYTE>((pAudioData[i] >> 8) & 0xFF); // Channel 2, High byte
		}

		delete[] pAudioData;
	}
	else {
		// TODO: fill with error message image
		for (int i = 0; i < cbData; ++i) pData[i] = 0;
	}

	if (_refClock != NULL) {
		REFERENCE_TIME duration = (REFERENCE_TIME)UNITS * cbData / (REFERENCE_TIME)(ps3eye::PS3EYEMic::kSampleRate * 4);
		REFERENCE_TIME t;
		_refClock->GetTime(&t);
		REFERENCE_TIME rtStart = t - _startTime; // compensate for frame buffer in PS3EYECam
		REFERENCE_TIME rtStop = rtStart + duration;

		pSample->SetTime(&rtStart, &rtStop);
		// Set TRUE on every sample for uncompressed frames
		pSample->SetSyncPoint(TRUE);
	}

	return S_OK;
}

HRESULT __stdcall PS3EyeMicPushPin::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
	OutputDebugString(L"PS3Mic: Set\n");
	if (guidPropSet == AMPROPSETID_Pin) {
		return E_PROP_ID_UNSUPPORTED;
	}
	else {
		return E_PROP_SET_UNSUPPORTED;
	}
}

HRESULT __stdcall PS3EyeMicPushPin::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD * pcbReturned)
{
	OutputDebugString(L"PS3Mic: Get\n");
	if (guidPropSet != AMPROPSETID_Pin)
		return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY)
		return E_PROP_ID_UNSUPPORTED;
	if (pPropData == NULL && pcbReturned == NULL)
		return E_POINTER;

	if (pcbReturned)
		*pcbReturned = sizeof(GUID);
	if (pPropData == NULL)
		return S_OK;
	if (cbPropData < sizeof(GUID))
		return E_UNEXPECTED;

	*(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
	return S_OK;
}

HRESULT __stdcall PS3EyeMicPushPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD * pTypeSupport)
{
	OutputDebugString(L"PS3Mic: QuerySupported\n");
	if (guidPropSet == AMPROPSETID_Pin) {
		if (dwPropID == AMPROPERTY_PIN_CATEGORY) {
			*pTypeSupport = KSPROPERTY_SUPPORT_GET;
			return S_OK;
		}
		else {
			return E_PROP_ID_UNSUPPORTED;
		}
	}
	else {
		return E_PROP_SET_UNSUPPORTED;
	}
}

HRESULT __stdcall PS3EyeMicPushPin::SetFormat(AM_MEDIA_TYPE * pmt)
{
	OutputDebugString(L"SETTING FORMAT\n");
	CheckPointer(pmt, E_POINTER)
	HRESULT hr = S_OK;
	CMediaType mt(*pmt, &hr);
	if (FAILED(hr)) return hr;

	if (SUCCEEDED(CheckMediaType(&mt))) {
		IPin *cpin;
		
		HRESULT hr2 = ConnectedTo(&cpin);
		if (SUCCEEDED(hr2)) {
			if (SUCCEEDED(cpin->QueryAccept(pmt))) {
				{
					CAutoLock cAutoLock(m_pFilter->pStateLock());
					CMediaType lastMT = _currentMediaType;
					_currentMediaType = mt;
				}
				
				hr = m_pFilter->GetFilterGraph()->Reconnect(cpin);
			}
			else {
				hr = E_FAIL;
			}
		}
		else {
			CAutoLock cAutoLock(m_pFilter->pStateLock());
			_currentMediaType = mt;
			hr = S_OK;
		}
		if (cpin != NULL) {
			cpin->Release();
		}
	}
	else {
		hr = E_FAIL;
	}
	return hr;
}

HRESULT __stdcall PS3EyeMicPushPin::GetFormat(AM_MEDIA_TYPE ** ppmt)
{
	OutputDebugString(L"PS3Mic: GetFormat\n");
	CheckPointer(ppmt, E_POINTER);

	*ppmt = CreateMediaType(&m_mt);
	return S_OK;
}

HRESULT __stdcall PS3EyeMicPushPin::GetNumberOfCapabilities(int * piCount, int * piSize)
{
	OutputDebugString(L"PS3Mic: GetNumberOfCapabilities\n");
	CheckPointer(piCount, E_POINTER);
	CheckPointer(piSize, E_POINTER);
	*piCount = 1;
	*piSize = sizeof(AUDIO_STREAM_CONFIG_CAPS);
	return S_OK;
}

HRESULT __stdcall PS3EyeMicPushPin::GetStreamCaps(int iIndex, AM_MEDIA_TYPE ** ppmt, BYTE * pSCC)
{
	OutputDebugString(L"PS3Mic: GetStreamCaps\n");
	CheckPointer(ppmt, E_POINTER);
	CheckPointer(pSCC, E_POINTER);

	*ppmt = CreateMediaType(&m_mt);
	WAVEFORMATEX *paf = (WAVEFORMATEX*)(*ppmt)->pbFormat;

	paf->nChannels = 2;
	paf->nSamplesPerSec = ps3eye::PS3EYEMic::kSampleRate;
	paf->nAvgBytesPerSec = ps3eye::PS3EYEMic::kSampleRate * 4;
	paf->nBlockAlign = 4;
	paf->wBitsPerSample = 16;
	paf->cbSize = 0;
	paf->wFormatTag = WAVE_FORMAT_PCM;

	HRESULT hr = ::CreateAudioMediaType(paf, *ppmt, FALSE);
	AUDIO_STREAM_CONFIG_CAPS *pascc = (AUDIO_STREAM_CONFIG_CAPS*)pSCC;

	pascc->guid = MEDIATYPE_Audio;
	pascc->BitsPerSampleGranularity = 16;
	pascc->ChannelsGranularity = 2;
	pascc->MaximumBitsPerSample = 16;
	pascc->MaximumChannels = 2;
	pascc->MaximumSampleFrequency = ps3eye::PS3EYEMic::kSampleRate;
	pascc->MinimumBitsPerSample = 16;
	pascc->MinimumChannels = 2;
	pascc->MinimumSampleFrequency = ps3eye::PS3EYEMic::kSampleRate;
	pascc->SampleFrequencyGranularity = ps3eye::PS3EYEMic::kSampleRate;

	return hr;
}

HRESULT __stdcall PS3EyeMicPushPin::SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES* pprop)
{
	OutputDebugString(L"PS3Mic: SuggestAllocatorProperties\n");
	HRESULT hr = S_OK;
	if (pprop) {
		if (IsConnected())
			hr = VFW_E_ALREADY_CONNECTED;
		else
			alloc_prop = *pprop;
	}
	else
		hr = E_POINTER;

	return hr;
}

HRESULT __stdcall PS3EyeMicPushPin::GetAllocatorProperties(ALLOCATOR_PROPERTIES* pprop)
{
	OutputDebugString(L"PS3Mic: GetAllocatorProperties\n");
	HRESULT hr = S_OK;
	if (pprop) {
		if (IsConnected())
			*pprop = alloc_prop;
		else
			hr = VFW_E_NOT_CONNECTED;
	}
	else
		hr = E_POINTER;

	return hr;
}