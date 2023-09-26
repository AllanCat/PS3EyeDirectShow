#include <streams.h>
#include <strsafe.h>

#include "PS3EyeSourceFilter.h"

PS3EyeMicSource::PS3EyeMicSource(IUnknown *pUnk, HRESULT *phr, const GUID id, int index)
	: CSource(NAME("PS3EyeMicSource"), pUnk, id),
	_pin(NULL)
{
	OutputDebugString(L"PS3Mic: PS3EyeMicSource\n");
	const std::vector<std::shared_ptr<ps3eye::camera>> &devices = ps3eye::list_devices();
	std::shared_ptr<ps3eye::camera> dev;
	// Enumerate device list and pick camera at specifc index
	for (unsigned i = 0; i < devices.size(); i++) {
		if (i == index)
		{
			dev = devices[i];
			break;
		}
	}
	_pin = new PS3EyeMicPushPin(phr, this, dev);
	if (phr) {
		if (_pin == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}
}

PS3EyeMicSource::~PS3EyeMicSource() {
	if(_pin != NULL) delete _pin;
}

CUnknown * WINAPI PS3EyeMicSource::CreateInstance(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeMicSource *pNewFilter = new PS3EyeMicSource(pUnk, phr, CLSID_PS3EyeSourceA, 0);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}