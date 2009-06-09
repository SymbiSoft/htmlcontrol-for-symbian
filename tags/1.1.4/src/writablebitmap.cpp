#include <bitstd.h>
#include <bitdev.h>

#include "writablebitmap.h"

CWritableBitmap* CWritableBitmap::NewL(const TSize& aSizeInPixels, TDisplayMode aDispMode)
{
	CWritableBitmap* self = new (ELeave)CWritableBitmap();
	CleanupStack::PushL(self);
	self->ConstructL(aSizeInPixels, aDispMode);
	CleanupStack::Pop();
	
	return self;
}

CWritableBitmap::~CWritableBitmap()
{
	delete iDevice;
	delete iGc;
}

CWritableBitmap::CWritableBitmap()
{
	
}

void CWritableBitmap::ConstructL(const TSize& aSizeInPixels, TDisplayMode aDispMode)
{
	User::LeaveIfError(Create(aSizeInPixels, aDispMode));
	iDevice = CFbsBitmapDevice::NewL(this);
	iDevice->CreateContext(iGc);
}

void CWritableBitmap::ResizeWithGcL(const TSize& aSizeInPixels)
{
	User::LeaveIfError(Resize(aSizeInPixels));

	delete iDevice;
	iDevice = NULL;
	
	delete iGc;
	iGc = NULL;
	
	iDevice = CFbsBitmapDevice::NewL(this);
	iDevice->CreateContext(iGc);
}


