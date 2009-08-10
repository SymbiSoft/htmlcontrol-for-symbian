#include "controlimpl.h"
#include "htmlparser.h"
#include "measurestatus.h"
#include "list.h"
#include "scrollbar.h"
#include "utils.h"

#include "element_div.h"
#include "element_text.h"
#include "element_a.h"

CHtmlElementDiv::CHtmlElementDiv(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeDiv;
	iTagName.Set(KHStrDiv);
}

CHtmlElementDiv::~CHtmlElementDiv() 
{
	iStyle.Close();;
	iFocusStyle.Close();
	iDownStyle.Close();
	delete iScrollbar;
	delete iList;
}

void CHtmlElementDiv::ClearContent()
{
	if(iNext!=iEnd)
		iOwner->Impl()->FreeElementList(iNext, iEnd->iPrev);
	if(iScrollbar)
		iScrollbar->SetTopPos();
	if(iList)
	{
		delete iList;
		iList = NULL;
	}
}

_LIT(KListConnector, "list-");
_LIT(KScrollConnector, "scroll-");
TBool CHtmlElementDiv::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.Left(5).CompareF(KListConnector)==0)
	{
		if(iList)
			return iList->GetProperty(aName, aBuffer);
		else if(aName.CompareF(KHStrListSelectedIndex)==0)
		{
			if(iFocusedElement)
				aBuffer.Num(iFocusedElement->Index(this));
			else
				aBuffer.Num(-1);
		}
	}
	else if(aName.Left(7).CompareF(KScrollConnector)==0)
	{
		if(iScrollbar)
			return iScrollbar->GetProperty(aName, aBuffer);	
	}
	else if(aName.CompareF(KHStrClass)==0) 
	{
		aBuffer.Copy(iStyle.Class().Left(aBuffer.MaxLength()));
	}
	else if(aName.CompareF(KHStrBufSize)==0)
	{
		TInt length = 0;
		CHtmlElementImpl* e = iNext;
		while(e!=iEnd)
		{
			if(e->TypeId()==EElementTypeText)
				length += ((CHtmlElementText*)e)->GetTextLengthL();
			e = e->iNext;
		}
		aBuffer.Num(length);
	}
	else if(aName.CompareF(KHStrInnerText)==0) 
	{
		CHtmlElementImpl* e = iNext;
		while(e!=iEnd)
		{
			if(e->TypeId()==EElementTypeText)
				((CHtmlElementText*)e)->GetTextL(aBuffer);
			e = e->iNext;
		}
	}
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.GetSingleStyle(aName.Mid(6), aBuffer);
	else
		return EFalse;
	return ETrue;
}

TBool CHtmlElementDiv::SetProperty(const TDesC& aName, const TDesC& aValue)
{	
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrFocusing)==0)
		iFlags.Assign(EFocusing, HcUtils::StrToInt(aValue));
	else if(aName.CompareF(KHStrAlign)==0) 
		iStyle.SetSingleStyleL(KHStrAlign, aValue);
	else if(aName.Compare(KHStrLineWrap)==0)
	{
		if(aValue.CompareF(KHStrAuto)==0)
			iLineWrapMode = ELWAuto;
		else if(aValue.CompareF(KHStrClip)==0)
			iLineWrapMode = ELWClip;
		else if(aValue.CompareF(KHStrScroll)==0)
			iLineWrapMode = ELWScroll;
	}
	else if(aName.CompareF(KHStrDisabled)==0) 
	{
		iFlags.Assign(EDisabled, HcUtils::StrToBool(aValue));
	}
	else if(aName.CompareF(KHStrInnerText)==0)
	{
		ClearContent();
		
		if(aValue.Length()>0)
		{
			CHtmlElementText* sub = new (ELeave)CHtmlElementText(iOwner);
			sub->iParent = this;
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
		if(aValue.Length()>0)
			iOwner->InsertContentL(this, EAfterBegin, aValue);
	}
	else if(aName.CompareF(KHStrClass)==0) 
		iStyle.SetClass(aValue);
	else if(aName.CompareF(KHStrStyle)==0) 
		iStyle.SetStyleL(aValue);
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.SetSingleStyleL(aName.Mid(6), aValue);
	else if(aName.CompareF(KHStrFocusClass)==0) 
		iFocusStyle.SetClass(aValue);
	else if(aName.CompareF(KHStrFocusStyle)==0)
		iFocusStyle.SetStyleL(aValue);
	else if(aName.CompareF(KHStrDownClass)==0) 
		iDownStyle.SetClass(aValue);
	else if(aName.CompareF(KHStrDownStyle)==0)
		iDownStyle.SetStyleL(aValue);
	else if(aName.Left(5).CompareF(KListConnector)==0)
	{
		if(iList)
			return iList->SetProperty(aName, aValue);	
	}
	else if(aName.Left(7).CompareF(KScrollConnector)==0)
	{
		if(!iState.IsSet(EElementStateRuntime) || iScrollbar)
		{
			if(!iScrollbar)
				iScrollbar = new (ELeave)CHcScrollbar(this);
			return iScrollbar->SetProperty(aName, aValue);	
		}
	}
	else
		return EFalse;

	return ETrue;
}

CHtmlElementImpl* CHtmlElementDiv::CloneL() const 
{
	CHtmlElementDiv* e = new (ELeave)CHtmlElementDiv(iOwner);
	BaseClone(*e);
	
	e->iStyle.CopyL(iStyle);
	e->iFocusStyle.CopyL(iFocusStyle);
	e->iLineWrapMode = iLineWrapMode;

	if(iScrollbar) 
	{
		e->iScrollbar = new (ELeave)CHcScrollbar(e);
		e->iScrollbar->iStep = iScrollbar->iStep;
		e->iScrollbar->iFlags = iScrollbar->iFlags;
	}

	return e;
}

void CHtmlElementDiv::InvokeL(TRefByValue< const TDesC16 > aCommand, ...) 
{
	VA_LIST arg_list;
	VA_START(arg_list, aCommand);
	
	const TDesC& cmd = (const TDesC&)aCommand;
	if(cmd.CompareF(KHStrUpdate)==0) 
	{
		if(iList)
			iList->UpdateL(VA_ARG(arg_list,TInt));
	}
	else if(cmd.CompareF(KHStrUpdateAll)==0)
	{
		if(iList)
			iList->UpdateL(-1);
	}
	else if(cmd.CompareF(KHStrCreateList)==0
			|| cmd.CompareF(KHStrCreateGrid)==0
			|| cmd.CompareF(KHStrCreateSlides)==0)
	{
		ClearContent();
		
		THcListLayout layout;
		if(cmd.CompareF(KHStrCreateList)==0)
			layout = ELayoutList;
		else if(cmd.CompareF(KHStrCreateGrid)==0)
			layout = ELayoutGrid;
		else
			layout = ELayoutSlides;
		iList = CHcList::NewL(this, layout);
		iList->SetRenderer(VA_ARG(arg_list,MListItemRenderer*));
		if(!iScrollbar)
			iScrollbar = new (ELeave)CHcScrollbar(this);
		iScrollbar->iFlags.Set(CHcScrollbar::ELoop);
	}
	else if(cmd.CompareF(KHStrClick)==0)
		HandleButtonEventL(EButtonEventClick);

    VA_END(arg_list);
}

void CHtmlElementDiv::PrepareL()
{
	CHtmlElementImpl::PrepareL();
	
	iStyle.SetTag(iTagName);
	iStyle.Update(iOwner->Impl());
	UpdateScrollbarsL();
}

void CHtmlElementDiv::UpdateScrollbarsL()
{
	TBool scrollbar = iStyle.Style().IsSet(CHcStyle::EOverflow)
			&& (iStyle.Style().iOverflow==EOverflowScroll || iStyle.Style().iOverflow==EOverflowAuto);
	if(scrollbar || iFlags.IsSet(EBody) || iList)
	{
		if(!iScrollbar)
			iScrollbar = new (ELeave)CHcScrollbar(this);
		iScrollbar->SetDimmed(!scrollbar);
		iScrollbar->SetAlwaysVisible(iStyle.Style().iOverflow==EOverflowScroll);
	}
	else if(iScrollbar)
	{
		delete iScrollbar;
		iScrollbar = NULL;
	}
}

void CHtmlElementDiv::Measure(CHcMeasureStatus& aStatus)
{
	iStyle.Update(iOwner->Impl());
	const CHcStyle& style = iStyle.Style();

	if(style.IsDisplayNone())
	{
		aStatus.SkipElements(iEnd);
		return;
	}
	
	if(!iFlags.IsSet(EBody))
	{
		TAlign align = ELeft;
		TVAlign valign = EVTop;
		
		if(style.IsClearLeft()
				|| aStatus.iPosition.iX>=iParent->iDisplayRect.iBr.iX)
			aStatus.NewLine();
		
		TRect displayRect = TRect(aStatus.iPosition, iParent->iDisplayRect.iBr);
		
		if(style.IsSet(CHcStyle::ELeft))
		{
			TInt left = style.iLeft.GetRealValue(iParent->iDisplayRect.Width(), 0);
			displayRect.iTl.iX = iParent->iDisplayRect.iTl.iX +  left;
			iState.Set(EElementStateFixedX);
			iState.Set(EElementStateFixedY);
		}
		else if(style.iMargins.GetAlign(align))
		{
			displayRect.iTl.iX = iParent->iDisplayRect.iTl.iX;
			iState.Set(EElementStateFixedX);
		}
		
		if(style.IsSet(CHcStyle::ETop))
		{
			TInt top = style.iTop.GetRealValue(iParent->iDisplayRect.Height(), 0);
			displayRect.iTl.iY = iParent->iDisplayRect.iTl.iY + top;
			iState.Set(EElementStateFixedX);
			iState.Set(EElementStateFixedY);
		}
		else if(style.iMargins.GetVAlign(valign))
		{
			displayRect.iTl.iY = iParent->iDisplayRect.iTl.iY;
			iState.Set(EElementStateFixedY);
		}
		
		iMargins = style.iMargins.GetMargins(iParent->iDisplayRect.Size());
		
		displayRect.iTl += TPoint(iMargins.iLeft, iMargins.iTop);
		if(displayRect.iBr.iY<displayRect.iTl.iY)
			displayRect.iBr.iY = displayRect.iTl.iY;
		
		TInt width, height;
		if(style.IsSet(CHcStyle::EWidth))
			width = style.iWidth.GetRealValue(iParent->iDisplayRect.Width(), displayRect.Width());
		else
			width = displayRect.Width();
		if(style.IsSet(CHcStyle::EHeight))
			height = style.iHeight.GetRealValue(iParent->iDisplayRect.Height(), displayRect.Height());
		else
			height = 100000000;
		
		if(displayRect.iTl.iX + width > iParent->iDisplayRect.iBr.iX
			&& aStatus.iPosition.iX!=iParent->iDisplayRect.iTl.iX
			&& !style.IsSet(CHcStyle::ELeft)
			&& !style.IsSet(CHcStyle::ETop))
		{
			aStatus.NewLine();
			displayRect = TRect(aStatus.iPosition, displayRect.Size());
			displayRect.iTl += TPoint(iMargins.iLeft, iMargins.iTop);
		}
		
		if(style.IsSet(CHcStyle::EMaxWidth))
		{
			TInt maxWidth = style.iMaxWidth.GetRealValue(iParent->iDisplayRect.Width(), width);
			if(maxWidth<width)
			{
				height = TInt( (TReal)maxWidth/width * height);
				width = maxWidth;
			}
		}
		iSize = TSize(width, height);
		iPosition = displayRect.iTl;
		iLineNumber = aStatus.LineNumber();

		if(style.IsHidden())
			iState.Set(EElementStateHidden);
		if(iState.IsSet(EElementStateHidden))
			aStatus.iHidden++;
		iState.Assign(EElementStateFaded, style.IsSet(CHcStyle::EFaded) && style.iFaded || iParent->iState.IsSet(EElementStateFaded));
		iState.Assign(EElementStateInFocus, iFlags.IsSet(EFocusing) && iState.IsSet(EElementStateFocused) || iParent->iState.IsSet(EElementStateInFocus));
	}
	else
	{
		iPosition = TPoint(0,0);
		iSize = iOwner->Size();
		iLineNumber = aStatus.LineNumber();
		
		iState.Assign(EElementStateFaded, style.IsSet(CHcStyle::EFaded) && style.iFaded);
		iState.Assign(EElementStateInFocus, iFlags.IsSet(EFocusing) && iState.IsSet(EElementStateFocused));
		
		UpdateScrollbarsL();
	}
	
	iDisplayRect = Rect();
	style.iBorders.ShrinkRect(iDisplayRect);
	
	if(iScrollbar && !iScrollbar->IsDisplayNone() && !iScrollbar->IsDimmed())
		iDisplayRect.iBr.iX = iDisplayRect.iBr.iX - iScrollbar->Width();

	iContentSize.iWidth = 0;
	iContentSize.iHeight = 0;

	iPaddings = style.iPaddings.GetMargins(iDisplayRect.Size());
	HcUtils::ShrinkRect(iDisplayRect, iPaddings);
	TAlign align = ELeft;
	if(style.iPaddings.GetAlign(align))
		iContentSize.iWidth = iDisplayRect.Width();
	
	aStatus.PushLineAlign(align);
	if(iList)
		iList->Measure(aStatus);
	
	THcTextStyle textStyle = style.iTextStyle;
	if(iState.IsSet(EElementStateInFocus)) 
	{
		iFocusStyle.Update(iOwner->Impl());
		if(!iFocusStyle.IsNull())
			textStyle.Add(iFocusStyle.Style().iTextStyle);
	}
	
	if(iState.IsSet(EElementStateDown))
	{
		iDownStyle.Update(iOwner->Impl());
		if(!iDownStyle.IsNull())
			textStyle.Add(iDownStyle.Style().iTextStyle);
	}

	aStatus.PushTextStyleL(textStyle);
	iClippingRect.iTl = aStatus.iPosition; //We use this the save orig position!
	aStatus.iLines->Append(THcLineInfo());
	aStatus.iLineNumber = aStatus.iLines->Count()-1;
	aStatus.iPosition = iDisplayRect.iTl;
}

void CHtmlElementDiv::EndMeasure(CHcMeasureStatus& aStatus)
{
	aStatus.NewLine();
	const CHcStyle& style = iStyle.Style();
	
	if(iList)
		iContentSize.iHeight = iList->Height();
	else
		iContentSize.iHeight = aStatus.iPosition.iY - iDisplayRect.iTl.iY;

	TInt deltaY = iDisplayRect.iTl.iY + iContentSize.iHeight - iDisplayRect.iBr.iY;
	
	if(!style.IsSet(CHcStyle::EHeight) ) //Not fixed height
	{
		iSize.iHeight +=  deltaY;
		iDisplayRect.iBr.iY += deltaY;
	}
	else //fixed height, only adjust when height increasing and overflow visible
	{
		if(deltaY>0)
		{
			if(style.IsSet(CHcStyle::EOverflow) && style.iOverflow==EOverflowVisible)
			{
				iDisplayRect.iBr.iY += deltaY;
				iSize.iHeight += deltaY;
			}
		}
	}
	
	if(!style.IsSet(CHcStyle::EWidth)) //Not fixed width
	{
		TInt deltaX = iDisplayRect.iTl.iX + iContentSize.iWidth - iDisplayRect.iBr.iX;
		if(deltaX<0)
		{
			iSize.iWidth += deltaX;
			iDisplayRect.iBr.iX += deltaX;
		}
	}

	if(iState.IsSet(EElementStateHidden))
		aStatus.iHidden--;
	aStatus.PopTextStyle();
	aStatus.PopLineAlign();
	
	aStatus.iPosition = iClippingRect.iTl;
	aStatus.iLineNumber = iLineNumber;
	iLineHeight = iPosition.iY + iSize.iHeight - aStatus.iPosition.iY + iMargins.iBottom;

	if(!style.IsSet(CHcStyle::ELeft))
		aStatus.iPosition.iX = iPosition.iX + iSize.iWidth + iMargins.iRight;
	if(!style.IsSet(CHcStyle::ETop))
		aStatus.CurrentLineInfo().SetHeightIfGreater(iLineHeight);
	
	if(style.IsClearRight())
	{
		aStatus.iCurrent = this;
		aStatus.NewLine();
		aStatus.iCurrent = iEnd;
	}
}

void CHtmlElementDiv::Layout()
{
	const CHcStyle& style = iStyle.Style();

	TPoint offset(0,0);
	TAlign align = ELeft; 
	TVAlign valign = EVTop;
	
	//handle margin
	if(!iFlags.IsSet(EBody))
	{
		if(style.iMargins.GetAlign(align))
		{
			if(align==ECenter)
				offset.iX = NO_BELOW_ZERO_MINUS(iParent->iDisplayRect.Width() , iSize.iWidth)/2;
			else if(align==ERight)
				offset.iX = NO_BELOW_ZERO_MINUS(iParent->iDisplayRect.Width() , iSize.iWidth + iMargins.iRight);
		}
		
		if(style.iMargins.GetVAlign(valign))
		{
			if(valign==EVCenter)
				offset.iY = NO_BELOW_ZERO_MINUS(iParent->iDisplayRect.Height() , iSize.iHeight)/2;
			else if(valign==EVBottom)
				offset.iY = NO_BELOW_ZERO_MINUS(iParent->iDisplayRect.Height() , iSize.iHeight + iMargins.iBottom);
		}
	}
	
	const THcLineInfo& lineInfo = iOwner->Impl()->LineInfo(iLineNumber);
	if(!iState.IsSet(EElementStateFixedX))
		offset.iX += lineInfo.iXOffset;
	if(!iState.IsSet(EElementStateFixedY))
	{
		if(iLineVAlign==EVCenter)
			offset.iY += (lineInfo.iHeight - iLineHeight )/2;
		else if(iLineVAlign==EVBottom)
			offset.iY += lineInfo.iHeight - iLineHeight;
	}
	
	iPosition += offset;
	iDisplayRect.Move(offset);
	
	//handle scroll offset
	if(iScrollbar)
	{
		iScrollbar->AdjustScrollPos();
		if(!iList || iList->IsEmpty())
			offset.iY -= iScrollbar->Pos();
	}
	
	//handle vertical padding
	TInt paddingOffsetY = 0;
	if(style.iPaddings.GetVAlign(valign))
	{
		if(valign==EVCenter)
			paddingOffsetY = NO_BELOW_ZERO_MINUS(iDisplayRect.Height() , iContentSize.iHeight)/2;
		else if(valign==EVBottom)
			paddingOffsetY = NO_BELOW_ZERO_MINUS(iDisplayRect.Height() , iContentSize.iHeight);
	}
	
	if(offset.iX!=0 || offset.iY!=0 || paddingOffsetY!=0)
	{
		CHtmlElementImpl* e = iNext;
		while(e!=iEnd)
		{
			if(e->TypeId()==EElementTypeDiv)
			{
				e->iPosition += offset;
				((CHtmlElementDiv*)e)->iDisplayRect.Move(offset);
				if(!e->iState.IsSet(EElementStateFixedY))
				{
					e->iPosition.iY += paddingOffsetY;
					((CHtmlElementDiv*)e)->iDisplayRect.Move(0, paddingOffsetY);
				}
			}
			else
			{
				e->iPosition += offset;
				if(!e->iState.IsSet(EElementStateFixedY))
					e->iPosition.iY += paddingOffsetY;
			}
			e = e->iNext;
		}
	}
}

void CHtmlElementDiv::Refresh()
{
	if(iScrollbar)
		iScrollbar->Refresh();

	iClippingRect = iDisplayRect;
	if(!iFlags.IsSet(EBody))
		HcUtils::Intersection(iClippingRect , iParent->iClippingRect);
}

void CHtmlElementDiv::Draw(CFbsBitGc& aGc) const
{
	aGc.SetFaded(iState.IsSet(EElementStateFaded));

	if(!iFlags.IsSet(EBody))
	{
		TRect rect = Rect();
		HcUtils::Intersection(rect , iParent->iClippingRect);
		aGc.SetClippingRect(rect);
	}
	
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, Rect(), iStyle.Style());

	if(iState.IsSet(EElementStateInFocus) && !iFocusStyle.IsNull())
	{
		TRect focusedRect = Rect();
		TMargins margins = iFocusStyle.Style().iMargins.GetMargins(iSize);
		HcUtils::ShrinkRect(focusedRect, margins);
		HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, focusedRect, iFocusStyle.Style());
	}
	
	if(iState.IsSet(EElementStateDown) && !iDownStyle.IsNull())
	{
		TRect downRect = Rect();
		TMargins margins = iDownStyle.Style().iMargins.GetMargins(iSize);
		HcUtils::ShrinkRect(downRect, margins);
		HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, downRect, iDownStyle.Style());
	}

	if(iScrollbar)
		iScrollbar->Draw(aGc);
	
	aGc.SetClippingRect(iClippingRect);
}

void CHtmlElementDiv::EndDraw(CFbsBitGc& aGc) const
{
	aGc.SetFaded(iParent->iState.IsSet(EElementStateFaded));
	aGc.SetClippingRect(iParent->iClippingRect);
}

void CHtmlElementDiv::SetFocus(TBool aFocus)
{
	CHtmlElementImpl::SetFocus(aFocus);
	
	if(IsContainer() && !iOwner->Impl()->iState.IsSet(EHCSAutoFocusDisabled))
	{
		if(aFocus)
		{
			if(iOwner->Impl()->iState.IsSet(EHCSNavKeyNext))
				SetFirstFocus();
			else if(iOwner->Impl()->iState.IsSet(EHCSNavKeyPrev))
				SetLastFocus();
			else if(iFocusedElement)
				iFocusedElement->SetFocus(ETrue);
			else
				SetFirstFocus();
		}
		else
		{
			if(iFocusedElement && iFocusedElement->IsFocused())
				iFocusedElement->SetFocus(EFalse);
		}
	}
}

void CHtmlElementDiv::FocusChangingTo(CHtmlElementImpl* aElement)
{
	if(iFocusedElement==aElement)
	{
	 	if(iFocusedElement && !iFocusedElement->IsFocused()) 
	 	{
	 		if(IsFocused())
	 		{
				iFocusedElement->SetFocus( ETrue );
				iState.Set(EElementStateFocusChanged);
				iOwner->Impl()->iState.Set(EHCSNeedRefresh);
	 		}
	 	}
	}
	else
	{
		if(iFocusedElement && iFocusedElement->IsFocused())
		{
			iFocusedElement->SetFocus( EFalse );
			iState.Set(EElementStateFocusChanged);
			iOwner->Impl()->iState.Set(EHCSNeedRefresh);
		}

		if(aElement) 
		{
			iFocusedElement = aElement;
			if(IsFocused())
			{
				iFocusedElement->SetFocus( ETrue );
				iState.Set(EElementStateFocusChanged);
				iOwner->Impl()->iState.Set(EHCSNeedRefresh);
			}
		}
	}
}

void CHtmlElementDiv::SetFirstFocus()
{
	if(iList && !iList->IsEmpty())
		iList->ScrollTop();
	else {
		iScrollbar->SetTopPos();
	
		CHtmlElementImpl* e = this->iNext;
		while(e!=iEnd)
		{
			if(e->CanFocus() && !e->iState.IsSet(EElementStateHidden) && e->iFlags.IsSet(CHtmlElementImpl::ETabStop) )
			{
				FocusChangingTo(e);
				break;
			}
	
			if(e->TypeId()==EElementTypeDiv 
					&& (((CHtmlElementDiv*)e)->IsContainer() || e->iState.IsSet(EElementStateHidden)))
				e = ((CHtmlElementDiv*)e)->iEnd;
			e = e->iNext;
		}
	}
}

void CHtmlElementDiv::SetLastFocus()
{
	if(iList && !iList->IsEmpty())
		iList->ScrollBottom();
	else {
		iScrollbar->SetBottomPos();
	
		CHtmlElementImpl* e = this->iEnd->iPrev;
		CHtmlElementImpl* testing;
		while(e!=this)
		{
			if(e->TypeId()==EElementTypeDivEnd)
				testing = e->iParent;
			else
				testing = e;
			if(testing->CanFocus() && testing->iFlags.IsSet(CHtmlElementImpl::ETabStop) && !testing->iState.IsSet(EElementStateHidden))
			{
				FocusChangingTo(testing);
				break;
			}
			
			if(testing->TypeId()==EElementTypeDiv 
					&& (((CHtmlElementDiv*)testing)->IsContainer() || testing->iState.IsSet(EElementStateHidden)))
				e = testing->iPrev;
			else
				e = e->iPrev;
		}
	}
}

TKeyResponse CHtmlElementDiv::OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType)
{
	if(aType==EEventKeyDown)
	{
		if(HcUtils::TranslateStdKey(aKeyEvent.iScanCode) == EStdKeyEnter) 
		{
			HandleButtonEventL(EButtonEventDown);
			return EKeyWasConsumed;	
		}
		else
			return EKeyWasNotConsumed;
	}
	else if(aType==EEventKey)
	{
		if(HcUtils::TranslateKey(aKeyEvent.iCode) == EKeyEnter)
		{
			HandleButtonEventL(EButtonEventUp);
			HandleButtonEventL(EButtonEventClick);
			return EKeyWasConsumed;
		}
		else
			return EKeyWasNotConsumed;
	}
	else
		return EKeyWasNotConsumed;
}

void CHtmlElementDiv::HandleButtonEventL(TInt aButtonEvent)
{
	if(!iFlags.IsSet(EFocusing))
		return;
	
	if(aButtonEvent==EButtonEventClick)
	{
		THtmlCtlEvent event;
		event.iType = THtmlCtlEvent::EOnClick;
		event.iControl = iOwner;
		event.iElement = this;
		event.iData = 1;
		iOwner->Impl()->FireEventL(event);
	}
	else if(aButtonEvent==EButtonEventSelect)
	{
		THtmlCtlEvent event;
		event.iType = THtmlCtlEvent::EOnClick;
		event.iControl = iOwner;
		event.iElement = this;
		event.iData = 0;
		iOwner->Impl()->FireEventL(event);
	}
	else
	{
		if(aButtonEvent==EButtonEventDown) 
		{
			iDownStyle.Update(iOwner->Impl());
			if(!iDownStyle.IsNull())
			{
				iState.Set(EElementStateDown);
				iOwner->Impl()->iState.Set(EHCSNeedRefresh);
			}
		}
		else if(aButtonEvent==EButtonEventUp)
		{
			if(iState.IsSet(EElementStateDown))
			{
				iState.Clear(EElementStateDown);
				iOwner->Impl()->iState.Set(EHCSNeedRefresh);
			}
		}
	}
}

_LIT(KHStrDivEnd, "div-end");
CHtmlElementDivEnd::CHtmlElementDivEnd(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeDivEnd;
	iTagName.Set(KHStrDivEnd);
}

CHtmlElementImpl* CHtmlElementDivEnd::CloneL() const 
{
	CHtmlElementDivEnd* e = new (ELeave)CHtmlElementDivEnd(iOwner);
	BaseClone(*e);
	return e;
}

void CHtmlElementDivEnd::Measure(CHcMeasureStatus& aStatus)
{
	iParent->EndMeasure(aStatus);
}

void CHtmlElementDivEnd::Draw(CFbsBitGc& aGc) const
{
	iParent->EndDraw(aGc);
}
