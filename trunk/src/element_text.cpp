#include <eikenv.h>

#include "controlimpl.h"
#include "htmlctlenv.h"
#include "htmlparser.h"
#include "measurestatus.h"
#include "stylesheet.h"
#include "utils.h"

#include "element_text.h"
#include "element_div.h"

_LIT(KHStrEClipsis, "...");
CHtmlElementText::CHtmlElementText(CHtmlControl* aOwner)
:CHtmlElementImpl(aOwner)
{
	iTypeId = EElementTypeText;
	iTagName.Set(KHStrText);
}

CHtmlElementText::~CHtmlElementText()
{
	for(TInt i=0;i<iBlockArray.Count();i++)
		delete iBlockArray[i];
	iBlockArray.Close();
	iRects.Close();
}

CHtmlElementImpl* CHtmlElementText::CloneL() const
{
	CHtmlElementText* e = new (ELeave)CHtmlElementText(iOwner);
	BaseClone(*e);
	for(TInt i=0;i<iBlockArray.Count();i++)
		e->iBlockArray.Append(iBlockArray[i]->CloneL());
	e->iStandAlone = iStandAlone;
	return e;
}

void CHtmlElementText::AppendTextL(const THcTextStyle& aStyle, HBufC* aText)
{
	if(iBlockArray.Count()>0) 
	{
		CTextBlock* lastBlock = iBlockArray[iBlockArray.Count()-1];
		if(!aText)
		{
			lastBlock->RemoveLastLineIfEmpty();
			
			if((iOwner->Impl()->Env()->HtmlParser().iFlags & KParserFlagsRemoveRedundantBr)
				&& lastBlock->IsEndingWithTwoBr())
				return;
		}
	
		if(lastBlock->iStyle==aStyle) 
		{
			lastBlock->AppendTextL(aText);
			return;
		}
	}

	CTextBlock* block = new (ELeave)CTextBlock();
	block->iStyle = aStyle;
	block->AppendTextL(aText);
	iBlockArray.Append(block);
}

void CHtmlElementText::SetTextL(const TDesC& aText)
{
	CTextBlock* block = new (ELeave)CTextBlock();
	if(iBlockArray.Count()>0)
	{
		THcTextStyle style = iBlockArray[0]->iStyle;
		for(TInt i=0;i<iBlockArray.Count();i++)
			delete iBlockArray[i];
		iBlockArray.Reset();
		block->iStyle = style;
	}
	iBlockArray.Append(block);
	
	TInt offset = 0;
	TPtrC p;
	TInt i=0;
	while(HcUtils::StringSplit(aText, offset, '\n' , p)) 
	{
		if(i!=0)
		{
			block->RemoveLastLineIfEmpty();
					
			if((!(iOwner->Impl()->Env()->HtmlParser().iFlags & KParserFlagsRemoveRedundantBr))
				|| !block->IsEndingWithTwoBr())
				block->AppendTextL(NULL);
		}
		i++;
		if(p.Length()>0)
		{
			if(p[p.Length()-1]=='\r') 
			{
				if(p.Length()>1)
					block->AppendTextL(p.Left(p.Length()-1).AllocL());
			}
			else
				block->AppendTextL(p.AllocL());	
		}
	}
}

void CHtmlElementText::GetTextL(TDes& aBuffer) const
{
	for(TInt i=0;i<iBlockArray.Count();i++)
	{
		for(TInt j=0;j<iBlockArray[i]->iTextArray.Count();j++)
		{
			HBufC* buf = iBlockArray[i]->iTextArray[j];
			if(buf)
				aBuffer.Append(*buf);
			else
				aBuffer.Append('\n');
		}
	}
}

TInt CHtmlElementText::GetTextLengthL() const
{
	TInt length = 0;
	for(TInt i=0;i<iBlockArray.Count();i++)
	{
		for(TInt j=0;j<iBlockArray[i]->iTextArray.Count();j++)
		{
			HBufC* buf = iBlockArray[i]->iTextArray[j];
			if(buf)
				length += buf->Length();
			else
				length++;
		}
	}
	return length;
}

TBool CHtmlElementText::GetProperty(const TDesC& aName, TDes& aBuffer) const
{
	if(CHtmlElementImpl::GetProperty(aName, aBuffer))
		return ETrue;
	
	if(aName.CompareF(KHStrValue)==0 || aName.CompareF(KHStrInnerText)==0)
	{
		GetTextL(aBuffer);
	}
	else if(aName.CompareF(KHStrBufSize)==0)
	{
		aBuffer.Num(GetTextLengthL());
	}
	else
		return EFalse;
	
	return ETrue;
}

TBool CHtmlElementText::SetProperty(const TDesC& aName, const TDesC& aValue)
{
	if(CHtmlElementImpl::SetProperty(aName, aValue))
		return ETrue;
	
	if(aName.CompareF(KHStrValue)==0 || aName.CompareF(KHStrInnerText)==0)
	{
		SetTextL(aValue);
	}
	else if(aName.CompareF(KHStrRes)==0)
	{
		TInt resId = HcUtils::StrToInt(aValue, EDecimal);
#ifdef __SYMBIAN_9_ONWARDS__
		if(CEikonEnv::Static()->IsResourceAvailableL(resId)) 
#else
		if(ETrue)
#endif
		{
			HBufC* data = CEikonEnv::Static()->AllocReadResourceLC(resId);
			SetTextL(*data);
			CleanupStack::PopAndDestroy();		
		}
	}
	else if(aName.CompareF(KHStrStyle)==0) 
	{
		CTextBlock* block;
		if(iBlockArray.Count()==0)
		{
			block = new (ELeave)CTextBlock();
			iBlockArray.Append(block);
		}
		else
			block = iBlockArray[0];
		HtmlParser::ParseTextStylesL(aValue, block->iStyle);
	}
	else if(aName.Left(6).CompareF(KHStrStyleDot)==0)
	{
		CTextBlock* block;
		if(iBlockArray.Count()==0)
		{
			block = new (ELeave)CTextBlock();
			iBlockArray.Append(block);
		}
		else
			block = iBlockArray[0];

		THcTextStyle style;
		if(HtmlParser::ParseSingleTextStyleL(aName.Mid(6), aValue, style))
			block->iStyle.Add(style);
	}
	else
		return EFalse;
	
	return ETrue;
}

TBool CHtmlElementText::IsEndingWithBr() const
{
	return iBlockArray.Count()>0 && iBlockArray[iBlockArray.Count()-1]->IsEndingWithBr();
}

TBool CHtmlElementText::IsBeginningWithBr() const
{
	return iBlockArray.Count()>0 && iBlockArray[0]->IsBeginningWithBr();
}

void CHtmlElementText::ClearCache()
{
	for(TInt i=0;i<iBlockArray.Count();i++)
		iBlockArray[i]->ClearCache();
	iRects.Reset();
}

void CHtmlElementText::Measure(CHcMeasureStatus& aStatus)
{
	iStyle = aStatus.CurrentTextStyle();
	const TRect& parentRect = iParent->iDisplayRect;

	if(aStatus.iPosition.iX>=parentRect.iBr.iX && !IsBeginningWithBr()
			|| iParent->LineWrapMode()!=ELWAuto)
		aStatus.NewLine();
	
	if(parentRect.Width()!=iCacheWidth)
	{
		if(iCacheWidth!=0)
		{
			for(TInt i=0;i<iBlockArray.Count();i++)
				iBlockArray[i]->iLeftToParent = -1;
		}
		iCacheWidth = parentRect.Width();
	}
	
	iPosition = aStatus.iPosition;
	iLineNumber = aStatus.LineNumber();
	TAlign align = aStatus.CurrentLineAlign();
	if(!iStyle.IsSet(THcTextStyle::EAlign))
		iStyle.iAlign = align;
	if(iBlockArray.Count()>0) 
	{
		CTextBlock* block = iBlockArray[0];
		if(block->iStyle.IsSet(THcTextStyle::EAlign))
		{
			iStyle.Set(THcTextStyle::EAlign);
			iStyle.iAlign = block->iStyle.iAlign;
		}
	}
	aStatus.SetCurrentLineAlign(iStyle.iAlign);
	iClippedRectIndex = -1;
	for(TInt i=0;i<iBlockArray.Count();i++) 
	{
		CTextBlock* tb = iBlockArray[i];
		THcTextStyle style = iStyle;
		style.Add(tb->iStyle);
		tb->iFont = iOwner->Impl()->GetFont(style);
		TInt leftToParent = aStatus.iPosition.iX - iParent->iDisplayRect.iTl.iX;
		if(leftToParent!=tb->iLeftToParent)
		{
			tb->ClearCache();
			tb->iLeftToParent = leftToParent;
		}
		tb->iTopToBeginning = aStatus.iPosition.iY - iPosition.iY;
		tb->iLineHeight = style.GetLineHeight(tb->iFont->HeightInPixels());
		
		if(iParent->LineWrapMode()==ELWScroll)
		{
			aStatus.CurrentLineInfo().SetHeightIfGreater(tb->iLineHeight);
			for(TInt j=0;j<tb->iTextArray.Count();j++)
			{
				HBufC* text = tb->iTextArray[j];
				if(text)
					aStatus.iPosition.iX += tb->iFont->TextWidthInPixels(*text);
			}
		}
		else
		{
			tb->WrapTextL(aStatus, iParent->LineWrapMode()==ELWClip);
			if(tb->iLines->Count()>1)
			{
				TPtrC line = (*tb->iLines)[0];
				aStatus.CurrentLineInfo().SetHeightIfGreater(tb->iLineHeight);
				aStatus.iPosition.iX += tb->iFont->TextWidthInPixels(line);
	
				aStatus.NewLine(ETrue);
				
				iParent->iContentSize.iWidth = iParent->iDisplayRect.Width();
				
				line.Set((*tb->iLines)[tb->iLines->Count()-1]);
				aStatus.CurrentLineInfo().iHeight = tb->iLineHeight;
				if(line.Length()>0)
					aStatus.iPosition.iX += tb->iFont->TextWidthInPixels(line) + 1;
				else
					aStatus.NewLine(ETrue);
				aStatus.iPosition.iY += (tb->iLines->Count()-2)*tb->iLineHeight;
			}
			else if(tb->iLines->Count()==1)
			{
				TPtrC line = (*tb->iLines)[0];
				aStatus.CurrentLineInfo().SetHeightIfGreater(tb->iLineHeight);
				
				if(line.Length()>0)
					aStatus.iPosition.iX +=  tb->iFont->TextWidthInPixels(line) + 1;
				else
					aStatus.NewLine(ETrue);
			}
			
			if(tb->iNewLineAtTheEnd)
			{
				aStatus.CurrentLineInfo().SetHeightIfGreater(tb->iLineHeight);
				aStatus.NewLine(ETrue);
			}
			
			tb->iLineNumber = aStatus.LineNumber();
			if(!tb->FullWrapped())
			{
				if(!iOwner->Impl()->iState.IsSet(EHCSInTransition))
					iOwner->Impl()->MeasureTextContinued();
				break;
			}
		}
	}
	
	if(iParent->LineWrapMode()==ELWScroll)
		aStatus.NewLine(EFalse);

	if(iStyle.IsSet(THcTextStyle::EAlign))
		aStatus.NewLine(EFalse);
	
	if(aStatus.LineNumber()!=iLineNumber)
	{
		iSize.iWidth = parentRect.Width();
		iPosition.iX = iParent->iDisplayRect.iTl.iX;
	}
	else
		iSize.iWidth = aStatus.iPosition.iX - iPosition.iX;
	if(aStatus.iPosition.iX!=iParent->iDisplayRect.iTl.iX)
		iSize.iHeight = aStatus.iPosition.iY - iPosition.iY + aStatus.CurrentLineInfo().iHeight;
	else
		iSize.iHeight = aStatus.iPosition.iY - iPosition.iY;
	
	aStatus.SetCurrentLineAlign(align);
}

void CHtmlElementText::Refresh()
{
	iRects.Reset();
	if(iState.IsSet(EElementStateHidden))
		return;
	
	if(iParent->LineWrapMode()==ELWScroll)
		RefreshSingleLine();
	else
		RefreshMultiLine();
}

void CHtmlElementText::RefreshSingleLine()
{
	TInt totalWidth = 0;
	iClippedRectIndex = -1;
	const THcLineInfo& aLineInfo = iOwner->Impl()->LineInfo(iLineNumber);
	for(TInt i=0;i<iBlockArray.Count();i++) 
	{
		CTextBlock* tb = iBlockArray[i];
		for(TInt j=0;j<tb->iTextArray.Count();j++)
		{
			HBufC* text = tb->iTextArray[j];
			if(text==NULL || text->Length()==0)
				continue;
			
			TInt lineWidth = tb->iFont->TextWidthInPixels(*text);
			TPoint pos(iParent->iDisplayRect.iTl.iX + totalWidth, tb->iTopToBeginning + iPosition.iY);

			TRect rect;
			rect.iTl.iX = pos.iX + aLineInfo.iXOffset;
			rect.iTl.iY = pos.iY + (aLineInfo.iHeight - tb->iLineHeight)/2;
			rect.iBr.iX = rect.iTl.iX + lineWidth;
			
#ifdef __SYMBIAN_9_ONWARDS__
#ifdef __SERIES60_3_MR__ //on 3rd_MR FontMaxHeight() would return 0
			rect.iTl.iY +=  (tb->iLineHeight - (tb->iFont->HeightInPixels()+2))/2 - 1;
			rect.iBr.iY = rect.iTl.iY + (tb->iFont->HeightInPixels()+2) + 2;
#else
			rect.iTl.iY +=  (tb->iLineHeight - tb->iFont->FontMaxHeight())/2 - 1;
			rect.iBr.iY = rect.iTl.iY + tb->iFont->FontMaxHeight() + 3;
#endif
#else
			rect.iTl.iY +=  (tb->iLineHeight - (tb->iFont->HeightInPixels()+2))/2 - 1;
			rect.iBr.iY = rect.iTl.iY + (tb->iFont->HeightInPixels()+2) + 2;
#endif
			
			if(rect.iTl.iY>=iParent->iClippingRect.iTl.iY-rect.Height() 
					&& rect.iBr.iY<=iParent->iClippingRect.iBr.iY+rect.Height())
			{
#ifdef __SYMBIAN_9_ONWARDS__
#ifdef __SERIES60_3_MR__ //on 3rd_MR FontMaxHeight() would return 0
				tb->iBaseLine = 1 + tb->iFont->AscentInPixels();
#else
				tb->iBaseLine = (tb->iFont->FontMaxHeight() - tb->iFont->HeightInPixels())/2 + Max(tb->iFont->AscentInPixels(), tb->iFont->FontMaxAscent());
#endif
#else
				tb->iBaseLine = 1 + tb->iFont->AscentInPixels();
#endif			
				TTextRect tr;
				tr.iBlock = tb;
				tr.iRect = rect;
				tr.iLineIndex = j;
				iRects.Append(tr);
				
				if(totalWidth + lineWidth>iParent->iDisplayRect.Width() && iClippedRectIndex==-1)
				{
					iClippedRectIndex = iRects.Count()-1;
					TInt spaceLeft =  iParent->iDisplayRect.Width() - totalWidth;
					TInt excessWidth;
					TInt k = tb->iFont->TextCount(*text, spaceLeft, excessWidth)-1;
					TInt dotlen = tb->iFont->TextWidthInPixels(KHStrEClipsis);
					while(excessWidth<dotlen && k>=0) 
					{
						excessWidth += tb->iFont->CharWidthInPixels((*text)[k]);
						k--;
					}
					iClippedCharCount = k+1;
				}
			}
			totalWidth += lineWidth;
		}
	}

	if(iClippedRectIndex!=-1 
			&& iParent->iState.IsSet(EElementStateInFocus)
			&& !iOwner->Impl()->iState.IsSet(EHCSInTransition))
		iOwner->Impl()->ScrollText(totalWidth, Rect());	
}

void CHtmlElementText::RefreshMultiLine()
{
	iClippedRectIndex = -1;
	TInt lineNumber = iLineNumber;
	TBool clipping = EFalse;
	for(TInt i=0;i<iBlockArray.Count();i++)
	{
		CTextBlock* tb = iBlockArray[i];
		if(!tb->iLines)
			break;
		
		const THcLineInfo& lineInfo = iOwner->Impl()->LineInfo(lineNumber);
		lineNumber = tb->iLineNumber;
		TPoint pos(tb->iLeftToParent + iParent->iDisplayRect.iTl.iX, tb->iTopToBeginning + iPosition.iY);
		TInt h = iParent->iClippingRect.iTl.iY - pos.iY;
		TInt firstLineHeight = lineInfo.iHeight;
		TInt startLine, endLine;
		if(h < firstLineHeight)
			startLine = 0;
		else
		{
			h -= firstLineHeight;
			startLine = 1 + h / tb->iLineHeight;
		}
		
		h = iParent->iClippingRect.iBr.iY - pos.iY;
		if(h<=0)
			endLine = 0;
		else if(h<firstLineHeight)
			endLine = 1;
		else
		{
			h -= firstLineHeight;
			endLine = 1 + h / tb->iLineHeight;

			//Plus 1 is to determin whether to display partial line
			if(iParent->LineWrapMode()!=ELWClip)
				endLine += 1; 
		}

		if(endLine > tb->iLines->Count())
			endLine = tb->iLines->Count();

		if(startLine>0)
		{
			pos.iX = iParent->iClippingRect.iTl.iX;
			pos.iY += firstLineHeight + (startLine-1)*tb->iLineHeight;
		}

		if(endLine-startLine>0) 
		{
			//RDebug::Print(_L("max-height=%i,height=%i,ascent=%i,descent=%i,max-ascent=%i,max-descent=%i"), tb->iFont->FontMaxHeight(), tb->iFont->HeightInPixels(), tb->iFont->AscentInPixels(), tb->iFont->DescentInPixels(), tb->iFont->FontMaxAscent(), tb->iFont->FontMaxDescent());
#ifdef __SYMBIAN_9_ONWARDS__
#ifdef __SERIES60_3_MR__ //on 3rd_MR FontMaxHeight() would return 0
			tb->iBaseLine = 1 + tb->iFont->AscentInPixels();
#else
			tb->iBaseLine = (tb->iFont->FontMaxHeight() - tb->iFont->HeightInPixels())/2 + Max(tb->iFont->AscentInPixels(), tb->iFont->FontMaxAscent());
#endif
#else
			tb->iBaseLine = 1 + tb->iFont->AscentInPixels();
#endif
			if(iParent->LineWrapMode()==ELWClip && iClippedRectIndex==-1 && endLine<tb->iLines->Count())
			{
				clipping = ETrue;
				iClippedRectIndex = iRects.Count() - 1;
			}
			
			for(TInt i=startLine;i<endLine;i++) 
			{
				TPtrC line = (*tb->iLines)[i];
				TRect rect;
				if(line.Length()>0)
				{
					TInt lineWidth = tb->iFont->TextWidthInPixels(line);
					if(i==0)
					{
						rect.iTl.iX = pos.iX + 1 + lineInfo.iXOffset;
						rect.iTl.iY = pos.iY + (lineInfo.iHeight - tb->iLineHeight)/2;
					}
					else if(i==tb->iLines->Count()-1 &&!tb->iNewLineAtTheEnd)
					{
						const THcLineInfo& lineInfo2 = iOwner->Impl()->LineInfo(tb->iLineNumber);
						rect.iTl.iX = pos.iX + 1 + lineInfo2.iXOffset;
						rect.iTl.iY = pos.iY + (lineInfo2.iHeight - tb->iLineHeight)/2;
					}
					else
					{
						if(iStyle.iAlign==ELeft)
							rect.iTl.iX = pos.iX + 1;
						else if(iStyle.iAlign==ECenter)
							rect.iTl.iX = pos.iX + (iParent->iDisplayRect.Width() - lineWidth)/2;
						else if(iStyle.iAlign==ERight)
							rect.iTl.iX = pos.iX + iParent->iDisplayRect.Width() - lineWidth;
						rect.iTl.iY = pos.iY;
					}
					
					TTextRect tr;
					tr.iBlock = tb;
#ifdef __SYMBIAN_9_ONWARDS__
#ifdef __SERIES60_3_MR__ //on 3rd_MR FontMaxHeight() would return 0
					rect.iTl.iY +=  (tb->iLineHeight - (tb->iFont->HeightInPixels()+2))/2 - 1;
					rect.iBr.iY = rect.iTl.iY + (tb->iFont->HeightInPixels()+2) + 2;
#else
					rect.iTl.iY +=  (tb->iLineHeight - tb->iFont->FontMaxHeight())/2 - 1;
					rect.iBr.iY = rect.iTl.iY + tb->iFont->FontMaxHeight() + 3;
#endif
#else
					rect.iTl.iY +=  (tb->iLineHeight - (tb->iFont->HeightInPixels()+2))/2 - 1;
					rect.iBr.iY = rect.iTl.iY + (tb->iFont->HeightInPixels()+2) + 2;
#endif
					rect.iBr.iX = rect.iTl.iX + lineWidth;
					tr.iRect = rect;
					tr.iLineIndex = i;
					iRects.Append(tr);
					
					if(clipping)
						iClippedRectIndex = iRects.Count()-1;
				}
				pos.iX = iParent->iDisplayRect.iTl.iX;
				if(i==0)
					pos.iY += firstLineHeight;
				else
					pos.iY += tb->iLineHeight;
			}
		}
		else
		{
			if(iParent->LineWrapMode()==ELWClip && iClippedRectIndex==-1 && endLine<tb->iLines->Count())
				iClippedRectIndex = iRects.Count() - 1;
		}
		
		if(iClippedRectIndex!=-1)
			break;
		
		if(!tb->FullWrapped())
			break;
	}

	if(iClippedRectIndex!=-1)
	{
		TTextRect& tr = iRects[iClippedRectIndex];
		TPtrC line = (*tr.iBlock->iLines)[tr.iLineIndex];
		TInt k = line.Length()-1;
		TInt dotlen = tr.iBlock->iFont->TextWidthInPixels(KHStrEClipsis);
		TInt excessWidth = iParent->iDisplayRect.iBr.iX - tr.iRect.iBr.iX;
		while(excessWidth<dotlen && k>=0)
		{
			excessWidth += tr.iBlock->iFont->CharWidthInPixels(line[k]);
			k--;
		}
		iClippedCharCount = k+1;
	}
}

void CHtmlElementText::Draw(CFbsBitGc& aGc) const
{
	if(iParent->LineWrapMode()==ELWScroll)
		DrawSingleLine(aGc);
	else
		DrawMultiLine(aGc);
	
	if(iStyle.IsSet(THcTextStyle::EBorder) && iRects.Count()>0 && iStyle.iBorder.iWidth>0)
		DrawBorder(aGc);
}

void CHtmlElementText::DrawSingleLine(CFbsBitGc& aGc) const
{
	for(TInt i=0;i<iRects.Count();i++) 
	{
		const TTextRect& tr = iRects[i];
		
		THcTextStyle style = iStyle;
		style.Add(tr.iBlock->iStyle);
		
		aGc.UseFontNoDuplicate(static_cast<CFbsBitGcFont*>(tr.iBlock->iFont));
		HcUtils::PrepareGcForTextDrawing(aGc, style);

		if(!iParent->iState.IsSet(EElementStateInFocus)
				|| iOwner->Impl()->iState.IsSet(EHCSInTransition) 
				|| iOwner->Impl()->TextScrollPos()==-1
				|| iClippedRectIndex==-1)
		{
			if(i!=iClippedRectIndex)
				aGc.DrawText(*tr.iBlock->iTextArray[tr.iLineIndex], tr.iRect,  tr.iBlock->iBaseLine,  CBitmapContext::ELeft, 0);
			else
			{
				aGc.DrawText(tr.iBlock->iTextArray[tr.iLineIndex]->Left(iClippedCharCount), tr.iRect,  tr.iBlock->iBaseLine, CBitmapContext::ELeft, 0);
				aGc.DrawText(KHStrEClipsis);
			}
		}
		else
		{
			TRect rect = tr.iRect;
			rect.iTl.iX -= iOwner->Impl()->TextScrollPos();
			aGc.DrawText(*tr.iBlock->iTextArray[tr.iLineIndex], rect,  tr.iBlock->iBaseLine, CBitmapContext::ELeft, 0);
		}
	}
}

void CHtmlElementText::DrawMultiLine(CFbsBitGc& aGc) const
{
	for(TInt i=0;i<iRects.Count();i++) 
	{
		const TTextRect& tr = iRects[i];
		
		THcTextStyle style = iStyle;
		style.Add(tr.iBlock->iStyle);

		aGc.UseFontNoDuplicate(static_cast<CFbsBitGcFont*>(tr.iBlock->iFont));
		HcUtils::PrepareGcForTextDrawing(aGc, style);
		
		if(i==iClippedRectIndex)
		{
			aGc.DrawText((*tr.iBlock->iLines)[tr.iLineIndex].Left(iClippedCharCount), tr.iRect,  tr.iBlock->iBaseLine, CBitmapContext::ELeft, 0);
			aGc.DrawText(KHStrEClipsis);
		}
		else
			aGc.DrawText((*tr.iBlock->iLines)[tr.iLineIndex], tr.iRect,  tr.iBlock->iBaseLine, CBitmapContext::ELeft, 0);
	}
}

void CHtmlElementText::DrawBorder(CFbsBitGc& aGc) const
{
	if(iStyle.iBorder.iStyle==THcBorder::ENone) 
		aGc.SetPenStyle(CGraphicsContext::ENullPen);
	else if(iStyle.iBorder.iStyle==THcBorder::EDotted)
		aGc.SetPenStyle(CGraphicsContext::EDottedPen);
	else if(iStyle.iBorder.iStyle==THcBorder::EDashed)
		aGc.SetPenStyle(CGraphicsContext::EDashedPen);
	else
		aGc.SetPenStyle(CGraphicsContext::ESolidPen);
	aGc.SetPenColor(iStyle.iBorder.iColor);
	aGc.SetPenSize(TSize(iStyle.iBorder.iWidth, iStyle.iBorder.iWidth));
	aGc.SetBrushStyle(CGraphicsContext::ENullBrush);
	
	TRect firstLine, lastLine;
	TInt secondLineTop = 0, lastSecondLineBottom = 0;
	TInt lines = 1;
	firstLine = iRects[0].iRect;
	lastLine = firstLine;
	for(TInt i=1;i<iRects.Count();i++)
	{
		if(iRects[i].iRect.iTl.iY>=lastLine.iBr.iY) //newline
		{
			if(lines==2)
				secondLineTop = lastLine.iTl.iY;
			lastSecondLineBottom = lastLine.iBr.iY;

			lines++;
			lastLine = iRects[i].iRect;
		}
		else
		{
			if(iRects[i].iRect.iTl.iY < lastLine.iTl.iY)
				lastLine.iTl.iY = iRects[i].iRect.iTl.iY;
			if(iRects[i].iRect.iBr.iY > lastLine.iBr.iY)
				lastLine.iBr.iY = iRects[i].iRect.iBr.iY;
			lastLine.iBr.iX = iRects[i].iRect.iBr.iX;
		}
	}
	
	firstLine.iTl.iY++;
	if(lines==1) //single line
	{
		aGc.DrawRect(firstLine);
	}
	else if(lines==2) //tow line
	{
		if(lastLine.iBr.iX < firstLine.iTl.iX)
		{
			lastLine.iTl.iY++;
			aGc.DrawRect(firstLine);
			aGc.DrawRect(lastLine);
		}
		else
		{
			TPoint points[8] = { 
				firstLine.iTl, 
				TPoint(firstLine.iBr.iX, firstLine.iTl.iY),
				firstLine.iBr,
				TPoint(lastLine.iBr.iX, firstLine.iBr.iY),
				lastLine.iBr,
				TPoint(lastLine.iTl.iX, lastLine.iBr.iY),
				lastLine.iTl,
				TPoint(firstLine.iTl.iX, lastLine.iTl.iY)
			};
			aGc.DrawPolygon(&points[0], 8);
		}
	}
	else //multi line
	{
		firstLine.iBr.iX = iParent->iDisplayRect.iBr.iX - 1;
		lastLine.iTl.iX =  iParent->iDisplayRect.iTl.iX;
		
		TPoint points[8] = {
			firstLine.iTl, 
			TPoint(firstLine.iBr.iX, firstLine.iTl.iY),
			TPoint(firstLine.iBr.iX, lastSecondLineBottom),
			TPoint(lastLine.iBr.iX, lastSecondLineBottom),
			lastLine.iBr,
			TPoint(lastLine.iTl.iX, lastLine.iBr.iY),
			TPoint(lastLine.iTl.iX, secondLineTop),
			TPoint(firstLine.iTl.iX, secondLineTop)
		};
		aGc.DrawPolygon(&points[0], 8);
	}
}

CTextBlock::~CTextBlock()
{
	for(TInt i=0;i<iTextArray.Count();i++)
		delete iTextArray[i];
	iTextArray.Close();
	
	delete iLines;
}

CTextBlock* CTextBlock::CloneL() const
{
	CTextBlock* item = new (ELeave)CTextBlock();
	for(TInt i=0;i<iTextArray.Count();i++) {
		if(iTextArray[i])
			item->iTextArray.Append((*iTextArray[i]).AllocL());
		else
			item->iTextArray.Append(NULL);
	}
	item->iStyle = iStyle;
	return item;
}

void CTextBlock::AppendTextL(HBufC* aText)
{	
	if(!aText) 
	{
		iTextArray.Append(aText);
	}
	else
	{
		TPtr p = aText->Des();
		HcUtils::StringReplace(p, '\t', ' ');
		HcUtils::StringReplace(p, '\r', ' ');
		
		if(iTextArray.Count()>0)
		{
			HBufC* buf = iTextArray[iTextArray.Count()-1];
			if(buf)
			{
				buf = buf->ReAllocL(buf->Length() + aText->Length());
				iTextArray[iTextArray.Count()-1] = buf;
				buf->Des().Append(*aText);
				delete aText;
			}
			else
				iTextArray.Append(aText);
		}
		else		
			iTextArray.Append(aText);
	}
}

void CTextBlock::RemoveLastLineIfEmpty()
{
	if(iTextArray.Count()>0
			&& iTextArray[iTextArray.Count()-1]
			&& HcUtils::IsEmptyText(*iTextArray[iTextArray.Count()-1]))
	{
		delete iTextArray[iTextArray.Count()-1];
		iTextArray.Remove(iTextArray.Count()-1);
	}	
}

void CTextBlock::ClearCache()
{
	delete iLines;
	iLines = NULL;
	iWrappingIndex = 0;
	iNewLineAtTheEnd = EFalse;
}

void CTextBlock::WrapTextL(CHcMeasureStatus& aStatus, TBool aForceFullWrap)
{
	if(!iLines)
		iLines = new (ELeave)CArrayFixFlat<TPtrC>(2);
	
	if(!aStatus.CanWrapText() && !aForceFullWrap)
		return;
	
	const TRect& parentRect = aStatus.Current()->iParent->iDisplayRect;
	while(iWrappingIndex<iTextArray.Count()) 
	{
		HBufC* text = iTextArray[iWrappingIndex];
		if(!text)
		{
			if(iNewLineAtTheEnd || iLines->Count()==0)
			{
				iLines->AppendL(TPtrC());
				iNewLineAtTheEnd = EFalse;
			}
			else
				iNewLineAtTheEnd = ETrue;
			iWrappingIndex++;
			continue;
		}
		else
			iNewLineAtTheEnd = EFalse;
		
		CArrayFix<TPtrC>* array = new (ELeave)CArrayFixFlat<TPtrC>(5);
		CleanupStack::PushL(array);
		if(iWrappingIndex==0) 
		{
			TInt firstLineWidth = parentRect.iBr.iX - aStatus.iPosition.iX - 1;
			aStatus.WrapTextL(*text, firstLineWidth, parentRect.Width() - 1,*iFont, *array);	
		}
		else
			aStatus.WrapTextL(*text, parentRect.Width() - 1, parentRect.Width() - 1,*iFont, *array);
		for(TInt i=0;i<array->Count();i++) 
		{
			TPtrC p = (*array)[i];
			iLines->AppendL(p);
		}
		CleanupStack::PopAndDestroy();
		iWrappingIndex++;
		
		if(!aStatus.CanWrapText() && !aForceFullWrap)
			break;
	}
}

