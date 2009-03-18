#include <e32math.h>

#include "transimpl.h"
#include "controlimpl.h"
#include "elementimpl.h"
#include "htmlctlenv.h"
#include "element_div.h"
#include "writablebitmap.h"
#include "timer.h"

CTransition* CTransition::NewL()
{
	CTransition* self = new (ELeave)CTransition();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
}

CTransition::CTransition():CActive(EPriorityHigh)
{
	CActiveScheduler::Add(this);
}

CTransition::~CTransition()
{
	Cancel();
	
	delete iHelperBitmap;
	iTimer.Close();
}

void CTransition::ConstructL()
{
	iTimer.CreateLocal();
}

void CTransition::Perform(CHtmlElement* aElement, 
		TTransitionType aType,
		TTimeIntervalMicroSeconds32 aDuration,
		TInt aFrames,
		EasingFunction aEasingFunc)
{
	if(!aElement->Owner()->OffScreenBitmap())
	{
		aElement->Owner()->RefreshAndDraw();
		return;
	}
	
	iElement = (CHtmlElementImpl*)aElement;
	iType = aType;
	iFrames = aFrames;
	iDuration = aDuration;
	iOriginRect = aElement->Rect();
	iEasingFunc = aEasingFunc;
	iCurrentFrame = 0;
	iPos = 0;
	TBool prevHidden = iElement->iState.IsSet(EElementStateHidden);

	CHtmlControlImpl* control = aElement->Owner()->Impl();
	control->Refresh(ETrue);
	iTargetRect = aElement->Rect();
	
	if(!iHelperBitmap)
		iHelperBitmap = CWritableBitmap::NewL(control->OffScreenBitmap()->SizeInPixels(), control->OffScreenBitmap()->DisplayMode());
	else if(iHelperBitmap->SizeInPixels()!=control->OffScreenBitmap()->SizeInPixels())
		iHelperBitmap->ResizeWithGcL(control->OffScreenBitmap()->SizeInPixels());
	
	if(aType==EFly || aType==EScale || aType==EFlyAndScale)
	{
		if(prevHidden)
			iOriginRect.SetSize(TSize(0,0));
		CHtmlElementImpl* prev = iElement->iPrev;
		CHtmlElementImpl* next;
		if(iElement->TypeId()==EElementTypeDiv)
			next = ((CHtmlElementDiv*)iElement)->iEnd->iNext;
		else
			next = iElement->iNext;
		prev->iNext = next;
		next->iPrev = prev;
		control->DrawOffscreen();
		prev->iNext = iElement;
		if(iElement->TypeId()==EElementTypeDiv)
			next->iPrev = ((CHtmlElementDiv*)iElement)->iEnd;
		else
			next->iPrev = iElement;
		iHelperBitmap->Gc().CancelClippingRect();
		iHelperBitmap->Gc().BitBlt(TPoint(0,0), control->OffScreenBitmap());
	}
	else
	{
		//Draw the new content on iTransitionBitmap
		CWritableBitmap* tmp = control->OffScreenBitmap();
		control->SetOffScreenBitmap(iHelperBitmap);
		control->DrawOffscreen();
		control->SetOffScreenBitmap(tmp);
	}

	DoTransition();
}

void CTransition::DoTransition()
{
	if(iCurrentFrame==iFrames)
	{
		iElement->Owner()->RefreshAndDraw();
		return;
	}
	
	RWindow& win = *(RWindow*)iElement->Owner()->DrawableWindow();
	TRect rect = iElement->Owner()->Rect();
	CWindowGc& gc = iElement->Owner()->SystemGc();
	win.Invalidate(rect);
	gc.Activate(win);
	win.BeginRedraw(rect);
	
	switch(iType)
	{
		case ESlideLeft:
			DoSlideLeft();
			break;
		case ESlideRight:
			DoSlideRight();
			break;
		case ESlideUp:
			DoSlideUp();
			break;
		case ESlideDown:
			DoSlideDown();
			break;	
		case EFade:
			DoFade();
			break;
		case EFly:
			DoFly();
			break;
		case EScale:
			DoScale();
			break;
		case EFlyAndScale:
			DoFlyAndScale();
			break;
	}
	
	gc.BitBlt(rect.iTl, iElement->Owner()->OffScreenBitmap());
	win.EndRedraw();
	gc.Deactivate();
	
	iCurrentFrame++;
	iTimer.After(iStatus, iDuration.Int()/iFrames);
	SetActive();
}

void CTransition::RunL()
{
	if(iStatus.Int()!=KErrNone)
		return;
	
	DoTransition();
}

TInt CTransition::RunError(TInt)
{
	return KErrNone;
}

void CTransition::DoCancel()
{
	iTimer.Cancel();
}

void CTransition::DoSlideLeft()
{
	CWritableBitmap* tmpscreen = iElement->Owner()->Impl()->Env()->HelperBitmap(2, iOriginRect.Size());
	CWritableBitmap* offscreen = iElement->Owner()->Impl()->OffScreenBitmap();
	offscreen->Gc().CancelClippingRect();
	
	TInt pos = (TInt)(*iEasingFunc)(iCurrentFrame, 0, iOriginRect.Width(), iFrames);
	TInt step = pos - iPos;
		
	tmpscreen->Gc().BitBlt(TPoint(0,0), offscreen, 
			TRect(iOriginRect.iTl + TPoint(step, 0), TSize(iOriginRect.Width()-step, iOriginRect.Height())));
	tmpscreen->Gc().BitBlt(TPoint(iOriginRect.Width()-step, 0), iHelperBitmap, 
			TRect(iOriginRect.iTl + TPoint(iPos, 0), TSize(step, iOriginRect.Height())));

	iPos = pos;

	offscreen->Gc().BitBlt(iOriginRect.iTl, tmpscreen, TRect(TPoint(0,0), iOriginRect.Size()));
}

void CTransition::DoSlideRight()
{
	CWritableBitmap* tmpscreen = iElement->Owner()->Impl()->Env()->HelperBitmap(2, iOriginRect.Size());
	CWritableBitmap* offscreen = iElement->Owner()->Impl()->OffScreenBitmap();
	offscreen->Gc().CancelClippingRect();
	
	TInt pos = (TInt)(*iEasingFunc)(iCurrentFrame, 0, iOriginRect.Width(), iFrames);
	TInt step = pos - iPos;
	
	tmpscreen->Gc().BitBlt(TPoint(step, 0), offscreen,
			TRect(iOriginRect.iTl, TSize(iOriginRect.Width()-step, iOriginRect.Height())));
	tmpscreen->Gc().BitBlt(TPoint(0,0), iHelperBitmap,
			TRect(iOriginRect.iTl + TPoint(iOriginRect.Width()-pos, 0), TSize(step, iOriginRect.Height())));
	
	iPos = pos;

	offscreen->Gc().BitBlt(iOriginRect.iTl, tmpscreen, TRect(TPoint(0,0), iOriginRect.Size()));
}

void CTransition::DoSlideUp()
{
	CWritableBitmap* tmpscreen = iElement->Owner()->Impl()->Env()->HelperBitmap(2, iOriginRect.Size());
	CWritableBitmap* offscreen = iElement->Owner()->Impl()->OffScreenBitmap();
	offscreen->Gc().CancelClippingRect();
	
	TInt pos = (TInt)(*iEasingFunc)(iCurrentFrame, 0, iOriginRect.Height(), iFrames);
	TInt step = pos - iPos;
	
	tmpscreen->Gc().BitBlt(TPoint(0,0), offscreen, 
			TRect(iOriginRect.iTl + TPoint(0,step), TSize(iOriginRect.Width(),iOriginRect.Height()-step)));
	tmpscreen->Gc().BitBlt(TPoint(0,iOriginRect.Height()-step), iHelperBitmap, 
			TRect(iOriginRect.iTl + TPoint(0,iPos), TSize(iOriginRect.Width(),step)));
	
	iPos = pos;

	offscreen->Gc().BitBlt(iOriginRect.iTl, tmpscreen, TRect(TPoint(0,0), iOriginRect.Size()));
}

void CTransition::DoSlideDown()
{
	CWritableBitmap* tmpscreen = iElement->Owner()->Impl()->Env()->HelperBitmap(2, iOriginRect.Size());
	CWritableBitmap* offscreen = iElement->Owner()->Impl()->OffScreenBitmap();
	offscreen->Gc().CancelClippingRect();
	
	TInt pos = (TInt)(*iEasingFunc)(iCurrentFrame, 0, iOriginRect.Height(), iFrames);
	TInt step = pos - iPos;
	
	tmpscreen->Gc().BitBlt(TPoint(0,step), offscreen,
			TRect(iOriginRect.iTl, TSize(iOriginRect.Width(), iOriginRect.Height()-step)));
	tmpscreen->Gc().BitBlt(TPoint(0,0), iHelperBitmap,
			TRect(iOriginRect.iTl + TPoint(0,iOriginRect.Height()-pos), TSize(iOriginRect.Width(),step)));
	
	iPos = pos;
	
	offscreen->Gc().BitBlt(iOriginRect.iTl, tmpscreen, TRect(TPoint(0,0), iOriginRect.Size()));
}

void CTransition::DoFade()
{
	CWritableBitmap* offscreen = iElement->Owner()->Impl()->OffScreenBitmap();
	offscreen->Gc().CancelClippingRect();
	
	CWritableBitmap* mask = iElement->Owner()->Impl()->Env()->HelperBitmap(1, iHelperBitmap->SizeInPixels());
	TInt alpha = (TInt)(*iEasingFunc)(iCurrentFrame, 0, 255, iFrames);
	mask->Gc().SetBrushColor(TRgb::Gray256(alpha));
	mask->Gc().Clear(iOriginRect);
	offscreen->Gc().BitBltMasked(iOriginRect.iTl, iHelperBitmap, TRect(iOriginRect.iTl, iOriginRect.Size()), 
			mask, ETrue);
}

void CTransition::DoFly()
{
	CWritableBitmap* offscreen = iElement->Owner()->Impl()->OffScreenBitmap();
	offscreen->Gc().CancelClippingRect();
	offscreen->Gc().BitBlt(TPoint(0,0), iHelperBitmap);
	TPoint pos;
	pos.iX = (TInt)(*iEasingFunc)(iCurrentFrame, iOriginRect.iTl.iX, iTargetRect.iTl.iX - iOriginRect.iTl.iX, iFrames);
	pos.iY = (TInt)(*iEasingFunc)(iCurrentFrame, iOriginRect.iTl.iY, iTargetRect.iTl.iY - iOriginRect.iTl.iY, iFrames);

	if(iElement->TypeId()==EElementTypeDiv)
	{
		offscreen->Gc().SetClippingRect(iElement->iParent->iClippingRect);
		
		TPoint step = pos - iElement->iPosition;
		iElement->iPosition = pos;	
		((CHtmlElementDiv*)iElement)->iDisplayRect.Move(step);
		iElement->Refresh();
		iElement->Draw(offscreen->Gc());
		
		CHtmlElementImpl* e = iElement->iNext;
		while(e!=((CHtmlElementDiv*)iElement)->iEnd)
		{
			e->iPosition += step;
			if(e->TypeId()==EElementTypeDiv)
				((CHtmlElementDiv*)e)->iDisplayRect.Move(step);
			e->Refresh();
			e->Draw(offscreen->Gc());
			e = e->iNext;
		}
	}
	else
	{
		offscreen->Gc().SetClippingRect(iElement->iParent->iClippingRect);
		iElement->iPosition = pos;	
		iElement->Refresh();
		iElement->Draw(offscreen->Gc());
	}
}

void CTransition::DoScale()
{
	CWritableBitmap* offscreen = iElement->Owner()->Impl()->OffScreenBitmap();
	offscreen->Gc().CancelClippingRect();
	offscreen->Gc().BitBlt(TPoint(0,0), iHelperBitmap);
	TSize size;
	size.iWidth = (TInt) (*iEasingFunc)(iCurrentFrame, iOriginRect.Width(), iTargetRect.Width()- iOriginRect.Width(), iFrames);
	size.iHeight = (TInt) (*iEasingFunc)(iCurrentFrame, iOriginRect.Height(), iTargetRect.Height() - iOriginRect.Height(), iFrames);

	if(iElement->TypeId()==EElementTypeDiv)
	{
		offscreen->Gc().SetClippingRect(iElement->iParent->iClippingRect);
		
		TSize step = size - iElement->iSize;
		iElement->iSize = size;	
		((CHtmlElementDiv*)iElement)->iDisplayRect.iBr.iX += step.iWidth;
		((CHtmlElementDiv*)iElement)->iDisplayRect.iBr.iY += step.iHeight;
		iElement->Refresh();
		iElement->Draw(offscreen->Gc());
		
		CHtmlElementImpl* e = iElement->iNext;
		while(e!=((CHtmlElementDiv*)iElement)->iEnd)
		{
			e->Refresh();
			e->Draw(offscreen->Gc());
			e = e->iNext;
		}
	}
	else
	{
		offscreen->Gc().SetClippingRect(iElement->iParent->iClippingRect);
		iElement->iSize = size;	
		iElement->Refresh();
		iElement->Draw(offscreen->Gc());
	}
}

void CTransition::DoFlyAndScale()
{
	CWritableBitmap* offscreen = iElement->Owner()->Impl()->OffScreenBitmap();
	offscreen->Gc().CancelClippingRect();
	offscreen->Gc().BitBlt(TPoint(0,0), iHelperBitmap);
	TPoint pos;
	pos.iX = (TInt) (*iEasingFunc)(iCurrentFrame, iOriginRect.iTl.iX, iTargetRect.iTl.iX - iOriginRect.iTl.iX, iFrames);
	pos.iY = (TInt) (*iEasingFunc)(iCurrentFrame, iOriginRect.iTl.iY, iTargetRect.iTl.iY - iOriginRect.iTl.iY, iFrames);
	TSize size;
	size.iWidth = (TInt) (*iEasingFunc)(iCurrentFrame, iOriginRect.Width(), iTargetRect.Width()- iOriginRect.Width(), iFrames);
	size.iHeight = (TInt) (*iEasingFunc)(iCurrentFrame, iOriginRect.Height(), iTargetRect.Height() - iOriginRect.Height(), iFrames);

	if(iElement->TypeId()==EElementTypeDiv)
	{
		offscreen->Gc().SetClippingRect(iElement->iParent->iClippingRect);
		
		TPoint posStep = pos - iElement->iPosition;
		iElement->iPosition = pos;	
		((CHtmlElementDiv*)iElement)->iDisplayRect.Move(posStep);
		TSize sizeStep = size - iElement->iSize;
		iElement->iSize = size;	
		((CHtmlElementDiv*)iElement)->iDisplayRect.iBr.iX += sizeStep.iWidth;
		((CHtmlElementDiv*)iElement)->iDisplayRect.iBr.iY += sizeStep.iHeight;
		iElement->Refresh();
		iElement->Draw(offscreen->Gc());
		
		CHtmlElementImpl* e = iElement->iNext;
		while(e!=((CHtmlElementDiv*)iElement)->iEnd)
		{
			e->iPosition += posStep;
			if(e->TypeId()==EElementTypeDiv)
				((CHtmlElementDiv*)e)->iDisplayRect.Move(posStep);
			e->Refresh();
			e->Draw(offscreen->Gc());
			e = e->iNext;
		}
	}
	else
	{
		offscreen->Gc().SetClippingRect(iElement->iParent->iClippingRect);
		iElement->iPosition = pos;
		iElement->iSize = size;	
		iElement->Refresh();
		iElement->Draw(offscreen->Gc());
	}	
}

TReal TEasingLinear::EaseNone(TReal t, TReal b, TReal c, TReal d)
{
	return c*t/d + b;
}

TReal TEasingCircular::EaseIn(TReal t, TReal b, TReal c, TReal d)
{
	TReal s;
	Math::Sqrt(s, 1 - (t/=d)*t);
	return -c * (s - 1) + b;
}

TReal TEasingCircular::EaseOut(TReal t, TReal b, TReal c, TReal d)
{
	TReal s;
	Math::Sqrt(s, 1 - (t=t/d-1)*t);
	return c * s + b;
}

TReal TEasingCircular::EaseInOut(TReal t, TReal b, TReal c, TReal d)
{
	return c*t/d + b;
}

TReal TEasingBack::EaseIn(TReal t, TReal b, TReal c, TReal d)
{
	TReal s = 1.70158;
	return c*(t/=d)*t*((s+1)*t - s) + b;
}

TReal TEasingBack::EaseOut(TReal t, TReal b, TReal c, TReal d)
{
	TReal s = 1.70158;
	return c*((t=t/d-1)*t*((s+1)*t + s) + 1) + b;
}

TReal TEasingBack::EaseInOut(TReal t, TReal b, TReal c, TReal d)
{
	TReal s = 1.70158;
	if ((t/=d/2) < 1) return c/2*(t*t*(((s*=(1.525))+1)*t - s)) + b;
	return c/2*((t-=2)*t*(((s*=(1.525))+1)*t + s) + 2) + b;
}


