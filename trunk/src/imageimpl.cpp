#include <ImageConversion.h>
#include <eikenv.h>
#include <COEUTILS.H>
#include <gulicon.h>
#include <apgcli.h>
#include <APGICNFL.H>

#include "htmlcontrol.hrh"

#ifdef __SERIES60__
#include <AknsConstants.hrh>
#include <aknutils.h>
#include <AknsDrawUtils.h>
#include <AknsControlContext.h>
#include <AknsBasicBackgroundControlContext.h>
#include <AknsFrameBackgroundControlContext.h>
#endif

#ifdef __UIQ__
#include <QSkinIds.h>
#include <QikContent.h>
#include <Skins.h>
#endif

#include "htmlctlenv.h"
#include "imagepool.h"
#include "imageimpl.h"
#include "gcproxy.h"
#include "writablebitmap.h"
#include "utils.h"

static CFbsBitmap* CreateMask(const TSize& aCorner, TInt aOpacity, const TSize& aSize, TBool aInvertMask, CFbsBitmap* aOrginal);

#define TR(r) TPoint(r.iBr.iX, r.iTl.iY)
#define BL(r) TPoint(r.iTl.iX, r.iBr.iY)

void CHcImageNull::ConstructL()
{
	User::Leave(KErrNotSupported);
}

TBool CHcImageNull::DoDraw(CBitmapContext& , const TRect& , const THcDrawImageParams&)
{
	return EFalse;
}

CHcImageGeneral::~CHcImageGeneral()
{
	Reset();
}

void CHcImageGeneral::Reset()
{
	for(TInt i=0;i<iFrames.Count();i++) 
	{
		const TFrame& frame = iFrames[i];
		delete frame.iBitmap;
		delete frame.iBitmapMask;
	}
	iFrames.Close();
	
	delete iDecodeProcess;
	iDecodeProcess = NULL;	
}

void CHcImageGeneral::ConstructL()
{	
	iDecodeProcess = CHcDecodeProcess::NewL(this);
}

TBool CHcImageGeneral::Refresh(TInt aType)
{
	if(aType==ELocalResourceChanged && iError)
	{
		Reset();
		iLocation.iValid = CHtmlCtlEnv::Static()->ResolvePath(iLocation.iFileName);
		
		Construct();
		
		return ETrue;
	}
	else
		return EFalse;
}

#define CHcImageGeneral_BitBlt(p, s) \
	if(mask)\
		aGc.BitBltMasked(p, frame.iBitmap, s, mask, EFalse);\
	else\
		aGc.BitBlt(p, frame.iBitmap, s)

#define CHcImageGeneral_DrawBitmap(d, s) \
	if(mask)\
		HcUtils::DrawBitmapMasked(aGc, d, frame.iBitmap, s, mask, EFalse);\
	else\
		aGc.DrawBitmap(d, frame.iBitmap, s)

TBool CHcImageGeneral::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams)
{
	TFrame& frame = iFrames[aParams.iFrameIndex];
	if(!frame.iConverted)
		return EFalse;
	
	CFbsBitmap* mask = CreateMask(aParams.iCorner, aParams.iOpacity, frame.iSize, EFalse, frame.iBitmapMask);
	if(iSize==aDestRect.Size())
	{
		CHcImageGeneral_BitBlt(aDestRect.iTl + frame.iPosition, TRect(TPoint(0,0), frame.iSize));
	}
	else if(!aParams.iScale9Grid.IsEmpty())
	{
		TReal ratioX = (TReal)aDestRect.Size().iWidth/iSize.iWidth;
		TReal ratioY = (TReal)aDestRect.Size().iHeight/iSize.iHeight;
		
		TRect destRect = TRect(frame.iPosition, frame.iSize);
		destRect.iTl.iX = TInt((TReal)destRect.iTl.iX * ratioX);
		destRect.iTl.iY = TInt((TReal)destRect.iTl.iY * ratioY);
		destRect.iBr.iX = TInt((TReal)destRect.iBr.iX * ratioX);
		destRect.iBr.iY = TInt((TReal)destRect.iBr.iY * ratioY);
		destRect.Move(aDestRect.iTl);
		
		TRect sourceRects[4], destRects[4];
		HcUtils::GetScale9GridRects(frame.iSize, destRect, aParams.iScale9Grid, sourceRects, destRects);
		
		//draw corners
		CHcImageGeneral_BitBlt(destRects[0].iTl, sourceRects[0]);
		CHcImageGeneral_BitBlt(destRects[1].iTl, sourceRects[1]);
		CHcImageGeneral_BitBlt(destRects[2].iTl, sourceRects[2]);
		CHcImageGeneral_BitBlt(destRects[3].iTl, sourceRects[3]);
		
		if(destRects[1].iTl.iX>destRects[0].iBr.iX) //draw mid top & mid bottom
		{
			CHcImageGeneral_DrawBitmap(TRect(TR(destRects[0]), BL(destRects[1])), TRect(TR(sourceRects[0]), BL(sourceRects[1])));
			CHcImageGeneral_DrawBitmap(TRect(TR(destRects[2]), BL(destRects[3])), TRect(TR(sourceRects[2]), BL(sourceRects[3])));
		}
		
		if(destRects[2].iTl.iY>destRects[0].iBr.iY>0) //draw mid left & mid right
		{
			CHcImageGeneral_DrawBitmap(TRect(BL(destRects[0]), TR(destRects[2])), TRect(BL(sourceRects[0]), TR(sourceRects[2])));
			CHcImageGeneral_DrawBitmap(TRect(BL(destRects[1]), TR(destRects[3])), TRect(BL(sourceRects[1]), TR(sourceRects[3])));
		}
		
		if(destRects[1].iTl.iX>destRects[0].iBr.iX && destRects[2].iTl.iY>destRects[0].iBr.iY>0) //draw middle
		{
			CHcImageGeneral_DrawBitmap(TRect(destRects[0].iBr, destRects[3].iTl), TRect(sourceRects[0].iBr, sourceRects[3].iTl));
		}
	}
	else
	{
		TReal ratioX = (TReal)aDestRect.Size().iWidth/iSize.iWidth;
		TReal ratioY = (TReal)aDestRect.Size().iHeight/iSize.iHeight;
		
		TRect drawRect = TRect(frame.iPosition, frame.iSize);
		drawRect.iTl.iX = TInt((TReal)drawRect.iTl.iX * ratioX);
		drawRect.iTl.iY = TInt((TReal)drawRect.iTl.iY * ratioY);
		drawRect.iBr.iX = TInt((TReal)drawRect.iBr.iX * ratioX);
		drawRect.iBr.iY = TInt((TReal)drawRect.iBr.iY * ratioY);
		drawRect.Move(aDestRect.iTl);
		
		CHcImageGeneral_DrawBitmap(drawRect, TRect(TPoint(0,0), frame.iSize));
	}
	
	return ETrue;
}

void CHcImageGeneral::DecodeFinished()
{
	for(TInt i=0;i<iSubscribers.Count();i++) 
		iSubscribers[i]->NotifyImageLoaded(this);
	iSubscribers.Reset();
}

//-----------------------------------------------
CHcImageMbm::~CHcImageMbm()
{
	delete iBitmap;
	delete iBitmapMask;
}

void CHcImageMbm::ConstructL()
{
	iBitmap = CEikonEnv::Static()->CreateBitmapL(iLocation.iFileName, iLocation.iIndex1);
	if(iLocation.iIndex2!=-1) 
	{
		TRAPD(error,
			iBitmapMask = CEikonEnv::Static()->CreateBitmapL(iLocation.iFileName, iLocation.iIndex2);
		);
	}
	iSize = iBitmap->SizeInPixels();
	iLoaded = ETrue;
}

TBool CHcImageMbm::Refresh(TInt aType)
{
	if(aType==ELocalResourceChanged && iError)
	{
		delete iBitmap;
		delete iBitmapMask;
		iBitmap = NULL;
		iBitmapMask = NULL;
		
		iLocation.iValid = CHtmlCtlEnv::Static()->ResolvePath(iLocation.iFileName);
		
		Construct();
		
		return ETrue;
	}
	else
		return EFalse;	
}

#define CHcImageMbm_BitBlt(p, s) \
	if(mask)\
		aGc.BitBltMasked(p, iBitmap, s, mask, ETrue);\
	else\
		aGc.BitBlt(p, iBitmap, s)

#define CHcImageMbm_DrawBitmap(d, s) \
	if(mask)\
		HcUtils::DrawBitmapMasked(aGc, d, iBitmap, s, mask, ETrue);\
	else\
		aGc.DrawBitmap(d, iBitmap, s)

TBool CHcImageMbm::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams)
{
	CFbsBitmap* mask = CreateMask(aParams.iCorner, aParams.iOpacity, iSize, ETrue, iBitmapMask);
	if(iSize==aDestRect.Size())
	{
		CHcImageMbm_BitBlt(aDestRect.iTl, TRect(TPoint(0,0),iSize));
	}
	else if(!aParams.iScale9Grid.IsEmpty())
	{
		TRect sourceRects[4], destRects[4];
		HcUtils::GetScale9GridRects(iSize, aDestRect, aParams.iScale9Grid, sourceRects, destRects);
		
		//draw corners
		CHcImageMbm_BitBlt(destRects[0].iTl, sourceRects[0]);
		CHcImageMbm_BitBlt(destRects[1].iTl, sourceRects[1]);
		CHcImageMbm_BitBlt(destRects[2].iTl, sourceRects[2]);
		CHcImageMbm_BitBlt(destRects[3].iTl, sourceRects[3]);
		
		if(destRects[1].iTl.iX>destRects[0].iBr.iX) //draw mid top & mid bottom
		{
			CHcImageMbm_DrawBitmap(TRect(TR(destRects[0]), BL(destRects[1])), TRect(TR(sourceRects[0]), BL(sourceRects[1])));
			CHcImageMbm_DrawBitmap(TRect(TR(destRects[2]), BL(destRects[3])), TRect(TR(sourceRects[2]), BL(sourceRects[3])));
		}
		
		if(destRects[2].iTl.iY>destRects[0].iBr.iY>0) //draw mid left & mid right
		{
			CHcImageMbm_DrawBitmap(TRect(BL(destRects[0]), TR(destRects[2])), TRect(BL(sourceRects[0]), TR(sourceRects[2])));
			CHcImageMbm_DrawBitmap(TRect(BL(destRects[1]), TR(destRects[3])), TRect(BL(sourceRects[1]), TR(sourceRects[3])));
		}
		
		if(destRects[1].iTl.iX>destRects[0].iBr.iX && destRects[2].iTl.iY>destRects[0].iBr.iY>0) //draw middle
		{
			CHcImageMbm_DrawBitmap(TRect(destRects[0].iBr, destRects[3].iTl), TRect(sourceRects[0].iBr, sourceRects[3].iTl));
		}
	}
	else
	{
		CHcImageMbm_DrawBitmap(aDestRect, TRect(TPoint(0,0),iSize));
	}
	
	return ETrue;
}

//-----------------------------------------------
#ifdef __SERIES60__

#ifdef __SERIES60_3_ONWARDS__
CHcIconFileProvider::CHcIconFileProvider(const TDesC& aFileName)
{
	iFileName.Copy(aFileName);
}

void CHcIconFileProvider::RetrieveIconFileHandleL(RFile &aFile, const TIconFileType /*aType*/)
{
	User::LeaveIfError(aFile.Open(CHtmlCtlEnv::Static()->ImagePool().FsSession(), iFileName, EFileRead | EFileShareAny));
}

void CHcIconFileProvider::Finished()
{
	delete this;
}
#endif

void CHcImageSvg::ConstructL()
{
#ifdef __SERIES60_3_ONWARDS__
	if(iLocation.iFileName[0]=='z' || iLocation.iFileName[0]=='Z')
		AknIconUtils::CreateIconL(iBitmap, iBitmapMask, iLocation.iFileName, iLocation.iIndex1, iLocation.iIndex2);
	else
		AknIconUtils::CreateIconL(iBitmap, iBitmapMask, *new (ELeave)CHcIconFileProvider(iLocation.iFileName), iLocation.iIndex1, iLocation.iIndex2);
	iLoaded = ETrue;
	iSize = TSize(16,16);
	AknIconUtils::SetSize(iBitmap, iSize);
#else
	User::Leave(KErrNotSupported);
#endif
}

TBool CHcImageSvg::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams)
{
#ifdef __SERIES60_3_ONWARDS__
	if(iSize!=aDestRect.Size()) 
	{
		iSize = aDestRect.Size();
		AknIconUtils::SetSize(iBitmap, iSize);
	}
#endif
	
	return CHcImageMbm::DoDraw(aGc, aDestRect, aParams);
}	
#endif

#ifdef __UIQ__
CHcImageSvg::~CHcImageSvg()
{
	delete iContent;
	delete iGcProxy;
}

void CHcImageSvg::ConstructL()
{
	iContent = new CQikContent();
	iContent->SetContentL(NULL, iLocation.iFileName);
	iContent->SetStretched(ETrue);
	iContent->SetStretchMaintainAspectRatio(EFalse);
	
	iLoaded = ETrue;
	if(iContent->Bitmap())
		iSize = iContent->Bitmap()->SizeInPixels();
	else
		iSize = TSize(0,0);
}

TBool CHcImageSvg::Refresh(TInt aType)
{
	if(aType==ELocalResourceChanged && iError)
	{
		delete iContent;
		iContent = NULL;
		
		iLocation.iValid = CHtmlCtlEnv::Static()->ResolvePath(iLocation.iFileName);
		
		Construct();
		
		return ETrue;
	}
	else	
		return EFalse;
}

TBool CHcImageSvg::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams&)
{
	if(!iGcProxy)
	{
		iGcProxy = new CGcProxy(&aGc);
	}
	else if(iGcProxy->iRealContext!=&aGc)
	{
		delete iGcProxy;
		iGcProxy = NULL;
		
		iGcProxy = new CGcProxy(&aGc);
	}
	iContent->Draw(*iGcProxy, aDestRect, aDestRect);

	return ETrue;	
}

#endif
//-----------------------------------------------
CHcImageAppIcon::~CHcImageAppIcon()
{
	if(iApaMaskedBitmap)
		iBitmapMask = NULL;
}

void CHcImageAppIcon::ConstructL()
{
#ifdef __SERIES60_3_ONWARDS__
	AknsUtils::CreateAppIconLC(AknsUtils::SkinInstance(), TUid::Uid(iLocation.iIndex1), (TAknsAppIconType)iLocation.iIndex2,
		iBitmap, iBitmapMask);
	iLoaded = ETrue;
	iSize = TSize(44, 44);
	AknIconUtils::SetSize(iBitmap, iSize);
	CleanupStack::Pop(2);
#else
	RApaLsSession ls;
    User::LeaveIfError(ls.Connect()); 
    CleanupClosePushL(ls);
    CApaMaskedBitmap* amb = CApaMaskedBitmap::NewLC();
	User::LeaveIfError(ls.GetAppIcon(TUid::Uid(iLocation.iIndex1), TSize(44, 44), *amb));
	iLoaded = ETrue;
	iSize = amb->SizeInPixels();
	CleanupStack::Pop(); //amb
	iApaMaskedBitmap = amb;
	iBitmap = iApaMaskedBitmap;
	iBitmapMask = iApaMaskedBitmap->Mask();
	CleanupStack::PopAndDestroy();//ls
#endif
}

TBool CHcImageAppIcon::Refresh(TInt)
{
	return EFalse;
}
//-----------------------------------------------
CHcImageGradient::~CHcImageGradient()
{
	for(TInt i=0;i<iBitmaps.Count();i++)
		delete iBitmaps[i];
	iBitmaps.Close();
}

void CHcImageGradient::ConstructL()
{
	iStartColor = TRgb(iLocation.iIndex1);
	iEndColor = TRgb(iLocation.iIndex2);
	iOrientation = (ColorUtils::TBitmapOrientation)iLocation.iIndex3;
	iSize = TSize(1,1);
	iLoaded = ETrue;
}

TBool CHcImageGradient::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams)
{	
	CFbsBitmap* bitmap = NULL;
	for(TInt i=0;i<iBitmaps.Count();i++)
	{
		if(iBitmaps[i]->SizeInPixels()==aDestRect.Size())
		{
			bitmap = iBitmaps[i];
			break;
		}
	}
	if(!bitmap)
	{
		TSize brushSize;
		if(iOrientation==ColorUtils::EBitmapOrientationVertical)
			brushSize = TSize(1, Max(aDestRect.Size().iHeight,1));
		else
			brushSize = TSize(Max(aDestRect.Size().iWidth,1), 1);
		
		CFbsBitmap* gradientBitmap = new (ELeave) CFbsBitmap;
		CleanupStack::PushL(gradientBitmap);
		ColorUtils::CreateGradientBitmapL(*gradientBitmap, 
			CEikonEnv::Static()->WsSession(), 
			Max(brushSize.iWidth, brushSize.iHeight),
			iOrientation,
			iStartColor,  iEndColor);
		
		bitmap = CWritableBitmap::NewL(aDestRect.Size(), gradientBitmap->DisplayMode());
		((CWritableBitmap*)bitmap)->Gc().DrawBitmap(TRect(TPoint(0,0), aDestRect.Size()), gradientBitmap);
		CleanupStack::PopAndDestroy();
		
		iBitmaps.Append(bitmap);
	}
	
	CFbsBitmap* mask = CreateMask(aParams.iCorner, aParams.iOpacity, aDestRect.Size(), ETrue, NULL);

	if(mask)
		aGc.BitBltMasked(aDestRect.iTl, bitmap, TRect(TPoint(0,0), aDestRect.Size()), mask, ETrue);
	else
		aGc.BitBlt(aDestRect.iTl, bitmap);
	
	return ETrue;
}

//-----------------------------------------------
#ifdef __SERIES60__

CHcImageAknsBackground::~CHcImageAknsBackground()
{
	delete iAknsBackground;
}

void CHcImageAknsBackground::ConstructL()
{
	TRect rect;
#ifdef __SYMBIAN_8_ONWARDS__
	AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EScreen, rect);
#else
	rect = TRect(TPoint(0,0), TSize(176,208));
#endif
	if(iLocation.iIndex1>0) 
	{
		TAknsItemID itemId = { iLocation.iIndex1, iLocation.iIndex2 };
		iAknsBackground = CAknsBasicBackgroundControlContext::NewL(itemId, rect, EFalse );
	}
	iLoaded = ETrue;
	iSize = rect.Size();
}

TBool CHcImageAknsBackground::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams)
{
	if(iAknsBackground)
	{
		TPoint screenPosition(0,0);
		if(aParams.iOwningControl)
			screenPosition = aParams.iOwningControl->PositionRelativeToScreen();
		TRect rect = TRect(aDestRect.iTl + screenPosition, aDestRect.Size());
		AknsDrawUtils::DrawBackground( AknsUtils::SkinInstance(),
		     iAknsBackground,
		     NULL,
		     aGc,
		     aDestRect.iTl,
		     rect,
		     KAknsDrawParamDefault );
	}
	
	return ETrue;
}

TBool CHcImageAknsBackground::Refresh(TInt aType)
{
	if(aType==ESystemResourceChanged)
	{
		delete iAknsBackground;
		iAknsBackground = NULL;
		
		Construct();
		
		return ETrue;
	}
	else
		return EFalse;
}

//-----------------------------------------------
CHcImageAknsFrame::~CHcImageAknsFrame()
{
	delete iGcProxy;
}

void CHcImageAknsFrame::ConstructL()
{
	iLoaded = ETrue;
	iSize = TSize(100,100);
}

TBool CHcImageAknsFrame::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams)
{
	TAknsItemID aCenterId;
	aCenterId.Set(iLocation.iIndex1, iLocation.iIndex2+9);
	
	TRect innerRect = aDestRect;
	innerRect.Shrink(aParams.iCorner);
	
	if(!iGcProxy)
	{
		iGcProxy = new CGcProxy(&aGc);
	}
	else if(iGcProxy->iRealContext!=&aGc)
	{
		delete iGcProxy;
		iGcProxy = NULL;
		
		iGcProxy = new CGcProxy(&aGc);
	}
	
	TAknsItemID itemId = { iLocation.iIndex1, iLocation.iIndex2 };
	AknsDrawUtils::DrawFrame(AknsUtils::SkinInstance(), *iGcProxy, aDestRect, innerRect, itemId, aCenterId);
	
	return ETrue;
}

//-----------------------------------------------
void CHcImageAknsIcon::ConstructL()
{
	TAknsItemID itemId;
	itemId.iMajor = iLocation.iIndex1;
	itemId.iMinor = iLocation.iIndex2;
#ifdef __SYMBIAN_8_ONWARDS__
	AknsUtils::CreateIconL(AknsUtils::SkinInstance(), itemId,  iBitmap, iBitmapMask, KNullDesC, 0,0);
#else
	CGulIcon* icon = AknsUtils::CreateGulIconL(AknsUtils::SkinInstance(), itemId, ETrue);
	icon->SetBitmapsOwnedExternally(ETrue);
	iBitmap = icon->Bitmap();
	iBitmapMask = icon->Mask();
	delete icon;
#endif
	iLoaded = ETrue;
#ifdef __SERIES60_3_ONWARDS__
	iSize = TSize(44, 44);
	AknIconUtils::SetSize(iBitmap, iSize);
#else
	iSize = iBitmap->SizeInPixels();
#endif
}

#endif

//-----------------------------------------------
#ifdef __UIQ__

CHcImageUIQSkin::~CHcImageUIQSkin()
{
}

void CHcImageUIQSkin::ConstructL()
{
	const CSkinPatch& sp = SkinManager::SkinPatch(TUid::Uid(iLocation.iIndex1), iLocation.iIndex2, NULL);
	iSize = sp.SizeInPixels();
	iLoaded = ETrue;
}

TBool CHcImageUIQSkin::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams)
{
	TUid skinUid = TUid::Uid(iLocation.iIndex1);
	const CSkinPatch& sp = SkinManager::SkinPatch(skinUid, iLocation.iIndex2, NULL);

	if(skinUid==KSkinUidScreen)
	{
		TRect sourceRect(TRect(TPoint(0,0),iSize));
		if(aParams.iOwningControl)
			sourceRect.Move(-aParams.iOwningControl->PositionRelativeToScreen());
		sp.DrawBitmap(aGc, sourceRect, aDestRect, iLocation.iIndex3);
	}
	else
		sp.DrawBitmap(aGc, aDestRect, aDestRect, iLocation.iIndex3);

	return ETrue;
}

//-----------------------------------------------
CHcImageUIQIcon::~CHcImageUIQIcon()
{
	delete iContent;
	delete iGcProxy;
}

void CHcImageUIQIcon::ConstructL()
{
	iContent = new CQikContent();
	iContent->SetContentL(NULL, iLocation.iFileName);
	iContent->SetStretched(ETrue);
	iContent->SetStretchMaintainAspectRatio(EFalse);
	
	iLoaded = ETrue;
	if(iContent->Bitmap())
		iSize = iContent->Bitmap()->SizeInPixels();
	else
		iSize = TSize(0,0);
}

TBool CHcImageUIQIcon::DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams&)
{
	if(!iGcProxy)
	{
		iGcProxy = new CGcProxy(&aGc);
	}
	else if(iGcProxy->iRealContext!=&aGc)
	{
		delete iGcProxy;
		iGcProxy = NULL;
		
		iGcProxy = new CGcProxy(&aGc);
	}
	iContent->Draw(*iGcProxy, aDestRect, aDestRect);

	return ETrue;
}

#endif

//-----------------------------------------------
CHcDecodeProcess* CHcDecodeProcess::NewL(CHcImageGeneral* aImage) 
{
	CHcDecodeProcess* self = new (ELeave)CHcDecodeProcess(aImage);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	
	return self;
}

CHcDecodeProcess::CHcDecodeProcess(CHcImageGeneral* aImage):CActive(EPriorityLow),iImage(aImage)
{
	CActiveScheduler::Add(this);	
}

CHcDecodeProcess::~CHcDecodeProcess()
{
	Cancel();
	if(iDecoder)
		CHtmlCtlEnv::Static()->ImagePool().DeleteImageDecoder(iDecoder);
	
	iImage->iDecodeProcess = NULL; //important
	iImage->DecodeFinished();
}

void CHcDecodeProcess::ConstructL()
{
	iDecoder = CHtmlCtlEnv::Static()->ImagePool().NewImageDecoderL(iImage->iLocation.iFileName);
	if(!iDecoder) 
		return;

	TInt cnt = iDecoder->FrameCount();
	if(cnt==0)
		User::Leave(KErrCorrupt);
	
	for(TInt i=0;i<cnt;i++)
	{
		CHcImageGeneral::TFrame frame;
		
		TFrameInfo frameInfo = iDecoder->FrameInfo(i);
		frame.iPosition = frameInfo.iFrameCoordsInPixels.iTl;
		frame.iBackColor = frameInfo.iBackgroundColor;
		frame.iSize = frameInfo.iOverallSizeInPixels;
		if(i==0)
			iImage->iSize = frameInfo.iOverallSizeInPixels;
		frame.iFlags = frameInfo.iFlags;
		frame.iDelay = frameInfo.iDelay;
		if(frame.iDelay.Int64()==0)
			frame.iDelay = 100*1000;
		iImage->iFrames.Append(frame);
	}
	
	ConvertL();
}

void CHcDecodeProcess::ContinueDecode() 
{
	TRAPD(error, 
		ConstructL();
	);
	if(error!=KErrNone) 
	{
#ifdef __WINSCW__
		RDebug::Print(_L("CHcDecodeProcess: decode failed %i: %S"), error, &iImage->iLocation.iFileName);
#endif
		iImage->iError = error;
		delete this;
	}
}

void CHcDecodeProcess::ConvertL()
{
	CHcImageGeneral::TFrame& frame = iImage->iFrames[iResolvingFrame];
	frame.iBitmap = new (ELeave) CFbsBitmap();
	frame.iBitmap->Create( frame.iSize, iDecoder->FrameInfo(iResolvingFrame).iFrameDisplayMode );

	if(frame.iFlags & TFrameInfo::ETransparencyPossible)
	{
		frame.iBitmapMask = new (ELeave) CFbsBitmap();
		if(frame.iFlags & TFrameInfo::EAlphaChannel)
			frame.iBitmapMask->Create(frame.iSize, EGray256);
		else
			frame.iBitmapMask->Create(frame.iSize, EGray2);
	}
	
	if(frame.iBitmapMask)
		iDecoder->Convert( &iStatus, *frame.iBitmap, *frame.iBitmapMask,  iResolvingFrame);
	else
		iDecoder->Convert( &iStatus, *frame.iBitmap , iResolvingFrame);
	
	SetActive();
}

void CHcDecodeProcess::RunL()
{
	iImage->iLoaded = ETrue;
	if(iStatus!=KErrNone) 
	{
#ifdef __WINSCW__
		RDebug::Print(_L("CHcDecodeProcess: decode failed %i: %S"), iStatus.Int(), &iImage->Location().iFileName);
#endif
		
		iImage->iError = iStatus.Int();
		delete this;
	}
	else 
	{
		iImage->iFrames[iResolvingFrame].iConverted = ETrue;
		iResolvingFrame++;
		if(iResolvingFrame<iImage->iFrames.Count()) 
			ConvertL();
		else
			delete this;
	}
}

void CHcDecodeProcess::DoCancel()
{
	if(iDecoder) 
		iDecoder->Cancel();
}

TInt CHcDecodeProcess::RunError(TInt aError)
{
	iImage->iError = aError;
	delete this;
	return KErrNone;
}

CFbsBitmap* CreateMask(const TSize& aCorner, TInt aOpacity, const TSize& aSize, TBool aInvertMask, CFbsBitmap* aOrginal)
{
	CFbsBitmap* mask =  aOrginal;
	if(aCorner.iWidth!=0 || aCorner.iHeight!=0 || aOpacity!=255) 
	{
		CWritableBitmap* helper = CHtmlCtlEnv::Static()->HelperBitmap(1, aSize);
		helper->Gc().SetBrushStyle(CGraphicsContext::ESolidBrush);
		helper->Gc().SetPenStyle(CGraphicsContext::ENullPen);
		
		if(aCorner.iWidth!=0 || aCorner.iHeight!=0) 
		{
			helper->Gc().SetBrushColor(TRgb::Gray256(0));
			helper->Gc().Clear();
			
			helper->Gc().SetBrushColor(TRgb::Gray256(aOpacity));
			if(aCorner.iWidth==9999)
				helper->Gc().DrawEllipse(TRect(TPoint(0,0), aSize));
			else
				helper->Gc().DrawRoundRect(TRect(TPoint(0,0), aSize), aCorner);
		}
		else 
		{
			helper->Gc().SetBrushColor(TRgb::Gray256(aOpacity));
			helper->Gc().Clear();
		}

		if(mask)
		{
			helper->Gc().SetBrushStyle(CGraphicsContext::ENullBrush);
			if(mask->DisplayMode()==EGray2)
				helper->Gc().BitBltMasked(TPoint(0,0), mask, TRect(TPoint(0,0), aSize), mask, !aInvertMask);
			else 
			{
				TBitmapUtil bu1(mask),bu2(helper);
				bu1.Begin(TPoint(0,0));
				bu2.Begin(TPoint(0,0));
				for(TInt i=0;i<aSize.iHeight;i++) 
				{
					bu1.SetPos(TPoint(0, i));
					bu2.SetPos(TPoint(0, i));
					for(TInt j=0;j<aSize.iWidth;j++) 
					{
						TUint32 c = bu1.GetPixel();
						c = c * aOpacity/255;
						bu2.SetPixel(c);
						bu1.IncXPos();
						bu2.IncXPos();
					}
				}
				bu1.End();
				bu2.End();
			}
		}
		
		mask = helper;
	}
	
	return mask;
}


