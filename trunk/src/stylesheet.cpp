#include <eikenv.h>

#include "stylesheet.h"
#include "htmlctlenv.h"
#include "htmlparser.h"

void THcSelector::Zero()
{
	iClass.Zero();
	iTag.Zero();
	iFakeClass = 0;
	iAttribute = 0;
}

TBool THcSelector::operator==(const THcSelector& aSelector) const
{
	return iClass.CompareF(aSelector.iClass)==0 
		&& iTag.CompareF(aSelector.iTag)==0 
		&& iFakeClass == aSelector.iFakeClass
		&& iAttribute == aSelector.iAttribute;
}

TInt THcSelector::Priority() const
{
	TInt m = 0;
	if(iAttribute!=0)
		m+= 1000;
	if(iFakeClass!=0)
		m += 100;
	if(iClass.Length()>0)
		m += 10;
	if(iTag.Length()>0)
		m += 1;
	return m;
}

TBool THcSelector::Select(const TDesC& aTag, const TDesC& aClass, TInt aFakeClass, TInt aAttribute) const
{
	if((iClass.Length()==0 || iClass.CompareF(aClass)==0)
			&& (iTag.Length()==0 || iTag.CompareF(aTag)==0)
			&& iFakeClass==aFakeClass
			&& (iAttribute==0 || iAttribute==aAttribute))
		return ETrue;
	else
		return EFalse;
			
}

CHcStyleSheet::CHcStyleSheet()
{
}

CHcStyleSheet::~CHcStyleSheet()
{
	Clear();
}

void CHcStyleSheet::AddL(const THcSelector& aSelector, const TDesC& aSource)
{
	iVersion++;
	
	for(TInt i=0;i<iItems.Count();i++) 
	{
		if(iItems[i].iSelector==aSelector) 
		{
			HtmlParser::ParseStylesL(aSource, *iItems[i].iStyle);
			return;
		}
	}
	
	THCHcStyleSheetItem item;
	item.iSelector = aSelector;
	item.iStyle = new (ELeave)CHcStyle();
	iItems.Append(item);
	
	HtmlParser::ParseStylesL(aSource, *item.iStyle);
}

void CHcStyleSheet::Remove(const THcSelector& aSelector)
{
	iVersion++;
	
	for(TInt i=0;i<iItems.Count();i++) 
	{
		if(iItems[i].iSelector==aSelector) 
		{
			delete iItems[i].iStyle;
			iItems.Remove( i );
			break;
		}
	}
}

void CHcStyleSheet::Clear()
{
	iVersion++;
	
	for(TInt i=0;i<iItems.Count();i++) 
		delete iItems[i].iStyle;
	iItems.Reset();
}

void CHcStyleSheet::Assign(const CHcStyleSheet& aSource)
{
	Clear();
	
	for(TInt i=0;i<aSource.iItems.Count();i++)
	{
		THCHcStyleSheetItem item;
		item.iSelector = aSource.iItems[i].iSelector;
		item.iStyle = new (ELeave)CHcStyle();
		item.iStyle->Add(*aSource.iItems[i].iStyle);
		iItems.Append(item);
	}
}

TBool CHcStyleSheet::GetStyle(const TDesC& aTag, const TDesC& aClass, TInt aFakeClass, TInt aAttribute, CHcStyle& aStyle) const 
{
	TInt ret = 0;
	for(TInt i=0;i<iItems.Count();i++) 
	{
		const THCHcStyleSheetItem& item = iItems[i];
		if(item.iSelector.Select(aTag, aClass, aFakeClass, aAttribute)) 
		{
			aStyle.Add( *item.iStyle );
			ret++;
		}
	}
	
	return ret;
}

TInt CHcStyleSheet::CompareSelector(const THCHcStyleSheetItem & a, const THCHcStyleSheetItem & b)
{
	return a.iSelector.Priority() - b.iSelector.Priority();
}

void CHcStyleSheet::Sort()
{
	TLinearOrder<THCHcStyleSheetItem> order(&CompareSelector);
	iItems.Sort(order);
}


