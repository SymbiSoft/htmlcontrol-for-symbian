#include "gcproxy.h"

CGcProxy::CGcProxy(CWindowGc* aGc):CWindowGc(CCoeEnv::Static()->ScreenDevice())
{
	iRealContext = aGc;
	iTypeWindowGc = ETrue;
}

CGcProxy::CGcProxy(CFbsBitGc* aGc):CWindowGc(CCoeEnv::Static()->ScreenDevice())
{
	iRealContext = aGc;	
}

CGcProxy::CGcProxy(CBitmapContext* aGc):CWindowGc(CCoeEnv::Static()->ScreenDevice())
{
	iRealContext = aGc;	
}

TInt CGcProxy::Construct()
{//no need to construct
	return KErrNone;
}

void CGcProxy::Activate(RDrawableWindow &/*aDevice*/)
{
}

void CGcProxy::Deactivate()
{
}

CGraphicsDevice* CGcProxy::Device() const 
{
	return iRealContext->Device();
}

void CGcProxy::SetOrigin(const TPoint &aPoint)
{
	iRealContext->SetOrigin(aPoint);
}

void CGcProxy::SetDrawMode(TDrawMode aDrawingMode)
{
	iRealContext->SetDrawMode(aDrawingMode);
}

void CGcProxy::SetClippingRect(const TRect& aRect)
{
	iRealContext->SetClippingRect(aRect);
}

void CGcProxy::CancelClippingRect()
{
	iRealContext->CancelClippingRect();
}

void CGcProxy::Reset()
{
	iRealContext->Reset();
}

void CGcProxy::UseFont(const CFont *aFont)
{
	iRealContext->UseFont(aFont);
}

void CGcProxy::DiscardFont()
{
	iRealContext->DiscardFont();
}

void CGcProxy::SetUnderlineStyle(TFontUnderline aUnderlineStyle)
{
	iRealContext->SetUnderlineStyle(aUnderlineStyle);
}

void CGcProxy::SetStrikethroughStyle(TFontStrikethrough aStrikethroughStyle)
{
	iRealContext->SetStrikethroughStyle(aStrikethroughStyle);
}

void CGcProxy::SetWordJustification(TInt aExcessWidth,TInt aNumGaps)
{
	iRealContext->SetWordJustification(aExcessWidth, aNumGaps);
}

void CGcProxy::SetCharJustification(TInt aExcessWidth,TInt aNumChars)
{
	iRealContext->SetCharJustification(aExcessWidth, aNumChars);
}

void CGcProxy::SetPenColor(const TRgb &aColor)
{
	iRealContext->SetPenColor(aColor);
}

void CGcProxy::SetPenStyle(TPenStyle aPenStyle)
{
	iRealContext->SetPenStyle(aPenStyle);
}

void CGcProxy::SetPenSize(const TSize& aSize)
{
	iRealContext->SetPenSize(aSize);
}

void CGcProxy::SetBrushColor(const TRgb &aColor)
{
	iRealContext->SetBrushColor(aColor);
}

void CGcProxy::SetBrushStyle(TBrushStyle aBrushStyle)
{
	iRealContext->SetBrushStyle(aBrushStyle);
}

void CGcProxy::SetBrushOrigin(const TPoint &aOrigin)
{
	iRealContext->SetBrushOrigin(aOrigin);
}

void CGcProxy::UseBrushPattern(const CFbsBitmap *aDevice)
{
	iRealContext->UseBrushPattern(aDevice);
}

void CGcProxy::DiscardBrushPattern()
{
	iRealContext->DiscardBrushPattern();
}

void CGcProxy::MoveTo(const TPoint &aPoint)
{
	iRealContext->MoveTo(aPoint);
}

void CGcProxy::MoveBy(const TPoint &aPoint)
{
	iRealContext->MoveBy(aPoint);
}

void CGcProxy::Plot(const TPoint &aPoint)
{
	iRealContext->Plot(aPoint);
}

void CGcProxy::DrawArc(const TRect &aRect,const TPoint &aStart,const TPoint &aEnd)
{
	iRealContext->DrawArc(aRect, aStart, aEnd);
}

void CGcProxy::DrawLine(const TPoint &aPoint1,const TPoint &aPoint2)
{
	iRealContext->DrawLine(aPoint1, aPoint2);
}

void CGcProxy::DrawLineTo(const TPoint &aPoint)
{
	iRealContext->DrawLineTo(aPoint);
}

void CGcProxy::DrawLineBy(const TPoint &aPoint)
{
	iRealContext->DrawLineBy(aPoint);
}

void CGcProxy::DrawPolyLine(const CArrayFix<TPoint> *aPointList)
{
	iRealContext->DrawPolyLine(aPointList);
}

void CGcProxy::DrawPolyLine(const TPoint* aPointList,TInt aNumPoints)
{
	iRealContext->DrawPolyLine(aPointList, aNumPoints);
}

void CGcProxy::DrawPie(const TRect &aRect,const TPoint &aStart,const TPoint &aEnd)
{
	iRealContext->DrawPie(aRect, aStart, aEnd);
}

void CGcProxy::DrawEllipse(const TRect &aRect)
{
	iRealContext->DrawEllipse(aRect);
}

void CGcProxy::DrawRect(const TRect &aRect)
{
	iRealContext->DrawRect(aRect);
}

void CGcProxy::DrawRoundRect(const TRect &aRect,const TSize &aEllipse)
{
	iRealContext->DrawRoundRect(aRect, aEllipse);
}

TInt CGcProxy::DrawPolygon(const CArrayFix<TPoint> *aPointList,TFillRule aFillRule)
{
	return iRealContext->DrawPolygon(aPointList, aFillRule);
}

TInt CGcProxy::DrawPolygon(const TPoint* aPointList,TInt aNumPoints,TFillRule aFillRule)
{
	return iRealContext->DrawPolygon(aPointList, aNumPoints, aFillRule);
}

void CGcProxy::DrawBitmap(const TPoint &aTopLeft, const CFbsBitmap *aDevice)
{
	iRealContext->DrawBitmap(aTopLeft, aDevice);
}

void CGcProxy::DrawBitmap(const TRect &aDestRect, const CFbsBitmap *aDevice)
{
	iRealContext->DrawBitmap(aDestRect, aDevice);
}

void CGcProxy::DrawBitmap(const TRect &aDestRect, const CFbsBitmap *aDevice, const TRect &aSourceRect)
{
	iRealContext->DrawBitmap(aDestRect, aDevice, aSourceRect);
}

#ifdef __SYMBIAN_9_ONWARDS__
void CGcProxy::DrawBitmapMasked(const TRect& aDestRect, const CFbsBitmap* aBitmap, const TRect& aSourceRect, const CFbsBitmap* aMaskBitmap, TBool aInvertMask)
{
	iRealContext->DrawBitmapMasked(aDestRect, aBitmap, aSourceRect, aMaskBitmap, aInvertMask);
}

void CGcProxy::DrawBitmapMasked(const TRect& aDestRect, const CWsBitmap* aBitmap, const TRect& aSourceRect, const CWsBitmap* aMaskBitmap, TBool aInvertMask)
{
	iRealContext->DrawBitmapMasked(aDestRect, aBitmap, aSourceRect, aMaskBitmap, aInvertMask);
}
#endif

void CGcProxy::DrawText(const TDesC &aBuf,const TPoint &aPos)
{
	iRealContext->DrawText(aBuf, aPos);
}

void CGcProxy::DrawText(const TDesC &aBuf,const TRect &aBox,TInt aBaselineOffset,TTextAlign aHoriz,TInt aLeftMrg)
{
	iRealContext->DrawText(aBuf, aBox, aBaselineOffset, aHoriz, aLeftMrg );
}

void CGcProxy::Clear()
{
	iRealContext->Clear();
}

void CGcProxy::Clear(const TRect &aRect)
{
	iRealContext->Clear(aRect);
}

void CGcProxy::CopyRect(const TPoint &anOffset,const TRect &aRect)
{
	iRealContext->CopyRect(anOffset, aRect);
}

void CGcProxy::BitBlt(const TPoint &aPos, const CFbsBitmap *aBitmap)
{
	iRealContext->BitBlt(aPos, aBitmap);
}

void CGcProxy::BitBlt(const TPoint &aDestination, const CFbsBitmap *aBitmap, const TRect &aSource)
{
	iRealContext->BitBlt(aDestination, aBitmap, aSource);
}

void CGcProxy::BitBltMasked(const TPoint& aPoint,const CFbsBitmap* aBitmap,const TRect& aSourceRect,const CFbsBitmap* aMaskBitmap,TBool aInvertMask)
{
	iRealContext->BitBltMasked(aPoint, aBitmap, aSourceRect, aMaskBitmap, aInvertMask);
}

void CGcProxy::BitBlt(const TPoint &aPoint, const CWsBitmap *aBitmap)
{
	iRealContext->BitBlt(aPoint, aBitmap);
}

void CGcProxy::BitBlt(const TPoint &aDestination, const CWsBitmap *aBitmap, const TRect &aSource)
{
	iRealContext->BitBlt(aDestination, aBitmap, aSource);
}

void CGcProxy::BitBltMasked(const TPoint& aPoint,const CWsBitmap *aBitmap,const TRect& aSourceRect,const CWsBitmap *aMaskBitmap,TBool aInvertMask)
{
	iRealContext->BitBltMasked(aPoint, aBitmap, aSourceRect, aMaskBitmap, aInvertMask);
}

void CGcProxy::MapColors(const TRect& aRect,const TRgb* aColors,TInt aNumPairs,TBool aMapForwards)
{
	if(iTypeWindowGc)
		((CWindowGc *)iRealContext)->MapColors(aRect, aColors, aNumPairs, aMapForwards);
	else
		((CFbsBitGc *)iRealContext)->MapColors(aRect, aColors, aNumPairs, aMapForwards);
}

void CGcProxy::DrawTextVertical(const TDesC& aText,const TPoint& aPos,TBool aUp)
{
	if(iTypeWindowGc)
		((CWindowGc *)iRealContext)->DrawTextVertical(aText, aPos, aUp);
	else
		((CFbsBitGc *)iRealContext)->DrawTextVertical(aText, aPos, aUp);
}

void CGcProxy::DrawTextVertical(const TDesC& aText,const TRect& aBox,TInt aBaselineOffset,TBool aUp,TTextAlign aVert,TInt aMargin)
{
	if(iTypeWindowGc)
		((CWindowGc *)iRealContext)->DrawTextVertical(aText, aBox,aBaselineOffset, aUp,aVert,aMargin);
	else
		((CFbsBitGc *)iRealContext)->DrawTextVertical(aText, aBox,aBaselineOffset, aUp,aVert,aMargin);
}

void CGcProxy::SetDitherOrigin(const TPoint& /*aPoint*/)
{
//	iRealContext->SetDitherOrigin(aPoint);
}

TInt CGcProxy::SetClippingRegion(const TRegion &aRegion)
{
	if(iTypeWindowGc)
		return ((CWindowGc *)iRealContext)->SetClippingRegion(aRegion);
	else
		((CFbsBitGc *)iRealContext)->SetClippingRegion(&aRegion);
	return KErrNone;
}

void CGcProxy::CancelClippingRegion()
{
	if(iTypeWindowGc)
		((CWindowGc *)iRealContext)->CancelClippingRegion();
	else
		((CFbsBitGc *)iRealContext)->CancelClippingRegion();
}

void CGcProxy::SetOpaque(TBool /*aDrawOpaque*/)
{
//	iRealContext->SetOpaque(aDrawOpaque);
}

void CGcProxy::SetFaded(TBool aFaded)
{
	iRealContext->SetFaded(aFaded);
}

void CGcProxy::SetFadingParameters(TUint8 aBlackMap,TUint8 aWhiteMap)
{
	iRealContext->SetFadingParameters(aBlackMap, aWhiteMap);
}

#ifdef __SYMBIAN_8_ONWARDS__
#ifdef __SYMBIAN_9_ONWARDS__
TInt
#else
void
#endif
CGcProxy::AlphaBlendBitmaps(const TPoint& aDestPt, const CFbsBitmap* aSrcBmp, const TRect& aSrcRect, const CFbsBitmap* aAlphaBmp, const TPoint& aAlphaPt)
{
	if(iTypeWindowGc)
		((CWindowGc *)iRealContext)->AlphaBlendBitmaps(aDestPt, aSrcBmp, aSrcRect, aAlphaBmp, aAlphaPt);
	else
		((CFbsBitGc *)iRealContext)->AlphaBlendBitmaps(aDestPt, aSrcBmp, aSrcRect, aAlphaBmp, aAlphaPt);
	
	#ifdef __SYMBIAN_9_ONWARDS__
	return KErrNone;
	#endif
}

#ifdef __SYMBIAN_9_ONWARDS__
TInt
#else
void
#endif
CGcProxy::AlphaBlendBitmaps(const TPoint& aDestPt, const CWsBitmap* aSrcBmp, const TRect& aSrcRect, const CWsBitmap* aAlphaBmp, const TPoint& aAlphaPt)
{
	if(iTypeWindowGc)
		((CWindowGc *)iRealContext)->AlphaBlendBitmaps(aDestPt, aSrcBmp, aSrcRect, aAlphaBmp, aAlphaPt);
	else
		((CFbsBitGc *)iRealContext)->AlphaBlendBitmaps(aDestPt, aSrcBmp, aSrcRect, aAlphaBmp, aAlphaPt);
	
	#ifdef __SYMBIAN_9_ONWARDS__
	return KErrNone;
	#endif
}

#endif
