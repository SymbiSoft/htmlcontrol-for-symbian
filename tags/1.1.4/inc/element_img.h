#ifndef HTMLELEMENTIMG_H
#define HTMLELEMENTIMG_H

#include "elementimpl.h"

class CHcImage;
class CWritableBitmap;
class CHcList;

class CHtmlElementImg : public CHtmlElementImpl, public MImageLoadedEventSubscriber
{
public:	
	CHtmlElementImg(CHtmlControl* aOwner);
	virtual ~CHtmlElementImg();
	
	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const;
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue);
	virtual void Measure(CHcMeasureStatus& aStatus);
	virtual void Refresh();
	virtual void Draw(CFbsBitGc& aGc) const;
	virtual CHtmlElementImpl* CloneL() const;
	
	void OnTimerL(TInt aError, TInt aIndex);
	void NotifyImageLoaded(CHcImage* aImage);

	//properties
	HBufC* iSrcStr;
	HBufC* iFocusSrcStr;
	HBufC* iAlt;
	CHcImage* iSrcImage;
	CHcImage* iFocusImage;
	TBool iAnimation;
	RHcStyle iStyle;
	CHcList* iOwnerList;
	
private:
	void SetCurrentImage(CHcImage* aImage);
	
	//runtime
	CHcImage* iImage;
	CTimer* iPlayer;
	TInt iCurrentFrame;
	TRect iViewRect;
	THcDrawImageParams iDrawParams;
	CWritableBitmap* iBackground;
};

#endif
