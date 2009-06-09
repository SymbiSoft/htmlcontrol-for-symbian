/*
* ============================================================================
*  Name     : HtmlCtlUtils
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef HTMLCTLUTILS_H
#define HTMLCTLUTILS_H

#include <e32base.h>
#include <gdi.h>

class CCoeControl;
class CHtmlControl;
class CHtmlElement;

enum TScreenOrientation
{
	EOrientationCurrent,
    EOrientationPortrait,
    EOrientationLandscape
};

class HtmlCtlUtils
{
public:
	static CFont* CreateFontL(const TDesC& aStyle);
	
	static TRgb ConvertHtmlColorToRgb(const TDesC& aSource);
	
	static void ConvertRgbToHtmlColor(const TRgb& aRgb, TDes& aBuf);

	static TBool IsOrientationLandscape();
	
	static TBool IsAppForeground();
	
	static TVersion PlatformVersion();
	
	static TSize ScreenSize(TScreenOrientation aOrientation=EOrientationCurrent);
	
	static HBufC* DecodeL(const TDesC& aSource);
	static HBufC8* DecodeL(const TDesC8& aSource);
	
	static HBufC* EncodeL(const TDesC& aSource);
	static HBufC8* EncodeL(const TDesC8& aSource);
};

#define VGA_ONWARDS (HtmlCtlUtils::ScreenSize(EOrientationLandscape).iWidth>=640)
#define QVGA_ONWARDS (HtmlCtlUtils::ScreenSize(EOrientationLandscape).iWidth>=320)

#endif
