#include <ImageConversion.h>

#include "htmlcontrol.h"
#include "htmlctlenv.h"
#include "imagepool.h"
#include "imageimpl.h"
#include "timer.h"
#include "utils.h"

CHcImagePool* CHcImagePool::NewL()
{
	CHcImagePool* self = new (ELeave)CHcImagePool();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
}

CHcImagePool::CHcImagePool()
{

}

CHcImagePool::~CHcImagePool()
{
	iDecoderExhausted = EFalse;
	for(TInt i=0;i<iImages.Count();i++) 
		delete iImages[i];
	iImages.Close();
	iFsSession.Close();
	delete iTimer;
}

void CHcImagePool::ConstructL()
{
	iTimer = CHcTimer<CHcImagePool>::NewL(*this,0,EPriorityLow);
	iFsSession.Connect();
	
#ifdef __SERIES60_3_ONWARDS__
	iFsSession.ShareProtected();
#endif
}

CHcImage* CHcImagePool::AddL(const THcImageLocation& aLocation)
{
	TInt idlePos = KErrNotFound;
	CHcImage* image = NULL;
	for(TInt i=0;i<iImages.Count();i++) 
	{
		CHcImage* k = iImages[i];
		if(!k) 
		{
			if(idlePos==KErrNotFound)
				idlePos = i;
		}
		else if(k->Location()==aLocation)
		{
			k->AddRef();
			if(!k->Location().iValid && aLocation.iValid)
				idlePos = i;
			else
				image = k;
			break;
		}
	}
	
	if(!image)
	{
		image = CHcImage::NewL(aLocation);
		if(idlePos==KErrNotFound) 
		{
			iImages.Append(image);
			idlePos =  iImages.Count()-1;
		}
		else
		{
			delete iImages[idlePos]; //harmless
			iImages[idlePos] = image;
		}
	}

#ifdef __WINSCW__
	if(!iTimer->IsActive()) 
		iTimer->After(10*1000);
#endif
	
	return image;
}

void CHcImagePool::Remove(CHcImage* aImage) 
{
	if(aImage)
	{
		aImage->Release();
		if(aImage->CanDestroy())
		{
			if(!iTimer->IsActive()) 
				iTimer->After(10*1000);
		}
	}
}

TInt CHcImagePool::Find(const THcImageLocation& aLocation)
{
	for(TInt i=0;i<iImages.Count();i++) 
	{
		CHcImage* k = iImages[i];
		if(k && k->Location()==aLocation)
		{
			return i;
		}
	}
	
	return KErrNotFound;
}

void CHcImagePool::Clear()
{
	TInt total = 0;
	for(TInt i=0;i<iImages.Count();i++) 
	{
		CHcImage* k = iImages[i];
		if(!k) 
			continue;
		
		if (k->CanDestroy())
		{
			delete k;
			iImages[i] = NULL;
		}
		else 
		{
			total++;
			#ifdef __WINSCW__ 
			RDebug::Print(_L("CHcImagePool: %i=fn(%S) ref=(%i) err(%i) loaded=(%i)"),
					i, &k->Location().iFileName, k->iRefCount, k->iError, k->iLoaded);
			#endif

		}

	}
#ifdef __WINSCW__ 
	RDebug::Print(_L("CHcImagePool: total=%i"), total);
#endif
}

void CHcImagePool::OnTimerL(TInt aError, TInt)
{
	if(aError!=KErrNone)
		return;
	
	Clear();
	
	if(!iDecoderExhausted) 
	{
		for(TInt i=0;i<iImages.Count();i++) 
		{
			CHcImage* k = iImages[i];
			if(k && k->Location().iType==ELTFileGeneral
					&& ((CHcImageGeneral*)k)->iDecodeProcess 
					&& !((CHcImageGeneral*)k)->iDecodeProcess->iDecoder)
			{
				((CHcImageGeneral*)k)->iDecodeProcess->ContinueDecode();
				if(iDecoderExhausted)
					break;
			}
		}
	}
}

CImageDecoder* CHcImagePool::NewImageDecoderL(const TDesC& aFileName) 
{
	if(iDecoderExhausted)
		return NULL;
	
	CImageDecoder* decoder = CImageDecoder::FileNewL( iFsSession, aFileName,
		 CImageDecoder::TOptions(	CImageDecoder::EAllowGeneratedMask | CImageDecoder::EOptionNoDither));

	iDecoderCount++;
	if(iDecoderCount>=1)
		iDecoderExhausted = ETrue;

	return decoder;
}

void CHcImagePool::DeleteImageDecoder(CImageDecoder* aDecoder) 
{
	delete aDecoder;
	iDecoderCount--;
	
	if(iDecoderExhausted) 
	{
		iDecoderExhausted = EFalse;
		if(!iTimer->IsActive()) 
			iTimer->After(10*1000);
	}
}
		
TBool CHcImagePool::RefreshAll(TInt aType)
{
	TBool ret = EFalse;
	for(TInt i=0;i<iImages.Count();i++) 
	{
		CHcImage* k = iImages[i];
		if(k)
			ret |= k->Refresh(aType);
	}
	
	return ret;
}

void CHcImagePool::RemoveLoadedEventSubscriber(MImageLoadedEventSubscriber* aSubscriber)
{
	for(TInt i=0;i<iImages.Count();i++) 
	{
		CHcImage* k = iImages[i];
		if(k)
			k->RemoveLoadedEventSubscriber(aSubscriber);
	}
}

