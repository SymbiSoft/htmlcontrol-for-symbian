#include "controlimpl.h"
#include "htmlparser.h"
#include "htmlelementiter.h"
#include "measurestatus.h"
#include "utils.h"

#include "element_a.h"
#include "element_text.h"
#include "element_div.h"

CHtmlElementA::CHtmlElementA(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeA;
	iTagName.Set(KHStrA);
	iStyle.SetTag(iTagName);
}

CHtmlElementA::~CHtmlElementA()
{
	delete iHref;
	delete iTarget;
	iStyle.Close();
}

TBool CHtmlElementA::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.CompareF(KHStrHref)==0) 
	{
		if(iHref)
			aBuffer.Copy(iHref->Left(aBuffer.MaxLength()));
		else
			aBuffer.Zero();
	}
	else if(aName.CompareF(KHStrTarget)==0) 
	{
		if(iTarget)
			aBuffer.Copy(iTarget->Left(aBuffer.MaxLength()));
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

TBool CHtmlElementA::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrHref)==0) 
	{
		delete iHref;
		iHref = aValue.AllocL();
	}
	else if(aName.CompareF(KHStrTarget)==0) 
	{
		delete iTarget;
		iTarget = aValue.AllocL();
	}
	else if(aName.CompareF(KHStrInnerText)==0)
	{
		ClearContent();
		
		if(aValue.Length()>0)
		{
			CHtmlElementText* sub = new (ELeave)CHtmlElementText(iOwner);
			sub->iParent = iParent;
			CleanupStack::PushL(sub);
			sub->PrepareL();
			sub->SetTextL(aValue);
			iOwner->Impl()->InsertContent(sub, sub, this, EAfterBegin);
			CleanupStack::Pop();//sub
		}
	}
	else if(aName.CompareF(KHStrInnerHtml)==0)
	{
		ClearContent();
		iOwner->InsertContentL(this, EAfterBegin, aValue);
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

CHtmlElementImpl* CHtmlElementA::CloneL()  const
{
	CHtmlElementA* e = new (ELeave)CHtmlElementA(iOwner);
	BaseClone(*e);
	
	if(iHref)
		e->iHref = (*iHref).AllocL();
	if(iTarget)
		e->iTarget = (*iTarget).AllocL();
	e->iStyle.CopyL(iStyle);
	return e;
}

void CHtmlElementA::ClearContent()
{
	if(iNext!=iEnd)
		iOwner->Impl()->FreeElementList(iNext, iEnd->iPrev);
}

void CHtmlElementA::Measure(CHcMeasureStatus& aStatus)
{
	if(!CanFocus())
		iStyle.SetFakeClass(0);
	else if(iState.IsSet(EElementStateFocused))
		iStyle.SetFakeClass(THcSelector::EFCHover);
	else
		iStyle.SetFakeClass(THcSelector::EFCLink);
	if(iStyle.Update(iOwner->Impl()))
	{
		if(iStyle.Style().IsSet(CHcStyle::EBackColor))
		{
			iStyle.Style().iTextStyle.Set(THcTextStyle::EBackColor);
			iStyle.Style().iTextStyle.iBackColor = iStyle.Style().iBackColor;
		}
		else
			iStyle.Style().iTextStyle.Clear(THcTextStyle::EBackColor);
		
		if(!iStyle.Style().iBorders.IsEmpty())
		{
			iStyle.Style().iTextStyle.Set(THcTextStyle::EBorder);
			iStyle.Style().iTextStyle.iBorder = iStyle.Style().iBorders.iTop;
		}
		else
			iStyle.Style().iTextStyle.Clear(THcTextStyle::EBorder);
	}
	
	if(iStyle.Style().IsDisplayNone())
	{
		aStatus.SkipElements(iEnd);
		return;
	}
	
	if(iState.IsSet(EElementStateFocused))
		aStatus.iInLink++;

	if(iStyle.Style().IsHidden())
		iState.Set(EElementStateHidden);
	
	if(iState.IsSet(EElementStateHidden))
		aStatus.iHidden++;

	iPosition = aStatus.iPosition;
	aStatus.PushTextStyleL(iStyle.Style().iTextStyle);
}

void CHtmlElementA::Draw(CFbsBitGc& ) const
{
}

TKeyResponse CHtmlElementA::OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType)
{
	if(aType==EEventKey && HcUtils::TranslateKey(aKeyEvent.iCode) == EKeyEnter)
	{
		HandleButtonEventL(EButtonEventClick);
		return EKeyWasConsumed;
	}
	else
		return EKeyWasNotConsumed;
}

void CHtmlElementA::HandleButtonEventL(TInt aButtonEvent)
{
	if(aButtonEvent==EButtonEventClick || aButtonEvent==EButtonEventSelect)
	{
		if(iHref && iHref->Length()>1 && (*iHref)[0]=='#')
		{
			THtmlElementIter it(iOwner->Body(), iHref->Mid(1), KHStrA);
			if(it.Next())
			{
				iOwner->ScrollToView(it());
				if(((CHtmlElementImpl*)it())->CanFocus())
					iOwner->SetFocusTo(it());
				
				return;
			}
		}

		THtmlCtlEvent event;
		event.iType = THtmlCtlEvent::EOnClick;
		event.iControl = iOwner;
		event.iElement = this;
		if(aButtonEvent==EButtonEventClick)
			event.iData = 1;
		else
			event.iData = 0;
		iOwner->Impl()->FireEventL(event);
	}
}

_LIT(KHStrAEnd, "a-end");
CHtmlElementAEnd::CHtmlElementAEnd(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeAEnd;
	iTagName.Set(KHStrAEnd);
}

CHtmlElementImpl* CHtmlElementAEnd::CloneL() const
{
	CHtmlElementAEnd* e = new (ELeave)CHtmlElementAEnd(iOwner);
	BaseClone(*e);
	return e;
}

void CHtmlElementAEnd::Measure(CHcMeasureStatus& aStatus) 
{
	if(iStart->iState.IsSet(EElementStateFocused))
		aStatus.iInLink--;
	
	if(iStart->iState.IsSet(EElementStateHidden))
		aStatus.iHidden--;
	
	aStatus.PopTextStyle();
}

void CHtmlElementAEnd::Draw(CFbsBitGc& )  const
{
}


