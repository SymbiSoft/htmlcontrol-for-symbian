#ifndef HCIMAGEPOOL_H
#define HCIMAGEPOOL_H

#include "htmlcontrol.hrh"
#include "image.h"

class CHtmlControl;
class CHtmlElementImpl;
class CImageDecoder;
class CTimer;
class CHcImagePool;

enum TRefreshType
{
	ESystemResourceChanged,
	ELocalResourceChanged
};

class CHcImagePool : public CBase
{
public:
	static CHcImagePool* NewL();
	virtual ~CHcImagePool();
	
	CHcImage* AddL(const THcImageLocation& aLocation);
	void Remove(CHcImage* aImage);
	TInt Find(const THcImageLocation& aLocation);
	void Clear();
	
	TBool RefreshAll(TInt aType);
	
	CImageDecoder* NewImageDecoderL(const TDesC& aFileName);
	void DeleteImageDecoder(CImageDecoder* aDecoder);
	
	void RemoveLoadedEventSubscriber(MImageLoadedEventSubscriber* aSubscriber);

	void OnTimerL(TInt aError, TInt aIndex);

	inline RFs& FsSession();
	
private:
	CHcImagePool();
	void ConstructL();

private:
	TBool iDecoderExhausted;
	TInt iDecoderCount;
	CTimer* iTimer;
	RFs iFsSession;
	RArray<CHcImage*> iImages;
};

inline RFs& CHcImagePool::FsSession()
{
	return iFsSession;
}

#endif

