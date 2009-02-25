#include <eikenv.h>

#include "slidetrans.h"
#include "htmlcontrol.h"
#include "writablebitmap.h"

CSlideTransition::CSlideTransition(CHtmlControl* aControl) :
	CActive(EPriorityStandard) // Standard priority
	,iControl(aControl)
{
	iInterval = 10000;
	iStep = 30;
}

CSlideTransition* CSlideTransition::NewL(CHtmlControl* aControl)
{
	CSlideTransition* self = new ( ELeave ) CSlideTransition(aControl);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(); // self;
	return self;
}

void CSlideTransition::ConstructL()
{
	User::LeaveIfError(iTimer.CreateLocal() ); // Initialize timer
	CActiveScheduler::Add( this); // Add to scheduler
}

CSlideTransition::~CSlideTransition()
{
	Cancel(); // Cancel any request, if outstanding
	iTimer.Close(); // Destroy the RTimer object
	delete iOldScreen;
	// Delete instance variables if any
}

void CSlideTransition::StartL(TMode aMode)
{	
	if(iControl->OffScreenBitmap()==NULL)
	{
		iControl->RefreshAndDraw();
		return;
	}
	
	if(!iOldScreen)
		iOldScreen = CWritableBitmap::NewL(iControl->Size(), CEikonEnv::Static()->ScreenDevice()->DisplayMode());
	else if(iOldScreen->SizeInPixels()!=iControl->Size())
		iOldScreen->ResizeWithGcL(iControl->Size());
	iOldScreen->Gc().BitBlt(TPoint(0,0), iControl->OffScreenBitmap());
	iControl->Refresh(KRefreshOptionDisplayOnly);
	iControl->DrawOffscreen();
	iContentVersion = iControl->ContentVersion();
	iNewScreen = iControl->OffScreenBitmap();
	iMode = aMode;
	iPos = iStep;
	Draw();
	
	if(!IsActive())
	{
		iTimer.After(iStatus, iInterval);
		SetActive();
	}
}

void CSlideTransition::SetParams(TInt aStep, TTimeIntervalMicroSeconds32 aInterval)
{
	iStep = aStep;
	iInterval = aInterval;
}

void CSlideTransition::Draw()
{
	RWindow& win = *(RWindow*)iControl->DrawableWindow();
	TRect rect = iControl->Rect();
	CWindowGc& gc = iControl->SystemGc();
	win.Invalidate(rect);
	gc.Activate(win);
	win.BeginRedraw(rect);

	switch(iMode)
	{
	case ESlideLeft:
		gc.BitBlt(rect.iTl, iOldScreen, TRect(TPoint(iPos, 0), TSize(rect.Size().iWidth-iPos, rect.Size().iHeight)));
		gc.BitBlt(rect.iTl+TPoint(rect.Size().iWidth-iPos,0), iNewScreen, TRect(TPoint(0, 0), TSize(iPos, rect.Size().iHeight)));
		break;
		
	case ESlideRight:
		gc.BitBlt(rect.iTl+TPoint(iPos,0), iOldScreen, TRect(TPoint(0, 0), TSize(rect.Size().iWidth-iPos, rect.Size().iHeight)));
		gc.BitBlt(rect.iTl, iNewScreen, TRect(TPoint(rect.Size().iWidth-iPos, 0), TSize(iPos, rect.Size().iHeight)));
		break;
		
	case ESlideUp:
		gc.BitBlt(rect.iTl, iOldScreen, TRect(TPoint(0, iPos), TSize(rect.Size().iWidth, rect.Size().iHeight-iPos)));
		gc.BitBlt(rect.iTl+TPoint(0,rect.Size().iHeight-iPos), iNewScreen, TRect(TPoint(0, 0), TSize(rect.Size().iWidth, iPos)));
		break;
		
	case ESlideDown:
		gc.BitBlt(rect.iTl+TPoint(0,iPos), iOldScreen, TRect(TPoint(0, 0), TSize(rect.Size().iWidth, rect.Size().iHeight-iPos)));
		gc.BitBlt(rect.iTl, iNewScreen, TRect(TPoint(0, rect.Size().iHeight-iPos), TSize(rect.Size().iWidth, iPos)));
		break;
		
	default:
		break;
	}
	
	win.EndRedraw();
	gc.Deactivate();
}

void CSlideTransition::DoCancel()
{
	iTimer.Cancel();
	delete iOldScreen;
	iOldScreen = NULL;
}

void CSlideTransition::RunL()
{
	if(iContentVersion!=iControl->ContentVersion())
	{
		delete iOldScreen;
		iOldScreen = NULL;
		return;
	}
	
	if(iMode==ESlideLeft || iMode==ESlideRight)
	{
		if(iPos>=iControl->Size().iWidth)
		{
			iControl->RefreshAndDraw();
			return;
		}
	}
	else
	{
		if(iPos>=iControl->Size().iHeight)
		{
			iControl->RefreshAndDraw();
			return;
		}
	}
	
	Draw();
	iPos += iStep;
	iTimer.After(iStatus, iInterval); 
	SetActive();
}

TInt CSlideTransition::RunError(TInt aError)
{
	return aError;
}
