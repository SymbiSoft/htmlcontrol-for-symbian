#include <eikenv.h>

#include "htmlcontrol.hrh"
#include "htmlcontrol.h"
#include "htmlelement.h"
#include "htmlelementiter.h"
#include "elementimpl.h"

CHtmlElement::CHtmlElement(CHtmlControl* aOwner)
{
	iOwner = aOwner;
}
	
CHtmlElement::~CHtmlElement()
{
	delete iRemark;
}

CHtmlElement* CHtmlElement::Parent() const
{
	return (CHtmlElement*)((CHtmlElementImpl*)this)->iParent;
}

TRect CHtmlElement::Rect() const
{
	return ((CHtmlElementImpl*)this)->Rect();
}

CHtmlElement* CHtmlElement::Element(const TDesC& aId, TInt aIndex) const
{
	THtmlElementIter it(this, aId, KNullDesC);
	while(it.Next())
	{
		if(aIndex--==0)
			return it();
	}
	
	return NULL;
}

CHtmlElement* CHtmlElement::ElementByTag(const TDesC& aTagName, TInt aIndex) const
{
	THtmlElementIter it(this, KNullDesC, aTagName);
	while(it.Next())
	{
		if(aIndex--==0)
			return it();
	}
	
	return NULL;
}

TInt CHtmlElement::Index(const CHtmlElement* aContainer) const 
{
	THtmlElementIter it(aContainer, Id());
	TInt i = 0;
	while(it.Next())
	{
		if(it()==this)
			return i;
		i++;
	}
	return -1;
}

TBool CHtmlElement::GetProperty(const TDesC& aName, TInt& aResult) const
{
	TBuf<20> buf;
	if(GetProperty(aName, buf)) 
	{
		TLex lex(buf);
		lex.Val(aResult);
		return ETrue;
	}
	else
		return EFalse;
}

TBool CHtmlElement::GetStyle(const TDesC& aStyle, TDes& aBuffer) const
{
	TBuf<30> tmp;
	tmp.Copy(KHStrStyleDot);
	tmp.Append(aStyle);
	return GetProperty(tmp, aBuffer);
}

TBool CHtmlElement::GetStyle(const TDesC& aStyle, TInt aBuffer) const
{
	TBuf<20> tmp;
	if(GetStyle(aStyle, tmp))
	{
		TLex lex(tmp);
		lex.Val(aBuffer);
		
		return ETrue;
	}
	else
		return EFalse;
}

TBool CHtmlElement::SetProperty(const TDesC& aName, TInt aValue)
{
	TBuf<20> tmp;
	tmp.Num(aValue);
	return SetProperty(aName, tmp);
}

TBool CHtmlElement::SetStyle(const TDesC& aStyle, const TDesC& aValue)
{
	TBuf<30> tmp;
	tmp.Copy(KHStrStyleDot);
	tmp.Append(aStyle);
	return SetProperty(tmp, aValue);
}

TBool CHtmlElement::SetStyle(const TDesC& aStyle, TInt aValue)
{
	TBuf<20> tmp;
	tmp.Num(aValue);
	return SetStyle(aStyle, tmp);
}

