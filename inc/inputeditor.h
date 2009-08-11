#ifndef HCINPUTEDITOR_H
#define HCINPUTEDITOR_H

#include <eikedwin.h>
#include <lafmain.h>

#include "htmlcontrol.hrh"
#include "style.h"

class CHtmlControl;

class CHcCustomDraw : public CLafEdwinCustomDrawBase
    {
public:
	static CHcCustomDraw* NewL(const MLafEnv& aEnv,const CCoeControl& aControl);
	virtual ~CHcCustomDraw();

public: // from MFormCustomDraw
	void DrawBackground(const TParam& aParam,const TRgb& aBackground,TRect& aDrawn) const;

private:
	CHcCustomDraw(const MLafEnv& aEnv,const CCoeControl& aControl);
    void ConstructL();
};

class CHcInputEditor : public CEikEdwin
{
public:
	CHcInputEditor(CHtmlControl* aOwner, TBool aMultiline);
	~CHcInputEditor();
	void SetDrawLines(TBool aDrawLines);
	void SetLineColor(THcColor aColor);
	void SetFontL(const THcTextStyle& aStyle);
	inline TInt RowHeight() const;
	inline CFont* Font() const;
	
protected:
	CLafEdwinCustomDrawBase* CreateCustomDrawL();
	
private:
    TBool iDrawLines;
    THcColor iLineColor;
    CHtmlControl* iOwner;
	TBool iStyleSet;
	TInt iRowHeight;
	CFont* iFont;
	TBool iMultiLine;
    
	CCharFormatLayer* iCharFormatLayer;
	CParaFormatLayer* iParaFormatLayer;
	
    friend class CHcCustomDraw;
};

inline TInt CHcInputEditor::RowHeight() const
{
	return iRowHeight;
}

inline CFont* CHcInputEditor::Font() const
{
	return iFont;
}

#endif
