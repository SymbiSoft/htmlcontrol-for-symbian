#ifndef HCUTILS_H
#define HCUTILS_H

#include <e32base.h>
#include <f32file.h>
#include <coemain.h>
#include <coedef.h>
#include <gdi.h>

#include "htmlcontrol.hrh"
#include "style.h"

class CHtmlControl;

class HcUtils
{
public:
	static TUint32 StrToInt(const TDesC& aSource, TRadix aRadix=EDecimal);
	static TBool StrToBool(const TDesC& aSource);

	static void Trim(TPtrC& aDest);
	static void TrimRedundant(TDes& aDest);
	static TBool IsEmptyText(const TDesC& aSource);

	static void Intersection(TRect& aRect, const TRect& aOtherRect);
	static TBool Intersects(const TRect& aRect, const TRect& aOtherRect);
	static void ShrinkRect(TRect& aRect, const TMargins& aMargins);
	static void GrowRect(TRect& aRect, const TMargins& aMargins);
	static void GetScale9GridRects(const TSize& aSourceSize, const TRect& aDestRect, const TRect& aScale9GridRect, TRect* aSourceRects, TRect* aDestRects);
	
	static HBufC* StringReplaceL(const TDesC& aSource, const TDesC& aTarget, const TDesC& aReplacement);
	static void StringReplace(const TDesC& aSource, const TDesC& aTarget, const TDesC& aReplacement, TDes& aDest);
	static void StringReplace(TDes& aTarget, TChar aSrcChar, TChar aDestChar);
	static TBool StringSplit(const TDesC& aSource, TInt& aOffset, TChar aSeperator, TPtrC& aRet);

	static TInt EnumTag(const TDesC& aSource, TInt aOffset, TDes& aTagName, TInt& aLength, TInt& aTagType);
	static TInt FindTag(const TDesC& aSource, TInt aOffset, const TDesC& aTagName, TBool aStartOrEnd, TInt& aLength);

	static TInt EnumAttribute(const TDesC& aSource, TInt aOffset, TDes& aAttrName, TInt& aAttrLength, TInt& aValueStart, TInt& aValueLength);
	static TInt GetAttributeValue(const TDesC& aSource, const TDesC& aAttrName, TPtrC& aDest);
	static TInt GetAttributeValue(const TDesC& aSource, const TDesC& aAttrName, TDes& aDest);

	static TUint16 GetEntityUnicode(const TDesC& aEntity);
	static HBufC* DecodeHttpCharactersL (const TDesC& aSource);
	static HBufC8* DecodeHttpCharactersL (const TDesC8& aSource);
	static HBufC* EncodeHttpCharactersL (const TDesC& aSource);
	static HBufC8* EncodeHttpCharactersL (const TDesC8& aSource);

	static void WrapTextL(const TDesC& aSource, TInt aFirstLineWidth, TInt aOtherLineWidth, const CFont& aFont, CArrayFix<TPtrC>& aWrappedArray);
	
	static TUint TranslateKey(TUint aKeyCode);
	static TInt TranslateStdKey(TInt aStdKeyCode);

	static void DrawBackgroundAndBorders(CHtmlControl& aControl, CBitmapContext& aGc, const TRect& aRect, const CHcStyle& aStyle);
	static void PrepareGcForTextDrawing(CBitmapContext& aGc, const THcTextStyle& aStyle);
	static void DrawBorder(CBitmapContext& aGc, const THcBorder& aBorder, const TPoint& p, const TRgb& aBackColor, TInt aPosition);
	
	static void DrawBitmapMasked(CBitmapContext& aGc, const TRect &aDestRect, const CFbsBitmap *aBitmap, const TRect &aSourceRect, const CFbsBitmap *aMaskBitmap, TBool aInvertMask);
	static void ConvertToGray256(CFbsBitGc& aGc, const TRect &aRect);
};

#endif
