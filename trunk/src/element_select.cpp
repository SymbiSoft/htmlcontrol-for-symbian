#include <badesca.h>
#include <eikenv.h>

#include "controlimpl.h"
#include "htmlparser.h"
#include "measurestatus.h"
#include "utils.h"

#include "element_select.h"
#include "element_div.h"

CHtmlElementSelect::CHtmlElementSelect(CHtmlControl* aOwner) 
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeSelect;
	iTagName.Set(KHStrSelect);
	iTextArray = new CDesCArrayFlat(3);
	iValueArray = new CDesCArrayFlat(3);
	iStyle.SetTag(iTagName);
	iButtonStyle.SetTag(iTagName);
	iButtonStyle.SetClass(_L("selectbutton"));
}

CHtmlElementSelect::~CHtmlElementSelect() 
{
	iStyle.Close();
	iButtonStyle.Close();
	delete iTextArray;
	delete iValueArray;
	delete iTitle;
	
#ifdef __UIQ__
	delete iPopout;
	iCommands.ResetAndDestroy();
#endif
}

TBool CHtmlElementSelect::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.CompareF(KHStrValue)==0) 
	{
		if(iValueArray->Count()>0)
			aBuffer.Copy((*iValueArray)[iSelected]);
		else
			aBuffer.Zero();
	}
	else if(aName.CompareF(KHStrText)==0) 
	{
		if(iTextArray->Count()>0)
			aBuffer.Copy((*iTextArray)[iSelected]);
		else
			aBuffer.Zero();
	}
	else if(aName.CompareF(KHStrBufSize)==0) 
	{
		if(iValueArray->Count()>0)
			aBuffer.Num((*iValueArray)[iSelected].Length());
		else
			aBuffer.Num(0);
	}
	else if(aName.CompareF(KHStrSelectedIndex)==0)
		aBuffer.Num(iSelected);
	else if(aName.CompareF(KHStrClass)==0) 
		aBuffer.Copy(iStyle.Class().Left(aBuffer.MaxLength()));
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.GetSingleStyle(aName.Mid(6), aBuffer);
	else
		return EFalse;

	return ETrue;
}

TBool CHtmlElementSelect::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrSelectedIndex)==0)
	{
		iSelected = HcUtils::StrToInt(aValue);
		if(iTextArray->Count()==0)
			iSelected = 0;
		else if(iSelected>=iTextArray->Count())
			iSelected = iTextArray->Count() - 1;
	}
	else if(aName.CompareF(KHStrDisabled)==0) 
		iFlags.Assign(EDisabled, HcUtils::StrToBool(aValue));
	else if(aName.CompareF(KHStrTitle)==0) 
	{
		delete iTitle;
		iTitle = aValue.AllocL();
	}
	else if(aName.CompareF(KHStrValue)==0) 
	{
		for(TInt i=0;i<iValueArray->Count();i++) 
		{
			if((*iValueArray)[i].Compare(aValue)==0) 
			{
				iSelected = i;
				break;
			}
		}
	}
	else if(aName.CompareF(KHStrTextArray)==0)
	{
		TInt resId = HcUtils::StrToInt(aValue, EDecimal);
#ifdef __SYMBIAN_9_ONWARDS__
		if(CEikonEnv::Static()->IsResourceAvailableL(resId)) 
#else
		if(ETrue)
#endif
		{
			delete iTextArray;
			iTextArray = NULL;
			
			iTextArray = CEikonEnv::Static()->ReadDesC16ArrayResourceL(resId);
			if(iTextArray->Count()==0)
				iSelected = 0;
			else if(iSelected>=iTextArray->Count())
				iSelected = iTextArray->Count() - 1;
			return ETrue;
		}
		else
			return EFalse;
	}
	else if(aName.CompareF(KHStrValueArray)==0)
	{
		TInt resId = HcUtils::StrToInt(aValue, EDecimal);
#ifdef __SYMBIAN_9_ONWARDS__
		if(CEikonEnv::Static()->IsResourceAvailableL(resId)) 
#else
		if(ETrue)
#endif
		{
			delete iValueArray;
			iValueArray = NULL;
			
			iValueArray = CEikonEnv::Static()->ReadDesC16ArrayResourceL(resId);		
			return ETrue;
		}
		else
			return EFalse;
	}
	else if(aName.CompareF(KHStrOptionArray)==0)
	{
		TInt resId = HcUtils::StrToInt(aValue, EDecimal);
#ifdef __SYMBIAN_9_ONWARDS__
		if(CEikonEnv::Static()->IsResourceAvailableL(resId)) 
#else
		if(ETrue)
#endif
		{
			iTextArray->Reset();
			iValueArray->Reset();
			
			CDesC16ArrayFlat* array = CEikonEnv::Static()->ReadDesC16ArrayResourceL(resId);
			CleanupStack::PushL(array);
			for(TInt i=0;i<array->Count();i++) 
			{
				const TDesC& str = (*array)[i];
				TInt pos = str.Locate('\t');
				if(pos!=KErrNotFound) 
				{
					iValueArray->AppendL(str.Mid(0,pos));
					iTextArray->AppendL(str.Mid(pos+1));
				}
				else 
				{
					TBuf<5> buf;
					buf.Num(i);
					iValueArray->AppendL(buf);
					iTextArray->AppendL(str);
				}
			}
			CleanupStack::PopAndDestroy();
			
			if(iTextArray->Count()==0)
				iSelected = 0;
			else if(iSelected>=iTextArray->Count())
				iSelected = iTextArray->Count() - 1;
			return ETrue;
		}
		else
			return EFalse;
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

void CHtmlElementSelect::InvokeL(TRefByValue< const TDesC16 > aCommand, ...) 
{
	VA_LIST arg_list;
	VA_START(arg_list, aCommand);
	
	const TDesC& cmd = (const TDesC&)aCommand;
	if(cmd.CompareF(KHStrAddOption)==0) 
	{
		const TDesC *value = VA_ARG(arg_list,const TDesC*);
		const TDesC *text = VA_ARG(arg_list,const TDesC*);
		
		iValueArray->AppendL(*value);
		iTextArray->AppendL(*text);
	}
	else if(cmd.CompareF(KHStrRemoveOption)==0) 
	{
		const TDesC *value = VA_ARG(arg_list,const TDesC*);
		for(TInt i=0;i<iValueArray->Count();i++) 
		{
			if((*iValueArray)[i].Compare(*value)==0) 
			{
				iTextArray->Delete(i);
				iValueArray->Delete(i);
				break;
			}
		}
		
		if(iTextArray->Count()==0)
			iSelected = 0;
		else if(iSelected>=iTextArray->Count())
			iSelected = iTextArray->Count() - 1;
	}
	else if(cmd.CompareF(KHStrClearOptions)==0) 
	{
		iTextArray->Reset();
		iValueArray->Reset();
		iSelected = 0;
	}
	else if(cmd.CompareF(KHStrClick)==0)
		HandleButtonEventL(EButtonEventClick);
	
    VA_END(arg_list);
}

CHtmlElementImpl* CHtmlElementSelect::CloneL()  const
{
	CHtmlElementSelect* e = new (ELeave)CHtmlElementSelect(iOwner);
	BaseClone(*e);
	
	e->iStyle.CopyL(iStyle);
	e->iSelected = iSelected;
	if(iTitle)
		e->iTitle = (*iTitle).AllocL();
	
	for(TInt i=0;i<iTextArray->Count();i++)
		e->iTextArray->AppendL((*iTextArray)[i]);
		
	for(TInt i=0;i<iValueArray->Count();i++)
		e->iValueArray->AppendL((*iValueArray)[i]);
		
	return e;
}

void CHtmlElementSelect::Measure(CHcMeasureStatus& aStatus)
{
	if(iStyle.Update(iOwner->Impl()))
	{
		if(!iStyle.Style().iTextStyle.IsSet(THcTextStyle::ESize))
			iStyle.Style().iTextStyle.iSize = aStatus.CurrentTextStyle().iSize;
	}
	iButtonStyle.Update(iOwner->Impl());

	if(iStyle.Style().IsDisplayNone()) 
	{
		iState.Set(EElementStateHidden);
		return;
	}

	const CFont* font = iOwner->Impl()->GetFont(iStyle.Style().iTextStyle);	
	DoMeasure(aStatus, iStyle.Style(), TSize(font->WidthZeroInPixels()*10, Max(font->HeightInPixels() + 12,15)));
}

void CHtmlElementSelect::Draw(CFbsBitGc& aGc) const
{
	if(iFlags.IsSet(EDisabled))
		aGc.SetUserDisplayMode(EGray256);
	
	TRect rect(iPosition, iSize);
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect, iStyle.Style());

	if(iState.IsSet(EElementStateFocused))
		DrawFocus(aGc);

	TRect dropBoxRect = TRect(TPoint(rect.iBr.iX - iSize.iHeight + 2, rect.iTl.iY), rect.iBr);
	dropBoxRect.Shrink(2,2);
	rect.Shrink(3, 1);
	rect.iBr.iX = dropBoxRect.iTl.iX - 4;

	CFont* font = iOwner->Impl()->GetFont(iStyle.Style().iTextStyle);
	HcUtils::PrepareGcForTextDrawing(aGc, iStyle.Style().iTextStyle);
	if(iFlags.IsSet(EDisabled))
		aGc.SetPenColor(KRgbGray);
	aGc.UseFontNoDuplicate(static_cast<CFbsBitGcFont*>(font));
	TInt baseline = (rect.Height()+ font->AscentInPixels())/2;
	
	if(iTextArray->Count()>0)
	{
		TPtrC p = (*iTextArray)[iSelected];
		aGc.DrawText(p, rect, baseline, CBitmapContext::ELeft, 0);
	}

	if(!iButtonStyle.Style().IsHidden())
	{
		HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, dropBoxRect, iButtonStyle.Style());

		aGc.SetPenColor(iButtonStyle.Style().iTextStyle.iColor.Rgb());
		aGc.SetBrushStyle(CGraphicsContext::ENullBrush);
		aGc.SetPenSize(TSize(2,2));
		TInt x = dropBoxRect.Width()/3, y1 = dropBoxRect.Height()/3, y2 = dropBoxRect.Height()/4;
		TPoint p(dropBoxRect.iTl.iX + x - 1, dropBoxRect.iTl.iY + y1);
		aGc.MoveTo(p);

		p = TPoint(dropBoxRect.Center().iX - 1 , p.iY + y2);
		aGc.DrawLineTo(p);

		p = TPoint(dropBoxRect.iBr.iX - x - 1, dropBoxRect.iTl.iY + y1);
		aGc.DrawLineTo(p);
	}
	
	if(iFlags.IsSet(EDisabled))
		aGc.SetUserDisplayMode(aGc.Device()->DisplayMode());
}

void CHtmlElementSelect::DrawFocus(CFbsBitGc& aGc) const
{
	const CHcStyle& focusedStyle = iOwner->Impl()->GetStyle(iTagName, iStyle.Class(), THcSelector::EFCFocus, 0);
	TRect rect2 = Rect();
	TMargins margins = focusedStyle.iMargins.GetMargins(rect2.Size());
	HcUtils::ShrinkRect(rect2, margins);
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect2, focusedStyle);
}

TKeyResponse CHtmlElementSelect::OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType)
{
	if(aType==EEventKey && HcUtils::TranslateKey(aKeyEvent.iCode) == EKeyEnter)
	{
		HandleButtonEventL(EButtonEventClick);
		return EKeyWasConsumed;	
	}
	else
		return EKeyWasNotConsumed;
}

void CHtmlElementSelect::HandleButtonEventL(TInt aButtonEvent)
{
	if(aButtonEvent==EButtonEventClick || aButtonEvent==EButtonEventSelect)
		PopupListBox();
}

#ifdef __SERIES60__

#ifdef __SERIES60_3_ONWARDS__
#include <akniconutils.h> 
#endif
#include <gulicon.h>
#include <avkon.mbg>
#include <aknlists.h> 
#include <aknpopup.h> 
#include <akniconarray.h> 
#include <akncolourselectiongrid.h> 

void CHtmlElementSelect::PopupListBox()
{
	CAknSingleGraphicPopupMenuStyleListBox* list = new(ELeave) CAknSingleGraphicPopupMenuStyleListBox;
    CleanupStack::PushL(list);

    CAknPopupList* popupList = CAknPopupList::NewL(list, R_AVKON_SOFTKEYS_OK_BACK, AknPopupLayouts::EMenuWindow );
    CleanupStack::PushL(popupList);

    list->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
    list->CreateScrollBarFrameL(ETrue);
    list->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);

	CArrayPtr<CGulIcon>* icons =new (ELeave) CAknIconArray(2);
	list->ItemDrawer()->ColumnData()->SetIconArray(icons);

#ifdef __SERIES60_3_ONWARDS__
	CFbsBitmap* bitmap = NULL, *mask = NULL;
	
 	AknIconUtils::CreateIconLC(bitmap, mask, AknIconUtils::AvkonIconFileName() , EMbmAvkonQgn_indi_radiobutt_off, EMbmAvkonQgn_indi_radiobutt_off_mask);
 	CGulIcon *icon = CGulIcon::NewL(bitmap, mask);
 	CleanupStack::Pop(2); //.bitmap, mask
	icons->AppendL(icon);
	
	AknIconUtils::CreateIconLC(bitmap, mask, AknIconUtils::AvkonIconFileName() , EMbmAvkonQgn_indi_radiobutt_on, EMbmAvkonQgn_indi_radiobutt_on_mask);
 	icon = CGulIcon::NewL(bitmap, mask);
 	CleanupStack::Pop(2); //.bitmap, mask
	icons->AppendL(icon);
#else
	CGulIcon *icon = CEikonEnv::Static()->CreateIconL(_L("z:\\system\\data\\avkon.mbm"), EMbmAvkonQgn_indi_radiobutt_off, EMbmAvkonQgn_indi_radiobutt_off_mask);
	icons->AppendL(icon);
	
	icon = CEikonEnv::Static()->CreateIconL(_L("z:\\system\\data\\avkon.mbm"), EMbmAvkonQgn_indi_radiobutt_on, EMbmAvkonQgn_indi_radiobutt_on_mask);
	icons->AppendL(icon);
#endif
	
	_LIT(KListItemFormat, "%i\t%S");
	CDesCArray *itemList = new (ELeave) CDesCArrayFlat(3);
	list->Model()->SetItemTextArray(itemList);
	list->Model()->SetOwnershipType(ELbmOwnsItemArray);
#ifdef __SERIES60_3_ONWARDS__
	list->ItemDrawer()->FormattedCellData()->EnableMarqueeL( ETrue );
#endif	
	TBuf<100> buf;
	for(TInt i=0;i<iTextArray->Count();i++) 
	{
		TPtrC p = (*iTextArray)[i];
		buf.Format(KListItemFormat, i==iSelected, &p);
		itemList->AppendL(buf);
		
		if(i==iSelected)
			list->SetCurrentItemIndex(i);
	}
	if(iTitle)
		popupList->SetTitleL(*iTitle);
    CleanupStack::Pop(); 
    TBool popupOk = popupList->ExecuteLD();
    if(popupOk) 
    {
		iOwner->Impl()->iState.Set(EHCSNeedRefresh);
		
		if(iSelected!=list->CurrentItemIndex()) 
		{
			iSelected = list->CurrentItemIndex();
			
			THtmlCtlEvent event;
			event.iType = THtmlCtlEvent::EOnChanged;
			event.iControl = iOwner;
			event.iElement = this;
			event.iData = 0;
			iOwner->Impl()->FireEventL(event);
		}
    }
    CleanupStack::PopAndDestroy();  // list

}
#endif

#ifdef __UIQ__

void CHtmlElementSelect::HandleCommandL(CQikCommand &aCommand)
{
	TInt index = aCommand.Id()-1;
	if(index!=iSelected)
	{
		iSelected = index;
		iOwner->RefreshAndDraw();
	}
}

void CHtmlElementSelect::PopupListBox()
{
	if(!iPopout)
		iPopout = CQikMenuPopout::NewL(*CEikonEnv::Static(), *this);
	else
		iPopout->ResetL();
	iCommands.ResetAndDestroy();
	
	for(TInt i=0;i<iTextArray->Count();i++) 
	{	
		CQikCommand* cmd = CQikCommand::NewL(i+1); 
		cmd->SetTextL((*iTextArray)[i]);
		cmd->SetType(EQikCommandTypeScreen);
		if(i==0)
			cmd->SetRadioStart(ETrue);
		else if(i==iTextArray->Count()-1)
			cmd->SetRadioEnd(ETrue);
		else
			cmd->SetRadioMiddle(ETrue);
		if(i==iSelected)
			cmd->SetChecked(ETrue);
		iCommands.Append(cmd);
		iPopout->AddCommandL(*cmd);
	}
	TPoint pos(iPosition.iX, iPosition.iY + iSize.iHeight);
	pos += iOwner->PositionRelativeToScreen();
	iPopout->DisplayL(pos, EPopupTargetTopLeft, EFalse);
}

#endif

