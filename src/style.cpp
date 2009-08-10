#include <eikenv.h>

#include "htmlcontrol.h"
#include "htmlctlenv.h"
#include "style.h"
#include "imagepool.h"
#include "utils.h"
#include "htmlparser.h"

#ifdef __SERIES60__
#include <aknsutils.h>
#endif

#ifdef __UIQ__
#include <Skins.h>
#include <coetextdrawer.h>
#endif

TInt THcLength::GetRealValue(TInt aMaxValue, TInt aAutoValue) const
{
	if(iValueType==ELenVTRelative)
		return aMaxValue + iValue;
	else if(iValueType==ELenVTPercent)
		return iValue * aMaxValue/100;
	else if(iValueType==ELenVTAuto)
		return aAutoValue;
	else //ELenVTAbsolute:
		return iValue;
}

TRgb THcColor::Rgb() const
{
#ifdef __SERIES60__
	if(iIndex1>0 || iIndex2>0)
	{
		TRgb rgb;
		TAknsItemID itemId = {iIndex1, iIndex2};
		if(AknsUtils::GetCachedColor(AknsUtils::SkinInstance(), rgb, itemId, iRgb.Value())==KErrNone)
			return rgb;
		else
			return KRgbBlack;
	}
#endif
		
#ifdef __UIQ__
	if(iIndex1>0 || iIndex2>0)
	{
		return SkinManager::TextDrawer(TUid::Uid(iIndex1), iIndex2, NULL).TextColor();
	}
#endif
	
	return iRgb;
}

void THcBorders::ShrinkRect(TRect& aRect) const
{
	if(iMask.IsSet(EBorderLeft))
		aRect.iTl.iX += iLeft.iWidth;
	if(iMask.IsSet(EBorderRight))
		aRect.iBr.iX -= iRight.iWidth;
	if(iMask.IsSet(EBorderTop))
		aRect.iTl.iY += iTop.iWidth;
	if(iMask.IsSet(EBorderBottom))
		aRect.iBr.iY -= iBottom.iWidth;
}

void THcBorders::Add(const THcBorders& aBorders)
{
	iMask.iFlags |= aBorders.iMask.iFlags;
	
	if(aBorders.iMask.IsSet(EBorderLeft))
		iLeft = aBorders.iLeft;
	if(aBorders.iMask.IsSet(EBorderRight))
		iRight = aBorders.iRight;
	if(aBorders.iMask.IsSet(EBorderTop))
		iTop = aBorders.iTop;
	if(aBorders.iMask.IsSet(EBorderBottom))
		iBottom = aBorders.iBottom;
}

void THcMargins::ShrinkRect(TRect& aRect) const
{
	HcUtils::ShrinkRect(aRect, GetMargins(aRect.Size()));
}

void THcMargins::GrowRect(TRect& aRect) const
{
	HcUtils::GrowRect(aRect, GetMargins(aRect.Size()));
}

TMargins THcMargins::GetMargins(const TSize& aSize) const
{
	TMargins ret;
	if(iMask.IsSet(EMarginLeft))
		ret.iLeft = iLeft.GetRealValue(aSize.iWidth,0);
	else
		ret.iLeft = 0;
	
	if(iMask.IsSet(EMarginRight))
		ret.iRight = iRight.GetRealValue(aSize.iWidth,0);
	else
		ret.iRight = 0;
	
	if(iMask.IsSet(EMarginTop))
		ret.iTop = iTop.GetRealValue(aSize.iHeight,0);
	else
		ret.iTop = 0;
	
	if(iMask.IsSet(EMarginBottom))
		ret.iBottom = iBottom.GetRealValue(aSize.iHeight,0);
	else
		ret.iBottom = 0;
	return ret;
}

void THcMargins::Add(const THcMargins& aMargins)
{
	iMask.iFlags |= aMargins.iMask.iFlags;
	if(aMargins.iMask.IsSet(EMarginLeft))
		iLeft = aMargins.iLeft;
	if(aMargins.iMask.IsSet(EMarginRight))
		iRight = aMargins.iRight;
	if(aMargins.iMask.IsSet(EMarginTop))
		iTop = aMargins.iTop;
	if(aMargins.iMask.IsSet(EMarginBottom))
		iBottom = aMargins.iBottom;
}

TBool THcMargins::GetAlign(TAlign& aAlign) const
{
	TBool a = iMask.IsSet(EMarginLeft) && iLeft.iValueType==THcLength::ELenVTAuto;
	TBool b = iMask.IsSet(EMarginRight) && iRight.iValueType==THcLength::ELenVTAuto;
	if(a && b) 
		aAlign = ECenter;
	else if(a)
		aAlign = ERight;
	else if(b)
		aAlign = ELeft;
	else
		return EFalse;
	return ETrue;	
}

TBool THcMargins::GetVAlign(TVAlign& aVAlign) const
{
	TBool a = iMask.IsSet(EMarginTop) && iTop.iValueType==THcLength::ELenVTAuto;
	TBool b = iMask.IsSet(EMarginBottom) && iBottom.iValueType==THcLength::ELenVTAuto;
	if(a && b)
		aVAlign = EVCenter;
	else if(a)
		aVAlign = EVBottom;
	else if(b)
		aVAlign = EVTop;
	else
		return EFalse;
	return ETrue;
}

TBool THcTextStyle::operator==(const THcTextStyle& aStyle) const 
{
	if(iMask.iFlags!=aStyle.iMask.iFlags
		|| iMask.IsSet(EColor) && !(iColor==aStyle.iColor)
		|| iMask.IsSet(EBackColor) && !(iBackColor==aStyle.iBackColor)
		|| iMask.IsSet(ESize) && iSize!=aStyle.iSize
		|| iMask.IsSet(EFamily) && iFamily!=aStyle.iFamily
		|| iMask.IsSet(ELineHeight) && !(iLineHeight==aStyle.iLineHeight)
		|| iMask.IsSet(EAlign) && iAlign!=aStyle.iAlign
		|| iMask.IsSet(EBorder) && !(iBorder==aStyle.iBorder))
		return EFalse;
	else
		return ETrue;
}

TInt THcTextStyle::FontMatchCode() const
{
	TInt result = 0;
	if(IsSet(THcTextStyle::EFamily))
		result |= (iFamily << 24);
	
	if(IsSet(THcTextStyle::ESize))
		result |= (iSize << 8);
	
	if(IsSet(THcTextStyle::EBold))
		result |= 1;
	
	if(IsSet(THcTextStyle::EItalics))
		result |= 2;
	
	if(IsSet(THcTextStyle::ESub))
		result |= 4;
	
	if(IsSet(THcTextStyle::ESup))
		result |= 8;
	
	return result;
}

void THcTextStyle::Add(const THcTextStyle& aStyle) {
	if(aStyle.IsEmpty())
		return;
	
	iMask.iFlags |= aStyle.iMask.iFlags;
	
	if(aStyle.IsSet(EColor)) 
		iColor = aStyle.iColor;
	
	if(aStyle.IsSet(EBackColor)) 
		iBackColor = aStyle.iBackColor;

	if(aStyle.IsSet(ESize)) 
		iSize = aStyle.iSize;	
	
	if(aStyle.IsSet(EFamily)) 
		iFamily = aStyle.iFamily;

	if(aStyle.IsSet(ELineHeight))
		iLineHeight = aStyle.iLineHeight;
	
	if(aStyle.IsSet(EAlign)) 
		iAlign = aStyle.iAlign;
	
	if(aStyle.IsSet(EBorder)) 
		iBorder = aStyle.iBorder;
}

CFont* THcTextStyle::CreateFont() const
{
	TFontSpec fontSpec;
	if(IsSet(THcTextStyle::EFamily)) 
	{
		switch(iFamily) 
		{
			case 0:
				fontSpec = CEikonEnv::Static()->AnnotationFont()->FontSpecInTwips();
				break;
			case 1:
				fontSpec = CEikonEnv::Static()->TitleFont()->FontSpecInTwips();
				break;
			case 2:
				fontSpec = CEikonEnv::Static()->LegendFont()->FontSpecInTwips();
				break;
			case 3:
				fontSpec = CEikonEnv::Static()->SymbolFont()->FontSpecInTwips();
				break;
			case 4:
				fontSpec = CEikonEnv::Static()->DenseFont()->FontSpecInTwips();
				break;
			default:
				fontSpec = TFontSpec(CHtmlCtlEnv::Static()->FontFamilies()[iFamily-10],1);
		}
	}
	else
		fontSpec = CEikonEnv::Static()->DenseFont()->FontSpecInTwips();

	if(IsSet(THcTextStyle::EBold))
		fontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightBold);
	else
		fontSpec.iFontStyle.SetStrokeWeight(EStrokeWeightNormal);
	if(IsSet(THcTextStyle::EItalics))
		fontSpec.iFontStyle.SetPosture(EPostureItalic);
	else
		fontSpec.iFontStyle.SetPosture(EPostureUpright);
	if(IsSet(THcTextStyle::ESub))
		fontSpec.iFontStyle.SetPrintPosition(EPrintPosSubscript);
	else if(IsSet(THcTextStyle::ESup))
		fontSpec.iFontStyle.SetPrintPosition(EPrintPosSuperscript);
	else
		fontSpec.iFontStyle.SetPrintPosition(EPrintPosNormal);
	if(IsSet(THcTextStyle::ESize) && iSize>0) 
		fontSpec.iHeight = CEikonEnv::Static()->ScreenDevice()->VerticalPixelsToTwips(iSize);
	
	CFont* font;
	CEikonEnv::Static()->ScreenDevice()->GetNearestFontInTwips( font, fontSpec );
	return font;
}

CHcStyle::~CHcStyle()
{
	if(iBackImage)
		CHtmlCtlEnv::Static()->ImagePool().Remove(iBackImage);
}

void CHcStyle::SetBackgroundImageL(const THcImageLocation& aLocation)
{
	if(iBackImage)
	{
		CHtmlCtlEnv::Static()->ImagePool().Remove(iBackImage);
		iBackImage = NULL;
	}
	iBackImage = CHtmlCtlEnv::Static()->ImagePool().AddL(aLocation);
}

void CHcStyle::Add(const CHcStyle& aStyle) 
{
	iBorders.Add(aStyle.iBorders);
	iMargins.Add(aStyle.iMargins);
	iPaddings.Add(aStyle.iPaddings);
	iTextStyle.Add(aStyle.iTextStyle);
	
	if(aStyle.IsEmpty())
		return;
	
	if(aStyle.IsSet(EBackColor))
		iBackColor = aStyle.iBackColor;
		
	if(aStyle.IsSet(EBackImage) && iBackImage!=aStyle.iBackImage)
	{
		if(iBackImage)
			CHtmlCtlEnv::Static()->ImagePool().Remove(iBackImage);
		iBackImage = aStyle.iBackImage;
		if(iBackImage)
			iBackImage->AddRef();
	}
	
	if(aStyle.IsSet(EBackPattern))
		iBackPattern = aStyle.iBackPattern;
	
	if(aStyle.IsSet(EBackPosition))
		iBackPosition = aStyle.iBackPosition;
	
	if(aStyle.IsSet(ECorner))
		iCorner = aStyle.iCorner;
	
	if(aStyle.IsSet(EPosition))
		iPosition = aStyle.iPosition;
	
	if(aStyle.IsSet(ELeft))
		iLeft = aStyle.iLeft;
	
	if(aStyle.IsSet(ETop))
		iTop = aStyle.iTop;
		
	if(aStyle.IsSet(EHeight))
		iHeight = aStyle.iHeight;
	
	if(aStyle.IsSet(EWidth))
		iWidth = aStyle.iWidth;
	
	if(aStyle.IsSet(EMaxWidth))
		iMaxWidth = aStyle.iMaxWidth;
	
	if(aStyle.IsSet(EAlign)) 
		iAlign = aStyle.iAlign;

	if(aStyle.IsSet(EOverflow))
		iOverflow = aStyle.iOverflow;
	
	if(aStyle.IsSet(EOpacity))
		iOpacity = aStyle.iOpacity;
	
	if(aStyle.IsSet(EScale9Grid))
		iScale9Grid = aStyle.iScale9Grid;
	
	if(aStyle.IsSet(EClear))
		iClear = aStyle.iClear;
	
	if(aStyle.IsSet(EDisplay))
		iDisplay = aStyle.iDisplay;
	
	if(aStyle.IsSet(EHidden))
		iHidden = aStyle.iHidden;
	
	if(aStyle.IsSet(EFaded))
		iFaded = aStyle.iFaded;
	
	iMask.iFlags |= aStyle.iMask.iFlags;
}

void CHcStyle::ClearAll() 
{
	iTextStyle.ClearAll();
	iMask.ClearAll();
	iBorders.Clear();
	iMargins.Clear();
	iPaddings.Clear();
	if(iBackImage)
	{
		CHtmlCtlEnv::Static()->ImagePool().Remove(iBackImage);
		iBackImage = NULL;
	}
}

TBool CHcStyle::IsEmpty() const
{
	return iMask.iFlags==0 && iTextStyle.IsEmpty() && iBorders.IsEmpty() && iMargins.IsEmpty() && iPaddings.IsEmpty();
}



