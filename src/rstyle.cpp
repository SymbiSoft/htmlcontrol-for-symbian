#include <eikenv.h>

#include "controlimpl.h"
#include "elementimpl.h"
#include "htmlctlenv.h"
#include "rstyle.h"
#include "imagepool.h"
#include "utils.h"
#include "htmlparser.h"

RHcAbstractStyle::RHcAbstractStyle()
{
	iClass = HBufC::NewL(0);
}

void RHcAbstractStyle::Close()
{
	delete iFragmentStyle;
	delete iStyle;
	delete iClass;
}

void RHcAbstractStyle::SetClass(const TDesC& aClass)
{
	if(iClass->Des().CompareF(aClass)==0)
		return;
	
	delete iClass;
	iClass = NULL;
	iClass = aClass.AllocL();
	iVersion = 0;
}

void RHcAbstractStyle::SetStyleL(const TDesC& aSource)
{
	if(!iFragmentStyle)
	{
		if(aSource.Length()>0)
		{
			iFragmentStyle = new (ELeave) CHcStyle();
			HtmlParser::ParseStylesL(aSource, *iFragmentStyle);
			iVersion = 0;
		}
	}
	else
	{
		HtmlParser::ParseStylesL(aSource, *iFragmentStyle);
		iVersion = 0;
	}
}

void RHcAbstractStyle::SetSingleStyleL(const TDesC& aName, const TDesC& aValue)
{
	if(!iFragmentStyle)
		iFragmentStyle = new (ELeave) CHcStyle();
	HtmlParser::ParseSingleStyleL(aName, aValue, *iFragmentStyle);
	iVersion = 0;
}

TBool RHcAbstractStyle::IsInvisibleStyle() const
{
	if(iFragmentStyle)
		return iFragmentStyle->IsDisplayNone() || iFragmentStyle->IsHidden(); 
	else
		return EFalse;
}

void RHcAbstractStyle::GetSingleStyle(const TDesC& aName, TDes& aBuf) const
{
	if(iFragmentStyle)
		return HtmlParser::GetStyleString(*iFragmentStyle, aName, aBuf);
	else
		aBuf.Zero();
}

void RHcAbstractStyle::SetTag(const TDesC& aTag)
{
	if(iTag.Compare(aTag)!=0)
	{
		iTag.Set(aTag);
		iVersion = 0;
	}
}

void RHcAbstractStyle::SetFakeClass(TInt aFakeClass)
{
	if(iFakeClass!=aFakeClass)
	{
		iFakeClass = aFakeClass;
		iVersion = 0;
	}
}
void RHcAbstractStyle::SetAttribute(TInt aAttribute)
{
	if(iAttribute!=aAttribute)
	{
		iAttribute = aAttribute;
		iVersion = 0;
	}
}

TBool RHcAbstractStyle::Update(CHtmlControlImpl* aControl)
{
	if(iVersion==aControl->StyleSheetVersion())
		return EFalse;

	#ifdef __WINSCW__
	//RDebug::Print(_L("Update stylesheet %S, %S"), &iTag, &Class());
	#endif
	iVersion = aControl->StyleSheetVersion();
	iStyle->ClearAll();
	aControl->GetStyle(iTag, Class(), iFakeClass, iAttribute, *iStyle);
	if(iFragmentStyle)
		iStyle->Add(*iFragmentStyle);

	return ETrue;
}

void RHcAbstractStyle::CopyL(const RHcAbstractStyle& aStyle)
{
	iTag.Set(aStyle.iTag);
	iFakeClass = aStyle.iFakeClass;
	iAttribute = aStyle.iAttribute;
	
	delete iClass;
	iClass = NULL;
	if(aStyle.iClass)
		iClass = aStyle.iClass->AllocL();
	
	if(iFragmentStyle)
		iFragmentStyle->ClearAll();
	if(aStyle.iFragmentStyle)
	{
		if(!iFragmentStyle)
			iFragmentStyle = new (ELeave)CHcStyle();		
		iFragmentStyle->Add(*aStyle.iFragmentStyle);
	}
	
	if(iStyle)
		iStyle->ClearAll();
	if(aStyle.iStyle)
	{
		if(!iStyle)
			iStyle = new (ELeave)CHcStyle();
		iStyle->Add(*aStyle.iStyle);
	}
	iVersion = aStyle.iVersion;
}

RHcStyle::RHcStyle()
{
	iStyle = new CHcStyle();
}

TBool RHcLightStyle::Update(CHtmlControlImpl* aControl)
{
	if(iVersion==aControl->StyleSheetVersion())
		return EFalse;

	iVersion = aControl->StyleSheetVersion();
	
	if(iClass->Length()==0 && !iFragmentStyle)
	{
		if(iStyle)
		{
			iStyle->ClearAll();
			return ETrue;
		}
		else
			return EFalse;
	}
	else
	{
		if(!iStyle)
			iStyle = new CHcStyle();
		else
			iStyle->ClearAll();
		aControl->GetStyle(iTag, Class(), iFakeClass, iAttribute, *iStyle);
		if(iFragmentStyle)
			iStyle->Add(*iFragmentStyle);
		
		return ETrue;
	}
}


