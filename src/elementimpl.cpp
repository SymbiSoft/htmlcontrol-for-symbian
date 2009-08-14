#include <eikenv.h>
#include <babitflags.h>

#include "controlimpl.h"
#include "elementimpl.h"
#include "htmlparser.h"
#include "measurestatus.h"
#include "utils.h"

#include "element_div.h"

CHtmlElementImpl::CHtmlElementImpl(CHtmlControl* aOwner):CHtmlElement(aOwner)
{
	iLineVAlign = EVCenter;
	iFlags.Set(ETabStop);
}

TBool CHtmlElementImpl::CanFocus() const 
{
	return EFalse;
}

void CHtmlElementImpl::SetFocus(TBool aFocus) 
{	
	if(aFocus)
	{
		if(!iState.IsSet(EElementStateFocused))
		{
			iState.Set(EElementStateFocused);
			
			THtmlCtlEvent event;
			event.iType = THtmlCtlEvent::EOnGainFocus;
			event.iControl = iOwner;
			event.iElement = this;
			iOwner->Impl()->FireEventL(event);
		}
	}
	else
	{
		if(iState.IsSet(EElementStateFocused))
		{
			iState.Clear(EElementStateFocused);
			
			THtmlCtlEvent event;
			event.iType = THtmlCtlEvent::EOnLostFocus;
			event.iControl = iOwner;
			event.iElement = this;
			iOwner->Impl()->FireEventL(event);
		}
	}
}

void CHtmlElementImpl::PrepareL()
{
	iState.Set(EElementStateRuntime);
}

TKeyResponse CHtmlElementImpl::OfferKeyEventL (const TKeyEvent &, TEventCode)
{
	return EKeyWasNotConsumed;
}

void CHtmlElementImpl::HandleButtonEventL(TInt)
{

}

void CHtmlElementImpl::BaseClone(CHtmlElementImpl& aTarget) const
{
	aTarget.iId.Copy(iId);
	aTarget.iLineVAlign = iLineVAlign;
	aTarget.iFlags = iFlags;
	aTarget.iAccessKey = iAccessKey;
}

TBool CHtmlElementImpl::GetProperty(const TDesC& aName , TDes& aBuffer)  const
{
	if(aName.CompareF(KHStrRemark)==0) 
	{
		if(iRemark) 
			aBuffer.Copy(*iRemark);
		else
			aBuffer.Zero();
		
		return ETrue;
	}
	
	return EFalse;
}

TBool CHtmlElementImpl::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(aName.Length()>0 && aName[0]=='*')
	{
		TInt resId = HcUtils::StrToInt(aValue, EDecimal);
#ifdef __SYMBIAN_9_ONWARDS__
		if(CEikonEnv::Static()->IsResourceAvailableL(resId)) 
#else
		if(ETrue)
#endif
		{
			HBufC* data = CEikonEnv::Static()->AllocReadResourceLC(resId);
			TBool ret = SetProperty(aName.Mid(1), *data);
			CleanupStack::PopAndDestroy();	
			return ret;		
		}
		else
			return SetProperty(aName.Mid(1), KNullDesC);
	}
	else if(aName.CompareF(KHStrId)==0 || aName.CompareF(KHStrName)==0) 
	{
		iId.Copy(aValue.Left(iId.MaxLength()));
	}
	else if(aName.CompareF(KHStrRemark)==0) 
	{
		if(iRemark) 
		{
			delete iRemark;
			iRemark = NULL;
		}
		
		iRemark = aValue.AllocL();
	}
	else if(aName.CompareF(KHStrLineVAlign)==0)
	{
		HtmlParser::ParseVAlign(aValue, iLineVAlign);
	}
	else if(aName.CompareF(KHStrTabStop)==0)
	{
		iFlags.Assign(ETabStop, HcUtils::StrToBool(aValue));
	}
	else if(aName.CompareF(KHStrAccessKey)==0)
	{
		if(aValue.Length()>0)
			iAccessKey = aValue[0];
		else
			iAccessKey = '\0'; 
	}
	else
		return EFalse;
	
	return ETrue;
}

void CHtmlElementImpl::InvokeL(TRefByValue< const TDesC16 > aCommand, ...)
{
	const TDesC& cmd = (const TDesC&)aCommand;
	if(cmd.CompareF(KHStrClick)==0)
		HandleButtonEventL(EButtonEventClick);
}

void CHtmlElementImpl::DoMeasure(CHcMeasureStatus& aStatus, const CHcStyle& aStyle, const TSize& aDefaultSize, TBool aForceSize)
{
	const TRect& parentRect = iParent->iDisplayRect;
	TAlign align = ELeft;
	TVAlign valign = EVTop;
	
	if(aStyle.IsClearLeft() || aStatus.iPosition.iX>=parentRect.iBr.iX)
		aStatus.NewLine();
	
	TRect displayRect = TRect(aStatus.iPosition, parentRect.iBr);
	if(aStyle.IsSet(CHcStyle::ELeft))
	{
		TInt left = aStyle.iLeft.GetRealValue(parentRect.Width(), 0);
		displayRect.iTl.iX = parentRect.iTl.iX +  left;
		iState.Set(EElementStateFixedX);
		iState.Set(EElementStateFixedY);
	}
	else if(aStyle.IsSet(CHcStyle::EAlign))
	{
		align = aStyle.iAlign;
		displayRect.iTl.iX = parentRect.iTl.iX;
		iState.Set(EElementStateFixedX);
	}
	else if(aStyle.iMargins.GetAlign(align))
	{
		displayRect.iTl.iX = parentRect.iTl.iX;
		iState.Set(EElementStateFixedX);
	}
	
	if(aStyle.IsSet(CHcStyle::ETop))
	{
		TInt top = aStyle.iTop.GetRealValue(parentRect.Height(), 0);
		displayRect.iTl.iY = parentRect.iTl.iY + top;
		iState.Set(EElementStateFixedX);
		iState.Set(EElementStateFixedY);
	}
	else if(aStyle.iMargins.GetVAlign(valign))
	{
		displayRect.iTl.iY = parentRect.iTl.iY;
		iState.Set(EElementStateFixedY);
	}
	
	TMargins margins = aStyle.iMargins.GetMargins(parentRect.Size());
	displayRect.iTl += TPoint(margins.iLeft, margins.iTop);
	if(displayRect.iTl.iX>=parentRect.iBr.iX)
		aStatus.NewLine();
	if(displayRect.iBr.iY<displayRect.iTl.iY)
		displayRect.iBr.iY = displayRect.iTl.iY;
	
	TInt width = 0, height = 0;
	if(!aForceSize)
	{
		if(aStyle.IsSet(CHcStyle::EWidth))
			width = aStyle.iWidth.GetRealValue(parentRect.Width(), displayRect.Width());
		else
			width = aDefaultSize.iWidth;
	
		if(aStyle.IsSet(CHcStyle::EHeight))
			height = aStyle.iHeight.GetRealValue(parentRect.Height(), displayRect.Height());
		else
			height = aDefaultSize.iHeight;
	}
	else
	{
		width = aDefaultSize.iWidth;
		height = aDefaultSize.iHeight;
	}
	
	if(displayRect.iTl.iX + width > parentRect.iBr.iX
		&& aStatus.iPosition.iX!=parentRect.iTl.iX
		&& !aStyle.IsSet(CHcStyle::ELeft)
		&& !aStyle.IsSet(CHcStyle::ETop))
	{
		aStatus.NewLine();
		displayRect = TRect(aStatus.iPosition, displayRect.Size());
		displayRect.iTl += TPoint(margins.iLeft, margins.iTop);
	}
	
	if(aStyle.IsSet(CHcStyle::EMaxWidth))
	{
		TInt maxWidth = aStyle.iMaxWidth.GetRealValue(parentRect.Width(), width);
		if(maxWidth<width)
		{
			height = TInt( (TReal)maxWidth/width * height);
			width = maxWidth;
		}
	}
	if(margins.iRight>0 && displayRect.iTl.iX + width >= parentRect.iBr.iX)
		width = parentRect.iBr.iX -  displayRect.iTl.iX - margins.iRight;

	iSize = TSize(width, height);
	iPosition = displayRect.iTl;

	if(align==ECenter)
		iPosition.iX += NO_BELOW_ZERO_MINUS(parentRect.Width(),width)/2;
	else if(align==ERight)
		iPosition.iX += parentRect.Width() - width - margins.iRight;

	if(valign==EVCenter)
		iPosition.iY += NO_BELOW_ZERO_MINUS(parentRect.Height(),height)/2;
	else if(valign==EVBottom)
		iPosition.iY +=  parentRect.Height() - height - margins.iBottom;

	iLineNumber = aStatus.LineNumber();
	iLineHeight = iPosition.iY + height - aStatus.iPosition.iY + margins.iBottom;

	if(!aStyle.IsSet(CHcStyle::ELeft))
		aStatus.iPosition.iX = iPosition.iX + width + margins.iRight;
	
	if(!aStyle.IsSet(CHcStyle::ETop))
		aStatus.CurrentLineInfo().SetHeightIfGreater(iLineHeight);

	if(aStyle.IsClearRight())
		aStatus.NewLine();

	if(aStyle.IsHidden())
		iState.Set(EElementStateHidden);
}

void CHtmlElementImpl::Layout()
{
	const THcLineInfo& lineInfo = iOwner->Impl()->LineInfo(iLineNumber);
	if(!iState.IsSet(EElementStateFixedX))
		iPosition.iX += lineInfo.iXOffset;
	if(!iState.IsSet(EElementStateFixedY))
	{
		if(iLineVAlign==EVCenter)
			iPosition.iY += (lineInfo.iHeight - iLineHeight )/2;
		else if(iLineVAlign==EVBottom)
			iPosition.iY += lineInfo.iHeight - iLineHeight;
	}
}

void CHtmlElementImpl::Refresh()
{
	
}
