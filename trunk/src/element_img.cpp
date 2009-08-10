#include <icl/imagedata.h>

#include "controlimpl.h"
#include "htmlctlenv.h"
#include "htmlparser.h"
#include "htmlctlutils.h"
#include "measurestatus.h"
#include "imagepool.h"
#include "imageimpl.h"
#include "writablebitmap.h"
#include "timer.h"
#include "utils.h"
#include "list.h"

#include "element_img.h"
#include "element_div.h"

CHtmlElementImg::CHtmlElementImg(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeImg;
	iTagName.Set(KHStrImg);
	iAnimation = ETrue;
	iStyle.SetTag(iTagName);
}

CHtmlElementImg::~CHtmlElementImg()
{
	if(iImage)
		iImage->RemoveLoadedEventSubscriber(this);
	
	iStyle.Close();
	iOwner->Impl()->Env()->ImagePool().Remove(iSrcImage);
	iOwner->Impl()->Env()->ImagePool().Remove(iFocusImage);

	delete iSrcStr;
	delete iFocusSrcStr;
	delete iAlt;
	delete iPlayer;
	delete iBackground;
}

TBool CHtmlElementImg::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.CompareF(KHStrSrc)==0)
	{
		if(iSrcStr)
			aBuffer.Copy(iSrcStr->Left(aBuffer.MaxLength()));
	}
	else if(aName.CompareF(KHStrFullPath)==0)
	{
		if(iImage && iImage->Location().iValid)
			aBuffer.Copy(iImage->Location().iFileName.Left(aBuffer.MaxLength()));
		else
			aBuffer.Zero();
	}
	else if(aName.CompareF(KHStrError)==0)
	{
		if(iImage && !iImage->IsError()) 
			aBuffer.Num(0);
		else
			aBuffer.Num(1);
	}
	else if(aName.Compare(KHStrAlt)==0)
	{
		if(iAlt)
			aBuffer.Copy(iAlt->Left(aBuffer.MaxLength()));
		else
			aBuffer.Zero();
	}
	else if(aName.CompareF(KHStrClass)==0) 
		aBuffer.Copy(iStyle.Class().Left(aBuffer.MaxLength()));
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.GetSingleStyle(aName.Mid(6), aBuffer);
	else
		return EFalse;
	
	return ETrue;
}

TBool CHtmlElementImg::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrSrc)==0)
	{	
		delete iSrcStr;
		iSrcStr = NULL;
		
		if(aValue.Length()>0)
		{
			iSrcStr = aValue.AllocL();
			iSrcStr->Des().Trim();
		}

		if(iSrcImage)
		{
			if(iImage==iSrcImage)
				SetCurrentImage(NULL);
			
			iOwner->Impl()->Env()->ImagePool().Remove(iSrcImage);
			iSrcImage = NULL;
		}
	}
	else if(aName.CompareF(KHStrFocusSrc)==0)
	{
		delete iFocusSrcStr;
		iFocusSrcStr = NULL;
		
		if(aValue.Length()>0)
		{
			iFocusSrcStr = aValue.AllocL();
			iFocusSrcStr->Des().Trim();
		}

		if(iFocusImage)
		{
			if(iImage==iFocusImage)
				SetCurrentImage(NULL);
			
			iOwner->Impl()->Env()->ImagePool().Remove(iFocusImage);
			iFocusImage = NULL;
		}
	}
	else if(aName.CompareF(KHStrWidth)==0) 
		iStyle.SetSingleStyleL(KHStrWidth, aValue);
	else if(aName.CompareF(KHStrHeight)==0) 
		iStyle.SetSingleStyleL(KHStrHeight, aValue);
	else if(aName.CompareF(KHStrAlign)==0)
		iStyle.SetSingleStyleL(KHStrAlign, aValue);
	else if(aName.CompareF(KHStrBorder)==0) 
		;
	else if(aName.CompareF(KHStrAnimation)==0) 
		iAnimation = HcUtils::StrToBool(aValue);
	else if(aName.Compare(KHStrAlt)==0)
	{
		delete iAlt;
		iAlt = NULL;
		iAlt = aValue.AllocL();
	}
	else if(aName.CompareF(KHStrClass)==0) 
		iStyle.SetClass(aValue);
	else if(aName.CompareF(KHStrStyle)==0) 
		iStyle.SetStyleL(aValue);
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.SetSingleStyleL(aName.Mid(6), aValue);
	else
		return EFalse;
	
	return ETrue;
}

CHtmlElementImpl* CHtmlElementImg::CloneL() const
{
	CHtmlElementImg* e = new (ELeave)CHtmlElementImg(iOwner);
	BaseClone(*e);
	
	e->iSrcImage = iSrcImage;
	if(iSrcImage)
		iSrcImage->AddRef();
	if(iSrcStr)
		e->iSrcStr = (*iSrcStr).AllocL();
	
	e->iFocusImage = iFocusImage;
	if(iFocusImage)
		iFocusImage->AddRef();
	if(iFocusSrcStr)
		e->iFocusSrcStr = (*iFocusSrcStr).AllocL();
	
	if(iAlt)
		e->iAlt = (*iAlt).AllocL();
	e->iAnimation = iAnimation;	
	e->iStyle.CopyL(iStyle);
	e->iAnimation = iAnimation;
	e->iOwnerList = iOwnerList;

	return e;
}

void CHtmlElementImg::SetCurrentImage(CHcImage* aImage)
{
	if(iImage && iImage!=aImage)
		iImage->RemoveLoadedEventSubscriber(this);
	iImage = aImage;
	if(iOwnerList && aImage)
		iOwnerList->NotifyChildSetImage(iImage);
}

void CHtmlElementImg::Measure(CHcMeasureStatus& aStatus)
{
	if(iStyle.Update(iOwner->Impl()))
	{
		iDrawParams.iCorner = iStyle.Style().IsSet(CHcStyle::ECorner)?iStyle.Style().iCorner:TSize(0,0);
		iDrawParams.iOpacity = iStyle.Style().IsSet(CHcStyle::EOpacity)?iStyle.Style().iOpacity:255;
		iDrawParams.iScale9Grid = iStyle.Style().IsSet(CHcStyle::EScale9Grid)?iStyle.Style().iScale9Grid:TRect();
	}
	
	if(iStyle.Style().IsDisplayNone()) {
		iState.Set(EElementStateHidden);
		return;
	}
	
	if(!iSrcImage && iSrcStr)
	{
		THcImageLocation location;
		HtmlParser::ParseImageSrc(*iSrcStr, location);
		iSrcImage = iOwner->Impl()->Env()->ImagePool().AddL(location);
	}
	
	if(!iFocusImage && iFocusSrcStr)
	{
		THcImageLocation location;
		HtmlParser::ParseImageSrc(*iFocusSrcStr, location);
		iFocusImage = iOwner->Impl()->Env()->ImagePool().AddL(location);
	}
	
	if(iParent->iState.IsSet(EElementStateInFocus) && iFocusImage)
		SetCurrentImage(iFocusImage);
	else
		SetCurrentImage(iSrcImage);

	if(iImage && !iImage->IsError()) 
	{
		TBool ani = iImage->Location().iType==ELTFileGeneral && ((CHcImageGeneral*)iImage)->FrameCount()>1
			&& iAnimation 
			&& !(iParent->iState.IsSet(EElementStateFaded) || iOwner->Impl()->iState.IsSet(EHCSInTransition));
		
		iState.Clear(EElementStateError);
		iState.Assign(EElementStateStatic, !ani);
	}
	else
	{
		if(iSrcStr && iSrcStr->Length()>0)
		{
			CHcImage*  errorImage = iOwner->Impl()->GetStyle(TagName(), iStyle.Class(), THcSelector::EFCError, 0).BackgroundImage();
			SetCurrentImage(errorImage);
		}
		iState.Set(EElementStateError);
		iState.Set(EElementStateStatic);
	}
	
	DoMeasure(aStatus, iStyle.Style(), iImage?iImage->Size():TSize(0,0), iState.IsSet(EElementStateError));
}

void CHtmlElementImg::Refresh()
{
	if(iState.IsSet(EElementStateHidden))
		return;
	
	if(iImage
			&& !iImage->IsLoaded() 
			&& !iOwner->Impl()->iState.IsSet(EHCSInTransition)) {
		iImage->AddLoadedEventSubscriber(this);
	}

	iViewRect = Rect();
	HcUtils::Intersection(iViewRect, iParent->iClippingRect);
	iViewRect.Move(-iPosition.iX, -iPosition.iY);
	
	if(!iViewRect.IsEmpty() && !iState.IsSet(EElementStateStatic))
 	{
		if(!iPlayer)
			iPlayer = CHcTimer<CHtmlElementImg>::NewL(*this);
		
		if(!iBackground)
			iBackground = CWritableBitmap::NewL(iSize, iOwner->SystemGc().Device()->DisplayMode());
		else if(iSize!=iBackground->SizeInPixels())
			iBackground->ResizeWithGcL(iSize);
 	}
}

void CHtmlElementImg::Draw(CFbsBitGc& aGc) const
{
	if(!iImage || iViewRect.IsEmpty())
		return;
	
	if(iState.IsSet(EElementStateError))
		iImage->Draw(aGc, Rect(), THcDrawImageParams());
	else if(iState.IsSet(EElementStateStatic))
	{
		((CHtmlElementImg*)this)->iDrawParams.iFrameIndex = 0;
		iImage->Draw(aGc, Rect(), iDrawParams);
	}
	else
	{
		if(iBackground)
			iBackground->Gc().BitBlt(TPoint(0,0), iOwner->OffScreenBitmap(), Rect());
		
		((CHtmlElementImg*)this)->iCurrentFrame = 0;
		((CHtmlElementImg*)this)->iDrawParams.iFrameIndex = 0;
		iImage->Draw(aGc, Rect(), iDrawParams);
		
		if(!iPlayer->IsActive())
			iPlayer->After(10000);
	}
	
	if(iState.IsSet(EElementStateInLink) && iSize.iHeight!=0 && iSize.iWidth!=0)
	{
		TRect rect = Rect();
		rect.Grow(1,1);
		aGc.SetBrushStyle(CGraphicsContext::ENullBrush);
		aGc.SetPenStyle(CGraphicsContext::ESolidPen);
		aGc.SetPenColor(KRgbBlue);
		aGc.SetPenSize(TSize(1,1));
		aGc.DrawRect(rect);
	}
}

void CHtmlElementImg::OnTimerL(TInt aError, TInt)
{
	if(aError!=KErrNone
			|| iState.IsSet(EElementStateStatic)
			|| iState.IsSet(EElementStateHidden)
			|| iViewRect.IsEmpty())
		return;

	CHcImageGeneral* image = (CHcImageGeneral*)iImage;
	const CHcImageGeneral::TFrame& frame = image->Frame(iCurrentFrame);
	if(!frame.iConverted) 
	{
		iPlayer->After(100000);
		return;
	}
	
	#ifdef __SYMBIAN_9_ONWARDS__
	iPlayer->After(TTimeIntervalMicroSeconds32(frame.iDelay.Int64()));
	#else
	iPlayer->After(TTimeIntervalMicroSeconds32(frame.iDelay.Int64().GetTInt()));
	#endif

	RWindow& win = *(RWindow*)iOwner->DrawableWindow();
	if(win.IsFaded() || !HtmlCtlUtils::IsAppForeground())
		return;
	
	TRect drawRect = iViewRect;
	drawRect.Move(iPosition);
	CFbsBitGc& gc = ((CWritableBitmap*)iOwner->OffScreenBitmap())->Gc();
	gc.SetClippingRect(drawRect);
	
	if(iCurrentFrame==0) 
		gc.BitBlt(iPosition, iBackground);

	if(iCurrentFrame>0 && (image->Frame(iCurrentFrame-1).iFlags & TFrameInfo::ERestoreToBackground)) 
	{
		const CHcImageGeneral::TFrame& prevFrame = image->Frame(iCurrentFrame - 1);

		CWritableBitmap* restoreMask = CHtmlCtlEnv::Static()->HelperBitmap(0, iSize);
		
		restoreMask->Gc().SetBrushStyle(CGraphicsContext::ESolidBrush);
		restoreMask->Gc().SetBrushColor(KRgbBlack);
		restoreMask->Gc().Clear(TRect(TPoint(0,0), iSize));
		if(prevFrame.iBitmapMask) 
		{
			if(iSize==iImage->Size()) 
				restoreMask->Gc().BitBlt(prevFrame.iPosition, prevFrame.iBitmapMask);
			else 
			{
				TRect rect = TRect(prevFrame.iPosition, prevFrame.iSize);
				TReal ratioX = (TReal)iSize.iWidth/iImage->Size().iWidth;
				TReal ratioY = (TReal)iSize.iHeight/iImage->Size().iHeight;
				rect.iTl.iX = TInt((TReal)rect.iTl.iX * ratioX);
				rect.iTl.iY = TInt((TReal)rect.iTl.iY * ratioY);
				rect.iBr.iX = TInt((TReal)rect.iBr.iX * ratioX);
				rect.iBr.iY = TInt((TReal)rect.iBr.iY * ratioY);

				restoreMask->Gc().DrawBitmap(rect, prevFrame.iBitmapMask);
			}
		}
		gc.BitBltMasked(iPosition, iBackground, TRect(TPoint(0,0), iSize), restoreMask, EFalse);
	}
	
	iDrawParams.iFrameIndex = iCurrentFrame;
	image->Draw(gc, Rect(), iDrawParams);
	gc.CancelClippingRect();
	
	iCurrentFrame++;
	if(iCurrentFrame>=image->FrameCount()) 
		iCurrentFrame = 0;
	
	CWindowGc& wgc = iOwner->SystemGc();
	TRect winRect = TRect(iOwner->Rect().iTl + drawRect.iTl, drawRect.Size());
	win.Invalidate(winRect);
	wgc.Activate(win);
	win.BeginRedraw(winRect);
	wgc.BitBlt(winRect.iTl, iOwner->OffScreenBitmap(), drawRect);
	win.EndRedraw();
	wgc.Deactivate();
}

void CHtmlElementImg::NotifyImageLoaded(CHcImage* aImage)
{
	if(iImage==aImage)
	{
		if(!iParent->iClippingRect.IsEmpty())
		{
			if(iPosition.iY>iParent->iClippingRect.iBr.iY)
				iOwner->Impl()->iState.Set(EHCSNeedRefresh);
			else
				iOwner->Impl()->NotifyImageLoaded(aImage);
		}
		else if(iParent->iDisplayRect.IsEmpty())
			iOwner->Impl()->NotifyImageLoaded(aImage);
		else
			iOwner->Impl()->iState.Set(EHCSNeedRefresh);
	}
}

