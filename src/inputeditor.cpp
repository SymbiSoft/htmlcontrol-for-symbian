#include <eikenv.h>
#include <TXTGLOBL.H>

#include "controlimpl.h"
#include "stylesheet.h"
#include "inputeditor.h"

CHcInputEditor::CHcInputEditor(CHtmlControl* aOwner,TBool aMultiline) 
: iOwner(aOwner),iMultiLine(aMultiline)
{
	
}

CHcInputEditor::~CHcInputEditor()
{
#ifdef __UIQ__
	delete iCharFormatLayer;
	delete iParaFormatLayer;
#endif
}

void CHcInputEditor::SetDrawLines(TBool aDrawLines)
{
	iDrawLines = aDrawLines;
}

void CHcInputEditor::SetLineColor(THcColor aColor)
{
	iLineColor = aColor;
}

void CHcInputEditor::SetFontL(const THcTextStyle& aStyle)
{
	if(iStyleSet)
		return;

	iStyleSet = ETrue;
	iFont = iOwner->Impl()->GetFont(aStyle);
	
	TFontSpec fontSpec = iFont->FontSpecInTwips();
    TCharFormat charFormat(fontSpec.iTypeface.iName, fontSpec.iHeight);
	if(aStyle.IsSet(THcTextStyle::EColor))
		charFormat.iFontPresentation.iTextColor = aStyle.iColor.Rgb();
	else
		charFormat.iFontPresentation.iTextColor = KRgbBlack;
   	if(aStyle.IsSet(THcTextStyle::EUnderline))
   		charFormat.iFontPresentation.iUnderline = EUnderlineOn;
   	else
   		charFormat.iFontPresentation.iUnderline = EUnderlineOff;
    TCharFormatMask charFormatMask;
    charFormatMask.SetAttrib(EAttFontTypeface);
    charFormatMask.SetAttrib(EAttFontHeight);
    charFormatMask.SetAttrib(EAttFontPosture);
   	charFormatMask.SetAttrib(EAttColor);

   	CCharFormatLayer* charFormatLayer = CEikonEnv::NewDefaultCharFormatLayerL();
   	CleanupStack::PushL(charFormatLayer);
   	charFormatLayer->SetL(charFormat, charFormatMask);
	
#ifdef __SERIES60__
	SetCharFormatLayer(charFormatLayer);
#endif
	
#ifdef __UIQ__
	((CGlobalText*)Text())->SetGlobalCharFormat(charFormatLayer);
	delete iCharFormatLayer;
	iCharFormatLayer = charFormatLayer;
#endif
	
	CleanupStack::Pop();//charFormatLayer
	
	if(iMultiLine)
	{
		if(aStyle.IsSet(THcTextStyle::ELineHeight)) 
			iRowHeight = aStyle.GetLineHeight(iFont->HeightInPixels());
		else
			iRowHeight = iFont->HeightInPixels() + 8; //+5
		
		TParaFormatMask paraFormatMask;
		paraFormatMask.SetAttrib(EAttLineSpacing);
		CParaFormat* paraFormat = CParaFormat::NewLC();	
		paraFormat->iLineSpacingInTwips = CCoeEnv::Static()->ScreenDevice()->VerticalPixelsToTwips(iRowHeight);
		
		CParaFormatLayer * paraFormatLayer = CEikonEnv::NewDefaultParaFormatLayerL();
		CleanupStack::PushL(paraFormatLayer);
		paraFormatLayer->SetL(paraFormat, paraFormatMask);
#ifdef __SERIES60__
		SetParaFormatLayer(paraFormatLayer);
#endif
		
#ifdef __UIQ__
		((CGlobalText*)Text())->SetGlobalParaFormat(paraFormatLayer);
		delete iParaFormatLayer;
		iParaFormatLayer = paraFormatLayer;
#endif
		CleanupStack::Pop(); //paraFormatLayer
		
		CleanupStack::PopAndDestroy(paraFormat);
	}
	
	TextView()->HandleGlobalChangeNoRedrawL();
}

CLafEdwinCustomDrawBase* CHcInputEditor::CreateCustomDrawL()
{
   return CHcCustomDraw::NewL(iEikonEnv->LafEnv(), *this);
}

CHcCustomDraw* CHcCustomDraw::NewL(const MLafEnv& aEnv,const CCoeControl& aControl)
{
    CHcCustomDraw* self = new(ELeave) CHcCustomDraw(aEnv, aControl);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
}

CHcCustomDraw::~CHcCustomDraw()
{

}

void CHcCustomDraw::DrawBackground(const TParam& aParam, const TRgb& ,TRect& aDrawn) const
{
	CHtmlControl* control = ((CHcInputEditor *)&iControl)->iOwner;
	if(!control->OffScreenBitmap())
		return;
	
	aDrawn= aParam.iDrawRect;
	TRect rect = aParam.iDrawRect;
	rect.Move(-control->Rect().iTl);
	((CBitmapContext *)&aParam.iGc)->BitBlt(aParam.iDrawRect.iTl, control->OffScreenBitmap(), rect);
	
	if(((CHcInputEditor*)&iControl)->iDrawLines) 
	{
		TRect lRect;
		((CEikEdwin*)&iControl)->TextLayout()->GetLineRect(0, lRect);
		TInt hLine = lRect.Height();
		
		TRect rect = iControl.Rect();
		
		TInt x1 = rect.iTl.iX + lRect.iTl.iX + 2;
		TInt x2 = rect.iBr.iX  - 2;
		
		TInt bot = rect.iBr.iY;
		TInt y = iControl.Position().iY + lRect.iTl.iY + hLine + 1;
		
		aParam.iGc.SetPenStyle( CGraphicsContext::ESolidPen );
		aParam.iGc.SetPenColor( ((CHcInputEditor*)&iControl)->iLineColor.Rgb() );
		while(y<bot)
		{
			aParam.iGc.DrawLine(TPoint(x1, y), TPoint(x2, y));
			y = y + hLine;
		}
	}	
}

CHcCustomDraw::CHcCustomDraw(const MLafEnv& aEnv,const CCoeControl& aControl)
 : CLafEdwinCustomDrawBase(aEnv, aControl)
{

}

void CHcCustomDraw::ConstructL()
{
}

