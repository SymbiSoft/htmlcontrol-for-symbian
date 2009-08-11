#ifndef HCIMAGE_H
#define HCIMAGE_H

#include <coemain.h>
#include <APGICNFL.H>
#include <gdi.h>

#include "style.h"

class CHcImage;

class MImageLoadedEventSubscriber
{
public:
	virtual void NotifyImageLoaded(CHcImage* aImage) = 0;
};

class THcDrawImageParams
{
public:
	TInt iFrameIndex;
	TSize iCorner;
	TInt iOpacity;
	TRect iScale9Grid;
	CCoeControl* iOwningControl;

	inline THcDrawImageParams()
	{
		iFrameIndex = 0;
		iCorner = TSize(0,0);
		iOpacity = 255;
		iScale9Grid = TRect(0,0,0,0);
		iOwningControl = NULL;
	}
};

class CHcImage : public CBase
{
public:
	static CHcImage* NewL(const THcImageLocation& aLocation);
	virtual ~CHcImage();
	
	void Construct();
	TBool Draw(CBitmapContext& aGc, const TRect& aDestRect, THcDrawImageParams aParams);
	TBool Draw(CBitmapContext& aGc, const TPoint& aDestPos, THcDrawImageParams aParams);
	
	virtual TBool Refresh(TInt aType);
	
	inline TSize Size() const;
	inline const THcImageLocation& Location() const;
	
	inline void AddRef();
	inline void Release();
	inline TBool CanDestroy() const;
	
	inline TInt Error() const;
	inline TBool IsError() const;
	inline TBool IsLoaded() const;
	
	void AddLoadedEventSubscriber(MImageLoadedEventSubscriber* aSubscriber);
	void RemoveLoadedEventSubscriber(MImageLoadedEventSubscriber* aSubscriber);

protected:
	virtual void ConstructL() = 0;
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams) = 0;
	
protected:
	THcImageLocation iLocation;
	TInt iRefCount;
	TSize iSize;
	TInt iError;
	TBool iLoaded;
	RPointerArray<MImageLoadedEventSubscriber> iSubscribers;
	
	friend class CHcImagePool;
};

//---------------------------------------------------
inline void CHcImage::AddRef() 
{
	iRefCount++;
}

inline void CHcImage::Release()
{
	iRefCount--;
	if(iRefCount<=0)
		iSubscribers.Reset();
}

inline TBool CHcImage::CanDestroy() const
{
	return iRefCount<=0;
}

inline TInt CHcImage::Error() const
{
	return iError;
}

inline TBool CHcImage::IsError() const
{
	return iError!=KErrNone;
}

inline TBool CHcImage::IsLoaded() const
{
	return iLoaded;
}

inline TSize CHcImage::Size() const
{
	return iSize;
}

inline const THcImageLocation& CHcImage::Location() const 
{
	return iLocation;
}

#endif
