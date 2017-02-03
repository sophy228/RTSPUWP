#pragma once
#include "stdafx.h"
#include <mfidl.h>
HRESULT LogMediaType(IMFMediaType *pType);
void DBGMSG(PCWSTR format, ...);
int debug_log(const char* format, ...);