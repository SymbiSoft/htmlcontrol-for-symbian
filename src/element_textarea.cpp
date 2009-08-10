#include <eikenv.h>

#include "controlimpl.h"
#include "measurestatus.h"
#include "htmlparser.h"
#include "inputeditor.h"
#include "utils.h"

#include "element_textarea.h"
#include "element_div.h"

CHtmlElementTextArea::CHtmlElementTextArea(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeTextArea;
	iTagName.Set(KHStrTextArea);
	iRows = 5;
	iLineColor.iRgb = TRgb(0xF7CB94);
	iText = KNullDesC().AllocL();
}

CHtmlElementTextArea::~CHtmlElementTextArea()
{
	iStyle.Close();
	if(iEmbedObject)
	{
		iOwner->Impl()->UnregisterControl(iEmbedObject);
		delete iEmbedObject;
	}
	delete iText;
	delete iTitle;
}

TBool CHtmlElementTextArea::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.CompareF(KHStrValue)==0) 
	{
		HBufC* buf  = ((CHcInputEditor*)iEmbedObject)->GetTextInHBufL();
		if(!buf)
			aBuffer.Zero();
		else 
		{
			TPtr p = buf->Des();
			HcUtils::StringReplace(p, CEditableText::ELineBreak, '\n');
			aBuffer.Copy(buf->Left(aBuffer.MaxLength()));
			delete buf;
		}
	}
	else if(aName.CompareF(KHStrBufSize)==0) 
	{
		aBuffer.Num(((CHcInputEditor*)iEmbedObject)->TextLength());
	}
	else if(aName.CompareF(KHStrClass)==0) 
		aBuffer.Copy(iStyle.Class().Left(aBuffer.MaxLength()));
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
		iStyle.GetSingleStyle(aName.Mid(6), aBuffer);
	else
		return EFalse;

	return ETrue;	
}

TBool CHtmlElementTextArea::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrValue)==0)
	{
		delete iText;
		iText = HcUtils::DecodeHttpCharactersL(aValue);
		
		if(iEmbedObject) 
		{
			SetEditorTextL(*iText);
			((CHcInputEditor*)iEmbedObject)->HandleTextChangedL();
			((CHcInputEditor*)iEmbedObject)->SetCursorPosL(0, EFalse);
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
	else if(aName.CompareF(KHStrRows)==0) 
	{
		iRows = HcUtils::StrToInt(aValue);
	}
	else if(aName.CompareF(KHStrDrawLines)==0) 
	{
		iFlags.Assign(EDrawLines, HcUtils::StrToBool(aValue));
		if(iEmbedObject)
			((CHcInputEditor*)iEmbedObject)->SetDrawLines(iFlags.IsSet(EDrawLines));
	}
	else if(aName.Compare(KHStrLineColor)==0) 
	{
		HtmlParser::ParseColor(aValue, iLineColor);
		if(iEmbedObject)
			((CHcInputEditor*)iEmbedObject)->SetLineColor(iLineColor);
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

CHtmlElementImpl* CHtmlElementTextArea::CloneL()  const
{
	CHtmlElementTextArea* e = new (ELeave)CHtmlElementTextArea(iOwner);
	BaseClone(*e);
	
	e->iStyle.CopyL(iStyle);
	e->iText = (*iText).AllocL();
	if(iTitle)
		e->iTitle = (*iTitle).AllocL();
	e->iRows = iRows;
	e->iLineColor = iLineColor;
	
	return e;
}

void CHtmlElementTextArea::SetEditorTextL(const TDesC& aText)
{
	CPlainText* text = ((CHcInputEditor*)iEmbedObject)->Text();
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
		text->InsertL (text->DocumentLength(), CEditableText::ELineBreak);
		pos1 = pos1+pos2+1;
	}
	if(pos1!=aText.Length())
		text->InsertL (text->DocumentLength(), aText.Mid(pos1));
}

void CHtmlElementTextArea::SetFocus(TBool aFocus) 
{
	CHtmlElementImpl::SetFocus(aFocus);
	
	if(iFlags.IsSet(EDirectFocus) || !aFocus)
		iEmbedObject->SetFocus(aFocus);

	if(!aFocus && ((CEikEdwin*)iEmbedObject)->TextLength()>0)
		((CEikEdwin*)iEmbedObject)->SetCursorPosL(0, EFalse);
}

void CHtmlElementTextArea::PrepareL() 
{
	CHtmlElementImpl::PrepareL();
	
	iStyle.SetTag(iTagName);
	
	CHcInputEditor* editor = new (ELeave) CHcInputEditor(iOwner, ETrue);
	iEmbedObject = editor;
	editor->SetContainerWindowL(*iOwner);
	
#ifdef __SERIES60__
	editor->SetAknEditorFlags(EAknEditorFlagEnableScrollBars  );
#endif
	
	TInt flags = CEikEdwin::ENoAutoSelection | CEikEdwin::ENoHorizScrolling;
	if(iFlags.IsSet(EReadOnly))
		flags |= CEikEdwin::EReadOnly;
#ifdef __UIQ__
		flags |= CEikEdwin::EDisableSkinning;
#endif
		editor->ConstructL( flags,0,0,0);
	
#ifdef __SERIES60__
		editor->EnableCcpuSupportL(ETrue);
#endif
	
	iOwner->Impl()->RegisterControl(((CHcInputEditor*)iEmbedObject));
	
	#ifdef __SERIES60_3_ONWARDS__
	#ifndef __SERIES60_30__
	editor->ScrollBarFrame()->DrawBackground(EFalse, EFalse);
	#endif
	editor->ScrollBarFrame()->VerticalScrollBar()->SetBackground(this);
	#endif
	
#ifdef __UIQ__
	editor->SetBorderViewMargins(TMargins8());
	editor->SetBorder(TGulBorder::ENone);
	editor->SetWordWrapL(ETrue);
	editor->CreateScrollBarFrameL();
	editor->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
	editor->UpdateScrollBarsL();
#endif
	
	editor->SetDrawLines(iFlags.IsSet(EDrawLines));
	editor->SetLineColor(iLineColor);
	SetEditorTextL(*iText);	
	editor->ActivateL();
	
	if(iFlags.IsSet(EDisabled))
		editor->SetDimmed(ETrue);
}

void CHtmlElementTextArea::Draw(CWindowGc& aGc, const CCoeControl& aControl, const TRect& aRect) const
{
	TRect rect = aRect;
	rect.Move(aControl.Position() - iOwner->Rect().iTl);
	aGc.BitBlt(aRect.iTl, iOwner->OffScreenBitmap(), rect);
}

void CHtmlElementTextArea::Measure(CHcMeasureStatus& aStatus)
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
	CFont* font = ((CHcInputEditor*)iEmbedObject)->Font();
	TRect lRect;
	((CHcInputEditor*)iEmbedObject)->TextLayout()->GetLineRect(0, lRect);
	
	TInt width = font->WidthZeroInPixels()*20;
	if(width>iParent->iDisplayRect.Width()-3)
		width = iParent->iDisplayRect.Width()-3;
	TInt height = lRect.Height()*iRows +  lRect.Height() - font->HeightInPixels() + 5;
	DoMeasure(aStatus, iStyle.Style(),TSize(width, height));
}

void CHtmlElementTextArea::Refresh()
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
		
	#ifdef __SERIES60_3_ONWARDS__
		CCoeControl* scrollbar = ((CHcInputEditor*)iEmbedObject)->ScrollBarFrame()->VerticalScrollBar();
		rect.SetSize(TSize(rect.Width() - scrollbar->Size().iWidth, rect.Height()));
	#endif
	    
		if(rect!=iEmbedObject->Rect()) 
		{
			iEmbedObject->SetRect(rect);
			TRect viewRect = ((CHcInputEditor*)iEmbedObject)->TextView()->ViewRect();
			TInt rowHeight = ((CHcInputEditor*)iEmbedObject)->RowHeight();
			TInt lineCount = viewRect.Height()/rowHeight;
			viewRect.SetHeight(lineCount * rowHeight);
			viewRect.Move(0, rect.iTl.iY + rowHeight - ((CHcInputEditor*)iEmbedObject)->Font()->HeightInPixels() - viewRect.iTl.iY - 1);
			((CHcInputEditor*)iEmbedObject)->TextView()->SetViewRect (viewRect);
		}
	}
}

void CHtmlElementTextArea::Draw(CFbsBitGc& aGc) const
{
	if(iFlags.IsSet(EDisabled))
		aGc.SetUserDisplayMode(EGray256);
	
	TRect rect(iPosition, iSize);
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect, iStyle.Style());
	
	if(iState.IsSet(EElementStateFocused))
		DrawFocus(aGc);
	
	if(iFlags.IsSet(EDisabled))
		aGc.SetUserDisplayMode(aGc.Device()->DisplayMode());
}

void CHtmlElementTextArea::DrawFocus(CFbsBitGc& aGc) const
{
	const CHcStyle& focusedStyle = iOwner->Impl()->GetStyle(iTagName, iStyle.Class(), THcSelector::EFCFocus, 0);
	TRect rect2 = Rect();
	TMargins margins = focusedStyle.iMargins.GetMargins(rect2.Size());
	HcUtils::ShrinkRect(rect2, margins);
	HcUtils::DrawBackgroundAndBorders(*iOwner, aGc, rect2, focusedStyle);
}

TKeyResponse CHtmlElementTextArea::OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType) 
{
	TInt keyCode = HcUtils::TranslateKey(aKeyEvent.iCode);
	if(keyCode == EKeyEnter)
	{
		if(!iEmbedObject->IsFocused())
		{
			iEmbedObject->SetFocus(ETrue);
			return EKeyWasConsumed;
		}	
	}
	if(aType==EEventKeyDown && aKeyEvent.iScanCode>32 && aKeyEvent.iScanCode<128)
	{
		if(!iEmbedObject->IsFocused())
			iEmbedObject->SetFocus(ETrue);
	}
	
	if(iEmbedObject->IsFocused())
	{
		TKeyResponse ret = iEmbedObject->OfferKeyEventL(aKeyEvent, aType);
		if(ret==EKeyWasNotConsumed)
		{
			if(!iFlags.IsSet(EDirectFocus) && (keyCode == EKeyUpArrow || keyCode==EKeyDownArrow))
			{
				iEmbedObject->SetFocus(EFalse);
				if(((CEikEdwin*)iEmbedObject)->TextLength()>0)
					((CEikEdwin*)iEmbedObject)->SetCursorPosL(0, EFalse);
				return EKeyWasConsumed;
			}
		}
		
		return ret;
	}
	else
		return EKeyWasNotConsumed;
}

void CHtmlElementTextArea::HandleButtonEventL(TInt aButtonEvent)
{
	if(aButtonEvent==EButtonEventClick || aButtonEvent==EButtonEventSelect)
	{
		iEmbedObject->SetFocus(ETrue);
	}
}


