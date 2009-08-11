#ifndef HCIMAGEIMPL_H
#define HCIMAGEIMPL_H

#include "htmlcontrol.hrh"

#ifdef __SERIES60_3_ONWARDS__
#include <AknIconUtils.h> 
#endif

#include "image.h"

class CHcDecodeProcess;
class CImageDecoder;
class CGcProxy;
#ifdef __UIQ__
class CQikContent;
#endif

class CHcImageNull : public CHcImage
{
protected:	
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);
};

class CHcImageGeneral : public CHcImage
{
public:
	struct TFrame
	{
		CFbsBitmap* iBitmap;
		CFbsBitmap* iBitmapMask;
		TUint32 iFlags;
		TPoint iPosition;
		TSize iSize;
		TRgb iBackColor;
		TTimeIntervalMicroSeconds iDelay;
		TBool iConverted;
		
		inline TFrame();
	};
	
	virtual ~CHcImageGeneral();
	virtual TBool Refresh(TInt aType);
	
	void DecodeFinished();
	
	inline const TFrame& Frame(TInt iIndex) const;
	inline TInt FrameCount() const;
	
protected:	
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);

	void Reset();
private:
	RArray<TFrame> iFrames;
	CHcDecodeProcess* iDecodeProcess;
	
	friend class CHcDecodeProcess;
	friend class CHcImagePool;
};

class CHcImageMbm : public CHcImage
{
public:
	virtual ~CHcImageMbm();
	virtual TBool Refresh(TInt aType);
	
protected:
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);

	CFbsBitmap* iBitmap;
	CFbsBitmap* iBitmapMask;
};

#ifdef __UIQ__
class CHcImageSvg : public CHcImageMbm
{
public:
	virtual ~CHcImageSvg();
	TBool Refresh(TInt aType);
	
protected:
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);
	
private:
	CQikContent* iContent;
	CGcProxy* iGcProxy;
};
#endif


#ifdef __SERIES60__

#ifdef __SERIES60_3_ONWARDS__
class CHcIconFileProvider : public MAknIconFileProvider
{
public:
	CHcIconFileProvider(const TDesC& aFileName);
	virtual void RetrieveIconFileHandleL(RFile &aFile, const TIconFileType aType);
	virtual void Finished();
	
private:
	TFileName iFileName;
};
#endif

class CHcImageSvg : public CHcImageMbm
{
protected:
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);
};
#endif


class CHcImageAppIcon : public CHcImageSvg
{
public:
	virtual ~CHcImageAppIcon();
	TBool Refresh(TInt aType);
	
protected:
	virtual void ConstructL();
	
private:
	CApaMaskedBitmap* iApaMaskedBitmap;
};

class CHcImageGradient : public CHcImage
{
public:
	virtual ~CHcImageGradient();

protected:
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);

private:
	RPointerArray<CFbsBitmap> iBitmaps;
	ColorUtils::TBitmapOrientation iOrientation;
	TRgb iStartColor;
	TRgb iEndColor;
};

#ifdef __SERIES60__

class MAknsControlContext;
class CHcImageAknsBackground : public CHcImage
{
public:
	virtual ~CHcImageAknsBackground();
	virtual TBool Refresh(TInt aType);
	
protected:
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);
	
private:
	MAknsControlContext* iAknsBackground;
};

class CHcImageAknsFrame : public CHcImage
{
public:
	virtual ~CHcImageAknsFrame();

protected:	
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);

private:
	CGcProxy* iGcProxy;
};

class CHcImageAknsIcon : public CHcImageSvg
{
protected:	
	virtual void ConstructL();
};

#endif

#ifdef __UIQ__

class CHcImageUIQSkin : public CHcImage
{
public:
	virtual ~CHcImageUIQSkin();
	
protected:	
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);
	

};

class CHcImageUIQIcon : public CHcImage
{
public:
	virtual ~CHcImageUIQIcon();
	
protected:	
	virtual void ConstructL();
	virtual TBool DoDraw(CBitmapContext& aGc, const TRect& aDestRect, const THcDrawImageParams& aParams);

private:
	CQikContent* iContent;
	CGcProxy* iGcProxy;
};

#endif

class CHcDecodeProcess : public CActive
{
public:
	static CHcDecodeProcess* NewL(CHcImageGeneral* aImage);
	virtual ~CHcDecodeProcess();

	void ContinueDecode();
	
protected:
	CHcDecodeProcess(CHcImageGeneral* aImage);
	void ConstructL();
	void ConvertL();
	
protected:
	void RunL();
	TInt RunError(TInt aError);
	void DoCancel();
	
private:
	TInt iResolvingFrame;
	CImageDecoder* iDecoder;
	CHcImageGeneral* iImage;
	
	friend class CHcImagePool;
};

inline CHcImageGeneral::TFrame::TFrame() 
{
	iBitmap = NULL;
	iBitmapMask = NULL;
	iFlags = 0;
	iPosition = TPoint(0,0);
	iSize = TSize(0,0);
	iBackColor = KRgbWhite;
	iDelay = 0;
	iConverted = EFalse;
}

inline const CHcImageGeneral::TFrame& CHcImageGeneral::Frame(TInt iIndex) const
{
	return iFrames[iIndex];
}

inline TInt CHcImageGeneral::FrameCount() const
{
	return iFrames.Count();
}

#endif
