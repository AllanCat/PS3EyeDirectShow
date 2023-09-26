#include <streams.h>
#include <strsafe.h>

#include "PS3EyeSourceFilter.h"

PS3EyePushPin::PS3EyePushPin(HRESULT *phr, CSource *pFilter, std::shared_ptr<ps3eye::camera> device) :
	CSourceStream(NAME("PS3 Eye Source"), phr, pFilter, L"Out"),
	_device(device)
{
	LPVOID refClock;
	HRESULT r = CoCreateInstance(CLSID_SystemClock, NULL, CLSCTX_INPROC_SERVER, IID_IReferenceClock, &refClock);
	if ((SUCCEEDED(r))) {
		_refClock = (IReferenceClock *)refClock;
	}
	else {
		OutputDebugString(L"PS3EyePushPin: failed to create reference clock\n");
		_refClock = NULL;
	}
}

PS3EyePushPin::~PS3EyePushPin() {
	if(_refClock != NULL) _refClock->Release();
}

HRESULT PS3EyePushPin::CheckMediaType(const CMediaType *pMediaType)
{
	CheckPointer(pMediaType, E_POINTER);

	if (pMediaType->IsValid() && *pMediaType->Type() == MEDIATYPE_Video &&
		pMediaType->Subtype() != NULL && *pMediaType->Subtype() == MEDIASUBTYPE_RGB24) {
		if (*pMediaType->FormatType() == FORMAT_VideoInfo &&
			pMediaType->Format() != NULL && pMediaType->FormatLength() > 0) {
			VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->Format();
			if ((pvi->bmiHeader.biWidth == 640 && pvi->bmiHeader.biHeight == 480) ||
				(pvi->bmiHeader.biWidth == 320 && pvi->bmiHeader.biHeight == 240)) {
				if (pvi->bmiHeader.biBitCount == 24 && pvi->bmiHeader.biCompression == BI_RGB
					&& pvi->bmiHeader.biPlanes == 1) {
					int minTime = 10000000 / 70;
					int maxTime = 10000000 / 2;
					if (pvi->AvgTimePerFrame >= minTime && pvi->AvgTimePerFrame <= maxTime) {
						return S_OK;
					}
				}
			}
		}
	}

	return E_FAIL;
}

HRESULT PS3EyePushPin::GetMediaType(int iPosition, CMediaType *pMediaType) {
	if (_currentMediaType.IsValid()) {
		// SetFormat from IAMStreamConfig has been called, only allow that format
		CheckPointer(pMediaType, E_POINTER);
		if (iPosition != 0) return E_UNEXPECTED;
		*pMediaType = _currentMediaType;
		return S_OK;
	}
	else {
		return _GetMediaType(iPosition, pMediaType);
	}
}

HRESULT PS3EyePushPin::_GetMediaType(int iPosition, CMediaType *pMediaType) {
	if (iPosition < 0 || iPosition >= 10) return E_UNEXPECTED;
	CheckPointer(pMediaType, E_POINTER);

	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
	if (pvi == 0)
		return(E_OUTOFMEMORY);
	ZeroMemory(pvi, pMediaType->cbFormat);

	const int fpsList[10] = {60, 50, 40, 30, 15,  // 640x480
							 60, 50, 40, 37, 30}; // 320x240

	int fps = 10;
	if (iPosition < 5) {
		// 640x480
		pvi->bmiHeader.biWidth = 640;
		pvi->bmiHeader.biHeight = 480;
	}
	else  {
		// 320x240
		pvi->bmiHeader.biWidth = 320;
		pvi->bmiHeader.biHeight = 240;
	}
	fps = fpsList[iPosition];

	uint64_t rate = (uint64_t)GetBitmapSize(&pvi->bmiHeader) * fps * 2;

	pvi->AvgTimePerFrame = 10000000 / fps;
	pvi->bmiHeader.biBitCount = 24;
	pvi->bmiHeader.biCompression = BI_RGB;
	pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biPlanes = 1;
	pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader);
	pvi->bmiHeader.biClrImportant = 0;
	pvi->dwBitRate = (DWORD)rate;

	SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
	SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

	pMediaType->SetType(&MEDIATYPE_Video);
	pMediaType->SetFormatType(&FORMAT_VideoInfo);
	pMediaType->SetTemporalCompression(FALSE);

	// Work out the GUID for the subtype from the header info.
	const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
	pMediaType->SetSubtype(&SubTypeGUID);
	pMediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);
	
	return NOERROR;
}

HRESULT PS3EyePushPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
	HRESULT hr;
	CAutoLock cAutoLock(m_pFilter->pStateLock());

	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pRequest, E_POINTER);

	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)m_mt.Format();

	// Ensure a minimum number of buffers
	if (pRequest->cBuffers == 0)
	{
		pRequest->cBuffers = 2;
	}
	pRequest->cbBuffer = pvi->bmiHeader.biSizeImage;

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pRequest, &Actual);
	if (FAILED(hr))
	{
		return hr;
	}

	// Is this allocator unsuitable?
	if (Actual.cbBuffer < pRequest->cbBuffer)
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT PS3EyePushPin::OnThreadStartPlay()
{
	if (_refClock != NULL) _refClock->GetTime(&_startTime);
	return S_OK;
}

HRESULT PS3EyePushPin::OnThreadCreate()
{
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)m_mt.Format();
	int fps = 10000000 / ((int)pvi->AvgTimePerFrame);
	ps3eye::resolution res;

	if ((pvi->bmiHeader.biWidth == 0 && pvi->bmiHeader.biHeight == 0)
		|| pvi->bmiHeader.biWidth > 320 || pvi->bmiHeader.biHeight > 240) {
		res = ps3eye::res_VGA;
	}
	else {
		res = ps3eye::res_QVGA;
	}

	OutputDebugString(L"initing device\n");
	if (_device.use_count() > 0) {
		bool didInit = _device->init(res, fps, ps3eye::format::BGR);
		if (didInit) {
			OutputDebugString(L"starting device\n");
			_device->set_flip_status(false, true);
			_device->set_auto_gain(true);
			_device->set_awb(true);
			_device->start();
			OutputDebugString(L"done\n");
			return S_OK;
		}
		else {
			OutputDebugString(L"failed to init device\n");
			return E_FAIL;
		}
	}
	else {
		// no device found but we'll render a blank frame so press on
		return S_OK;
	}
}

HRESULT PS3EyePushPin::OnThreadDestroy()
{
	if (_device.use_count() > 0) {
		_device->stop();
	}
	return S_OK;
}

HRESULT PS3EyePushPin::FillBuffer(IMediaSample *pSample)
{
	BYTE *pData;
	long cbData;

	CheckPointer(pSample, E_POINTER);

	pSample->GetPointer(&pData);
	cbData = pSample->GetSize();

	// Check that we're still using video
	ASSERT(m_mt.formattype == FORMAT_VideoInfo);

	if (_device.use_count() > 0) {
		/*
		char msgbuf[64];
		wchar_t wmsgbuf[64];
		size_t outSize;
		snprintf(msgbuf, 64, "cbData: %lu\n", cbData);
		mbstowcs_s(&outSize, wmsgbuf, strlen(msgbuf) + 1, msgbuf, strlen(msgbuf));
		OutputDebugString(wmsgbuf);
		*/
		_device->get_frame(pData);
	}
	else {
		// TODO: fill with error message image
		for (int i = 0; i < cbData; ++i) pData[i] = 0;
	}

	if (_refClock != NULL) {
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)m_mt.Format();
		REFERENCE_TIME t;
		_refClock->GetTime(&t);
		REFERENCE_TIME rtStart = t - _startTime - pvi->AvgTimePerFrame; // compensate for frame buffer in PS3EYECam
		//REFERENCE_TIME rtStart = t - _startTime; // compensate for frame buffer in PS3EYECam
		REFERENCE_TIME rtStop = rtStart + pvi->AvgTimePerFrame;

		pSample->SetTime(&rtStart, &rtStop);
		// Set TRUE on every sample for uncompressed frames
		pSample->SetSyncPoint(TRUE);
	}

	return S_OK;
}

HRESULT __stdcall PS3EyePushPin::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
	if (guidPropSet == AMPROPSETID_Pin) {
		return E_PROP_ID_UNSUPPORTED;
	}
	else {
		return E_PROP_SET_UNSUPPORTED;
	}
}

HRESULT __stdcall PS3EyePushPin::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD * pcbReturned)
{
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

HRESULT __stdcall PS3EyePushPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD * pTypeSupport)
{
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

HRESULT __stdcall PS3EyePushPin::SetFormat(AM_MEDIA_TYPE * pmt)
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

HRESULT __stdcall PS3EyePushPin::GetFormat(AM_MEDIA_TYPE ** ppmt)
{
	CheckPointer(ppmt, E_POINTER);
	*ppmt = NULL;
	if (!_currentMediaType.IsValid()) {
		CMediaType dmt;
		_GetMediaType(0, &dmt);
		*ppmt = CreateMediaType(&dmt);
	}
	else {
		*ppmt = CreateMediaType(&_currentMediaType);
	}
	if (*ppmt == NULL) {
		return E_FAIL;
	}
	else {
		return S_OK;
	}
}

HRESULT __stdcall PS3EyePushPin::GetNumberOfCapabilities(int * piCount, int * piSize)
{
	CheckPointer(piCount, E_POINTER);
	CheckPointer(piSize, E_POINTER);
	*piCount = 10;
	*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
	return S_OK;
}

HRESULT __stdcall PS3EyePushPin::GetStreamCaps(int iIndex, AM_MEDIA_TYPE ** ppmt, BYTE * pSCC)
{
	CheckPointer(ppmt, E_POINTER);
	CheckPointer(pSCC, E_POINTER);
	CMediaType mt;
	HRESULT hr = _GetMediaType(iIndex, &mt);
	if (SUCCEEDED(hr)) {
		*ppmt = CreateMediaType(&mt);
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)mt.Format();

		const SIZE inputSize = { pvi->bmiHeader.biWidth, pvi->bmiHeader.biHeight };
		VIDEO_STREAM_CONFIG_CAPS *cc = (VIDEO_STREAM_CONFIG_CAPS *)pSCC;
		cc->guid = MEDIATYPE_Video;
		cc->VideoStandard = 0;
		cc->MinFrameInterval = 10000000 / 70;
		cc->MaxFrameInterval = 10000000 / 2;
		cc->MinOutputSize = inputSize;
		cc->MaxOutputSize = inputSize;
		cc->InputSize = inputSize;
		cc->MinCroppingSize = inputSize;
		cc->MaxCroppingSize = inputSize;
		cc->CropGranularityX = pvi->bmiHeader.biWidth;
		cc->CropGranularityY = pvi->bmiHeader.biHeight;
		cc->StretchTapsX = 0;
		cc->StretchTapsY = 0;
		cc->ShrinkTapsX = 0;
		cc->ShrinkTapsY = 0;
		cc->MinBitsPerSecond = pvi->dwBitRate;
		cc->MaxBitsPerSecond = cc->MinBitsPerSecond;
	}
	return hr;
}
