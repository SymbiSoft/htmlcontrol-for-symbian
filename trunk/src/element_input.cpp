#include <BARSREAD.H>

#include "controlimpl.h"
#include "htmlctlenv.h"
#include "htmlparser.h"
#include "htmlelementiter.h"
#include "measurestatus.h"
#include "inputeditor.h"
#include "utils.h"

#include "element_input.h"
#include "element_div.h"

CHtmlElementInput::CHtmlElementInput(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeInput;
	iTagName.Set(KHStrInput);
	iText = KNullDesC().AllocL();
}

CHtmlElementInput::~CHtmlElementInput()
{
	if(iEmbedObject) 
	{
		iOwner->Impl()->UnregisterControl(iEmbedObject);
		delete iEmbedObject;
	}
	iStyle.Close();
	
	delete iText;
	delete iTitle;
}

TBool CHtmlElementInput::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.CompareF(KHStrValue)==0) 
	{
		if(iEmbedObject) 
		{
			HBufC* buf = ((CEikEdwin*)iEmbedObject)->GetTextInHBufL();	
			if(!buf)
				aBuffer.Zero();
			else
			{
				aBuffer.Copy(buf->Left(aBuffer.MaxLength()));
				delete buf;
			}
		}
		else
			aBuffer.Copy((*iText).Left(aBuffer.MaxLength()));
	}
	else if(aName.CompareF(KHStrBufSize)==0) 
	{
		if(iEmbedObject) 
			aBuffer.Num(((CEikEdwin*)iEmbedObject)->TextLength());
		else
			aBuffer.Num(iText->Length());
	}
	else if(aName.CompareF(KHStrChecked)==0) 
	{
		if(iFlags.IsSet(EChecked))
			aBuffer.Num(1);
		else
			aBuffer.Num(0);
	}
	else if(aName.CompareF(KHStrType)==0) 
	{
		switch(iInputType)
		{
		case EText:
			aBuffer.Copy(KHStrText);
			break;
		case ECheckBox:
			aBuffer.Copy(KHStrCheckbox);
			break;
		case ERadio:
			aBuffer.Copy(KHStrRadio);
			break;
		case EPassword:
			aBuffer.Copy(KHStrPassword);
			break;
		case EButton:
			aBuffer.Copy(KHStrButton);
			break;
		case EHidden:
			aBuffer.Copy(KHStrHidden);
			break;
		case ESubmit:
			aBuffer.Copy(KHStrSubmit);
			break;
		default:
			aBuffer.Zero();
		}
	}
	else if(aName.CompareF(KHStrClass)==0) 
		aBuffer.Copy(iStyle.Class().Left(aBuffer.MaxLength()));
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.GetSingleStyle(aName.Mid(6), aBuffer);
	else
		return EFalse;

	return ETrue;	
}

TBool CHtmlElementInput::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrType)==0) 
	{
		TInt i;
		HtmlParser::ParseInputType(aValue, i);
		iInputType = (TInputType)i;
	}
	else if(aName.CompareF(KHStrChecked)==0) 
	{
		TBool checked = HcUtils::StrToBool(aValue);
		if(iInputType==ERadio && checked!=iFlags.IsSet(EChecked) && checked) 
		{
			THtmlElementIter it(iOwner->Body(), Id(), KHStrInput);
			while(it.Next())
			{
				if(it()!=this && ((CHtmlElementInput*)it())->iInputType==ERadio)
				{
					((CHtmlElementImpl*)it())->iFlags.Clear(EChecked);
				}
			}
		}	

		iFlags.Assign(EChecked, checked);
	}
	else if(aName.CompareF(KHStrValue)==0)
	{
		delete iText;
		iText = HcUtils::DecodeHttpCharactersL(aValue);
			
		if(iEmbedObject) 
		{
			SetEditorTextL(*iText);
			((CEikEdwin*)iEmbedObject)->HandleTextChangedL();
			((CEikEdwin*)iEmbedObject)->SetCursorPosL(0, EFalse);
		}
	}
	else if(aName.CompareF(KHStrDisabled)==0) 
	{
		iFlags.Assign(EDisabled, HcUtils::StrToBool(aValue));
		if(iEmbedObject)
			iEmbedObject->SetDimmed(iFlags.IsSet(EDisabled));
	}
	else if(aName.CompareF(KHStrReadOnly)==0) 
	{
		iFlags.Assign(EReadOnly, HcUtils::StrToBool(aValue));
		if(iEmbedObject)
			((CEikEdwin*)iEmbedObject)->SetReadOnly(iFlags.IsSet(EReadOnly));
	}
	else if(aName.CompareF(KHStrTitle)==0) 
	{
		delete iTitle;
		iTitle = aValue.AllocL();
	}
	else if(aName.CompareF(KHStrInputMode)==0) 
	{
#ifdef __SERIES60__
		//text/number/all/alpha
		if(aValue.CompareF(KHStrText)==0)
			iInputMode = EAknEditorTextInputMode;
		else if(aValue.CompareF(KHStrNumber)==0)
			iInputMode = EAknEditorNumericInputMode;
		else if(aValue.CompareF(KHStrAll)==0)
			iInputMode = EAknEditorAllInputModes;
		else if(aValue.CompareF(KHStrAlpha)==0)
			iInputMode = EAknEditorNumericInputMode | EAknEditorSecretAlphaInputMode;
		else
			iInputMode = EAknEditorAllInputModes;		
		
		if(iEmbedObject)
			((CEikEdwin*)iEmbedObject)->SetAknEditorAllowedInputModes(iInputMode);
#endif
	}
	else if(aName.Compare(KHStrMaxLength)==0)
	{
#ifdef __SERIES60__
		iMaxLength = HcUtils::StrToInt(aValue);
		if(iEmbedObject && iMaxLength>0)
			((CEikEdwin*)iEmbedObject)->SetMaxLength(iMaxLength);
#endif
	}
	else if(aName.Compare(KHStrDirectFocus)==0)
	{
		iFlags.Assign(EDirectFocus, HcUtils::StrToBool(aValue));
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

CHtmlElementImpl* CHtmlElementInput::CloneL()  const
{
	CHtmlElementInput* e = new (ELeave)CHtmlElementInput(iOwner);
	BaseClone(*e);
	
	e->iStyle.CopyL(iStyle);
	e->iText = (*iText).AllocL();
	if(iTitle)
		e->iTitle = (*iTitle).AllocL();
	e->iInputType = iInputType;
	e->iInputMode = iInputMode;
	e->iMaxLength = iMaxLength;
	
	return e;
}
	
void CHtmlElementInput::SetEditorTextL(const TDesC& aText)
{
	CPlainText* text = ((CEikEdwin*)iEmbedObject)->Text();
	if(text->DocumentLength()>0)
		text->DeleteL(0, text->DocumentLength());
	
	TInt pos1 = 0, pos2 = 0; 
	while((pos2=aText.Mid(pos1).Locate('\n'))!=KErrNotFound) 
	{
		TPtrC ptr = aText.Mid(pos1, pos2);
		if(ptr.Length()>=1 && ptr[ptr.Length()-1]=='\r')
			text->InsertL (text->DocumentLength(), ptr.Left(ptr.Length()-1));
		else
			text->InsertL (text->DocumentLength(), ptr);
		text->InsertL (text->DocumentLength(), ' ');
		pos1 = pos1+pos2+1;
	}
	if(pos1!=aText.Length())
		text->InsertL (text->DocumentLength(), aText.Mid(pos1));
}

void CHtmlElementInput::SetFocus(TBool aFocus) 
{
	CHtmlElementImpl::SetFocus(aFocus);
	
	if(iEmbedObject) 
	{
		if(iFlags.IsSet(EDirectFocus) || !aFocus)
			iEmbedObject->SetFocus(aFocus);

		if(!aFocus && ((CEikEdwin*)iEmbedObject)->TextLength()>0)
			((CEikEdwin*)iEmbedObject)->SetCursorPosL(0, EFalse);
	}
}

void CHtmlElementInput::PrepareL() 
{
	CHtmlElementImpl::PrepareL();
	
	iStyle.SetTag(iTagName);
	if(iInputType==EPassword)
		iStyle.SetAttribute(100+EText);
	else if(iInputType==ESubmit)
		iStyle.SetAttribute(100+EButton);
	else
		iStyle.SetAttribute(100+iInputType);
	
	if(iInputType==EText || iInputType==EPassword) 
	{
		CHcInputEditor* editor = new (ELeave) CHcInputEditor(iOwner, EFalse);
		iEmbedObject = editor;
		
		editor->SetContainerWindowL(*iOwner);

#ifdef __SERIES60__
		if(iInputMode!=0) 
		{
			editor->SetAknEditorInputMode(iInputMode);
			editor->SetAknEditorAllowedInputModes(iInputMode);
		}
		else if(iInputType==EPassword) 
		{
			editor->SetAknEditorInputMode(EAknEditorSecretAlphaInputMode);
			editor->SetAknEditorAllowedInputModes(EAknEditorSecretAlphaInputMode);	
		}
#endif
		
		TInt flags = CEikEdwin::ENoAutoSelection | CEikEdwin::ENoHorizScrolling | CEikEdwin::ENoWrap;
		if(iFlags.IsSet(EReadOnly))
			flags |= CEikEdwin::EReadOnly;
#ifdef __UIQ__
		flags |= CEikEdwin::EDisableSkinning;
#endif
		editor->ConstructL(flags,0,0,1);

#ifdef __SERIES60__
		editor->EnableCcpuSupportL(ETrue);
#endif
		
		if(iMaxLength>0)
			editor->SetTextLimit(iMaxLength);
#ifdef __UIQ__
		editor->SetBorderViewMargins(TMargins8());
		editor->SetBorder(TGulBorder::ENone);
#endif
		iOwner->Impl()->RegisterControl(editor);
		editor->ActivateL();
		
		if(iFlags.IsSet(EDisabled))
			editor->SetDimmed(ETrue);
		SetEditorTextL(*iText);
	}
}

void CHtmlElementInput::Measure(CHcMeasureStatus& aStatus)
{
	if(iInputType==EText || iInputType==EPassword)
		MeasureTextEditor(aStatus);
	else if(iInputType==ECheckBox)
		MeasureCheckBox(aStatus);
	else if(iInputType==ERadio)
		MeasureRadio(aStatus);
	else if(iInputType==EButton || iInputType==ESubmit)
		MeasureButton(aStatus);
	else //if(iInputType==EHidden)
		; //nothing	
}

void CHtmlElementInput::Refresh()
{
	if(iEmbedObject)
	{
		TBool hidden = iState.IsSet(EElementStateHidden);
		if(iEmbedObject->IsVisible() && hidden
				|| !iEmbedObject->IsVisible() && !hidden)
		{
			iEmbedObject->MakeVisible(!hidden);
		
			if(hidden && iEmbedObject->IsFocused())
				iEmbedObject->SetFocus(EFalse);
		}
		if(!hidden)
		{
			TRect rect = TRect(iPosition, iSize);
			iStyle.Style().iBorders.ShrinkRect(rect);
			iStyle.Style().iPaddings.ShrinkRect(rect); 
	
			rect.Move(iOwner->Rect().iTl);
			if(rect!=iEmbedObject->Rect())
				iEmbedObject->SetRect(rect);
		}
	}
}

void CHtmlElementInput::Draw(CFbsBitGc& aGc) const
{
	if(iFlags.IsSet(EDisabled))
		aGc.SetUserDisplayMode(EGray256);
	
	if(iInputType==EText || iInputType==EPassword)
		DrawTextEditor(aGc);
	else if(iInputType==ECheckBox)
		DrawCheckBox(aGc);
	else if(iInputType==ERadio)
		DrawRadio(aGc);
	else if(iInputType==EButton || iInputType==ESubmit)
		DrawButton(aGc);
	else //if(iInputType==EHidden)
		; //nothing
	
	if(iFlags.IsSet(EDisabled))
		aGc.SetUserDisplayMode(aGc.Device()->DisplayMode());
}

void CHtmlElementInput::DrawFocus(CFbsBitGc& aGc) const
{
	const CHcStyle& focusedStyle = iOwner->Impl()->GetStyle(iTagName, iStyle.Class(), THcSelector::EFCFocus, iStyle.Attribute());
	TRect rect2 = Rect();
	TMargins margins = focusedStyle.iMargins.GetMargins(rect2.Size());
	HcUtils::ShrinkRect(rect2, margins);
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect2, focusedStyle);
}

void CHtmlElementInput::MeasureTextEditor(CHcMeasureStatus& aStatus)
{
	if(iStyle.Update(iOwner->Impl()))
	{
		if(!iStyle.Style().iTextStyle.IsSet(THcTextStyle::ESize))
			iStyle.Style().iTextStyle.iSize = aStatus.CurrentTextStyle().iSize;
	}
	
	if(iStyle.Style().IsDisplayNone())
	{
		iState.Set(EElementStateHidden);
		return;
	}
	
	((CHcInputEditor*)iEmbedObject)->SetFontL(iStyle.Style().iTextStyle);
	TRect lRect;
	((CEikEdwin*)iEmbedObject)->TextLayout()->GetLineRect(0, lRect);
	DoMeasure(aStatus, iStyle.Style(),
		TSize( ((CHcInputEditor*)iEmbedObject)->Font()->WidthZeroInPixels()*10, lRect.Height()+6));
}

void CHtmlElementInput::DrawTextEditor(CFbsBitGc& aGc) const
{
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, Rect(), iStyle.Style());
	
	if(iState.IsSet(EElementStateFocused))
		DrawFocus(aGc);
}

void CHtmlElementInput::MeasureCheckBox(CHcMeasureStatus& aStatus)
{
	iStyle.Update(iOwner->Impl());
	
	if(iStyle.Style().IsDisplayNone())
	{
		iState.Set(EElementStateHidden);
		return;
	}
	
	TInt i = iOwner->Impl()->Env()->BaseFontSize(-1);
	DoMeasure(aStatus, iStyle.Style(), TSize(i,i));
}

void CHtmlElementInput::DrawCheckBox(CFbsBitGc& aGc) const
{
	TRect rect(iPosition, iSize);
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect, iStyle.Style());
	
	if(iState.IsSet(EElementStateFocused))
		DrawFocus(aGc);
	
	if(iFlags.IsSet(EChecked)) 
	{
		aGc.SetPenColor(iStyle.Style().iTextStyle.iColor.Rgb());
		aGc.SetPenStyle(CBitmapContext::ESolidPen);
		if(iSize.iWidth<20)
		{
			aGc.SetPenSize(TSize(2,2));
			aGc.DrawLine(TPoint(rect.iTl.iX+3, rect.iTl.iY+3), TPoint(rect.iBr.iX-4, rect.iBr.iY-4));
			aGc.DrawLine(TPoint(rect.iTl.iX+3, rect.iBr.iY-4), TPoint(rect.iBr.iX-4, rect.iTl.iY+3));
		}
		else
		{
			aGc.SetPenSize(TSize(4,4));
			aGc.DrawLine(TPoint(rect.iTl.iX+5, rect.iTl.iY+5), TPoint(rect.iBr.iX-6, rect.iBr.iY-6));
			aGc.DrawLine(TPoint(rect.iTl.iX+5, rect.iBr.iY-6), TPoint(rect.iBr.iX-6, rect.iTl.iY+5));
		}
	}
}

void CHtmlElementInput::MeasureRadio(CHcMeasureStatus& aStatus)
{
	iStyle.Update(iOwner->Impl());
	
	if(iStyle.Style().IsDisplayNone())
	{
		iState.Set(EElementStateHidden);
		return;
	}
	
	TInt i = iOwner->Impl()->Env()->BaseFontSize(-1);
	DoMeasure(aStatus, iStyle.Style(), TSize(i,i));
}

void CHtmlElementInput::DrawRadio(CFbsBitGc& aGc) const
{
	TRect rect(iPosition, iSize);
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect, iStyle.Style());
	
	if(iState.IsSet(EElementStateFocused))
		DrawFocus(aGc);
	
	if(iFlags.IsSet(EChecked))
	{
		if(iSize.iWidth<20)
			rect.Shrink(3,3);
		else
			rect.Shrink(5,5);
		aGc.SetPenStyle(CBitmapContext::ENullPen);
		aGc.SetBrushStyle(CGraphicsContext::ESolidBrush);
		aGc.SetBrushColor(iStyle.Style().iTextStyle.iColor.Rgb());
		aGc.DrawEllipse(rect);
	}
}

void CHtmlElementInput::MeasureButton(CHcMeasureStatus& aStatus)
{
	if(iStyle.Update(iOwner->Impl()))
	{
		if(!iStyle.Style().iTextStyle.IsSet(THcTextStyle::ESize))
			iStyle.Style().iTextStyle.iSize = aStatus.CurrentTextStyle().iSize;
	}
	
	if(iStyle.Style().IsDisplayNone())
	{
		iState.Set(EElementStateHidden);
		return;
	}
	
	const CFont* font = iOwner->Impl()->GetFont(iStyle.Style().iTextStyle);
	DoMeasure(aStatus, iStyle.Style(),  
			TSize(Max(font->TextWidthInPixels(*iText) + font->WidthZeroInPixels(),iOwner->Impl()->Env()->BaseFontSize()*3),
			Max(font->HeightInPixels() + font->WidthZeroInPixels(), 15)));
}

void CHtmlElementInput::DrawButton(CFbsBitGc& aGc) const
{
	TRect rect(iPosition, iSize);
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect, iStyle.Style());

	if(iState.IsSet(EElementStateFocused))
		DrawFocus(aGc);
	
	if(iState.IsSet(EElementStateDown))
	{
		const CHcStyle& downStyle = iOwner->Impl()->GetStyle(iTagName, iStyle.Class(), THcSelector::EFCDown, iStyle.Attribute());
		TRect rect2 = Rect();
		TMargins margins = downStyle.iMargins.GetMargins(rect2.Size());
		HcUtils::ShrinkRect(rect2, margins);
		HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect2, downStyle);
	}

	CFont* font = iOwner->Impl()->GetFont(iStyle.Style().iTextStyle);
	TInt baseline = (rect.Height()+ font->AscentInPixels())/2;
	HcUtils::PrepareGcForTextDrawing(aGc, iStyle.Style().iTextStyle);
	if(iFlags.IsSet(EDisabled))
		aGc.SetPenColor(KRgbGray);
	aGc.UseFontNoDuplicate(static_cast<CFbsBitGcFont*>(font));
	aGc.DrawText(*iText, rect, baseline, CBitmapContext::ECenter, iFlags.IsSet(EChecked)?2:0);
}

TKeyResponse CHtmlElementInput::OfferKeyEventL(const TKeyEvent &aKeyEvent, TEventCode aType) 
{
	if(iInputType==EText || iInputType==EPassword)  
	{
		TInt keyCode = HcUtils::TranslateKey(aKeyEvent.iCode);
		if(keyCode == EKeyEnter )
		{
			if(!iEmbedObject->IsFocused())
			{
				iEmbedObject->SetFocus(ETrue);
				return EKeyWasConsumed;
			}	
		}
		else if(keyCode == EKeyUpArrow || keyCode==EKeyDownArrow)
		{
			if(!iFlags.IsSet(EDirectFocus) && iEmbedObject->IsFocused())
			{
				iEmbedObject->SetFocus(EFalse);
				if(((CEikEdwin*)iEmbedObject)->TextLength()>0)
					((CEikEdwin*)iEmbedObject)->SetCursorPosL(0, EFalse);
				return EKeyWasConsumed;
			}
			else
			{
				return EKeyWasNotConsumed;
			}
		}
		
		if(aType==EEventKeyDown && aKeyEvent.iScanCode>32 && aKeyEvent.iScanCode<128)
		{
			if(!iEmbedObject->IsFocused())
				iEmbedObject->SetFocus(ETrue);
		}

		if(iEmbedObject->IsFocused())		
			return iEmbedObject->OfferKeyEventL(aKeyEvent, aType);
		else
			return EKeyWasNotConsumed;
	}
	else if(iInputType==ECheckBox) 
	{
		if(aType==EEventKey && HcUtils::TranslateKey(aKeyEvent.iCode) == EKeyEnter )
		{
			HandleButtonEventL(EButtonEventClick);
			return EKeyWasConsumed;
		}
		else
			return EKeyWasNotConsumed;
	}
	else if(iInputType==ERadio) 
	{
		if(aType==EEventKey && HcUtils::TranslateKey(aKeyEvent.iCode) == EKeyEnter)
		{
			HandleButtonEventL(EButtonEventClick);
			return EKeyWasConsumed;	
		}
		else
			return EKeyWasNotConsumed;
	}
	else if(iInputType==EButton || iInputType==ESubmit)
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
	else
		return EKeyWasNotConsumed;
}

void CHtmlElementInput::HandleButtonEventL(TInt aButtonEvent)
{
	if(iInputType==EText || iInputType==EPassword)
	{
		if(!iEmbedObject->IsFocused())
			iEmbedObject->SetFocus(ETrue);
	}
	else if(iInputType==ECheckBox) 
	{
		if(aButtonEvent==EButtonEventClick || aButtonEvent==EButtonEventSelect)
		{
			iFlags.Toggle(EChecked);
			iOwner->Impl()->iState.Set(EHCSNeedRefresh);
			
			THtmlCtlEvent event;
			event.iType = THtmlCtlEvent::EOnChanged;
			event.iControl = iOwner;
			event.iElement = this;
			if(aButtonEvent==EButtonEventClick)
				event.iData = 1;
			else
				event.iData = 0;
			iOwner->Impl()->FireEventL(event);
		}
	}
	else if(iInputType==ERadio) 
	{
		if(!iFlags.IsSet(EChecked))
		{
			if(aButtonEvent==EButtonEventClick || aButtonEvent==EButtonEventSelect) 
			{
				THtmlElementIter it(iOwner->Body(), Id(), KHStrInput);
				while(it.Next())
				{
					if(it()!=this && ((CHtmlElementInput*)it())->iInputType==ERadio)
					{
						((CHtmlElementImpl*)it())->iFlags.Clear(EChecked);
					}
				}					
				iFlags.Set(EChecked);
				iOwner->Impl()->iState.Set(EHCSNeedRefresh);
				
				THtmlCtlEvent event;
				event.iType = THtmlCtlEvent::EOnChanged;
				event.iControl = iOwner;
				event.iElement = this;
				if(aButtonEvent==EButtonEventClick)
					event.iData = 1;
				else
					event.iData = 0;
				iOwner->Impl()->FireEventL(event);
			}
		}
	}
	else if(iInputType==EButton || iInputType==ESubmit)
	{
		if(aButtonEvent==EButtonEventClick || aButtonEvent==EButtonEventSelect)
		{
			THtmlCtlEvent event;
			event.iType = THtmlCtlEvent::EOnClick;
			event.iControl = iOwner;
			event.iElement = this;
			if(aButtonEvent==EButtonEventClick)
				event.iData = 1;
			else
				event.iData = 0;
			iOwner->Impl()->FireEventL(event);
			
			if(iInputType==ESubmit)
			{
				CHtmlElementImpl* e = this->iPrev;
				while(e!=iOwner->Body()) {
					if(e->TypeId()==EElementTypeForm) {
						iFlags.Set(EChecked);
						THtmlCtlEvent event;
						event.iType = THtmlCtlEvent::EOnSubmit;
						event.iControl = iOwner;
						event.iElement = e;
						event.iData = 0;
						TRAPD(error,
							iOwner->Impl()->FireEventL(event);
						)
						iFlags.Clear(EChecked);
						User::LeaveIfError(error);
						break;
					}
					e = e->iPrev;
				}
			}
		}
		else if(aButtonEvent==EButtonEventDown) 
		{
			iState.Set(EElementStateDown);
			iOwner->Impl()->iState.Set(EHCSNeedRefresh);
		}
		else if(aButtonEvent==EButtonEventUp)
		{
			iState.Clear(EElementStateDown);
			iOwner->Impl()->iState.Set(EHCSNeedRefresh);
		}
	}
}

