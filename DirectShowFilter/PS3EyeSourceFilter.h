#pragma once

#include <queue>
#include "ps3eye.hpp"
#include "ps3mic.h"
#include "PS3EyeGuids.h"

// Filter name strings
#define g_ps3PS3EyeSourceA    L"PS3 Eye UniversalA"
#define g_ps3PS3EyeSource     L"PS3 Eye Universal"
#define g_ps3PS3EyeSource2    L"PS3 Eye Universal2"
#define g_ps3PS3EyeSource3    L"PS3 Eye Universal3"
#define g_ps3PS3EyeSource4    L"PS3 Eye Universal4"
#define g_ps3PS3EyeSource5    L"PS3 Eye Universal5"
#define g_ps3PS3EyeSource6    L"PS3 Eye Universal6"
#define g_ps3PS3EyeSource7    L"PS3 Eye Universal7"
#define g_ps3PS3EyeSource8    L"PS3 Eye Universal8"

class PS3EyePushPin;

class IAMVideoProcAmpWrapper : public IAMVideoProcAmp
{
public:
	virtual HRESULT __stdcall VideoProcAmpGetRange(long Property, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags) = 0;

	virtual HRESULT __stdcall VideoProcAmpSet(long Property, long lValue, long Flags) = 0;

	virtual HRESULT __stdcall VideoProcAmpGet(long Property, long *lValue, long *Flags) = 0;

	// Inherited via IAMVideoProcAmp
	virtual HRESULT __stdcall GetRange(long Property, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags)
	{
		return VideoProcAmpGetRange(Property, pMin, pMax, pSteppingDelta, pDefault, pCapsFlags);
	}

	virtual HRESULT __stdcall Set(long Property, long lValue, long Flags)
	{
		return VideoProcAmpSet(Property, lValue, Flags);
	}

	virtual HRESULT __stdcall Get(long Property, long *lValue, long *Flags)
	{
		return VideoProcAmpGet(Property, lValue, Flags);
	}
};

class IAMCameraControlWrapper : public IAMCameraControl
{
public:
	virtual HRESULT __stdcall CameraControlGetRange(long Property, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags) = 0;

	virtual HRESULT __stdcall CameraControlSet(long Property, long lValue, long Flags) = 0;

	virtual HRESULT __stdcall CameraControlGet(long Property, long *lValue, long *Flags) = 0;

	// Inherited via IAMVideoProcAmp
	virtual HRESULT __stdcall GetRange(long Property, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags)
	{
		return CameraControlGetRange(Property, pMin, pMax, pSteppingDelta, pDefault, pCapsFlags);
	}

	virtual HRESULT __stdcall Set(long Property, long lValue, long Flags)
	{
		return CameraControlSet(Property, lValue, Flags);
	}

	virtual HRESULT __stdcall Get(long Property, long *lValue, long *Flags)
	{
		return CameraControlGet(Property, lValue, Flags);
	}
};

class PS3EyePushPin : public CSourceStream, public IKsPropertySet, public IAMStreamConfig
{
protected:
	std::shared_ptr<ps3eye::camera> _device;
	CMediaType _currentMediaType;
	HRESULT _GetMediaType(int iPosition, CMediaType *pMediaType);
	REFERENCE_TIME _startTime;
	IReferenceClock *_refClock;

public:
	PS3EyePushPin(HRESULT *phr, CSource *pFilter, std::shared_ptr<ps3eye::camera> device);
	~PS3EyePushPin();

	std::shared_ptr<ps3eye::camera> GetDevice() { return _device; }

	DECLARE_IUNKNOWN

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_IKsPropertySet)
		{
			return GetInterface((IKsPropertySet*)this, ppv);
		}
		else if (riid == IID_IAMStreamConfig) {
			return GetInterface((IAMStreamConfig*)this, ppv);
		}
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
	}

	HRESULT CheckMediaType(const CMediaType *);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	
	HRESULT OnThreadStartPlay();
	HRESULT OnThreadCreate();
	HRESULT OnThreadDestroy();
	HRESULT FillBuffer(IMediaSample *pSample);
	void FillError(BYTE *pData);

	// Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
	STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
	{
		return E_FAIL;
	}

	// Inherited via IKsPropertySet
	virtual HRESULT __stdcall Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData) override;

	virtual HRESULT __stdcall Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD * pcbReturned) override;

	virtual HRESULT __stdcall QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD * pTypeSupport) override;

	// Inherited via IAMStreamConfig
	virtual HRESULT __stdcall SetFormat(AM_MEDIA_TYPE * pmt) override;

	virtual HRESULT __stdcall GetFormat(AM_MEDIA_TYPE ** ppmt) override;

	virtual HRESULT __stdcall GetNumberOfCapabilities(int * piCount, int * piSize) override;

	virtual HRESULT __stdcall GetStreamCaps(int iIndex, AM_MEDIA_TYPE ** ppmt, BYTE * pSCC) override;
};

class PS3EyeSource : public CSource, public ISpecifyPropertyPages, public IAMVideoProcAmpWrapper, public IAMCameraControlWrapper
{
private:
	PS3EyeSource(IUnknown *pUnk, HRESULT *phr, const GUID id, int index);
	~PS3EyeSource();

	PS3EyePushPin *_pin;

public:
	DECLARE_IUNKNOWN

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_ISpecifyPropertyPages) {
			return GetInterface((ISpecifyPropertyPages*)this, ppv);
		}
		else if (riid == IID_IAMVideoProcAmp) {
			return GetInterface((IAMVideoProcAmp*)this, ppv);
		}
		else if (riid == IID_IAMCameraControl) {
			return GetInterface((IAMCameraControl*)this, ppv);
		}
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}

	// Support up to 8 devices
	static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance2(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance3(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance4(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance5(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance6(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance7(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance8(IUnknown *pUnk, HRESULT *phr);

	// Inherited via ISpecifyPropertyPages
	virtual HRESULT __stdcall GetPages(CAUUID *pPages) override;

	// Inherited via IAMVideoProcAmpWrapper
	virtual HRESULT __stdcall VideoProcAmpGetRange(long Property, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags);

	virtual HRESULT __stdcall VideoProcAmpSet(long Property, long lValue, long Flags);

	virtual HRESULT __stdcall VideoProcAmpGet(long Property, long *lValue, long *Flags);

	// Inherited via IAMCameraControlWrapper
	virtual HRESULT __stdcall CameraControlGetRange(long Property, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags);

	virtual HRESULT __stdcall CameraControlSet(long Property, long lValue, long Flags);

	virtual HRESULT __stdcall CameraControlGet(long Property, long *lValue, long *Flags);
};

struct MyAudioCallback : ps3eye::AudioCallback
{
	MyAudioCallback()
	{
		// Clean up the queue
		size_ = 0;
		std::queue<uint16_t>().swap(buffer_);
	}

	[[nodiscard]]
	bool dequeue(uint16_t* dest, unsigned samples) {
		size_ = samples;

		std::unique_lock<std::mutex> lock(mutex_);
		using namespace std::chrono_literals;

		// If there is no data in the buffer, wait until data becomes available
		bool status = notify_frame_.wait_for(lock, 1s, [this]() { return buffer_.size() >= size_; });
		if (!status)
			return false;

		// Copy from internal buffer
		for (unsigned i = 0; i < samples; i++) {
			dest[i] = buffer_.front();
			buffer_.pop();
		}

		return true;
	}

	virtual void handleAudioData(const ps3eye::AudioFrame * frames, const int numFrames) override
	{
		std::lock_guard<std::mutex> lock(mutex_);

		for (int i = 0; i < numFrames; ++i)
		{
			// Get avg of 4 channels
			uint32_t avgChannel = (frames[i].channel[0] + frames[i].channel[1] + frames[i].channel[2] + frames[i].channel[3]) / 4;

			buffer_.push((uint16_t)avgChannel);
		}
		notify_frame_.notify_one();
	}

private:
	std::mutex mutex_;
	std::condition_variable notify_frame_;
	std::queue<uint16_t> buffer_;

	unsigned size_ = 0;
};

class PS3EyeMicPushPin : public CSourceStream, public IKsPropertySet, public IAMStreamConfig, public IAMBufferNegotiation
{
protected:
	std::shared_ptr<ps3eye::camera> _device;
	ps3eye::PS3EYEMic _mic;
	MyAudioCallback *_audioCallback;
	CMediaType _currentMediaType;
	REFERENCE_TIME _startTime;
	IReferenceClock *_refClock;

	//share_queue queue;
	uint64_t obs_start_ts = 0;
	uint64_t dshow_start_ts = 0;
	uint64_t sync_timeout = 0;
	uint64_t system_start_time = 0;
	REFERENCE_TIME  prev_end_ts = 0;
	ALLOCATOR_PROPERTIES alloc_prop;
public:
	PS3EyeMicPushPin(HRESULT *phr, CSource *pFilter, std::shared_ptr<ps3eye::camera> device);
	~PS3EyeMicPushPin();

	std::shared_ptr<ps3eye::camera> GetDevice() { return _device; }

	DECLARE_IUNKNOWN

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_IKsPropertySet)
		{
			return GetInterface((IKsPropertySet*)this, ppv);
		}
		else if (riid == IID_IAMStreamConfig) {
			return GetInterface((IAMStreamConfig*)this, ppv);
		}
		else if (riid == IID_IAMBufferNegotiation) {
			return GetInterface((IAMBufferNegotiation*)this, ppv);
		}
		return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
	}

	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(CMediaType *pMediaType);
	HRESULT SetMediaType(const CMediaType *pMediaType);
	HRESULT ChangeMediaType(int nMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);

	HRESULT OnThreadStartPlay();
	HRESULT Stop();
	HRESULT Pause();
	HRESULT OnThreadCreate();
	HRESULT OnThreadDestroy();
	HRESULT FillBuffer(IMediaSample *pSample);

	// Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
	STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
	{
		return E_FAIL;
	}

	// Inherited via IKsPropertySet
	virtual HRESULT __stdcall Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData) override;

	virtual HRESULT __stdcall Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD * pcbReturned) override;

	virtual HRESULT __stdcall QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD * pTypeSupport) override;

	// Inherited via IAMStreamConfig
	virtual HRESULT __stdcall SetFormat(AM_MEDIA_TYPE * pmt) override;

	virtual HRESULT __stdcall GetFormat(AM_MEDIA_TYPE ** ppmt) override;

	virtual HRESULT __stdcall GetNumberOfCapabilities(int * piCount, int * piSize) override;

	virtual HRESULT __stdcall GetStreamCaps(int iIndex, AM_MEDIA_TYPE ** ppmt, BYTE * pSCC) override;

	// Inherited via IAMBufferNegotiation
	virtual HRESULT __stdcall SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES* pprop) override;
	
	virtual HRESULT __stdcall GetAllocatorProperties(ALLOCATOR_PROPERTIES* pprop) override;
};

class PS3EyeMicSource : public CSource, public IAMFilterMiscFlags
{
private:
	PS3EyeMicSource(IUnknown *pUnk, HRESULT *phr, const GUID id, int index);
	~PS3EyeMicSource();

	PS3EyeMicPushPin *_pin;

public:
	DECLARE_IUNKNOWN

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_ISpecifyPropertyPages) {
			return GetInterface((IAMFilterMiscFlags*)this, ppv);
		}
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}

	// Support up to 8 devices
	static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);
	/*
	static CUnknown * WINAPI CreateInstance2(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance3(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance4(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance5(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance6(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance7(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstance8(IUnknown *pUnk, HRESULT *phr);
	static CUnknown * WINAPI CreateInstanceA(IUnknown *pUnk, HRESULT *phr);
	*/

	// Inherited via IAMFilterMiscFlags
	virtual ULONG STDMETHODCALLTYPE GetMiscFlags(void)
	{
		return AM_FILTER_MISC_FLAGS_IS_SOURCE;
	}

	IFilterGraph *GetGraph() { return m_pGraph; }
};