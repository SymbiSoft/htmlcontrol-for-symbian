#include <eikenv.h>
#include <eikscrlb.h>

#include "htmlcontrol.hrh"

#ifdef __SERIES60__
#include <AknsConstants.h>
#include <AknsConstants.hrh>
#include <AknsDrawUtils.h>
#include <AknsBasicBackgroundControlContext.h>
#include <AknAppUi.h>
#include <aknutils.h>
#endif

#ifdef __UIQ__
#include <Skins.h>
#include <QSkinIds.h>
#endif

#include "controlimpl.h"
#include "htmlctlenv.h"
#include "htmlctlutils.h"
#include "scrollbar.h"
#include "utils.h"
#include "writablebitmap.h"
#include "list.h"
#include "element_div.h"
#include "stylesheet.h"
#include "htmlparser.h"

#define KPointerRepeatStartInerval TTimeIntervalMicroSeconds32(300000)
#define KPointerRepeatInerval TTimeIntervalMicroSeconds32(100000)

CHcScrollbar::CHcScrollbar(CHtmlElementDiv* aDiv)
{
	iDiv = aDiv;
	iStep = KScrollStep;
	iDrawer = &CHtmlCtlEnv::Static()->ScrollbarDrawer();
}

TBool CHcScrollbar::GetProperty(const TDesC& aName, TDes& aBuffer)
{
	if(aName.Compare(KHStrScrollPos)==0)
		aBuffer.Num(iRealPos);
	else if(aName.Compare(KHStrScrollMax)==0)
		aBuffer.Num(iMax);
	else
		return EFalse;
	
	return ETrue;
}

TBool CHcScrollbar::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(aName.Compare(KHStrScrollLoop)==0)
		iFlags.Assign(ELoop, HcUtils::StrToBool(aValue));
	else if(aName.Compare(KHStrScrollStep)==0)
		iStep = HcUtils::StrToInt(aValue);
	else if(aName.Compare(KHStrScrollPos)==0)
		iPos = HcUtils::StrToInt(aValue);
	else if(aName.Compare(KHStrScrollVisible)==0)
		iFlags.Assign(EDisplayNone,!HcUtils::StrToInt(aValue));
	else if(aName.Compare(KHStrScrollOnSlide)==0)
		iFlags.Assign(EScrollOnSlide, HcUtils::StrToInt(aValue));
	else
		return EFalse;
	
	return ETrue;
}

void CHcScrollbar::AdjustScrollPos()
{
	if(iFlags.IsSet(EDimmed))
	{
		iPos = 0;
		iMax = 0;
		iRealPos = 0;
		return;
	}
	
	TInt vpadings = iDiv->iPaddings.iTop + iDiv->iPaddings.iBottom;
	iContentHeight = iDiv->iContentSize.iHeight + vpadings;
	iMax = iDiv->iContentSize.iHeight - iDiv->iDisplayRect.Height();
	if(iMax<0)
		iMax = 0;
	if(iPos>iMax)
		iPos = iMax;
	if(iPos<0)
		iPos = 0;
	iRealPos = iPos;
}

void CHcScrollbar::Refresh()
{
	if(iMax==0 && !iFlags.IsSet(EAlwaysVisible) 
			|| iFlags.IsSet(EDimmed)
			|| iFlags.IsSet(EDisplayNone))
	{
		iState.Set(EHidden);
		return;
	}
	
	iState.Clear(EHidden);	
	iRect.iTl.iX = iDiv->iDisplayRect.iBr.iX + iDiv->iPaddings.iRight;
	iRect.iBr.iX = iDiv->iDisplayRect.iBr.iX + iDiv->iPaddings.iRight + iDrawer->iWidth;
	iRect.iTl.iY = iDiv->iDisplayRect.iTl.iY -  iDiv->iPaddings.iTop;
	iRect.iBr.iY = iDiv->iDisplayRect.iBr.iY +  iDiv->iPaddings.iBottom;
	
	TInt thumSize, scrollPos;
	if(iMax>0) 
	{
		thumSize = (iRect.Height() - 2*iDrawer->iArrowHeight) *  iDiv->iDisplayRect.Height()/iContentHeight;
		if(thumSize<iDrawer->iWidth*2)
			thumSize = iDrawer->iWidth*2;
		scrollPos = iPos * (iRect.Height() - thumSize - 2*iDrawer->iArrowHeight)/iMax;
	}
	else
	{
		thumSize =  iRect.Height() - 4  - 2*iDrawer->iArrowHeight;
		scrollPos = 0;
	}
	
	TPoint sliderTop(iRect.iTl.iX, iRect.iTl.iY + scrollPos + iDrawer->iArrowHeight);
	iThumbRect = TRect(sliderTop, TPoint(iRect.iBr.iX, sliderTop.iY + thumSize));

	if(iDrawer->iArrowHeight>0)
	{
		iUpArrowRect = TRect(iRect.iTl.iX, iRect.iTl.iY, iRect.iBr.iX, iRect.iTl.iY + iDrawer->iArrowHeight);
		iDownArrowRect = TRect(iRect.iTl.iX, iRect.iBr.iY - iDrawer->iArrowHeight, iRect.iBr.iX, iRect.iBr.iY);
	}
}

void CHcScrollbar::Draw(CFbsBitGc& aGc) const
{
	if(!iState.IsSet(EHidden))
		iDrawer->Draw(aGc, *this);
}

TBool CHcScrollbar::RemoveStepPos(TBool aAllowLoop)
{
	if(iMax>0)
	{
		if(iPos>0)
		{
			iPos -= iStep;
			if(iPos<=iStep/2)
				iPos = 0;
			return ETrue;
		}
		else if(aAllowLoop && iFlags.IsSet(ELoop))
		{
			iPos = iMax;
			return ETrue;
		}
	}
	
	return EFalse;
}

TBool CHcScrollbar::AddStepPos(TBool aAllowLoop)
{
	if(iMax>0)
	{
		if(iPos<iMax)
		{
			iPos += iStep;
			if(iPos>=iMax - iStep/2)
				iPos = iMax;
			return ETrue;
		}
		else if(aAllowLoop && iFlags.IsSet(ELoop))
		{
			iPos = 0;
			return ETrue;
		}
	}
	
	return EFalse;
}

void CHcScrollbar::AddPos(TInt aDelta)
{
	if(iMax>0)
	{
		iPos += aDelta;
		if(iPos>iMax)
			iPos = iMax;
		else if(iPos<0)
			iPos = 0;
	}
}

void CHcScrollbar::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
	if(aPointerEvent.iType==TPointerEvent::EButton1Down)
	{
		if(iState.IsSet(EHidden))
			return;

		if(iDrawer->iArrowHeight>0
				&& aPointerEvent.iPosition.iY>iUpArrowRect.iTl.iY 
				&& aPointerEvent.iPosition.iY<iUpArrowRect.iBr.iY)
		{
			iState.Set(EUpArrowPressed);
			iDiv->Owner()->Impl()->iState.Set(EHCSNeedRedraw);
			if(RemoveStepPos())
			{
				iDiv->Owner()->DrawableWindow()->RequestPointerRepeatEvent(KPointerRepeatStartInerval, iUpArrowRect);
				iDiv->Owner()->Impl()->NotifyScrollPosChanged();
			}
		}
		else if(iDrawer->iArrowHeight>0
				&& aPointerEvent.iPosition.iY>iDownArrowRect.iTl.iY 
				&& aPointerEvent.iPosition.iY<iDownArrowRect.iBr.iY)
		{
			iState.Set(EDownArrowPressed);
			iDiv->Owner()->Impl()->iState.Set(EHCSNeedRedraw);
			
			if(AddStepPos())
			{
				iDiv->Owner()->DrawableWindow()->RequestPointerRepeatEvent(KPointerRepeatStartInerval, iDownArrowRect);
				iDiv->Owner()->Impl()->NotifyScrollPosChanged();
			}
		}
		else if(aPointerEvent.iPosition.iY>iThumbRect.iTl.iY && aPointerEvent.iPosition.iY<iThumbRect.iBr.iY)
		{
			iState.Set(EThumbPressed);
			iDiv->Owner()->Impl()->iState.Set(EHCSNeedRedraw);
			iDragStartPos = aPointerEvent.iPosition.iY - iThumbRect.iTl.iY;
		}
		else
		{
			iState.Set(EShaftPressed);
			iDiv->Owner()->Impl()->iState.Set(EHCSNeedRedraw);
			if(aPointerEvent.iPosition.iY>iThumbRect.iBr.iY)
			{
				if(iPos!=iMax)
				{
					iPos = iMax;
					iDiv->Owner()->Impl()->NotifyScrollPosChanged();
				}
			}
			else
			{
				if(iPos!=0)
				{
					iPos = 0;
					iDiv->Owner()->Impl()->NotifyScrollPosChanged();
				}
			}
		}
#ifdef TOUCH_FEEDBACK_SUPPORT
		iDiv->Owner()->Impl()->Env()->TouchFeedback()->InstantFeedback(ETouchFeedbackBasic);
#endif
	}
	else if(aPointerEvent.iType==TPointerEvent::EButton1Up)
	{
		if(iState.IsSet(EShaftPressed) || iState.IsSet(EThumbPressed)
				|| iState.IsSet(EUpArrowPressed) || iState.IsSet(EDownArrowPressed))
			iDiv->Owner()->Impl()->iState.Set(EHCSNeedRedraw);
		
		iState.Clear(EShaftPressed);
		iState.Clear(EThumbPressed);
		iState.Clear(EUpArrowPressed);
		iState.Clear(EDownArrowPressed);
	}
	else if(aPointerEvent.iType==TPointerEvent::EDrag)
	{
		if(iState.IsSet(EThumbPressed))
		{
			TInt pos = aPointerEvent.iPosition.iY - iDragStartPos - iRect.iTl.iY  - iDrawer->iArrowHeight;
			TInt t = iRect.Height() - iThumbRect.Height() - 2*iDrawer->iArrowHeight;
			if(t!=0)
				pos = pos * iMax/t;
			else
				pos = 0;
			
			if(pos<0)
				pos = 0;
			else if(pos>iMax)
				pos = iMax;
			
			if(pos!=iPos)
			{
				iPos = pos;
				iDiv->Owner()->Impl()->NotifyScrollPosChanged();
			}
		}
	}
	else if(aPointerEvent.iType==TPointerEvent::EButtonRepeat)
	{
		if(iState.IsSet(EUpArrowPressed))
		{
			if(RemoveStepPos())
			{
				iDiv->Owner()->DrawableWindow()->RequestPointerRepeatEvent(KPointerRepeatInerval, iUpArrowRect);
				iDiv->Owner()->Impl()->NotifyScrollPosChanged();
			}
		}
		else if(iState.IsSet(EDownArrowPressed))
		{
			if(AddStepPos())
			{
				iDiv->Owner()->DrawableWindow()->RequestPointerRepeatEvent(KPointerRepeatInerval, iDownArrowRect);
				iDiv->Owner()->Impl()->NotifyScrollPosChanged();
			}
		}
	}
}

CHcScrollbarDrawer* CHcScrollbarDrawer::NewL()
{
	CHcScrollbarDrawer* self = new (ELeave)CHcScrollbarDrawer();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
}

CHcScrollbarDrawer::~CHcScrollbarDrawer()
{
	CHcImagePool& pool = CHtmlCtlEnv::Static()->ImagePool();
	for(TInt i=0;i<8;i++)
	{
		if(iComponents[i].iImage)
			pool.Remove(iComponents[i].iImage);
	}
	
	delete iBgBmp;
	delete iBgBmpMask;
	delete iHandleBmp;
	delete iHandleBmpMask;
}

#ifdef __SERIES60_3_ONWARDS__
static void DrawScrollSkinImage(const TAknsItemID& aItemID, CWritableBitmap* aBitmap, CWritableBitmap* aBitmapMask, TSize& aSize)
{
	TRect rect = TRect(TPoint(0,0), aSize);
	aBitmap->Gc().SetBrushColor(TRgb(0x123456));
	aBitmap->Gc().Clear(rect);	
	aBitmapMask->Gc().SetBrushColor(TRgb::Gray256(0));
	aBitmapMask->Gc().Clear(rect);
	
	CAknsBasicBackgroundControlContext* ctx = CAknsBasicBackgroundControlContext::NewL(aItemID,	rect, EFalse );
	AknsDrawUtils::DrawBackground( AknsUtils::SkinInstance(),
                                 ctx,
                                 NULL,
                                 aBitmap->Gc(),
                                 rect.iTl,
                                 rect,
                                 KAknsDrawParamRGBOnly);
	AknsDrawUtils::DrawBackground( AknsUtils::SkinInstance(),
                                 ctx,
                                 NULL,
                                 aBitmapMask->Gc(),
                                 rect.iTl,
                                 rect,
                                 KAknsSDMAlphaOnly);
	delete ctx;
	
	TSize ret;
	TInt last = 0;
	TBitmapUtil bu(aBitmap);
	bu.Begin(TPoint(aSize.iWidth-1,aSize.iHeight-1));
	for(TInt i=aSize.iHeight-1;i>=0;i--) 
	{
		bu.SetPos(TPoint(0, i));
		for(TInt j=0;j<aSize.iWidth;j++) 
		{
			TUint32 c = bu.GetPixel();
			if(c!=0x563412)
			{
				last = i+1;
				goto out;
			}
			bu.IncXPos();
		}
	}
out:
	ret.iHeight = last;
	last = 0;
	
	bu.SetPos(TPoint(aSize.iWidth-1,aSize.iHeight-1));
	for(TInt i=aSize.iWidth-1;i>=0;i--) 
	{
		bu.SetPos(TPoint(i,0));
		for(TInt j=0;j<aSize.iHeight;j++) 
		{
			TUint32 c = bu.GetPixel();
			if(c!=0x563412)
			{
				last = i+1;
				goto out2;
			}
			bu.IncYPos();
		}
	}
	
out2:
	ret.iWidth = last;
	
	bu.End();
	
	aSize = ret;
}
#endif

class CTempControl : public CCoeControl
{
public:
	CTempControl() {
		CreateWindowL();
	}
};
void CHcScrollbarDrawer::ConstructL()
{
#ifdef __SERIES60_3_DOWNWARDS__
	iDefaultScrollBarBreadth = 5;
#endif

#ifdef __SERIES60_3_ONWARDS__
	CCoeControl* control = new (ELeave)CTempControl();
	CleanupStack::PushL(control);
	CAknDoubleSpanScrollBar* sb = new (ELeave) CAknDoubleSpanScrollBar(control);
	CleanupStack::PushL(sb);
	sb->ConstructL(EFalse, NULL, control,  CEikScrollBar::EVertical, 0);
	iDefaultScrollBarBreadth = sb->ScrollBarBreadth();
	if(iDefaultScrollBarBreadth==0)
		iDefaultScrollBarBreadth = 8;
	CleanupStack::PopAndDestroy(2);

	TSize size = HtmlCtlUtils::ScreenSize(EOrientationPortrait);
	size.iWidth = iDefaultScrollBarBreadth;
	iBgBmp = CWritableBitmap::NewL(size, EColor16M);
	iBgBmpMask = CWritableBitmap::NewL(size, EGray256);
	iHandleBmp = CWritableBitmap::NewL(size, EColor16M);
	iHandleBmpMask = CWritableBitmap::NewL(size, EGray256);
#endif

#ifdef __UIQ__
	iDefaultScrollBarBreadth = CEikScrollBar::DefaultScrollBarBreadth();
#endif
	
	InitDrawer();
}

void CHcScrollbarDrawer::InitDrawer()
{
	if(iCustomDraw)
		return;

	iWidth = iDefaultScrollBarBreadth;
	iArrowHeight = 0;
	
#ifdef __SERIES60_3_ONWARDS__	
	CWritableBitmap* helper = CWritableBitmap::NewL(iBgBmp->SizeInPixels(), EColor16M);
	CWritableBitmap* helperMask = CWritableBitmap::NewL(iBgBmp->SizeInPixels(), EGray256);
	TRect rect;
	
	TSize sizeTop = TSize(iWidth, iWidth);
	DrawScrollSkinImage(KAknsIIDQsnCpScrollBgTop, helper, helperMask, sizeTop);
	rect = TRect(TPoint(0,0),TSize(iWidth, sizeTop.iHeight));
	iBgBmp->Gc().DrawBitmap(rect, helper, TRect(TPoint(0,0), sizeTop));
	iBgBmpMask->Gc().DrawBitmap(rect, helperMask, TRect(TPoint(0,0), sizeTop));
	
	TSize sizeMid = TSize(iWidth ,100);
	DrawScrollSkinImage(KAknsIIDQsnCpScrollBgMiddle, helper, helperMask, sizeMid);
	rect = TRect(TPoint(0,sizeTop.iHeight), TSize(iWidth, iBgBmp->SizeInPixels().iHeight));
	iBgBmp->Gc().DrawBitmap(rect, helper, TRect(TPoint(0,0), sizeMid));
	iBgBmpMask->Gc().DrawBitmap(rect, helperMask, TRect(TPoint(0,0), sizeMid));
	
	TSize sizeBot = TSize(iWidth, iWidth);
	DrawScrollSkinImage(KAknsIIDQsnCpScrollBgBottom, helper, helperMask, sizeBot);
	rect = TRect(TPoint(0, iBgBmp->SizeInPixels().iHeight-sizeBot.iHeight), TSize(iWidth, sizeBot.iHeight));
	iBgBmp->Gc().DrawBitmap(rect, helper, TRect(TPoint(0,0), sizeBot));
	iBgBmpMask->Gc().DrawBitmap(rect, helperMask, TRect(TPoint(0,0), sizeBot));
	
	sizeTop = TSize(iWidth, iWidth);
	DrawScrollSkinImage(KAknsIIDQsnCpScrollHandleTop, helper, helperMask, sizeTop);
	rect = TRect(TPoint(0,0), TSize(iWidth, sizeTop.iHeight));
	iHandleBmp->Gc().DrawBitmap(rect, helper, TRect(TPoint(0,0), sizeTop));
	iHandleBmpMask->Gc().DrawBitmap(rect, helperMask, TRect(TPoint(0,0), sizeTop));
	
	sizeMid = TSize(iWidth, 100);
	DrawScrollSkinImage(KAknsIIDQsnCpScrollHandleMiddle, helper, helperMask, sizeMid);
	rect = TRect(TPoint(0,sizeTop.iHeight), TSize(iWidth, iHandleBmp->SizeInPixels().iHeight));
	iHandleBmp->Gc().DrawBitmap(rect, helper, TRect(TPoint(0,0), sizeMid));
	iHandleBmpMask->Gc().DrawBitmap(rect, helperMask, TRect(TPoint(0,0), sizeMid));
	
	sizeBot = TSize(iWidth, iWidth);
	DrawScrollSkinImage(KAknsIIDQsnCpScrollHandleBottom, helper, helperMask, sizeBot);
	rect = TRect(TPoint(0, iHandleBmp->SizeInPixels().iHeight-sizeBot.iHeight), TSize(iWidth, sizeBot.iHeight));
	iHandleBmp->Gc().DrawBitmap(rect, helper, TRect(TPoint(0,0), sizeBot));
	iHandleBmpMask->Gc().DrawBitmap(rect, helperMask, TRect(TPoint(0,0), sizeBot));
	
	delete helper;
	delete helperMask;
#endif
	
#ifdef __UIQ__
	for(TInt i=0;i<4;i++)
		iSkinPatchs[i] = &SkinManager::SkinPatch(KSkinUidVScrollBar, i, NULL);
	iSkinBackground = &SkinManager::SkinPatch(KSkinUidScreen, 0, NULL);
	
	if(iWidth>10)
		iArrowHeight = iSkinPatchs[2]->SizeInPixels().iHeight;
	else
		iArrowHeight = 0;
#endif
}

void CHcScrollbarDrawer::HandleResourceChange()
{
	InitDrawer();
}

void CHcScrollbarDrawer::SetCustomImagesL(const TDesC& aDefinition)
{
	CHcImagePool& pool = CHtmlCtlEnv::Static()->ImagePool();
	for(TInt i=0;i<8;i++)
	{
		if(iComponents[i].iImage)
		{
			pool.Remove(iComponents[i].iImage);
			iComponents[i].iImage = NULL;
		}
		iComponents[i].iScale9Grid = TRect();
	}
	
	if(aDefinition.Length()==0)
	{
		iCustomDraw = EFalse;
		InitDrawer();
		return;
	}
	
	CHcStyleSheet* stylesheet = new (ELeave)CHcStyleSheet();
	CleanupStack::PushL(stylesheet);
	
	CHcStyle* style = new (ELeave)CHcStyle();
	CleanupStack::PushL(style);
	
	const TText* names[8] =	{
		_S("shaft"), _S("shaft_pressed"),
		_S("thumb"), _S("thumb_pressed"),
		_S("uparrow"), _S("uparrow_pressed"),
		_S("downarrow"), _S("downarrow_pressed")
	};
	
	HtmlParser::ParseStyleSheetL(aDefinition, *stylesheet);
	for(TInt i=0;i<8;i++)
	{
		if(stylesheet->GetStyle(KNullDesC, TPtrC(names[i]), 0, 0, *style) 
				&& style->IsSet(CHcStyle::EBackImage))
		{
			iComponents[i].iImage = style->BackgroundImage();
			iComponents[i].iImage->AddRef();
			if(style->IsSet(CHcStyle::EScale9Grid))
				iComponents[i].iScale9Grid = style->iScale9Grid;
		}
	}
	
	iWidth = iComponents[0].iImage->Size().iWidth;
	if(iComponents[4].iImage)
		iArrowHeight = iComponents[4].iImage->Size().iHeight;
	
	iCustomDraw = ETrue;
	
	CleanupStack::PopAndDestroy(2); //style, stylesheet
}

void CHcScrollbarDrawer::Draw(CFbsBitGc& aGc, const CHcScrollbar& aScrollbar) const
{
	if(iCustomDraw)
	{
		DrawCustom(aGc, aScrollbar);
		return;
	}
	
	aGc.SetBrushStyle(CGraphicsContext::ENullBrush);

#ifdef __SERIES60_3_ONWARDS__
	TRect rect;
	TPoint pt;
	
	rect = aScrollbar.iRect;
	rect.iTl.iX -= 1;
	rect.iBr.iX -= 1;
	rect.iBr.iY = rect.iTl.iY + aScrollbar.iRect.Height()/2;
	aGc.AlphaBlendBitmaps(rect.iTl, iBgBmp, TRect(TPoint(0,0), rect.Size()), iBgBmpMask, TPoint(0,0));
	
	rect.iTl.iY = rect.iBr.iY;
	rect.iBr.iY = aScrollbar.iRect.iBr.iY;
	pt = TPoint(0, iBgBmp->SizeInPixels().iHeight-rect.Height());
	aGc.AlphaBlendBitmaps(rect.iTl, iBgBmp, TRect(pt, rect.Size()), iBgBmpMask, pt);
	
	rect = aScrollbar.iThumbRect;
	rect.iTl.iX -= 1;
	rect.iBr.iX -= 1;
	rect.iBr.iY = rect.iTl.iY + aScrollbar.iThumbRect.Height()/2;
	aGc.AlphaBlendBitmaps(rect.iTl, iHandleBmp, TRect(TPoint(0,0), rect.Size()), iHandleBmpMask, TPoint(0,0));
	
	rect.iTl.iY = rect.iBr.iY;
	rect.iBr.iY = aScrollbar.iThumbRect.iBr.iY;
	pt = TPoint(0, iHandleBmp->SizeInPixels().iHeight-rect.Height());
	aGc.AlphaBlendBitmaps(rect.iTl, iHandleBmp, TRect(pt, rect.Size()), iHandleBmpMask, pt);
	
#endif
		
#ifdef __SERIES60_3_DOWNWARDS__
	aGc.SetPenStyle(CBitmapContext::ESolidPen);
	aGc.SetPenSize(TSize(1,1));
	TRect rect(aScrollbar.iRect);
			
	aGc.SetPenColor(KRgbDarkGray);
	aGc.DrawLine(TPoint(rect.iTl.iX+2, rect.iTl.iY), TPoint(rect.iTl.iX+2, rect.iBr.iY));
	aGc.DrawLine(TPoint(rect.iTl.iX+3, rect.iTl.iY), TPoint(rect.iTl.iX+3, rect.iBr.iY));
	
	aGc.SetPenColor(TRgb(0xD0D3D4));
	aGc.DrawLine(TPoint(rect.iTl.iX, aScrollbar.iThumbRect.iTl.iY+1), TPoint(rect.iTl.iX, aScrollbar.iThumbRect.iBr.iY-1));
	
	aGc.SetPenColor(TRgb(0xD0D3D4));
	aGc.DrawLine(TPoint(rect.iTl.iX+1, aScrollbar.iThumbRect.iTl.iY+1), TPoint(rect.iTl.iX+1, aScrollbar.iThumbRect.iBr.iY-1));

	aGc.SetPenColor(TRgb(0xD0D3D4));
	aGc.DrawLine(TPoint(rect.iTl.iX+2, aScrollbar.iThumbRect.iTl.iY), TPoint(rect.iTl.iX+2, aScrollbar.iThumbRect.iBr.iY));

	aGc.SetPenColor(TRgb(0xA5A7AD));
	aGc.DrawLine(TPoint(rect.iTl.iX+3, aScrollbar.iThumbRect.iTl.iY), TPoint(rect.iTl.iX+3, aScrollbar.iThumbRect.iBr.iY));

	aGc.SetPenColor(TRgb(0xA5A7AD));
	aGc.DrawLine(TPoint(rect.iTl.iX+4,  aScrollbar.iThumbRect.iTl.iY+1), TPoint(rect.iTl.iX+4, aScrollbar.iThumbRect.iBr.iY-1));	
#endif
		
#ifdef __UIQ__
	TRect screenRect(TRect(TPoint(0,0),iSkinBackground->SizeInPixels()));
	screenRect.Move(-aScrollbar.iDiv->Owner()->PositionRelativeToScreen());
	iSkinBackground->DrawBitmap(aGc, screenRect, aScrollbar.iRect, 0);

	iSkinPatchs[0]->DrawBitmap(aGc, aScrollbar.iRect, aScrollbar.iRect, 
			aScrollbar.iState.IsSet(CHcScrollbar::EShaftPressed)?ESkinVScrollBarShaftPressed:ESkinVScrollBarShaft);
	iSkinPatchs[1]->DrawBitmap(aGc, aScrollbar.iThumbRect, aScrollbar.iThumbRect,
			aScrollbar.iState.IsSet(CHcScrollbar::EThumbPressed)?ESkinVScrollBarThumbPressed:ESkinVScrollBarThumb);
	
	TSize gripSize = iSkinPatchs[3]->SizeInPixels();
	if(aScrollbar.iThumbRect.Height()>gripSize.iHeight+2)
	{
		TRect gripRect;
		gripRect.iTl.iY = aScrollbar.iThumbRect.iTl.iY + (aScrollbar.iThumbRect.Height() - gripSize.iHeight)/2;
		gripRect.iBr.iY = gripRect.iTl.iY + gripSize.iHeight;
		
		gripRect.iTl.iX = aScrollbar.iThumbRect.iTl.iX + (aScrollbar.iThumbRect.Width() - gripSize.iWidth)/2;
		gripRect.iBr.iX = gripRect.iTl.iX + gripSize.iWidth;
		
		iSkinPatchs[3]->DrawBitmap(aGc, gripRect, gripRect, 0);
	}
	
	if(iArrowHeight>0)
	{
		iSkinPatchs[2]->DrawBitmap(aGc, aScrollbar.iUpArrowRect, 
				aScrollbar.iUpArrowRect, aScrollbar.iState.IsSet(CHcScrollbar::EUpArrowPressed)?ESkinVScrollBarArrowPressedNudgeUp:ESkinVScrollBarArrowNudgeUp);
		iSkinPatchs[2]->DrawBitmap(aGc, aScrollbar.iDownArrowRect,
				aScrollbar.iDownArrowRect, aScrollbar.iState.IsSet(CHcScrollbar::EDownArrowPressed)?ESkinVScrollBarArrowPressedNudgeDown:ESkinVScrollBarArrowNudgeDown);
	}
#endif
}

void CHcScrollbarDrawer::DrawCustom(CFbsBitGc& aGc, const CHcScrollbar& aScrollbar) const
{
	//0-shaft, 1-shaft-pressed, 2-thumb, 3-thumb-pressed
	//4-up-arrow,5-up-arrow-pressed,6-down-arrow,7-down-arrow-pressed
	
	THcDrawImageParams params;
	if(!aScrollbar.iState.IsSet(CHcScrollbar::EShaftPressed) || !iComponents[1].iImage)
	{
		params.iScale9Grid = iComponents[0].iScale9Grid;
		iComponents[0].iImage->Draw(aGc, aScrollbar.iRect, params);
	}
	else
	{
		params.iScale9Grid = iComponents[1].iScale9Grid;
		iComponents[1].iImage->Draw(aGc, aScrollbar.iRect, params);
	}
	
	if(!aScrollbar.iState.IsSet(CHcScrollbar::EThumbPressed) || !iComponents[3].iImage)
	{
		params.iScale9Grid = iComponents[2].iScale9Grid;
		iComponents[2].iImage->Draw(aGc, aScrollbar.iThumbRect, params);
	}
	else
	{
		params.iScale9Grid = iComponents[3].iScale9Grid;
		iComponents[3].iImage->Draw(aGc, aScrollbar.iThumbRect, params);
	}
	
	if(iArrowHeight>0)
	{
		if(!aScrollbar.iState.IsSet(CHcScrollbar::EUpArrowPressed) || !iComponents[5].iImage)
		{
			params.iScale9Grid = iComponents[4].iScale9Grid;
			iComponents[4].iImage->Draw(aGc, aScrollbar.iUpArrowRect, params);
		}
		else
		{
			params.iScale9Grid = iComponents[5].iScale9Grid;
			iComponents[5].iImage->Draw(aGc, aScrollbar.iUpArrowRect, params);
		}
		
		if(!aScrollbar.iState.IsSet(CHcScrollbar::EDownArrowPressed) || !iComponents[7].iImage)
		{
			params.iScale9Grid = iComponents[6].iScale9Grid;
			iComponents[6].iImage->Draw(aGc, aScrollbar.iDownArrowRect, params);
		}
		else
		{
			params.iScale9Grid = iComponents[7].iScale9Grid;
			iComponents[7].iImage->Draw(aGc, aScrollbar.iDownArrowRect, params);
		}
	}
}


