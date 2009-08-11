#ifndef HCSTYLE_H
#define HCSTYLE_H

#include <babitflags.h>
#include <gdi.h>
#include <gulutil.h>
#include <gulalign.h>
#include "htmlcontrol.hrh"

class CHcImage;
class CHcStyleSheet;

enum THcOverflow
{
	EOverflowHidden,
	EOverflowVisible,
	EOverflowAuto,
	EOverflowScroll
};

enum THcLocationType
{
	ELTFileGeneral,
	ELTFileMbm,
	ELTFileSvg,
	ELTAppIcon,
	ELTGradient,
	ELTSkin,
	ELTFrame,
	ELTIcon
};

typedef TGulVAlignment TVAlign;

enum THcLineWrap
{
	ELWAuto,
	ELWClip,
	ELWScroll
};

enum THcPosition
{
	EPosRelative,
	EPosAbsolute
};

enum THcClear
{
	EClearNone,
	EClearLeft,
	EClearRight,
	EClearBoth
};

enum THcDisplay
{
	EDisplayBlock,
	EDisplayNone
};

class THcLength
{
public:
	enum TValueType
	{
		ELenVTAbsolute,
		ELenVTRelative,
		ELenVTPercent,
		ELenVTAuto
	};
	TValueType iValueType;
	TInt iValue;
	
	THcLength()
	{
		iValueType = ELenVTAbsolute;
		iValue = 0;
	}
	inline TBool operator==(const THcLength& aLength) const
	{
		return iValueType==aLength.iValueType && iValue==aLength.iValue;
	}
	TInt GetRealValue(TInt aMaxValue, TInt aAutoValue) const;
};

class THcColor
{
public:
	TRgb iRgb;
	TInt iIndex1;
	TInt iIndex2;

	THcColor()
	{
		iIndex1 = 0;
		iIndex2 = 0;
	}
	
	inline TBool operator==(const THcColor& aColor) const
	{
		return iRgb==aColor.iRgb && iIndex1==aColor.iIndex1 && iIndex2==aColor.iIndex2;
	}
	inline TBool IsNone() const
	{
		return iIndex1==-1 && iIndex2==-1;
	}
	
	TRgb Rgb() const;
};

class THcBorder
{
public:
	enum TBorderStyle
	{
		ENone,
		ESolid,
		EDotted,
		EDashed,
		EOutset,
		EInset
	};
	
	TInt iWidth;
	TRgb iColor;
	TBorderStyle iStyle;
	
	THcBorder()
	{
		iWidth = 0;
		iColor = KRgbBlack;
		iStyle = ESolid;
	}
	
	TBool operator==( const THcBorder& aBorder) const
	{
		return iWidth==aBorder.iWidth && iColor==aBorder.iColor	&& iStyle==aBorder.iStyle;
	}
};

class THcBorders
{
public:
	enum TBorderId
	{
		EBorderLeft,
		EBorderRight,
		EBorderTop,
		EBorderBottom
	};
	THcBorder iTop;
	THcBorder iRight;
	THcBorder iBottom;
	THcBorder iLeft;
	TBitFlags8 iMask;
	
	void ShrinkRect(TRect& aRect) const;
	void Add(const THcBorders& aBorders);
	
	inline TBool IsEmpty() const
	{
		return iMask.iFlags==0;
	}
	inline void Clear()
	{
		iMask.ClearAll();
	}
};

class THcMargins
{
public:
	enum TMarginId
	{
		EMarginLeft,
		EMarginRight,
		EMarginTop,
		EMarginBottom
	};
	
	THcLength iLeft;
	THcLength iRight;
	THcLength iTop;
	THcLength iBottom;
	TBitFlags8 iMask;
	
	void ShrinkRect(TRect& aRect) const;
	void GrowRect(TRect& aRect) const;
	void Add(const THcMargins& aMargins);
	
	TBool GetAlign(TAlign& aAlign) const;
	TBool GetVAlign(TVAlign& aVAlign) const;
	
	TMargins GetMargins(const TSize& aSize) const;
	
	inline TBool IsEmpty() const
	{
		return iMask.iFlags==0;
	}
	inline void Clear()
	{
		iMask.ClearAll();
	}
};

class THcImageLocation
{
public:
	THcLocationType iType;
	TFileName iFileName;
	TInt iIndex1;
	TInt iIndex2;
	TInt iIndex3;
	TBool iValid;
	
	THcImageLocation()
	{
		iFileName.Zero();
		iIndex1 = 0;
		iIndex2 = 0;
		iIndex3 = 0;
		iType = ELTFileGeneral;
		iValid = EFalse;
	}
	TBool operator==( const THcImageLocation& aLocation) const
	{
		return iType==aLocation.iType
			&& iIndex1==aLocation.iIndex1 
			&& iIndex2==aLocation.iIndex2
			&& iIndex3==aLocation.iIndex3
			&& iFileName.Compare(aLocation.iFileName)==0;
	}
};

class THcBackgroundPosition
{
public:
	THcLength iX;
	THcLength iY;
	TAlign iAlign;
	TVAlign iVAlign;
	TBool iStretch;
	
	void Clear()
	{
		iX.iValueType = THcLength::ELenVTAbsolute;
		iX.iValue = 0;
		iY.iValueType = THcLength::ELenVTAbsolute;
		iY.iValue = 0;
		iAlign = ELeft;
		iVAlign = EVTop;
		iStretch = EFalse;
	}
};

class THcBackgroundPattern
{
public:
	CGraphicsContext::TBrushStyle iStyle; 
	TRgb iColor;
	
	THcBackgroundPattern()
	{
		iStyle = CGraphicsContext::ENullBrush;
	}
};

class THcTextStyle
{
public:
	enum TTextStyleId
	{
		EColor,
		EBackColor,
		ESize,
		EFamily,
		ELineHeight,
		EAlign,
		EBorder,
		
		EBold,
		EItalics,
		EUnderline,
		ELineThrough,
		ESub,
		ESup
	};
	
	inline void Set(TTextStyleId aStyleId)
	{
		iMask.Set(aStyleId);
	}
	inline TBool IsSet(TTextStyleId aStyleId) const
	{
		return iMask.IsSet(aStyleId);
	}
	inline void Clear(TTextStyleId aStyleId)
	{
		iMask.Clear(aStyleId);
	}
	inline void ClearAll()
	{
		iMask.ClearAll();
	}
	inline TBool IsEmpty() const
	{
		return iMask.iFlags==0;
	}
	inline TInt GetLineHeight(TInt aFontHeight) const
	{
		if(iMask.IsSet(ELineHeight)) 
		{
			TInt ret = iLineHeight.GetRealValue(aFontHeight, aFontHeight);
			return ret==0?(aFontHeight + 4):ret;
		}
		else
			return aFontHeight + 4;
	}
	
	void Add(const THcTextStyle& aStyle);
	TBool operator==(const THcTextStyle& aStyle) const;
	TInt FontMatchCode() const;
	CFont* CreateFont() const;
public:
	THcColor iColor;
	THcColor iBackColor;
	TInt iSize;
	TInt iFamily;
	THcLength iLineHeight;
	TAlign iAlign;
	THcBorder iBorder;

private:
	TBitFlags16 iMask;
};

class CHcStyle : public CBase
{
public:	
	enum TStyleId
	{
		EPosition,
		ELeft,
		ETop,
		EWidth,
		EHeight,
		EMaxWidth,
		EBackColor,
		EBackImage,
		EBackPattern,
		EBackPosition,
		EBackRepeatX,
		EBackRepeatY,
		ECorner,
		EAlign,
		EClear,
		EOverflow,
		EHidden,
		EDisplay,
		EFaded,
		EOpacity,
		EScale9Grid
	};
	inline void Set(TStyleId aStyleId)
	{
		iMask.Set(aStyleId);
	}
	inline TBool IsSet(TStyleId aStyleId) const
	{
		return iMask.IsSet(aStyleId);
	}
	inline void Clear(TStyleId aStyleId)
	{
		iMask.Clear(aStyleId);
	}
	
	void Add(const CHcStyle& aStyle);
	void ClearAll();
	TBool IsEmpty() const;
	~CHcStyle();
	
	void SetBackgroundImageL(const THcImageLocation& aLocation);
	inline CHcImage* BackgroundImage() const;

	inline TBool IsDisplayNone() const;
	inline TBool IsHidden() const;
	inline TBool IsClearLeft() const;
	inline TBool IsClearRight() const;
	
public:
	THcTextStyle iTextStyle;

	THcColor iBackColor;
	THcBackgroundPattern iBackPattern;
	THcBackgroundPosition iBackPosition;
	THcPosition iPosition;
	THcLength iLeft;
	THcLength iTop;
	THcLength iWidth;
	THcLength iHeight;
	THcLength iMaxWidth;
	TSize iCorner;
	TAlign iAlign;
	THcOverflow iOverflow;
	TUint8 iOpacity;
	TRect iScale9Grid;
	THcClear iClear;
	THcDisplay iDisplay;
	TBool iHidden;
	TBool iFaded;

	THcBorders iBorders;
	THcMargins iMargins;
	THcMargins iPaddings;
	
private:
	TBitFlags32 iMask;
	CHcImage* iBackImage;
};

inline CHcImage* CHcStyle::BackgroundImage() const
{
	return iBackImage;
}

inline TBool CHcStyle::IsDisplayNone() const
{
	return IsSet(EDisplay) && iDisplay==EDisplayNone;
}

inline TBool CHcStyle::IsHidden() const
{
	return IsSet(EHidden) && iHidden;
}

inline TBool CHcStyle::IsClearLeft() const
{
	return IsSet(EClear) && (iClear==EClearLeft || iClear==EClearBoth);
}

inline TBool CHcStyle::IsClearRight() const
{
	return IsSet(EClear) && (iClear==EClearRight || iClear==EClearBoth);
}

#endif
