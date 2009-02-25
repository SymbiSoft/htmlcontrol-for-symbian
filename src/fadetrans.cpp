#include <eikenv.h>

#include "fadetrans.h"
#include "htmlcontrol.h"
#include "writablebitmap.h"

CFadeTransition::CFadeTransition(CHtmlControl* aControl) :
	CActive(EPriorityStandard) // Standard priority
	,iControl(aControl)
{
	iInterval = 20000;
	iStep = 50;
}

CFadeTransition* CFadeTransition::NewL(CHtmlControl* aControl)
{
	CFadeTransition* self = new ( ELeave ) CFadeTransition(aControl);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(); // self;
	return self;
}

void CFadeTransition::ConstructL()
{
	User::LeaveIfError(iTimer.CreateLocal() ); // Initialize timer
	CActiveScheduler::Add( this); // Add to scheduler
}

CFadeTransition::~CFadeTransition()
{
	Cancel(); // Cancel any request, if outstanding
	iTimer.Close(); // Destroy the RTimer object
	// Delete instance variables if any
	delete iMask;
	delete iOldScreen;
}

void CFadeTransition::StartL()
{	
	if(iControl->OffScreenBitmap()==NULL)
	{
		iControl->RefreshAndDraw();
		return;
	}
	
	if(!iMask)
		iMask = CWritableBitmap::NewL(iControl->Size(), EGray256);
	else if(iMask->SizeInPixels()!=iControl->Size())
		iMask->ResizeWithGcL(iControl->Size());
	
	if(!iOldScreen)
		iOldScreen = CWritableBitmap::NewL(iControl->Size(), CEikonEnv::Static()->ScreenDevice()->DisplayMode());
	else if(iOldScreen->SizeInPixels()!=iControl->Size())
		iOldScreen->ResizeWithGcL(iControl->Size());
	
	iOldScreen->Gc().BitBlt(TPoint(0,0), iControl->OffScreenBitmap());
	iControl->Refresh(KRefreshOptionDisplayOnly);
	iControl->DrawOffscreen();
	iContentVersion = iControl->ContentVersion();
	iNewScreen = iControl->OffScreenBitmap();
	iOpacity = iStep;
	Draw();
	
	if(!IsActive())
	{
		iTimer.After(iStatus, iInterval);
		SetActive();
	}
}

void CFadeTransition::SetParams(TInt aStep, TTimeIntervalMicroSeconds32 aInterval)
{
	iStep = aStep;
	iInterval = aInterval;
}

void CFadeTransition::Draw()
{
	RWindow& win = *(RWindow*)iControl->DrawableWindow();
	TRect rect = iControl->Rect();
	CWindowGc& gc = iControl->SystemGc();
	win.Invalidate(rect);
	gc.Activate(win);
	win.BeginRedraw(rect);

	gc.BitBlt(rect.iTl, iOldScreen);
	
	iMask->Gc().SetBrushColor(TRgb::Gray256(iOpacity));
	iMask->Gc().Clear();
	gc.BitBltMasked(rect.iTl, iNewScreen, TRect(TPoint(0,0), iNewScreen->SizeInPixels()), iMask, ETrue);
	
	win.EndRedraw();
	gc.Deactivate();
}

void CFadeTransition::DoCancel()
{
	iTimer.Cancel();
	delete iMask;
	iMask =  NULL;
	delete iOldScreen;
	iOldScreen = NULL;
}

void CFadeTransition::RunL()
{
	if(iContentVersion!=iControl->ContentVersion())
	{
		delete iMask;
		iMask = NULL;
		
		delete iOldScreen;
		iOldScreen = NULL;
		
		return;
	}
	
	if(iOpacity>=255)
	{
		iControl->RefreshAndDraw();
		return;
	}
	
	Draw();
	iOpacity += iStep;
	iTimer.After(iStatus, iInterval); 
	SetActive();
}

TInt CFadeTransition::RunError(TInt aError)
{
	return aError;
}

