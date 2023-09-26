#include <streams.h>
#include <strsafe.h>

#include "PS3EyeSourceFilter.h"

PS3EyeSource::PS3EyeSource(IUnknown *pUnk, HRESULT *phr, const GUID id, int index)
	: CSource(NAME("PS3EyeSource"), pUnk, id),
	_pin(NULL)
{
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
	_pin = new PS3EyePushPin(phr, this, dev);
	if (phr) {
		if (_pin == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}
}

PS3EyeSource::~PS3EyeSource() {
	if(_pin != NULL) delete _pin;
}

CUnknown * WINAPI PS3EyeSource::CreateInstance(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeSource *pNewFilter = new PS3EyeSource(pUnk, phr, CLSID_PS3EyeSource, 0);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

CUnknown * WINAPI PS3EyeSource::CreateInstance2(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeSource *pNewFilter = new PS3EyeSource(pUnk, phr, CLSID_PS3EyeSource2, 1);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

CUnknown * WINAPI PS3EyeSource::CreateInstance3(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeSource *pNewFilter = new PS3EyeSource(pUnk, phr, CLSID_PS3EyeSource3, 2);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

CUnknown * WINAPI PS3EyeSource::CreateInstance4(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeSource *pNewFilter = new PS3EyeSource(pUnk, phr, CLSID_PS3EyeSource4, 3);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

CUnknown * WINAPI PS3EyeSource::CreateInstance5(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeSource *pNewFilter = new PS3EyeSource(pUnk, phr, CLSID_PS3EyeSource5, 4);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

CUnknown * WINAPI PS3EyeSource::CreateInstance6(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeSource *pNewFilter = new PS3EyeSource(pUnk, phr, CLSID_PS3EyeSource6, 5);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

CUnknown * WINAPI PS3EyeSource::CreateInstance7(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeSource *pNewFilter = new PS3EyeSource(pUnk, phr, CLSID_PS3EyeSource7, 6);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

CUnknown * WINAPI PS3EyeSource::CreateInstance8(IUnknown * pUnk, HRESULT * phr)
{
	PS3EyeSource *pNewFilter = new PS3EyeSource(pUnk, phr, CLSID_PS3EyeSource8, 7);

	if (phr)
	{
		if (pNewFilter == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}

	return pNewFilter;
}

HRESULT __stdcall PS3EyeSource::GetPages(CAUUID *pPages)
{
	CheckPointer(pPages, E_POINTER);

	pPages->cElems = 2;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID) * 2);
	CheckPointer(pPages->pElems, E_OUTOFMEMORY);
	pPages->pElems[0] = CLSID_VideoProcAmpPropertyPage;
	pPages->pElems[1] = CLSID_CameraControlPropertyPage;

	return S_OK;
}

HRESULT __stdcall PS3EyeSource::VideoProcAmpGetRange(long Property, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags)
{
	CheckPointer(pMin, E_POINTER);
	CheckPointer(pMax, E_POINTER);
	CheckPointer(pSteppingDelta, E_POINTER);
	CheckPointer(pDefault, E_POINTER);
	CheckPointer(pCapsFlags, E_POINTER);

	HRESULT hr = S_OK;
	switch (Property)
	{
	case VideoProcAmpProperty::VideoProcAmp_Brightness:
		/*
		*pMin = 0;
		*pMax = 255;
		*pSteppingDelta = 1;
		*pDefault = 20;
		*pCapsFlags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		*/
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Contrast:
		/*
		*pMin = 0;
		*pMax = 255;
		*pSteppingDelta = 1;
		*pDefault = 32;
		*pCapsFlags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		*/
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Hue:
		/*
		*pMin = 0;
		*pMax = 180;
		*pSteppingDelta = 1;
		*pDefault = 90;
		*pCapsFlags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		*/
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Saturation:
		/*
		*pMin = 0;
		*pMax = 255;
		*pSteppingDelta = 1;
		*pDefault = 0;
		*pCapsFlags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		*/
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Sharpness:
		*pMin = 0;
		*pMax = 63;
		*pSteppingDelta = 1;
		*pDefault = 0;
		*pCapsFlags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		break;
	case VideoProcAmpProperty::VideoProcAmp_WhiteBalance:
		*pMin = 0;
		*pMax = 255;
		*pSteppingDelta = 1;
		*pDefault = 128;
		*pCapsFlags = VideoProcAmpFlags::VideoProcAmp_Flags_Auto;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Gain:
		*pMin = 0;
		*pMax = 63;
		*pSteppingDelta = 1;
		*pDefault = 20;
		*pCapsFlags = VideoProcAmpFlags::VideoProcAmp_Flags_Auto;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Gamma:
	case VideoProcAmpProperty::VideoProcAmp_BacklightCompensation:
	case VideoProcAmpProperty::VideoProcAmp_ColorEnable:
	default:
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	}
	return hr;
}

HRESULT __stdcall PS3EyeSource::VideoProcAmpSet(long Property, long lValue, long Flags)
{
	HRESULT hr = S_OK;

	if (!_pin->GetDevice()->is_initialized())
	{
		hr = S_FALSE;
		return hr;
	}

	switch (Property)
	{
	case VideoProcAmpProperty::VideoProcAmp_Brightness:
		//_pin->GetDevice()->set_brightness((int)lValue);
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Contrast:
		//_pin->GetDevice()->set_contrast((int)lValue);
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Hue:
		//_pin->GetDevice()->set_hue((int)lValue);
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Saturation:
		//_pin->GetDevice()->set_saturation((int)lValue);
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Sharpness:
		_pin->GetDevice()->set_sharpness((int)lValue);
		break;
	case VideoProcAmpProperty::VideoProcAmp_WhiteBalance:
		if (Flags == VideoProcAmpFlags::VideoProcAmp_Flags_Auto)
		{
			_pin->GetDevice()->set_awb(TRUE);
		}
		else
		{
			_pin->GetDevice()->set_awb(FALSE);
			_pin->GetDevice()->set_red_balance((int)lValue);
			_pin->GetDevice()->set_green_balance((int)lValue);
			_pin->GetDevice()->set_blue_balance((int)lValue);
		}
		break;
	case VideoProcAmpProperty::VideoProcAmp_Gain:
		if (Flags == VideoProcAmpFlags::VideoProcAmp_Flags_Auto)
		{
			_pin->GetDevice()->set_auto_gain(TRUE);
		}
		else
		{
			_pin->GetDevice()->set_auto_gain(FALSE);
			_pin->GetDevice()->set_gain((int)lValue);
		}
		break;
	case VideoProcAmpProperty::VideoProcAmp_Gamma:
	case VideoProcAmpProperty::VideoProcAmp_BacklightCompensation:
	case VideoProcAmpProperty::VideoProcAmp_ColorEnable:
	default:
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	}
	return hr;
}

HRESULT __stdcall PS3EyeSource::VideoProcAmpGet(long Property, long *lValue, long *Flags)
{
	CheckPointer(lValue, E_POINTER);
	CheckPointer(Flags, E_POINTER);

	HRESULT hr = S_OK;
	switch (Property)
	{
	case VideoProcAmpProperty::VideoProcAmp_Brightness:
		/*
		*lValue = _pin->GetDevice()->brightness();
		*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		*/
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Contrast:
		/*
		*lValue = _pin->GetDevice()->contrast();
		*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		*/
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Hue:
		/*
		*lValue = _pin->GetDevice()->hue();
		*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		*/
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Saturation:
		/*
		*lValue = _pin->GetDevice()->saturation();
		*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		*/
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	case VideoProcAmpProperty::VideoProcAmp_Sharpness:
		*lValue = _pin->GetDevice()->sharpness();
		*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		break;
	case VideoProcAmpProperty::VideoProcAmp_WhiteBalance:
		*lValue = _pin->GetDevice()->red_balance();
		if (_pin->GetDevice()->awb())
		{
			*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Auto;
		}
		else
		{
			*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		}
		break;
	case VideoProcAmpProperty::VideoProcAmp_Gain:
		*lValue = _pin->GetDevice()->gain();
		if (_pin->GetDevice()->auto_gain())
		{
			*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Auto;
		}
		else
		{
			*Flags = VideoProcAmpFlags::VideoProcAmp_Flags_Manual;
		}
		break;
	case VideoProcAmpProperty::VideoProcAmp_Gamma:
	case VideoProcAmpProperty::VideoProcAmp_BacklightCompensation:
	case VideoProcAmpProperty::VideoProcAmp_ColorEnable:
	default:
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	}
	return hr;
}

HRESULT __stdcall PS3EyeSource::CameraControlGetRange(long Property, long *pMin, long *pMax, long *pSteppingDelta, long *pDefault, long *pCapsFlags)
{
	CheckPointer(pMin, E_POINTER);
	CheckPointer(pMax, E_POINTER);
	CheckPointer(pSteppingDelta, E_POINTER);
	CheckPointer(pDefault, E_POINTER);
	CheckPointer(pCapsFlags, E_POINTER);

	HRESULT hr = S_OK;
	switch (Property)
	{
	case CameraControlProperty::CameraControl_Exposure:
		*pMin = 0;
		*pMax = 255;
		*pSteppingDelta = 1;
		*pDefault = 255;
		*pCapsFlags = CameraControlFlags::CameraControl_Flags_Auto;
		break;
	default:
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	}
	return hr;
}

HRESULT __stdcall PS3EyeSource::CameraControlSet(long Property, long lValue, long Flags)
{
	HRESULT hr = S_OK;

	if (!_pin->GetDevice()->is_initialized())
	{
		hr = S_FALSE;
		return hr;
	}

	switch (Property)
	{
	case CameraControlProperty::CameraControl_Exposure:
		if (Flags == CameraControlFlags::CameraControl_Flags_Auto)
		{
			_pin->GetDevice()->set_aec(TRUE);
		}
		else
		{
			_pin->GetDevice()->set_exposure((int)lValue);
			_pin->GetDevice()->set_aec(FALSE);
		}
		break;
	default:
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	}
	return hr;
}

HRESULT __stdcall PS3EyeSource::CameraControlGet(long Property, long *lValue, long *Flags)
{
	CheckPointer(lValue, E_POINTER);
	CheckPointer(Flags, E_POINTER);

	HRESULT hr = S_OK;

	switch (Property)
	{
	case CameraControlProperty::CameraControl_Exposure:
		*lValue = _pin->GetDevice()->exposure();
		if (_pin->GetDevice()->aec())
		{
			*Flags = CameraControlFlags::CameraControl_Flags_Auto;
		}
		else
		{
			*Flags = CameraControlFlags::CameraControl_Flags_Manual;
		}
		break;
	default:
		hr = E_PROP_ID_UNSUPPORTED;
		break;
	}
	return hr;
}