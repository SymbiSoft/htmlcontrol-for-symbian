#ifndef HTMLCTLENV_H
#define HTMLCTLENV_H

#include <coemain.h>
#include "htmlcontrol.hrh"

#ifdef TOUCH_FEEDBACK_SUPPORT
#include <touchfeedback.h>
#endif

#include "controlimpl.h"

class CHcImagePool;
class CHtmlParser;
class CHcScrollbarDrawer;
class CHcStyle;
class CHcStyleSheet;
class CHcMeasureStatus;
class CWritableBitmap;

class CHtmlCtlEnv : public CCoeStatic
{
public:
	static CHtmlCtlEnv* Static();
		
	CHtmlCtlEnv();
	~CHtmlCtlEnv();
	void ConstructL();

	void AddSearchPathL(const TDesC& aPath);
	TBool ResolvePath(TDes& aPath) const;
	void AddObjectFactory(NewHtmlObjectL aFactory);
	CCoeControl* CreateObjectL(const TDesC& aClassId, const TDesC& aMimeType, const TDesC& aData, CHtmlControl& aParent, CHtmlElement& aElement);
	
	void AddDestroyMoinitor(CHtmlControlImpl* aObject);
	TBool RemoveDestroyMonitor(CHtmlControlImpl* aObject);

	CWritableBitmap* HelperBitmap(TInt aIndex, const TSize& aSize);

	inline CHcStyleSheet& DefaultStyleSheet() const;
	inline CHcImagePool& ImagePool() const;
	inline CDesCArray& FontFamilies() const;
	inline CHtmlParser& HtmlParser() const;
	inline CHcMeasureStatus& MeasureStatus() const;
	inline CHcScrollbarDrawer& ScrollbarDrawer() const;
	inline TVersion PlatformVersion() const;
	inline TInt BaseFontSize(TInt aLevel=0) const;

#ifdef TOUCH_FEEDBACK_SUPPORT
	inline MTouchFeedback* TouchFeedback() const;
#endif
	
private:
	TVersion iPlatformVersion;
	CDesCArray* iSearchPaths;
	CHcImagePool* iImagePool;
	CDesCArray* iFontFamilies;
	CHtmlParser* iHtmlParser;
	CHcMeasureStatus* iMeasureStatus;
	CHcScrollbarDrawer* iScrollbarDrawer;
	const TInt* iBaseFontSize;

	CHcStyleSheet* iStyleSheet;
	RArray<NewHtmlObjectL> iObjectFactories;
	RPointerArray<CHtmlControlImpl> iDestroyMonitors;
	
	CWritableBitmap* iHelperBitmap[3];
	
#ifdef TOUCH_FEEDBACK_SUPPORT
	MTouchFeedback* iTouchFeedback;
	TBool iOwnTouchFeedbackInstance;
#endif
};

inline TVersion CHtmlCtlEnv::PlatformVersion() const
{
	return iPlatformVersion;
}

inline CHcStyleSheet& CHtmlCtlEnv::DefaultStyleSheet() const
{
	return *iStyleSheet;
}

inline CHcImagePool& CHtmlCtlEnv::ImagePool() const
{
	return *iImagePool;
}

inline CDesCArray& CHtmlCtlEnv::FontFamilies() const
{
	return *iFontFamilies;
}

inline CHtmlParser& CHtmlCtlEnv::HtmlParser() const
{
	return *iHtmlParser;
}

inline CHcMeasureStatus& CHtmlCtlEnv::MeasureStatus() const
{
	return *iMeasureStatus;
}

inline CHcScrollbarDrawer& CHtmlCtlEnv::ScrollbarDrawer() const
{
	return *iScrollbarDrawer;
}

inline TInt CHtmlCtlEnv::BaseFontSize(TInt aLevel) const
{
	return *(iBaseFontSize + aLevel + 3);
}

#ifdef TOUCH_FEEDBACK_SUPPORT
inline MTouchFeedback* CHtmlCtlEnv::TouchFeedback() const
{
	return iTouchFeedback;
}
#endif

#endif

