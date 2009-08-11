#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include "htmlcontrol.hrh"
#include "stylesheet.h"
#include "stack.h"

class CHtmlElementImpl;
class CHtmlElementDiv;

class HtmlParser
{
public:
	static void ParseLength(const TDesC& aSource, THcLength& aLength, TBool aSupportRelative = EFalse);
	static void ParseHexColor(const TDesC& aSource, TRgb& aRgb);
	static void ParseColor(const TDesC& aSource, TRgb& aResult);
	static void ParseColor(const TDesC& aSource, THcColor& aResult);
	static TBool ParseAlign(const TDesC& aSource, TAlign& aAlign);
	static TBool ParseVAlign(const TDesC& aSource, TVAlign& aAlign);
	static void ParseBorder(const TDesC& aSource, THcBorder& aBorder);
	static void ParseBorderWidth(const TDesC& aSource, THcBorders& aBorders);
	static void ParseBorderColor(const TDesC& aSource, THcBorders& aBorders);
	static void ParseMargins(const TDesC& aSource, THcMargins& aMargins);
	static void ParseFontSize(const TDesC& aSource, TInt& aSize);
	static void ParseFontFamily(const TDesC& aSource, TInt& aFontFamily);
	static void ParseFilter(const TDesC& aSource, CHcStyle& aStyle);
	static void ParseImageSrc(const TDesC& aURL, THcImageLocation& aLocation);
	static void ParseBackgroundPattern(const TDesC& aSource, THcBackgroundPattern& aBackPattern);
	static void ParseBackgroundPosition(const TDesC& aSource, THcBackgroundPosition& aBackPosition);
	static TBool ParseInputType(const TDesC& aSource, TInt& aInputType);
	static void ParseAttributeSelector(const TDesC& aSource, TInt& aAttribute);
	static void ParseFakeClass(const TDesC& aSource, TInt& aFakeClass);
	static void ParseSelector(const TDesC& aSource, THcSelector& aSelector);
	static void ParseTag(const TDesC& aSource, CHtmlElementImpl* aElement);
	
	static void ParseStylesL(const TDesC& aSource, CHcStyle& aStyle);
	static void ParseTextStylesL(const TDesC& aSource, THcTextStyle& aStyle);
	static TBool ParseSingleStyleL(const TDesC& aName, const TDesC& aValue, CHcStyle& aStyle);
	static TBool ParseSingleTextStyleL(const TDesC& aName, const TDesC& aValue, THcTextStyle& aStyle);
	static void ParseStyleSheetL(const TDesC& aSource, CHcStyleSheet& aStyleSheet);
	
	static void GetStyleString(const CHcStyle& aStyle, const TDesC& aName, TDes& aBuf);
	static void GetRgb(const TRgb& aRgb, TDes& aBuf);
	static void GetLength(const THcLength& aLength, TDes& aBuf);
	static void GetAlign(TAlign align, TDes& aBuf);
	static void GetVAlign(TVAlign valign, TDes& aBuf);
	static void GetFontFamily(TInt aFamily, TDes& aBuf);
	static void GetBackgroundPosition(const THcBackgroundPosition& aPositoin, TDes& aBuf);
	static void GetBackgroundPattern(const THcBackgroundPattern& aPattern, TDes& aBuf);
};

class CHtmlParser : public CBase
{
public:
	static CHtmlParser* NewL();
	~CHtmlParser();
	void ParseL(const TDesC& aSource, CHtmlElementImpl* aHead, TInt aFlags);
	
	TInt iFlags;
private:
	void ConstructL();
	void AppendBrL(TBool aForce);
	void AppendTextL(const TDesC& aText, TBool aRaw);
	void AppendElementL(const TDesC& aSource, CHtmlElementImpl* aNewElement);
	void RemoveDiv(CHtmlElementDiv* aDiv);
	
private:
	CHtmlElementImpl* iCurrent;
	CHtmlElementImpl* iHead;

	RHcStack<CHtmlElementDiv*> iBlocks;
	THcTextStyle iTextStyle;
	THcTextStyle iFontStyle;
};

#endif
