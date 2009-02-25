#include "htmlcontrol.h"
#include "htmlctlenv.h"
#include "image.h"
#include "imagepool.h"
#include "imageimpl.h"
#include "gcproxy.h"
#include "utils.h"

CHcImage* CHcImage::NewL(const THcImageLocation& aLocation)
{
	CHcImage* self;
	switch(aLocation.iType)
	{
	case ELTFileGeneral:
		self = new (ELeave)CHcImageGeneral();
		break;
	case ELTFileMbm:
		self = new (ELeave)CHcImageMbm();
		break;
	case ELTFileSvg:
		self = new (ELeave)CHcImageSvg();
		break;
	case ELTAppIcon:
		self = new (ELeave)CHcImageAppIcon();
		break;
	case ELTGradient:
		self = new (ELeave)CHcImageGradient();
		break;
	case ELTSkin:
#ifdef __SERIES60__
		self = new (ELeave)CHcImageAknsBackground();
#endif
		
#ifdef __UIQ__
		self = new (ELeave)CHcImageUIQSkin();
#endif
		break;
		
#ifdef __SERIES60__
	case ELTFrame:
		self = new (ELeave)CHcImageAknsFrame();
		break;
#endif
		
	case ELTIcon:
#ifdef __SERIES60__
		self = new (ELeave)CHcImageAknsIcon();
#endif
		
#ifdef __UIQ__
		self = new (ELeave)CHcImageUIQIcon();
#endif	
		break;
	default:
		self = new (ELeave)CHcImageNull();
	}
	self->iLocation = aLocation;
	self->Construct();
	self->AddRef();
	return self;
}

CHcImage::~CHcImage()
{	
	iSubscribers.Close();
}

void CHcImage::Construct()
{
	if(!iLocation.iValid)
		iError = KErrNotFound;
	else
	{
		TRAPD(error, ConstructL());
		iError = error;
	}
	
#ifdef __WINSCW__
	if(iError)
		RDebug::Print(_L("CHcImage: load failed %i: %S"), iError, &iLocation.iFileName);
#endif
}

void CHcImage::AddLoadedEventSubscriber(MImageLoadedEventSubscriber* aSubscriber)
{
	TInt pos = iSubscribers.Find(aSubscriber);
	if(pos==KErrNotFound)
		iSubscribers.Append(aSubscriber);
}

void CHcImage::RemoveLoadedEventSubscriber(MImageLoadedEventSubscriber* aSubscriber)
{
	TInt pos = iSubscribers.Find(aSubscriber);
	if(pos!=KErrNotFound)
		iSubscribers.Remove(pos);
}

TBool CHcImage::Refresh(TInt)
{
	return EFalse;
}

TBool CHcImage::Draw(CBitmapContext& aGc, const TRect& aDestRect, THcDrawImageParams aParams)
{
	if(iError || !iLoaded)
		return EFalse;
	
	aGc.SetBrushStyle(CGraphicsContext::ENullBrush);
	return DoDraw(aGc, aDestRect, aParams);
}

TBool CHcImage::Draw(CBitmapContext& aGc, const TPoint& aDestPos, THcDrawImageParams aParams)
{
	return Draw(aGc, TRect(aDestPos, iSize), aParams);	
}

