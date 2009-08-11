#include <Uri16.h>
#include <eikenv.h>

#include "htmlcontrol.hrh"

#ifdef __SERIES60__
#include <aknutils.h>
#include <AknsConstants.hrh>
#include <AknsConstants.h>

#ifdef __SERIES60_3_ONWARDS__
#include <akniconutils.h>
#endif

#endif

#include "controlimpl.h"
#include "elementimpl.h"
#include "htmlctlenv.h"
#include "htmlparser.h"
#include "imagepool.h"
#include "utils.h"

#include "element_div.h"
#include "element_hr.h"
#include "element_img.h"
#include "element_input.h"
#include "element_select.h"
#include "element_a.h"
#include "element_text.h"
#include "element_textarea.h"
#include "element_object.h"
#include "element_p.h"
#include "element_form.h"

_LIT(KHStrComment,"!--");

CHtmlParser* CHtmlParser::NewL()
{
	CHtmlParser* self = new (ELeave)CHtmlParser();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;	
}

CHtmlParser::~CHtmlParser()
{
	iBlocks.Close();
}

void CHtmlParser::ConstructL()
{

}

void CHtmlParser::AppendBrL(TBool aForce)
{
	if(iCurrent->TypeId()!=EElementTypeText || ((CHtmlElementText*)iCurrent)->iStandAlone)
	{
		if(!aForce && iCurrent==iHead)
			return;
		
		AppendElementL(KNullDesC, new (ELeave)CHtmlElementText(iHead->Owner()));
		
		THcTextStyle style = iTextStyle;
		style.Add(iFontStyle);
		if(iFlags & KParserFlagsIgnoreFontSize)
			style.Clear(THcTextStyle::ESize);
		
		((CHtmlElementText*)iCurrent)->AppendTextL(style, NULL);	
	}
	else
	{
		if(!aForce && ((CHtmlElementText*)iCurrent)->IsEndingWithBr())
			return;
		
		((CHtmlElementText*)iCurrent)->AppendTextL(iTextStyle, NULL);
	}
}

void CHtmlParser::AppendTextL(const TDesC& aText, TBool aRaw)
{
	HBufC* text;
	if(!aRaw)
	{
		text = aText.AllocL();
		CleanupStack::PushL(text);
		TPtr textp = text->Des();
		HcUtils::TrimRedundant(textp);
		if(text->Length()>0 && (text->Length()>1 || !(TChar((*text)[0]).IsSpace()))) {
			HBufC* text2 = HcUtils::DecodeHttpCharactersL(*text);
			CleanupStack::PopAndDestroy();
			text = text2;
		}
		else
		{
			CleanupStack::PopAndDestroy();
			return;
		}
	}
	else
		text = aText.Alloc();
	CleanupStack::PushL(text);

	if(iCurrent->TypeId()!=EElementTypeText || ((CHtmlElementText*)iCurrent)->iStandAlone)
		AppendElementL(KNullDesC, new (ELeave)CHtmlElementText(iHead->Owner()));			
			
	THcTextStyle style = iTextStyle;
	style.Add(iFontStyle);
	if(iFlags & KParserFlagsIgnoreFontSize)
		style.Clear(THcTextStyle::ESize);
	
	((CHtmlElementText*)iCurrent)->AppendTextL(style, text);
	CleanupStack::Pop();
}

void CHtmlParser::AppendElementL(const TDesC& aSource, CHtmlElementImpl* aNewElement)
{
	if(iBlocks.Count()==0)
		aNewElement->iParent = iHead->iParent;
	else
		aNewElement->iParent = iBlocks.Top();
	iCurrent->iNext = aNewElement;
	aNewElement->iPrev = iCurrent;
	iCurrent = aNewElement;
	HtmlParser::ParseTag(aSource, iCurrent);
}

void CHtmlParser::RemoveDiv(CHtmlElementDiv* aDiv)
{
	if(aDiv->iStyle.IsInvisibleStyle())
	{
		CHtmlElementImpl* e = aDiv->iNext;
		CHtmlElementImpl* n;
		while(e!=aDiv->iEnd)
		{
			n = e->iNext;
			delete e;
			e = n;
		}
		aDiv->iNext = aDiv->iEnd;
		aDiv->iEnd->iPrev = aDiv;
	}
	else
	{
		CHtmlElementImpl* e = aDiv->iNext;
		while(e!=aDiv->iEnd)
		{
			e->iParent = aDiv->iParent;
			e = e->iNext;
		}
	}
	
	if(aDiv->iNext!=aDiv->iEnd)
	{
		iCurrent = aDiv->iEnd->iPrev;
		iCurrent->iNext = NULL;
		
		aDiv->iPrev->iNext = aDiv->iNext;
		aDiv->iNext->iPrev = aDiv->iPrev;
	}
	else
	{
		iCurrent = aDiv->iPrev;
		iCurrent->iNext = NULL;
	}
	delete aDiv->iEnd;
	delete aDiv;		
}

void CHtmlParser::ParseL(const TDesC& aSource, CHtmlElementImpl* aHead, TInt aFlags)
{
	TInt pos = 0, pos2, length, tagType;
	TBuf<50> name;
	TBuf<50> waitingEndTag;
	TPtrC tagSource;
	TInt skipText = 0;
	CHtmlElementImpl* packElement = NULL;
	CHtmlElementA* link = NULL;
	CHtmlElementForm* form = NULL;
	
	iFlags = aFlags;
	iCurrent = aHead;
	iHead = aHead;
	iBlocks.Reset();
	iTextStyle.ClearAll();
	iFontStyle.ClearAll();
	
	RHcStack<CHtmlElementP*> PStack;
	CleanupClosePushL(PStack);

	while((pos2=HcUtils::EnumTag(aSource, pos, name, length, tagType))!=KErrNotFound) 
	{
		name.LowerCase();
			
		if(waitingEndTag.Length()>0)
		{
			if(tagType==1 && name.Compare(waitingEndTag)==0) 
				waitingEndTag.Zero();
			
			pos = pos2 + length;
			continue;
		}
		
		if(pos!=pos2 && skipText==0)
			AppendTextL(aSource.Mid(pos, pos2 - pos), EFalse);
		
		tagSource.Set(aSource.Mid(pos2, length));
		if(name.Compare(KHStrB)==0 || name.Compare(KHStrStrong)==0)
		{
			if(tagType==0) 
				iTextStyle.Set(THcTextStyle::EBold);	
			else if(tagType==1)
				iTextStyle.Clear(THcTextStyle::EBold);
		}
		else if(name.Compare(KHStrI)==0) 
		{
			if(tagType==0)
				iTextStyle.Set(THcTextStyle::EItalics);
			else
				iTextStyle.Clear(THcTextStyle::EItalics);
		}
		else if(name.Compare(KHStrU)==0)
		{
			if(tagType==0)
				iTextStyle.Set(THcTextStyle::EUnderline);
			else
				iTextStyle.Clear(THcTextStyle::EUnderline);
		}
		else if(name.Compare(KHStrSub)==0)
		{
			if(tagType==0) 
				iTextStyle.Set(THcTextStyle::ESub);	
			else if(tagType==1)
				iTextStyle.Clear(THcTextStyle::ESub);
		}
		else if(name.Compare(KHStrSup)==0)
		{
			if(tagType==0) 
				iTextStyle.Set(THcTextStyle::ESup);	
			else if(tagType==1)
				iTextStyle.Clear(THcTextStyle::ESup);
		}
		else if(name.Compare(KHStrFont)==0)
		{
			if(tagType==0) 
			{
				TInt attrStart = 0, attrLength, valueStart, valueLength;
				TPtrC value;
				while((attrStart = HcUtils::EnumAttribute(tagSource, attrStart, name, attrLength, valueStart, valueLength))!=KErrNotFound) 
				{
					value.Set(tagSource.Mid(valueStart, valueLength));
					if(name.Compare(KHStrColor)==0) 
					{
						iFontStyle.Set(THcTextStyle::EColor);
						HtmlParser::ParseColor(value, iFontStyle.iColor);
					}
					else if(name.Compare(KHStrSize)==0) 
					{
						iFontStyle.Set(THcTextStyle::ESize);
						HtmlParser::ParseFontSize(value, iFontStyle.iSize);
					}
					else if(name.Compare(KHStrStyle)==0) 
						HtmlParser::ParseTextStylesL(value, iFontStyle);
					attrStart+=attrLength;
				}
			}
			else
				iFontStyle.ClearAll();
		}
		else if(name.Compare(KHStrBr)==0) 
		{
			AppendBrL(ETrue);
		}
		else if(name.Compare(KHStrP)==0)
		{
			if(tagType==0)
			{
				TAlign align = ELeft;
				if(HcUtils::GetAttributeValue(aSource.Mid(pos2, length), KHStrAlign, name)!=KErrNotFound
						&& HtmlParser::ParseAlign(name, align)
						&& align!=ELeft)
				{
					AppendElementL(tagSource, new (ELeave)CHtmlElementP(iHead->Owner()));
					((CHtmlElementP*)iCurrent)->iAlign = align;
					PStack.Push((CHtmlElementP*)iCurrent);
				}
				else
				{
					AppendBrL(EFalse);
					PStack.Push(NULL);
				}
			}
			else if(tagType==1)
			{
				if(PStack.Count()>0)
				{
					CHtmlElementP* start = PStack.Pop();
					if(start)
					{
						AppendElementL(tagSource, new (ELeave)CHtmlElementPEnd(iHead->Owner()));
						start->iEnd = (CHtmlElementPEnd*)iCurrent;
					}
				}
				else
					AppendBrL(ETrue);
			}
		}
		else if(name.Compare(KHStrHr)==0) 
		{
			if(tagType==0 || tagType==2) 
				AppendElementL(tagSource, new (ELeave)CHtmlElementHr(iHead->Owner()));
		}
		else if(name.Compare(KHStrImg)==0)
		{
			if(tagType==0 || tagType==2)
				AppendElementL(tagSource, new (ELeave)CHtmlElementImg(iHead->Owner()));
		}
		else if(name.Compare(KHStrA)==0)
		{
			if(tagType==0 || tagType==2)
			{
				if(link)
				{
					AppendElementL(KNullDesC, new (ELeave)CHtmlElementAEnd(iHead->Owner()));	
					link->iEnd = (CHtmlElementAEnd*)iCurrent;
					((CHtmlElementAEnd*)iCurrent)->iStart = link;
					link = NULL;
				}
				
				AppendElementL(tagSource, new (ELeave)CHtmlElementA(iHead->Owner()));
				link = (CHtmlElementA*)iCurrent;
			}
			
			if((tagType==1 || tagType==2) && link)
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementAEnd(iHead->Owner()));	
				link->iEnd = (CHtmlElementAEnd*)iCurrent;
				((CHtmlElementAEnd*)iCurrent)->iStart = link;
				link = NULL;
			}
		}
		else if(name.Compare(KHStrForm)==0)
		{
			if(tagType==0 || tagType==2)
			{
				if(form)
				{
					AppendElementL(KNullDesC, new (ELeave)CHtmlElementFormEnd(iHead->Owner()));	
					form->iEnd = (CHtmlElementFormEnd*)iCurrent;
					((CHtmlElementFormEnd*)iCurrent)->iStart = form;
					form = NULL;
				}
				
				AppendElementL(tagSource, new (ELeave)CHtmlElementForm(iHead->Owner()));
				form = (CHtmlElementForm*)iCurrent;
			}
			
			if((tagType==1 || tagType==2) && form)
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementFormEnd(iHead->Owner()));	
				form->iEnd = (CHtmlElementFormEnd*)iCurrent;
				((CHtmlElementFormEnd*)iCurrent)->iStart = form;
				form = NULL;
			}
		}
		else if(name.Compare(KHStrDiv)==0) 
		{
			if(tagType==0 || tagType==2) 
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementDiv(iHead->Owner()));	
				iBlocks.Push((CHtmlElementDiv*)iCurrent);
			}
			
			if((tagType==1 || tagType==2) && iBlocks.Count()>0)
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementDivEnd(iHead->Owner()));
				CHtmlElementDiv* start = iBlocks.Pop();
				start->iEnd = (CHtmlElementDivEnd*)iCurrent;
				
				if(iFlags & KParserFlagsIgnoreLayout)
				{
					RemoveDiv(start);	
					AppendBrL(EFalse);
				}
			}
		}
		else if(name.Compare(KHStrSpan)==0) 
		{
			/*if(!(iFlags & KParserFlagsIgnoreLayout)) 
			{
				if(tagType==0 || tagType==2) 
				{
					AppendElementL(tagSource, new (ELeave)CHtmlElementSpan(iHead->Owner()));	
					iBlocks.Push((CHtmlElementDiv*)iCurrent);
				}
				
				if((tagType==1 || tagType==2) && iBlocks.Count()>0)
				{
					AppendElementL(tagSource, new (ELeave)CHtmlElementSpanEnd(iHead->Owner()));
					CHtmlElementDiv* start = iBlocks.Pop();
					start->iEnd = (CHtmlElementDivEnd*)iCurrent;
				}				
			}*/
			
			//ignore
		}
		else if(name.Compare(KHStrBlockQuote)==0)
		{
			if(tagType==0 || tagType==2) 
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementDiv(iHead->Owner()));	
				iBlocks.Push((CHtmlElementDiv*)iCurrent);
				iCurrent->SetProperty(KHStrClass, KHStrBlockQuote);
			}
			
			if((tagType==1 || tagType==2) && iBlocks.Count()>0)
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementDivEnd(iHead->Owner()));
				CHtmlElementDiv* start = iBlocks.Pop();
				start->iEnd = (CHtmlElementDivEnd*)iCurrent;
			}
		}		
		else if(name.Compare(KHStrLi)==0 || name.Compare(KHStrTr)==0)
		{
			if(tagType==1)
				AppendBrL(EFalse);
		}
		else if(name.Compare(KHStrTd)==0)
		{
			if(tagType==1)
				AppendTextL(_L("  "), ETrue);
		}
		else if(name.Compare(KHStrInput)==0)
		{
			if(tagType==0 || tagType==2) 
				AppendElementL(tagSource, new (ELeave)CHtmlElementInput(iHead->Owner()));
		}
		else if(name.Compare(KHStrSelect)==0)
		{
			if(tagType==0 || tagType==2) 
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementSelect(iHead->Owner()));
				if(tagType!=2) 
				{
					packElement = iCurrent;
					skipText++;		
				}
			}
			else if(tagType==1) 
			{
				if(packElement) 
				{
					skipText--;
					packElement = NULL;
				}
			}
		}
		else if(name.Compare(KHStrOption)==0)
		{
			if(packElement && packElement->TypeId()==EElementTypeSelect)
			{
				CHtmlElementSelect* select = (CHtmlElementSelect*)packElement;
				if(tagType==0 || tagType==2) 
				{
					if(HcUtils::GetAttributeValue(aSource.Mid(pos2, length), KHStrValue, name)!=KErrNotFound) 
						select->ValueArray()->AppendL(name);
					else
						select->ValueArray()->AppendL(KNullDesC);
					
					if(HcUtils::GetAttributeValue(aSource.Mid(pos2, length), KHStrSelected, name)!=KErrNotFound)
						select->iSelected = select->ValueArray()->Count()-1;
				}
				
				if(tagType==1 || tagType==2) 
				{
					HBufC* text = HcUtils::DecodeHttpCharactersL(aSource.Mid(pos, pos2 - pos));
					CleanupStack::PushL(text);
					text->Des().TrimAll();
					select->TextArray()->AppendL(*text);
					CleanupStack::PopAndDestroy();
				}
			}
		}
		else if(name.Compare(KHStrEmbed)==0 || name.Compare(KHStrObject)==0)
		{
			if(tagType==0 || tagType==2)
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementObject(iHead->Owner()));
				if(tagType==0)
				{
					packElement = iCurrent;
					skipText++;
				}
			}
			else if(tagType==1) 
			{
				if(packElement && packElement->TypeId()==EElementTypeObject)
				{
					CHtmlElementObject* object = (CHtmlElementObject*)packElement;
					HBufC* text = HcUtils::DecodeHttpCharactersL(aSource.Mid(pos, pos2 - pos));
					text->Des().TrimAll();
					if(text->Length()>0)
						object->AppendAlterTextL(text);
					else
						delete text;
				}
				
				skipText--;
				packElement = NULL;
			}
		}
		else if(name.Compare(KHStrParam)==0)
		{
			if(packElement && packElement->TypeId()==EElementTypeObject)
			{
				CHtmlElementObject* object = (CHtmlElementObject*)packElement;
				if(tagType==0 || tagType==2) 
				{
					TPtrC p1, p2;
					if(HcUtils::GetAttributeValue(aSource.Mid(pos2, length), KHStrName, p1)!=KErrNotFound) 
					{
						if(HcUtils::GetAttributeValue(aSource.Mid(pos2, length), KHStrValue, p2)!=KErrNotFound)
						{
							HBufC* name = HcUtils::DecodeHttpCharactersL(p1);
							CleanupStack::PushL(name);
							HBufC* value = HcUtils::DecodeHttpCharactersL(p2);
							CleanupStack::PushL(value);
							
							name->Des().LowerCase();
							object->SetProperty(*name, *value);
							
							CleanupStack::PopAndDestroy(2);
						}
					}
					
					HBufC* text = HcUtils::DecodeHttpCharactersL(aSource.Mid(pos, pos2 - pos));
					text->Des().TrimAll();
					if(text->Length()>0)
						object->AppendAlterTextL(text);
					else
						delete text;
				}
			}
		}
		else if(name.Compare(KHStrTextArea)==0)
		{
			if(tagType==0 || tagType==2) 
			{
				AppendElementL(tagSource, new (ELeave)CHtmlElementTextArea(iHead->Owner()));
				if(tagType==0)
				{
					packElement = iCurrent;
					skipText++;
				}
			}
			else if(tagType==1) 
			{
				if(packElement) 
				{
					packElement->SetProperty(KHStrValue, aSource.Mid(pos, pos2 - pos));
					skipText--;
					packElement = NULL;
				}
			}
		}
		else if(name.Compare(KHStrText)==0)
		{
			if(tagType==0 || tagType==2) 
			{
				if(HcUtils::GetAttributeValue(tagSource, KHStrId, name)!=KErrNotFound) 
				{
					CHtmlElementText* e = new (ELeave)CHtmlElementText(iHead->Owner());
					e->iStandAlone = ETrue;
					AppendElementL(tagSource,  e);
				}
				else
				{
					TPtrC res;
					if(HcUtils::GetAttributeValue(tagSource, KHStrRes, res)!=KErrNotFound)
					{
						TInt resId = HcUtils::StrToInt(res, EDecimal);
				#ifdef __SYMBIAN_9_ONWARDS__
						if(CEikonEnv::Static()->IsResourceAvailableL(resId)) 
				#else
						if(ETrue)
				#endif
						{
							HBufC* text = CEikonEnv::Static()->AllocReadResourceLC(resId);
							AppendTextL(*text, ETrue);
							CleanupStack::PopAndDestroy();
						}
					}
				}
			}
		}
		else if(name.Compare(KHStrBody)==0)
		{
			if(tagType==0) 
			{
				iBlocks.Reset();
				PStack.Reset();
				link = NULL;
				packElement = NULL;
				waitingEndTag.Zero();
				skipText = 0;
				if(iCurrent!=aHead) 
				{
					CHtmlElementImpl* e = aHead->iNext;
					CHtmlElementImpl* n;
					while(ETrue) 
					{
						n = e->iNext;
						delete e;
						if(e==iCurrent)
							break;
						e = n;
					}
				}
				iCurrent = aHead;
				HtmlParser::ParseTag(tagSource, (CHtmlElementImpl*)aHead->Owner()->Body());
			}
			else
			{
				pos = aSource.Length();
				break;
			}
		}
		else if(name.Compare(KHStrStyle)==0)
		{
			if(tagType==0)
			{
				skipText++;
			}
			else if(tagType==1) 
			{
				HBufC* text = HcUtils::DecodeHttpCharactersL(aSource.Mid(pos, pos2 - pos));
				CleanupStack::PushL(text);
				aHead->Owner()->AddStyleSheetL(*text);
				CleanupStack::PopAndDestroy();
				skipText--;
			}
		}
		else if(name.Compare(KHStrHead)==0)
		{
			if(tagType==0)
				skipText++;
			else if(tagType==1)
				skipText--;
		}
		else
		{
			#ifdef __WINSCW__
			RDebug::Print(_L("CHtmlParser: ignore tag %S"), &name);
			#endif
			
			//if(name.Compare(KHStrComment)==0) //comment
			if(name.Compare(KHStrScript)==0)
			{
				if(tagType==0) 
					waitingEndTag.Copy(name);
			}
		}
		pos = pos2 + length;
	}
	
	if(pos!=aSource.Length()) 
		AppendTextL(aSource.Mid(pos), EFalse);
	
	while(iBlocks.Count()>0)
	{
		AppendElementL(KNullDesC, new (ELeave)CHtmlElementDivEnd(iHead->Owner()));
		CHtmlElementDiv* start = iBlocks.Pop();
		start->iEnd = (CHtmlElementDivEnd*)iCurrent;
		
		if(iFlags & KParserFlagsIgnoreLayout)
			RemoveDiv(start);
	}
	
	while(PStack.Count()>0)
	{
		CHtmlElementP* start = PStack.Pop();
		if(start)
		{
			AppendElementL(KNullDesC, new (ELeave)CHtmlElementPEnd(iHead->Owner()));
			start->iEnd = (CHtmlElementPEnd*)iCurrent;
		}
	}
	
	if(link)
	{
		link->iPrev->iNext = link->iNext;
		if(link->iNext)
			link->iNext->iPrev = link->iPrev;
		if(link==iCurrent)
			iCurrent = link->iPrev;
		delete link;
	}
	
	if(form)
	{
		form->iPrev->iNext = form->iNext;
		if(form->iNext)
			form->iNext->iPrev = form->iPrev;
		if(form==iCurrent)
			iCurrent = form->iPrev;
		delete form;
	}
	
	iCurrent->iNext = aHead;
	aHead->iPrev = iCurrent;
	
	//validation
	iCurrent = aHead->iNext;
	CHtmlElementImpl* end;
	while(iCurrent!=aHead)
	{
		if(iCurrent->TypeId()==EElementTypeA)
			end = ((CHtmlElementA*)iCurrent)->iEnd;
		else if(iCurrent->TypeId()==EElementTypeForm)
			end = ((CHtmlElementForm*)iCurrent)->iEnd;
		else if(iCurrent->TypeId()==EElementTypeP)
			end =((CHtmlElementP*)iCurrent)->iEnd;
		else
			end = NULL;
		
		if(end && iCurrent->iParent!=end->iParent)
		{
			iCurrent->iPrev->iNext = iCurrent->iNext;
			iCurrent->iNext->iPrev = iCurrent->iPrev;
			
			end->iPrev->iNext = end->iNext;
			end->iNext->iPrev = end->iPrev;
			
			delete iCurrent;
			iCurrent = end->iNext;
			delete end;
		}
		else
			iCurrent = iCurrent->iNext;
	}
	
	CleanupStack::PopAndDestroy();
}

void HtmlParser::ParseTag(const TDesC& aSource, CHtmlElementImpl* aElement)
{
	TBuf<50> name;

	TInt attrStart = 0, attrLength, valueStart, valueLength;
	while((attrStart = HcUtils::EnumAttribute(aSource, attrStart, name, attrLength, valueStart, valueLength))!=KErrNotFound) 
	{
		name.LowerCase();
		HBufC* value = HcUtils::DecodeHttpCharactersL(aSource.Mid(valueStart, valueLength));

		if(!aElement->SetProperty(name, *value)) 
		{
			#ifdef __WINSCW__
			//RDebug::Print(_L("CHtmlParser: ignore property %S"), &name);
			#endif
		}
		delete value;
		attrStart+=attrLength;
	}
}

void HtmlParser::ParseLength(const TDesC& aSource, THcLength& aLength, TBool aSupportRelative) 
{
	if(aSource[0]=='%') 
	{
		aLength.iValueType = THcLength::ELenVTPercent;
		TLex lex = aSource.Mid(1);
		lex.Val(aLength.iValue);
	}
	else if(aSource[aSource.Length()-1]=='%')
	{
		aLength.iValueType = THcLength::ELenVTPercent;
		TLex lex = aSource.Mid(0, aSource.Length()-1);
		lex.Val(aLength.iValue);
	}
	else if(aSupportRelative && (aSource[0]=='+' || aSource[0]=='-'))
	{
		aLength.iValueType = THcLength::ELenVTRelative;
		TLex lex = aSource;
		lex.Val(aLength.iValue);
	}
	else if(aSource.CompareF(KHStrAuto)==0)
	{
		aLength.iValueType = THcLength::ELenVTAuto;
		aLength.iValue = 0;
	}
	else
	{
		aLength.iValueType = THcLength::ELenVTAbsolute;
		TLex lex = aSource;
		lex.Val(aLength.iValue);
	}
}

void HtmlParser::ParseHexColor(const TDesC& aSource, TRgb& aRgb)
{
	TBuf<6> s;
	s.Copy(aSource.Right(6));
	for(TInt i=s.Length();i<6;i++)
		s.Append('0');
	aRgb.SetRed(HcUtils::StrToInt(s.Mid(0,2), EHex));
	aRgb.SetGreen(HcUtils::StrToInt(s.Mid(2,2), EHex));
	aRgb.SetBlue(HcUtils::StrToInt(s.Mid(4,2), EHex));
	
#ifdef __SYMBIAN_9_ONWARDS__
	if(aSource.Length()>=8)
	{
		TUint32 alpha = HcUtils::StrToInt(aSource.Mid(0,2), EHex);
		aRgb.SetAlpha(alpha);
	}
#endif	
}

void HtmlParser::ParseColor(const TDesC& aSource, TRgb& aRgb)
{
	if(aSource.Left(1).Compare(_L("#"))==0)
		ParseHexColor(aSource.Mid(1), aRgb);
	else 
	{
		if(aSource.CompareF(KHStrBlack)==0)
			aRgb = KRgbBlack;
		else if(aSource.CompareF(KHStrRed)==0)
			aRgb = KRgbRed;
		else if(aSource.CompareF(KHStrGreen)==0)
			aRgb = KRgbGreen;
		else if(aSource.CompareF(KHStrBlue)==0)
			aRgb = KRgbBlue;
		else if(aSource.CompareF(KHStrYellow)==0)
			aRgb = KRgbYellow;
		else if(aSource.CompareF(KHStrMagenta)==0)
			aRgb = KRgbMagenta;
		else if(aSource.CompareF(KHStrCyan)==0)
			aRgb = KRgbCyan;
		else if(aSource.CompareF(KHStrGray)==0)
			aRgb = KRgbGray;
		else if(aSource.CompareF(KHStrWhite)==0)
			aRgb = KRgbWhite;
		else
			aRgb = KRgbBlack;
	}
}

void HtmlParser::ParseColor(const TDesC& aSource, THcColor& aResult)
{
	_LIT(K, "skin#");
	if(aSource.CompareF(KHStrNone)==0)
	{
		aResult.iIndex1 = -1;
		aResult.iIndex2 = -1;
	}
	
#ifdef __SERIES60__
	else if(aSource.Left(5).CompareF(K)==0)
	{
		TInt offset = 0;
		TPtrC p;
		TInt i=0;
		TInt param1 = -1, param2 = -1, param3 = -1;
		TPtrC source = aSource.Mid(5);
		while(HcUtils::StringSplit(source, offset, ',' , p)) 
		{
			HcUtils::Trim(p);
			if(i==0)
				param1 = HcUtils::StrToInt(p, EHex);
			else if(i==1) 
				param2 = HcUtils::StrToInt(p, EHex);
			else if(i==2) 
				param3 = HcUtils::StrToInt(p, EHex);
			else
				break;
			i++;
		}
		if(i==1) 
		{
			aResult.iIndex1 = EAknsMajorSkin;
#ifdef __SYMBIAN_8_ONWARDS__
			aResult.iIndex2 = EAknsMinorQsnTextColors;
#else
			aResult.iIndex2 = EAknsMinorQsnComponentColors;
#endif
			aResult.iRgb = TRgb(param1);
		}
		else if(i==2) 
		{
			aResult.iIndex1 = EAknsMajorSkin;
			aResult.iIndex2 = param1;
			aResult.iRgb = TRgb(param2);
		}
		else
		{
			aResult.iIndex1 = param1;
			aResult.iIndex2 = param2;
			aResult.iRgb = TRgb(param3);
		}
	}
#endif
		
#ifdef __UIQ__
	else if(aSource.Left(5).CompareF(K)==0)
	{
		TInt offset = 0;
		TPtrC p;
		TInt i=0;
		TInt param1 = -1, param2 = -1, param3 = -1;
		TPtrC source = aSource.Mid(5);
		while(HcUtils::StringSplit(source, offset, ',' , p)) 
		{
			HcUtils::Trim(p);
			if(i==0)
				param1 = HcUtils::StrToInt(p, EHex);
			else if(i==1) 
				param2 = HcUtils::StrToInt(p, EHex);
			else if(i==2) 
				param3 = HcUtils::StrToInt(p, EHex);
			else
				break;
			i++;
		}
		if(i==1) 
		{
			aResult.iIndex1 = param1;
			aResult.iIndex2 = 0;
		}
		else
		{
			aResult.iIndex1 = param1;
			aResult.iIndex2 = param2;
		}
		return;
	}
#endif
	else
	{
		aResult.iIndex1 = 0;
		aResult.iIndex2 = 0;
		ParseColor(aSource, aResult.iRgb);
	}
}

void HtmlParser::ParseFontSize(const TDesC& aSource, TInt& aSize)
{
	if(aSource[0]=='+')
		aSize = CHtmlCtlEnv::Static()->BaseFontSize() + HcUtils::StrToInt(aSource.Mid(1));
	else if(aSource[0]=='-')
		aSize = CHtmlCtlEnv::Static()->BaseFontSize() - HcUtils::StrToInt(aSource.Mid(1));
	else
	{
		aSize = HcUtils::StrToInt(aSource);	
		if(aSize==0)
		{
			if(aSource.CompareF(KHStrMedium)==0)
				aSize = CHtmlCtlEnv::Static()->BaseFontSize(0);
			else if(aSource.CompareF(KHStrSmall)==0)
				aSize = CHtmlCtlEnv::Static()->BaseFontSize(-1);
			else if(aSource.CompareF(KHStrXSmall)==0)
				aSize = CHtmlCtlEnv::Static()->BaseFontSize(-2);
			else if(aSource.CompareF(KHStrXXSmall)==0)
				aSize = CHtmlCtlEnv::Static()->BaseFontSize(-3);
			else if(aSource.CompareF(KHStrLarge)==0)
				aSize = CHtmlCtlEnv::Static()->BaseFontSize(1);
			else if(aSource.CompareF(KHStrXLarge)==0)
				aSize = CHtmlCtlEnv::Static()->BaseFontSize(2);
			else if(aSource.CompareF(KHStrXXLarge)==0)
				aSize = CHtmlCtlEnv::Static()->BaseFontSize(3);
		}
	}
}

TBool HtmlParser::ParseAlign(const TDesC& aSource, TAlign& aAlign)
{
	if(aSource.CompareF(KHStrLeft)==0) 
		aAlign = ELeft;
	else if(aSource.CompareF(KHStrRight)==0) 
		aAlign = ERight;
	else if(aSource.CompareF(KHStrMiddle)==0 || aSource.CompareF(KHStrCenter)==0) 
		aAlign = ECenter;
	else
		return EFalse;
	return ETrue;
}

TBool HtmlParser::ParseVAlign(const TDesC& aSource, TVAlign& aAlign)
{
	if(aSource.CompareF(KHStrTop)==0) 
		aAlign = EVTop;
	else if(aSource.CompareF(KHStrMiddle)==0 || aSource.CompareF(KHStrCenter)==0) 
		aAlign = EVCenter;
	else if(aSource.CompareF(KHStrBottom)==0) 
		aAlign = EVBottom;
	else
		return EFalse;
	return ETrue;
}

void HtmlParser::ParseBorder(const TDesC& aSource, THcBorder& aBorder)
{
	aBorder.iStyle = THcBorder::ESolid;
	aBorder.iWidth = 1;
	TInt offset = 0;
	TPtrC p;
	TInt i=0;
	while(HcUtils::StringSplit(aSource, offset, ' ' , p)) 
	{
		HcUtils::Trim(p);
		if(p.Length()==0)
			continue;
		
		if(i==0)
			aBorder.iWidth = HcUtils::StrToInt(p);
		else if(i==1) 
		{
			if(p.CompareF(KHStrNone)==0)
				aBorder.iStyle = THcBorder::ENone;
			else if(p.CompareF(KHStrDotted)==0)
				aBorder.iStyle = THcBorder::EDotted;
			else if(p.CompareF(KHStrDashed)==0)
				aBorder.iStyle = THcBorder::EDashed;
			else if(p.CompareF(KHStrOutset)==0)
				aBorder.iStyle = THcBorder::EOutset;
			else if(p.CompareF(KHStrInset)==0)
				aBorder.iStyle = THcBorder::EInset;
			else
				aBorder.iStyle = THcBorder::ESolid;
		}
		else if(i==2) 
		{
			ParseColor(p, aBorder.iColor);
		}
		else
			break;
		i++;
	}
}

void HtmlParser::ParseBorderWidth(const TDesC& aSource, THcBorders& aBorders)
{
	TInt offset = 0;
	TPtrC p;
	TInt i=0;
	while(HcUtils::StringSplit(aSource, offset, ' ' , p)) 
	{
		HcUtils::Trim(p);
		if(p.Length()==0)
			continue;
		
		if(i==0)
			aBorders.iTop.iWidth = HcUtils::StrToInt(p);
		else if(i==1) 
			aBorders.iRight.iWidth = HcUtils::StrToInt(p);
		else if(i==2) 
			aBorders.iBottom.iWidth = HcUtils::StrToInt(p);
		else if(i==3)
			aBorders.iLeft.iWidth = HcUtils::StrToInt(p);
		else
			break;
		i++;
	}
	
	if(i==1) 
	{
		aBorders.iRight.iWidth = aBorders.iTop.iWidth;
		aBorders.iBottom.iWidth = aBorders.iTop.iWidth;
		aBorders.iLeft.iWidth = aBorders.iTop.iWidth;	
	}
	else if(i==2) 
	{
		aBorders.iBottom.iWidth = aBorders.iTop.iWidth;
		aBorders.iLeft.iWidth = aBorders.iTop.iWidth;
	}
	else if(i==3) 
	{
		aBorders.iLeft.iWidth = aBorders.iTop.iWidth;
	}
	aBorders.iMask.SetAll();
}

void HtmlParser::ParseBorderColor(const TDesC& aSource, THcBorders& aBorders)
{
	TInt offset = 0;
	TPtrC p;
	TInt i=0;
	while(HcUtils::StringSplit(aSource, offset, ' ' , p)) 
	{
		HcUtils::Trim(p);
		if(p.Length()==0)
			continue;
		
		if(i==0)
			ParseColor(p,aBorders.iTop.iColor);
		else if(i==1) 
			ParseColor(p,aBorders.iRight.iColor);
		else if(i==2) 
			ParseColor(p,aBorders.iBottom.iColor);
		else if(i==3)
			ParseColor(p,aBorders.iLeft.iColor);
		else
			break;
		i++;
	}
	
	if(i==1) 
	{
		aBorders.iRight.iColor = aBorders.iTop.iColor;
		aBorders.iBottom.iColor = aBorders.iTop.iColor;
		aBorders.iLeft.iColor = aBorders.iTop.iColor;	
	}
	else if(i==2) 
	{
		aBorders.iBottom.iColor = aBorders.iTop.iColor;
		aBorders.iLeft.iColor = aBorders.iRight.iColor;
	}
	else if(i==3) 
	{
		aBorders.iLeft.iColor = aBorders.iRight.iColor;
	}	
	aBorders.iMask.SetAll();
}

void HtmlParser::ParseMargins(const TDesC& aSource, THcMargins& aMargins)
{
	TInt offset = 0;
	TPtrC p;
	TInt i=0;
	while(HcUtils::StringSplit(aSource, offset, ' ' , p)) 
	{
		HcUtils::Trim(p);
		if(p.Length()==0)
			continue;
		
		if(i==0)
			ParseLength(p, aMargins.iTop);
		else if(i==1)
			ParseLength(p, aMargins.iRight);
		else if(i==2) 
			ParseLength(p, aMargins.iBottom);
		else if(i==3)
			ParseLength(p, aMargins.iLeft);
		else
			break;
		i++;
	}
	
	if(i==1) 
	{
		aMargins.iRight = aMargins.iTop;
		aMargins.iBottom = aMargins.iTop;
		aMargins.iLeft = aMargins.iTop;
	}
	else if(i==2) 
	{
		aMargins.iBottom = aMargins.iTop;
		aMargins.iLeft = aMargins.iRight;
	}
	else if(i==3) 
	{
		aMargins.iLeft = aMargins.iRight;
	}
	aMargins.iMask.SetAll();
}

void HtmlParser::ParseFontFamily(const TDesC& aSource, TInt& aFontFamily)
{
	if(aSource.Length()==0 || aSource.CompareF(KHStrDense)==0)
		aFontFamily = 0;
	else if(aSource.CompareF(KHStrAnnotation)==0)
		aFontFamily = 1;
	else if(aSource.CompareF(KHStrTitle)==0)
		aFontFamily = 2;
	else if(aSource.CompareF(KHStrLegend)==0)
		aFontFamily = 3;
	else if(aSource.CompareF(KHStrSymbol)==0)
		aFontFamily = 4;
	else
	{
		CDesCArray& fontFamilies = CHtmlCtlEnv::Static()->FontFamilies();
		for(TInt i=0;i<fontFamilies.Count();i++)
		{
			if(fontFamilies[i].CompareF(aSource)==0)
			{
				aFontFamily = 10+i;
				return;
			}
		}
		aFontFamily = 0;
	}
}

void HtmlParser::ParseFilter(const TDesC& aSource, CHcStyle& aStyle)
{
	TPtrC name, params;
	TInt pos = aSource.Locate('(');
	if(pos==KErrNotFound) 
		name.Set(aSource);
	else 
	{
		name.Set(aSource.Mid(0, pos));
		pos++;
		
		TInt pos2 = aSource.Mid(pos).Locate(')');
		if(pos2!=KErrNotFound)
			params.Set(aSource.Mid(pos, pos2));
	}
		
	HcUtils::Trim(name);
		
	_LIT(KAlpha, "alpha");
	_LIT(KOpacity, "opacity");
	_LIT(KScale9Grid, "scale9Grid");

	_LIT(KFade, "fade");
	_LIT(KCorner, "corner");
					
	TInt offset = 0;
	TPtrC param, value;
	if(name.CompareF(KAlpha)==0)
	{
		while(HcUtils::StringSplit(params, offset, ',' , param)) 
		{
			HcUtils::Trim(param);
			pos = param.Locate('=');			
			if(pos!=KErrNotFound) 
			{
				name.Set(param.Mid(0, pos));
				value.Set(param.Mid(pos+1));
				HcUtils::Trim(name);
				HcUtils::Trim(value);
				
				if(name.CompareF(KOpacity)==0) 
				{
					aStyle.Set(CHcStyle::EOpacity);
					aStyle.iOpacity = HcUtils::StrToInt(value)*255/100;
				}
			}
		}
	}
	else if(name.CompareF(KFade)==0)
	{
		while(HcUtils::StringSplit(params, offset, ',' , param)) 
		{
			HcUtils::Trim(param);
			pos = param.Locate('=');			
			if(pos!=KErrNotFound)
			{
				name.Set(param.Mid(0, pos));
				value.Set(param.Mid(pos+1));
				HcUtils::Trim(name);
				HcUtils::Trim(value);
				
				if(name.CompareF(KHStrEnabled)==0) 
				{
					if(value.Length()>0)
					{
						aStyle.Set(CHcStyle::EFaded);
						if(HcUtils::StrToBool(value))
							aStyle.iFaded = ETrue;
						else
							aStyle.iFaded = EFalse;
					}
					else
						aStyle.Clear( CHcStyle::EFaded);
				}
					
			}
		}	
	}
	else if(name.CompareF(KCorner)==0)
	{
		TInt pos = params.Locate(',');
		if(pos!=KErrNotFound) 
		{
			aStyle.iCorner.iWidth = HcUtils::StrToInt(params.Mid(0, pos));
			aStyle.iCorner.iHeight =  HcUtils::StrToInt(params.Mid(pos+1));
		}
		else 
		{
			aStyle.iCorner.iWidth = HcUtils::StrToInt(params);
			aStyle.iCorner.iHeight =  aStyle.iCorner.iWidth;
		}
		
		if(aStyle.iCorner.iWidth==0 && aStyle.iCorner.iHeight==0)
			aStyle.Clear(CHcStyle::ECorner);
		else
			aStyle.Set(CHcStyle::ECorner);
	}
	else if(name.CompareF(KScale9Grid)==0)
	{
		aStyle.Set(CHcStyle::EScale9Grid);
		TInt i = 0;
		while(HcUtils::StringSplit(params, offset, ',' , param)) 
		{
			HcUtils::Trim(param);
			if(i==0)
				aStyle.iScale9Grid.iTl.iX = HcUtils::StrToInt(param);
			else if(i==1)
				aStyle.iScale9Grid.iTl.iY = HcUtils::StrToInt(param);
			else if(i==2)
				aStyle.iScale9Grid.iBr.iX = HcUtils::StrToInt(param);
			else
			{
				aStyle.iScale9Grid.iBr.iY = HcUtils::StrToInt(param);
				break;
			}
			i++;
		}
	}
}

void HtmlParser::ParseImageSrc(const TDesC& aURL, THcImageLocation& aLocation)
{
	TUriParser16 parser;
	parser.Parse(aURL);
	
	const TDesC& scheme = parser.Extract(EUriScheme);
	if(scheme.CompareF(_L("file"))==0) 
	{
		const TDesC& path = parser.Extract( EUriPath );
		if(path.Length()>0)
		{
			aLocation.iFileName.Copy(path.Mid(1));
			if(aLocation.iFileName.Length()>0)
			{
				HcUtils::StringReplace(aLocation.iFileName, '/', '\\');
				if(aLocation.iFileName[1]!=':')
					aLocation.iFileName.Insert(1, _L(":"));
			}
		}
		else
			aLocation.iFileName.Zero();
	}
	else
		aLocation.iFileName.Copy(parser.Extract( EUriPath ));

	TPtrC param1, param2, param3, param4;
	const TDesC& frag = parser.Extract(EUriFragment);
	TInt i=0;
	if(frag.Length()>0)
	{
		TInt offset = 0;
		TPtrC p;
		while(HcUtils::StringSplit(frag, offset, ',' , p)) 
		{
			if(i==0)
				param1.Set(p);
			else if(i==1)
				param2.Set(p);
			else if(i==2)
				param3.Set(p);
			else if(i==3)
				param4.Set(p);
			else
				break;
			i++;
		}
	}
	
	_LIT(KAppIcon, "appicon");
	_LIT(KGradient, "gradient");
	_LIT(KSkin, "skin");
	_LIT(KIcon, "icon");
#ifdef __SERIES60__
	_LIT(KFrame, "frame");
#endif
	
	if(aLocation.iFileName.CompareF(KAppIcon)==0) 
	{
		aLocation.iType = ELTAppIcon;
		if(i==1)
		{
			aLocation.iIndex1 = HcUtils::StrToInt(param1, EHex);
			aLocation.iIndex2 = 0;
		}
		else
		{
			aLocation.iIndex1 = HcUtils::StrToInt(param1, EHex);
			aLocation.iIndex2 = HcUtils::StrToInt(param2, EHex);
		}
		aLocation.iValid = ETrue;
	}
	else if(aLocation.iFileName.CompareF(KGradient)==0) 
	{
		aLocation.iType = ELTGradient;
		aLocation.iFileName.Zero();
		
		TRgb rgb;
		ParseHexColor(param1, rgb);
		aLocation.iIndex1 = rgb.Value();
		ParseHexColor(param2, rgb);
		aLocation.iIndex2 = rgb.Value();

		if(i>2)
			aLocation.iIndex3 = HcUtils::StrToInt(param3, EHex);
		else
			aLocation.iIndex3 = 0;
		aLocation.iValid = ETrue;
	}
	else if(aLocation.iFileName.CompareF(KSkin)==0) 
	{
		aLocation.iType = ELTSkin;
		aLocation.iValid = ETrue;
#ifdef __SERIES60__
		if(i==0)
		{
			aLocation.iIndex1 = KAknsIIDQsnBgScreen.iMajor;
			aLocation.iIndex2 = KAknsIIDQsnBgScreen.iMinor;
		}
		else if(i==1) 
		{
			aLocation.iIndex1 = EAknsMajorSkin;
			aLocation.iIndex2 = HcUtils::StrToInt(param1, EHex);
			if(aLocation.iIndex2==0)
				aLocation.iIndex2 = EAknsMinorQsnBgScreen;
		}
		else
		{
			aLocation.iIndex1 = HcUtils::StrToInt(param1, EHex);
			aLocation.iIndex2 = HcUtils::StrToInt(param2, EHex);
		}
#endif

#ifdef __UIQ__
		aLocation.iIndex1 = HcUtils::StrToInt(param1, EHex);
		aLocation.iIndex2 = i>1?HcUtils::StrToInt(param2, EHex):0;
		aLocation.iIndex3 = i>2?HcUtils::StrToInt(param3, EHex):0;
		_LIT(KFormat, "uiq_themed:%x?frame=%i&index=%i");
		aLocation.iFileName.Format(KFormat, aLocation.iIndex1, aLocation.iIndex2, aLocation.iIndex3);
#endif		
	}

#ifdef __SERIES60__
	else if(aLocation.iFileName.CompareF(KFrame)==0)
	{
		aLocation.iType = ELTFrame;
		aLocation.iValid = ETrue;
		/*
		1 | minor
		1,2 | major,minor
		*/
		if(i==1)
		{
			aLocation.iIndex1 = EAknsMajorSkin;
			aLocation.iIndex2 = HcUtils::StrToInt(param1, EHex);
		}
		else if(i==2)
		{
			aLocation.iIndex1 = HcUtils::StrToInt(param1, EHex);
			aLocation.iIndex2 = HcUtils::StrToInt(param2, EHex);
		}
	}
#endif
	
	else if(aLocation.iFileName.CompareF(KIcon)==0) 
	{
		aLocation.iType = ELTIcon;
		aLocation.iValid = ETrue;
#ifdef __SERIES60__
		if(i==1)
		{
			aLocation.iIndex1 = EAknsMajorSkin;
			aLocation.iIndex2 = HcUtils::StrToInt(param1, EHex);
		}
		else
		{
			aLocation.iIndex1 = HcUtils::StrToInt(param1, EHex);
			aLocation.iIndex2 = HcUtils::StrToInt(param2, EHex);
		}
#endif
		

#ifdef __UIQ__
		aLocation.iIndex1 = HcUtils::StrToInt(param1, EHex);
		_LIT(KFormat, "uiq_icon:%x?size=large");
		aLocation.iFileName.Format(KFormat, aLocation.iIndex1);
#endif
	}
	else
	{
		TPtrC ext;
		TInt pos = aLocation.iFileName.LocateReverse('.');
		if(pos!=KErrNotFound) 
			ext.Set(aLocation.iFileName.Mid(pos+1));

		if(ext.CompareF(_L("mif"))==0 || ext.CompareF(_L("svg"))==0)
		{
			aLocation.iType = ELTFileSvg;
			aLocation.iIndex1 = 16384;
			aLocation.iIndex2 = 16385;
			aLocation.iValid = CHtmlCtlEnv::Static()->ResolvePath(aLocation.iFileName);
			return;
		}
		
		TInt index1 = -1, index2 = -1;
		if(param1.Length()>0)
			index1 = HcUtils::StrToInt(param1, EHex);
		if(param2.Length()>0)
			index2 = HcUtils::StrToInt(param2, EHex);
		if(param1.Length()==0 && ext.CompareF(_L("mbm"))==0)
		{
			index1 = 0;
			index2 = 1;
		}
		
		if(index1==-1)
		{
			aLocation.iType = ELTFileGeneral;
			if(aLocation.iFileName.Length()>0)
				aLocation.iValid = CHtmlCtlEnv::Static()->ResolvePath(aLocation.iFileName);
			else
				aLocation.iValid = EFalse;
		}
		else
		{
			aLocation.iType = ELTFileMbm;
			aLocation.iIndex1 = index1;
			aLocation.iIndex2 = index2;
			
			if(aLocation.iFileName.Length()>0) 
			{				
				if(aLocation.iFileName[0]!='*')
				{
#ifdef __SERIES60__
					_LIT(KAvkonMbm, "avkon.mbm");
					if(aLocation.iFileName.CompareF(KAvkonMbm)==0)
					{
#ifdef __SERIES60_3_ONWARDS__
						aLocation.iType = ELTFileSvg;
						aLocation.iFileName.Copy(AknIconUtils::AvkonIconFileName());
#else
						_LIT(KPath, "z:\\system\\data\\avkon.mbm");
						aLocation.iFileName.Copy(KPath);
#endif
						aLocation.iValid = ETrue;
						return;
					}
#endif

#ifdef __UIQ__
					_LIT(KQikonMbm, "qikon.mbm");
					if(aLocation.iFileName.CompareF(KQikonMbm)==0)
					{
						_LIT(KPath, "z:\\resource\\apps\\qikon.mbm");
						aLocation.iFileName.Copy(KPath);
						aLocation.iValid = ETrue;
						return;
					}
#endif
					aLocation.iValid = CHtmlCtlEnv::Static()->ResolvePath(aLocation.iFileName);
				}
				else
					aLocation.iValid = ETrue;
			}
			else
				aLocation.iValid = EFalse;
		}
	}
}

TBool HtmlParser::ParseInputType(const TDesC& aSource, TInt& aInputType)
{
	if(aSource.CompareF(KHStrText)==0) 
		aInputType = CHtmlElementInput::EText;
	else if(aSource.CompareF(KHStrCheckbox)==0) 
		aInputType = CHtmlElementInput::ECheckBox;
	else if(aSource.CompareF(KHStrPassword)==0) 
		aInputType = CHtmlElementInput::EPassword;
	else if(aSource.CompareF(KHStrRadio)==0) 
		aInputType = CHtmlElementInput::ERadio;
	else if(aSource.CompareF(KHStrButton)==0) 
		aInputType = CHtmlElementInput::EButton;
	else if(aSource.CompareF(KHStrHidden)==0) 
		aInputType = CHtmlElementInput::EHidden;
	else if(aSource.CompareF(KHStrSubmit)==0) 
		aInputType = CHtmlElementInput::ESubmit;
	else
		return EFalse;
	return ETrue;
}

void HtmlParser::ParseBackgroundPattern(const TDesC& aSource, THcBackgroundPattern& aBackPattern)
{
	TPtrC style, color;
	TInt pos = aSource.Locate(' ');
	if(pos!=KErrNotFound)
	{
		style.Set(aSource.Left(pos));
		color.Set(aSource.Mid(pos+1));
	}
	else
		style.Set(aSource);
	
	if(style.CompareF(KHStrSolid)==0) 
		aBackPattern.iStyle = CGraphicsContext::ESolidBrush;
	else if(style.CompareF(KHStrVerticalHatch)==0) 
		aBackPattern.iStyle = CGraphicsContext::EVerticalHatchBrush;
	else if(style.CompareF(KHStrForwardDiagonalHatch)==0) 
		aBackPattern.iStyle = CGraphicsContext::EForwardDiagonalHatchBrush;
	else if(style.CompareF(KHStrHorizontalHatch)==0) 
		aBackPattern.iStyle = CGraphicsContext::EHorizontalHatchBrush;
	else if(style.CompareF(KHStrRearwardDiagonalHatch)==0) 
		aBackPattern.iStyle = CGraphicsContext::ERearwardDiagonalHatchBrush;
	else if(style.CompareF(KHStrSquareCrossHatch)==0) 
		aBackPattern.iStyle = CGraphicsContext::ESquareCrossHatchBrush;
	else if(style.CompareF(KHStrDiamondCrossHatch)==0) 
		aBackPattern.iStyle = CGraphicsContext::EDiamondCrossHatchBrush;
	else
		aBackPattern.iStyle = CGraphicsContext::ENullBrush;
	
	ParseColor(color, aBackPattern.iColor);
}

void HtmlParser::ParseBackgroundPosition(const TDesC& aSource, THcBackgroundPosition& aBackPosition)
{
	aBackPosition.Clear();
	if(aSource.CompareF(KHStrStretch)==0) 
		aBackPosition.iStretch = ETrue;
	else
	{
		TInt pos = aSource.Locate(' ');
		if(pos!=KErrNotFound)
		{
			TPtrC p = aSource.Mid(0, pos);
			HcUtils::Trim(p);
			if(p.Length()>0)
			{
				if(!ParseAlign(p, aBackPosition.iAlign)) 
					ParseLength(p, aBackPosition.iX);
			}
			
			p.Set(aSource.Mid(pos+1));
			HcUtils::Trim(p);
			
			if(p.Length()>0)
			{
				if(!ParseVAlign(p, aBackPosition.iVAlign)) 
					ParseLength(p, aBackPosition.iY);
			}
		}
		else 
		{
			if(!ParseAlign(aSource, aBackPosition.iAlign)) 
			{
				ParseLength(aSource, aBackPosition.iX);
				ParseLength(_L("50%"), aBackPosition.iY);
			}
		}				
	}	
}

void HtmlParser::ParseAttributeSelector(const TDesC& aSource, TInt& aAttribute)
{
	TInt pos = aSource.Locate('=');
	if(pos!=KErrNotFound && pos<aSource.Length()-1)
	{
		TPtrC name(aSource.Left(pos));
		TPtrC value;
		if(aSource[pos+1]=='"')
			value.Set(aSource.Mid(pos+2,aSource.Length()-pos-3));
		else
			value.Set(aSource.Mid(pos+1, aSource.Length()-pos-1));
		if(name.CompareF(KHStrType)==0)
		{
			ParseInputType(value, aAttribute);
			aAttribute += 100;
		}
	}
}

void HtmlParser::ParseFakeClass(const TDesC& aSource, TInt& aFakeClass)
{
	if(aSource.CompareF(KHStrHover)==0)
		aFakeClass = THcSelector::EFCHover;
	else if(aSource.CompareF(KHStrFocus)==0)
		aFakeClass = THcSelector::EFCFocus;
	else if(aSource.CompareF(KHStrLink)==0)
		aFakeClass = THcSelector::EFCLink;
	else if(aSource.CompareF(KHStrDown)==0)
		aFakeClass = THcSelector::EFCDown;
	else if(aSource.CompareF(KHStrError)==0)
		aFakeClass = THcSelector::EFCError;
}

void HtmlParser::ParseSelector(const TDesC& aSource, THcSelector& aSelector)
{	
	TBuf<50> tag, cls;
	TInt attr = 0, fcls = 0;
	TInt pos = aSource.Locate('.');
	if(pos!=KErrNotFound)
	{
		tag.Copy(aSource.Left(pos));
		if(pos<aSource.Length()-1)
			cls.Copy(aSource.Mid(pos+1));
	}
	else
		tag.Copy(aSource);
	
	pos = tag.Locate(':');
	if(pos!=KErrNotFound)
	{
		ParseFakeClass(tag.Mid(pos+1), fcls);
		tag.SetLength(pos);
	}
	
	pos = cls.Locate(':');
	if(pos!=KErrNotFound)
	{
		ParseFakeClass(cls.Mid(pos+1), fcls);
		cls.SetLength(pos);
	}
	
	pos = tag.Locate('#'); //not support id
	if(pos!=KErrNotFound)
		tag.SetLength(pos);

	pos = cls.Locate('#'); //not support id
	if(pos!=KErrNotFound)
		cls.SetLength(pos);
	
	pos = tag.Locate('[');
	if(pos!=KErrNotFound)
	{
		TInt pos2 = tag.Locate(']');
		if(pos2!=KErrNotFound)
			ParseAttributeSelector(tag.Mid(pos+1, pos2-pos-1), attr);
		tag.SetLength(pos);
	}

	pos = cls.Locate('[');
	if(pos!=KErrNotFound)
	{
		TInt pos2 = cls.Locate(']');
		if(pos2!=KErrNotFound)
			ParseAttributeSelector(cls.Mid(pos+1, pos2-pos-1), attr);
		cls.SetLength(pos);
	}
	
	aSelector.iTag.Copy(tag.Left(aSelector.iTag.MaxLength()));
	aSelector.iClass.Copy(cls.Left(aSelector.iClass.MaxLength()));
	aSelector.iFakeClass = fcls;
	aSelector.iAttribute = attr;
}

void HtmlParser::ParseStylesL(const TDesC& aSource, CHcStyle& aStyle)
{
	TPtrC p = aSource;
	HcUtils::Trim(p);
	if(p.Length()==0) 
	{
		aStyle.ClearAll();
		return;
	}
	
	TInt offset = 0;
	TBuf<50> name;
	TBuf<512> value;
	while(HcUtils::StringSplit(aSource, offset, ';' , p)) 
	{
		HcUtils::Trim(p);
		TInt pos = p.Locate(':');
		if(pos==KErrNotFound)
			continue;

		name.Copy(p.Mid(0, pos));
		name.Trim();
		name.LowerCase();
		
		if(pos!=p.Length()-1)
		{
			value.Copy(p.Mid(pos+1));
			value.Trim();
			value.LowerCase();
		}
		else
			value.Zero();
		
		ParseSingleStyleL(name, value, aStyle);
	}
}

void HtmlParser::ParseTextStylesL(const TDesC& aSource, THcTextStyle& aStyle)
{
	TPtrC p = aSource;
	HcUtils::Trim(p);
	if(p.Length()==0) 
	{
		aStyle.ClearAll();
		return;
	}
	
	TInt offset = 0;
	TBuf<50> name;
	TBuf<512> value;
	while(HcUtils::StringSplit(aSource, offset, ';' , p)) 
	{
		HcUtils::Trim(p);
		TInt pos = p.Locate(':');
		if(pos==KErrNotFound)
			continue;

		name.Copy(p.Mid(0, pos));
		name.Trim();
		name.LowerCase();
		
		if(pos!=p.Length()-1)
		{
			value.Copy(p.Mid(pos+1));
			value.Trim();
			value.LowerCase();
		}
		else
			value.Zero();
		
		if(name.Compare(KHStrBackgroundColor)==0)
		{
			if(value.Length()!=0)
			{
				aStyle.Set(THcTextStyle::EBackColor);
				ParseColor(value, aStyle.iBackColor);
			}
			else
				aStyle.Clear(THcTextStyle::EBackColor);
		}
		else if(name.Compare(KHStrBorder)==0)
		{
			if(value.Length()!=0)
			{
				aStyle.Set(THcTextStyle::EBorder);
				ParseBorder(value, aStyle.iBorder);
			}
			else
				aStyle.Clear(THcTextStyle::EBorder);
		}
		else
			ParseSingleTextStyleL(name, value, aStyle);
	}
}

TBool HtmlParser::ParseSingleTextStyleL(const TDesC& aName, const TDesC& aValue, THcTextStyle& aStyle)
{
	if(aName.Compare(KHStrColor)==0)
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(THcTextStyle::EColor);
			ParseColor(aValue, aStyle.iColor);
		}
		else
			aStyle.Clear(THcTextStyle::EColor);
	}
	else if(aName.Compare(KHStrFontSize)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.Set(THcTextStyle::ESize);
			ParseFontSize(aValue, aStyle.iSize);
		}
		else
			aStyle.Clear(THcTextStyle::ESize);
	}
	else if(aName.Compare(KHStrFontStyle)==0)
	{
		if(aValue.Compare(KHStrItalic)==0 || aValue.Compare(KHStrOblique)==0) 
			aStyle.Set(THcTextStyle::EItalics);
		else
			aStyle.Clear(THcTextStyle::EItalics);
	}
	else if(aName.Compare(KHStrFontWeight)==0)
	{
		if(aValue.Compare(KHStrBold)==0) 
			aStyle.Set(THcTextStyle::EBold);
		else
			aStyle.Clear(THcTextStyle::EBold);	
	}
	else if(aName.Compare(KHStrTextDecoration)==0)
	{
		if(aValue.Compare(KHStrUnderline)==0) 
			aStyle.Set(THcTextStyle::EUnderline);
		else if(aValue.Compare(KHStrLineThrough)==0)
			aStyle.Set(THcTextStyle::ELineThrough);
		else
		{
			aStyle.Clear(THcTextStyle::EUnderline);
			aStyle.Clear(THcTextStyle::ELineThrough);
		}
	}
	else if(aName.Compare(KHStrFontFamily)==0)
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(THcTextStyle::EFamily);
			ParseFontFamily(aValue, aStyle.iFamily);
		}
		else
			aStyle.Clear(THcTextStyle::EFamily);
	}
	else if(aName.Compare(KHStrLineHeight)==0) 
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(THcTextStyle::ELineHeight);
			ParseLength(aValue, aStyle.iLineHeight, ETrue);
		}
		else 
			aStyle.Clear(THcTextStyle::ELineHeight);
	}
	else if(aName.Compare(KHStrTextAlign)==0) 
	{
		if(aValue.Length()!=0 && ParseAlign(aValue, aStyle.iAlign))
			aStyle.Set(THcTextStyle::EAlign);
		else
			aStyle.Clear(THcTextStyle::EAlign);
	}
	else
		return EFalse;
	
	return ETrue;
}

TBool HtmlParser::ParseSingleStyleL(const TDesC& aName, const TDesC& aValue, CHcStyle& aStyle)
{
	if(ParseSingleTextStyleL(aName, aValue, aStyle.iTextStyle)) 
		return ETrue;
	
	if(aName.Compare(KHStrBackgroundColor)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.Set(CHcStyle::EBackColor);
			ParseColor(aValue, aStyle.iBackColor);
		}
		else
			aStyle.Clear(CHcStyle::EBackColor);
	}
	else if(aName.Compare(KHStrBackgroundImage)==0) 
	{
		if(aValue.Length()!=0)  
		{
			if(aValue.CompareF(KHStrNone)!=0)
			{
				THcImageLocation location;
				TInt pos = aValue.Locate('(');
				if(pos!=KErrNotFound) 
				{
					TInt pos2 = aValue.LocateReverse(')');
					if(pos2!=KErrNotFound)
					{
						TPtrC tmp = aValue.Mid(pos+1, pos2-pos-1);
						HcUtils::Trim(tmp);
						ParseImageSrc(tmp, location);
					}
					else
						ParseImageSrc(aValue, location);
				}
				else
					ParseImageSrc(aValue, location);
				
				aStyle.Set(CHcStyle::EBackImage);
				aStyle.SetBackgroundImageL(location);					
			}
			else
				aStyle.Clear(CHcStyle::EBackImage);
		}
		else 
			aStyle.Clear(CHcStyle::EBackImage);
	}
	else if(aName.Compare(KHStrBackgroundPosition)==0) 
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(CHcStyle::EBackPosition);
			ParseBackgroundPosition(aValue, aStyle.iBackPosition);
		}
		else
			aStyle.Clear(CHcStyle::EBackPosition);
	}
	else if(aName.Compare(KHStrBackgroundRepeat)==0) 
	{
		if(aValue.Length()!=0 && aValue.CompareF(KHStrNoRepeat)!=0) 
		{		
			if(aValue.CompareF(KHStrRepeatX)==0)
				aStyle.Set(CHcStyle::EBackRepeatX);
			else if(aValue.CompareF(KHStrRepeatY)==0)
				aStyle.Set(CHcStyle::EBackRepeatY);
			else
			{
				aStyle.Set(CHcStyle::EBackRepeatX);
				aStyle.Set(CHcStyle::EBackRepeatY);
			}
		}
		else 
		{
			aStyle.Clear(CHcStyle::EBackRepeatX);
			aStyle.Clear(CHcStyle::EBackRepeatY);
		}
	}
	else if(aName.Compare(KHStrBackgroundPattern)==0)
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(CHcStyle::EBackPattern);
			ParseBackgroundPattern(aValue, aStyle.iBackPattern);
		}
		else
			aStyle.Clear(CHcStyle::EBackPattern);
	}
	else if(aName.Compare(KHStrBorder)==0) 
	{
		if(aValue.Length()!=0)
		{
			ParseBorder(aValue, aStyle.iBorders.iLeft);
			aStyle.iBorders.iRight = aStyle.iBorders.iLeft;
			aStyle.iBorders.iTop = aStyle.iBorders.iLeft;
			aStyle.iBorders.iBottom= aStyle.iBorders.iLeft;
			aStyle.iBorders.iMask.SetAll();
		}
		else
			aStyle.iBorders.Clear();
	}
	else if(aName.Compare(KHStrBorderWidth)==0) 
	{	
		ParseBorderWidth(aValue, aStyle.iBorders);
	}
	else if(aName.Compare(KHStrBorderColor)==0)
	{
		ParseBorderColor(aValue, aStyle.iBorders);
	}
	else if(aName.Compare(KHStrBorderLeft)==0) 
	{
		if(aValue.Length()!=0)
		{
			aStyle.iBorders.iMask.Set(THcBorders::EBorderLeft);
			ParseBorder(aValue, aStyle.iBorders.iLeft);
		}
		else
			aStyle.iBorders.iMask.Clear(THcBorders::EBorderLeft);
	}
	else if(aName.Compare(KHStrBorderRight)==0) 
	{
		if(aValue.Length()!=0)
		{
			aStyle.iBorders.iMask.Set(THcBorders::EBorderRight);
			ParseBorder(aValue, aStyle.iBorders.iRight);
		}
		else
			aStyle.iBorders.iMask.Clear(THcBorders::EBorderRight);	
	}
	else if(aName.Compare(KHStrBorderTop)==0) 
	{
		if(aValue.Length()!=0)
		{
			aStyle.iBorders.iMask.Set(THcBorders::EBorderTop);
			ParseBorder(aValue, aStyle.iBorders.iTop);
		}
		else
			aStyle.iBorders.iMask.Clear(THcBorders::EBorderTop);
	}
	else if(aName.Compare(KHStrBorderBottom)==0) 
	{
		if(aValue.Length()!=0)
		{
			aStyle.iBorders.iMask.Set(THcBorders::EBorderBottom);
			ParseBorder(aValue, aStyle.iBorders.iBottom);
		}
		else
			aStyle.iBorders.iMask.Clear(THcBorders::EBorderBottom);
	}
	else if(aName.Compare(KHStrPosition)==0)
	{
		if(!(CHtmlCtlEnv::Static()->HtmlParser().iFlags & KParserFlagsIgnoreLayout))
		{
			if(aValue.Length()!=0) 
			{
				aStyle.Set(CHcStyle::EPosition);
				if(aValue.Compare(KHStrAbsolute)==0)
					aStyle.iPosition = EPosAbsolute;
				else
					aStyle.iPosition = EPosRelative;
			}
			else
				aStyle.Clear(CHcStyle::EPosition);
		}
	}
	else if(aName.Compare(KHStrLeft)==0)
	{
		if(!(CHtmlCtlEnv::Static()->HtmlParser().iFlags & KParserFlagsIgnoreLayout))
		{
			if(aValue.Length()!=0) 
			{
				aStyle.Set(CHcStyle::ELeft);
				ParseLength(aValue, aStyle.iLeft);
			}
			else
				aStyle.Clear(CHcStyle::ELeft);
		}
	}
	else if(aName.Compare(KHStrTop)==0) 
	{
		if(!(CHtmlCtlEnv::Static()->HtmlParser().iFlags & KParserFlagsIgnoreLayout))
		{
			if(aValue.Length()!=0) 
			{
				aStyle.Set(CHcStyle::ETop);
				ParseLength(aValue, aStyle.iTop);
			}
			else
				aStyle.Clear(CHcStyle::ETop);
		}
	}
	else if(aName.Compare(KHStrWidth)==0)
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(CHcStyle::EWidth);
			ParseLength(aValue, aStyle.iWidth);
		}
		else
			aStyle.Clear(CHcStyle::EWidth);
	}
	else if(aName.Compare(KHStrHeight)==0) 
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(CHcStyle::EHeight);
			ParseLength(aValue, aStyle.iHeight);
		}
		else
			aStyle.Clear(CHcStyle::EHeight);	
	}
	else if(aName.Compare(KHStrMaxWidth)==0)
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(CHcStyle::EMaxWidth);
			ParseLength(aValue, aStyle.iMaxWidth);
		}
		else
			aStyle.Clear(CHcStyle::EMaxWidth);
	}
	else if(aName.Compare(KHStrAlign)==0)
	{
		if(aValue.Length()!=0) 
		{
			aStyle.Set(CHcStyle::EAlign);
			ParseAlign(aValue, aStyle.iAlign);
		}
		else
			aStyle.Clear(CHcStyle::EAlign);
	}
	else if(aName.Compare(KHStrMargin)==0)
	{
		if(aValue.Length()!=0) 
			ParseMargins(aValue, aStyle.iMargins);
		else
			aStyle.iMargins.Clear();
	}
	else if(aName.Compare(KHStrMarginTop)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.iMargins.iMask.Set(THcMargins::EMarginTop);
			ParseLength(aValue, aStyle.iMargins.iTop);
		}
		else
			aStyle.iMargins.iMask.Clear(THcMargins::EMarginTop);
	}
	else if(aName.Compare(KHStrMarginRight)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.iMargins.iMask.Set(THcMargins::EMarginRight);
			ParseLength(aValue, aStyle.iMargins.iRight);
		}
		else
			aStyle.iMargins.iMask.Clear(THcMargins::EMarginRight);
	}
	else if(aName.Compare(KHStrMarginBottom)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.iMargins.iMask.Set(THcMargins::EMarginBottom);
			ParseLength(aValue, aStyle.iMargins.iBottom);
		}
		else
			aStyle.iMargins.iMask.Clear(THcMargins::EMarginBottom);
	}
	else if(aName.Compare(KHStrMarginLeft)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.iMargins.iMask.Set(THcMargins::EMarginLeft);
			ParseLength(aValue, aStyle.iMargins.iLeft);
		}
		else
			aStyle.iMargins.iMask.Clear(THcMargins::EMarginLeft);
	}
	else if(aName.Compare(KHStrPadding)==0)
	{
		if(aValue.Length()!=0) 
			ParseMargins(aValue, aStyle.iPaddings);
		else
			aStyle.iPaddings.Clear();
	}
	else if(aName.Compare(KHStrPaddingTop)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.iPaddings.iMask.Set(THcMargins::EMarginTop);
			ParseLength(aValue, aStyle.iPaddings.iTop);
		}
		else
			aStyle.iPaddings.iMask.Clear(THcMargins::EMarginTop);
	}
	else if(aName.Compare(KHStrPaddingRight)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.iPaddings.iMask.Set(THcMargins::EMarginRight);
			ParseLength(aValue, aStyle.iPaddings.iRight);
		}
		else
			aStyle.iPaddings.iMask.Clear(THcMargins::EMarginRight);
	}
	else if(aName.Compare(KHStrPaddingBottom)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.iPaddings.iMask.Set(THcMargins::EMarginBottom);
			ParseLength(aValue, aStyle.iPaddings.iBottom);
		}
		else
			aStyle.iPaddings.iMask.Clear(THcMargins::EMarginBottom);
	}
	else if(aName.Compare(KHStrPaddingLeft)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.iPaddings.iMask.Set(THcMargins::EMarginLeft);
			ParseLength(aValue, aStyle.iPaddings.iLeft);
		}
		else
			aStyle.iPaddings.iMask.Clear(THcMargins::EMarginLeft);
	}
	else if(aName.Compare(KHStrFilter)==0) 
	{
		ParseFilter(aValue, aStyle);
	}
	else if(aName.Compare(KHStrClear)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.Set(CHcStyle::EClear);
			if(aValue.CompareF(KHStrLeft)==0)
				aStyle.iClear = EClearLeft;
			else if(aValue.CompareF(KHStrRight)==0) 
				aStyle.iClear = EClearRight;
			else if(aValue.CompareF(KHStrBoth)==0) 
				aStyle.iClear = EClearBoth;
			else
				aStyle.iClear = EClearNone;
		}
		else
			aStyle.Clear(CHcStyle::EClear);
	}
	else if(aName.Compare(KHStrOverflow)==0)
	{
		// visible | auto | hidden | scroll
		aStyle.Set(CHcStyle::EOverflow);
		if(aValue.CompareF(KHStrVisible)==0) 
			aStyle.iOverflow = EOverflowVisible;
		else if(aValue.CompareF(KHStrAuto)==0) 
			aStyle.iOverflow = EOverflowAuto;
		else if(aValue.CompareF(KHStrHidden)==0) 
			aStyle.iOverflow = EOverflowHidden;
		else if(aValue.CompareF(KHStrScroll)==0)
			aStyle.iOverflow = EOverflowScroll;
		else
			aStyle.iOverflow = EOverflowHidden;
	}
	else if(aName.Compare(KHStrVisibility)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.Set(CHcStyle::EHidden);
			if(aValue.CompareF(KHStrHidden)==0) 
				aStyle.iHidden = ETrue;
			else
				aStyle.iHidden = EFalse;
		}
		else
			aStyle.Clear(CHcStyle::EHidden);
	}
	else if(aName.Compare(KHStrDisplay)==0)
	{
		if(aValue.Length()!=0)
		{
			aStyle.Set(CHcStyle::EDisplay);
			if(aValue.CompareF(KHStrNone)==0) 
				aStyle.iDisplay = EDisplayNone;
			else
				aStyle.iDisplay = EDisplayBlock;
		}
		else
			aStyle.Clear(CHcStyle::EDisplay);
	}
	else
		return EFalse;

	return ETrue;
}

void HtmlParser::ParseStyleSheetL(const TDesC& aSource, CHcStyleSheet& aStyleSheet)
{
	TInt pos = 0, pos2 = 0;
	TPtrC ptr;
	THcSelector selector;
	while((pos2=aSource.Mid(pos).Locate('{'))!=KErrNotFound)
	{
		ptr.Set(aSource.Mid(pos, pos2));
		HcUtils::Trim(ptr);
		pos2+=pos+1;
		pos = aSource.Mid(pos2).Locate('}');
		if(pos!=KErrNotFound) 
		{
			ParseSelector(ptr, selector);
			aStyleSheet.AddL(selector, aSource.Mid(pos2, pos));
			pos += pos2 + 1;
		}
		else
			break;
	}

	aStyleSheet.Sort();
}

void HtmlParser::GetStyleString(const CHcStyle& aStyle, const TDesC& aName, TDes& aBuf)
{
	if(aName.Compare(KHStrColor)==0)
	{
		if(aStyle.iTextStyle.IsSet(THcTextStyle::EColor))
			GetRgb(aStyle.iTextStyle.iColor.Rgb(), aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrFontSize)==0)
	{
		if(aStyle.iTextStyle.IsSet(THcTextStyle::ESize))
			aBuf.Num(aStyle.iTextStyle.iSize);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrFontStyle)==0)
	{
		if(aStyle.iTextStyle.IsSet(THcTextStyle::EItalics))
			aBuf.Copy(KHStrItalic);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrFontWeight)==0)
	{
		if(aStyle.iTextStyle.IsSet(THcTextStyle::EBold))
			aBuf.Copy(KHStrBold);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrTextDecoration)==0)
	{
		if(aStyle.iTextStyle.IsSet(THcTextStyle::EUnderline))
			aBuf.Copy(KHStrUnderline);
		else if(aStyle.iTextStyle.IsSet(THcTextStyle::ELineThrough))
			aBuf.Copy(KHStrLineThrough);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrFontFamily)==0)
	{
		if(aStyle.iTextStyle.IsSet(THcTextStyle::EFamily))
			GetFontFamily(aStyle.iTextStyle.iFamily, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrLineHeight)==0) 
	{
		if(aStyle.iTextStyle.IsSet(THcTextStyle::ELineHeight))
			GetLength(aStyle.iTextStyle.iLineHeight, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrTextAlign)==0) 
	{
		if(aStyle.iTextStyle.IsSet(THcTextStyle::EAlign))
			GetAlign(aStyle.iTextStyle.iAlign, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrLeft)==0)
	{
		if(aStyle.IsSet(CHcStyle::ELeft))
			GetLength(aStyle.iLeft, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrTop)==0) 
	{
		if(aStyle.IsSet(CHcStyle::ETop))
			GetLength(aStyle.iTop, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrWidth)==0)
	{
		if(aStyle.IsSet(CHcStyle::EWidth))
			GetLength(aStyle.iWidth, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrHeight)==0) 
	{
		if(aStyle.IsSet(CHcStyle::EHeight))
			GetLength(aStyle.iHeight, aBuf);
		else
			aBuf.Zero();	
	}
	else if(aName.Compare(KHStrMaxWidth)==0)
	{
		if(aStyle.IsSet(CHcStyle::EMaxWidth))
			GetLength(aStyle.iMaxWidth, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrAlign)==0)
	{
		if(aStyle.IsSet(CHcStyle::EAlign))
			GetAlign(aStyle.iAlign, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrBackgroundColor)==0)
	{
		if(aStyle.IsSet(CHcStyle::EBackColor))
			GetRgb(aStyle.iBackColor.Rgb(), aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrBackgroundImage)==0) 
	{
		if(aStyle.IsSet(CHcStyle::EBackImage))
			aBuf.Copy(aStyle.BackgroundImage()->Location().iFileName.Left(aBuf.MaxLength()));
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrBackgroundPosition)==0) 
	{
		if(aStyle.IsSet(CHcStyle::EBackPosition))
			GetBackgroundPosition(aStyle.iBackPosition, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrBackgroundRepeat)==0) 
	{
		if(aStyle.IsSet(CHcStyle::EBackRepeatX))
			aBuf.Copy(KHStrRepeatX);
		else if(aStyle.IsSet(CHcStyle::EBackRepeatY))
			aBuf.Copy(KHStrRepeatY);
		else//KHStrNoRepeat
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrBackgroundPattern)==0)
	{
		if(aStyle.IsSet(CHcStyle::EBackPattern))
			GetBackgroundPattern(aStyle.iBackPattern, aBuf);
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrPosition)==0)
	{
		if(aStyle.IsSet(CHcStyle::EPosition))
		{
			if(aStyle.iPosition==EPosAbsolute)
				aBuf.Copy(KHStrAbsolute);
			else
				aBuf.Copy(KHStrRelative);
		}
		else
			aBuf.Zero();
	}
	/*else if(aName.Compare(KHStrBorder)==0) 
	{
	}
	else if(aName.Compare(KHStrBorderWidth)==0) 
	{	
	}
	else if(aName.Compare(KHStrBorderColor)==0)
	{
	}
	else if(aName.Compare(KHStrBorderLeft)==0) 
	{
	}
	else if(aName.Compare(KHStrBorderRight)==0) 
	{
	}
	else if(aName.Compare(KHStrBorderTop)==0) 
	{
	}
	else if(aName.Compare(KHStrBorderBottom)==0) 
	{
	}
	else if(aName.Compare(KHStrMargin)==0)
	{
	}
	else if(aName.Compare(KHStrMarginTop)==0)
	{
	}
	else if(aName.Compare(KHStrMarginRight)==0)
	{
	}
	else if(aName.Compare(KHStrMarginBottom)==0)
	{
	}
	else if(aName.Compare(KHStrMarginLeft)==0)
	{
	}
	else if(aName.Compare(KHStrPadding)==0)
	{
	}
	else if(aName.Compare(KHStrPaddingTop)==0)
	{
	}
	else if(aName.Compare(KHStrPaddingRight)==0)
	{
	}
	else if(aName.Compare(KHStrPaddingBottom)==0)
	{
	}
	else if(aName.Compare(KHStrPaddingLeft)==0)
	{
	}
	else if(aName.Compare(KHStrFilter)==0) 
	{
	}
	*/
	else if(aName.Compare(KHStrClear)==0)
	{
		if(aStyle.IsSet(CHcStyle::EClear))
		{
			if(aStyle.iClear==EClearLeft)
				aBuf.Copy(KHStrLeft);
			else if(aStyle.iClear==EClearRight)
				aBuf.Copy(KHStrRight);
			else if(aStyle.iClear==EClearBoth)
				aBuf.Copy(KHStrBoth);
			else
				aBuf.Copy(KHStrNone);
		}
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrOverflow)==0)
	{
		if(aStyle.IsSet(CHcStyle::EOverflow))
		{
			if(aStyle.iOverflow==EOverflowAuto)
				aBuf.Copy(KHStrAuto);
			else if(aStyle.iOverflow==EOverflowHidden)
				aBuf.Copy(KHStrHidden);
			else if(aStyle.iOverflow==EOverflowScroll)
				aBuf.Copy(KHStrScroll);
			else //if(aStyle.iOverflow==EOverflowVisible)
				aBuf.Copy(KHStrVisible);
		}
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrVisibility)==0)
	{
		if(aStyle.IsSet(CHcStyle::EHidden))
		{
			if(aStyle.iHidden)
				aBuf.Copy(KHStrHidden);
			else
				aBuf.Copy(KHStrVisible);
		}
		else
			aBuf.Zero();
	}
	else if(aName.Compare(KHStrDisplay)==0)
	{
		if(aStyle.IsSet(CHcStyle::EDisplay))
		{
			if(aStyle.iDisplay==EDisplayNone)
				aBuf.Copy(KHStrNone);
			else
				aBuf.Copy(KHStrBlock);
		}
		else
			aBuf.Zero();
	}
	else
		aBuf.Zero();
}


void HtmlParser::GetRgb(const TRgb& aRgb, TDes& aBuf)
{
	_LIT(KFormat, "%02x");
	aBuf.Zero();
	aBuf.Append('#');
#ifdef __SYMBIAN_9_ONWARDS__
	if(aRgb.Alpha()!=255)
		aBuf.AppendFormat(KFormat, aRgb.Alpha());
#endif
	aBuf.AppendFormat(KFormat, aRgb.Red());
	aBuf.AppendFormat(KFormat, aRgb.Green());
	aBuf.AppendFormat(KFormat, aRgb.Blue());
}

void HtmlParser::GetLength(const THcLength& aLength, TDes& aBuf)
{
	if(aLength.iValueType==THcLength::ELenVTAbsolute || aLength.iValueType==THcLength::ELenVTRelative)
		aBuf.Num(aLength.iValue);
	else if(aLength.iValueType==THcLength::ELenVTPercent)
	{
		aBuf.Zero();
		aBuf.Append('%');
		aBuf.AppendNum(aLength.iValue);
	}
	else
		aBuf.Copy(KHStrAuto);
}

void HtmlParser::GetAlign(TAlign align, TDes& aBuf)
{
	if(align==ELeft)
		aBuf.Copy(KHStrLeft);
	else if(align==ECenter)
		aBuf.Copy(KHStrCenter);
	else
		aBuf.Copy(KHStrRight);
}

void HtmlParser::GetVAlign(TVAlign valign, TDes& aBuf)
{
	if(valign==EVTop)
		aBuf.Copy(KHStrTop);
	else if(valign==EVCenter)
		aBuf.Copy(KHStrMiddle);
	else
		aBuf.Copy(KHStrBottom);
}

void HtmlParser::GetFontFamily(TInt aFamily, TDes& aBuf)
{
	switch(aFamily)
	{
	case 0:
		aBuf.Copy(KHStrDense);
		break;
	case 1:
		aBuf.Copy(KHStrAnnotation);
		break;
	case 2:
		aBuf.Copy(KHStrTitle);
		break;
	case 3:
		aBuf.Copy(KHStrLegend);
		break;
	case 4:
		aBuf.Copy(KHStrSymbol);
		break;
	default:
		{
			CDesCArray& fontFamilies = CHtmlCtlEnv::Static()->FontFamilies();
			if(aFamily>=10 && aFamily<10+fontFamilies.Count())
				aBuf.Copy(fontFamilies[aFamily-10]);
			else
				aBuf.Zero();
		}
	}
}

void HtmlParser::GetBackgroundPosition(const THcBackgroundPosition& aPositoin, TDes& aBuf)
{
	if(aPositoin.iStretch)
		aBuf.Copy(KHStrStretch);
	else
	{
		if(aPositoin.iAlign!=ELeft)
			GetAlign(aPositoin.iAlign, aBuf);
		else
			GetLength(aPositoin.iX, aBuf);
		aBuf.Append(' ');
		TBuf<10> tmp;
		if(aPositoin.iVAlign!=EVTop)
			GetVAlign(aPositoin.iVAlign, aBuf);
		else
			GetLength(aPositoin.iY, aBuf);
		aBuf.Append(tmp);
	}
}

void HtmlParser::GetBackgroundPattern(const THcBackgroundPattern& aPattern, TDes& aBuf)
{
	if(aPattern.iStyle==CGraphicsContext::ESolidBrush)
		aBuf.Copy(KHStrSolid);
	else if(aPattern.iStyle==CGraphicsContext::EVerticalHatchBrush)
		aBuf.Copy(KHStrVerticalHatch);
	else if(aPattern.iStyle==CGraphicsContext::EForwardDiagonalHatchBrush)
		aBuf.Copy(KHStrForwardDiagonalHatch);
	else if(aPattern.iStyle==CGraphicsContext::EHorizontalHatchBrush)
		aBuf.Copy(KHStrHorizontalHatch);
	else if(aPattern.iStyle==CGraphicsContext::ERearwardDiagonalHatchBrush)
		aBuf.Copy(KHStrRearwardDiagonalHatch);
	else if(aPattern.iStyle==CGraphicsContext::ESquareCrossHatchBrush)
		aBuf.Copy(KHStrSquareCrossHatch);
	else if(aPattern.iStyle==CGraphicsContext::EDiamondCrossHatchBrush)
		aBuf.Copy(KHStrDiamondCrossHatch);
	else if(aPattern.iStyle==CGraphicsContext::ENullBrush)
		aBuf.Copy(KHStrNone);
	
	if(aPattern.iColor!=KRgbBlack)
	{
		aBuf.Append(' ');
		TBuf<10> tmp;
		GetRgb(aPattern.iColor, tmp);
		aBuf.Append(tmp);
	}
}


