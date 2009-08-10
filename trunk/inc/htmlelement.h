/*
* ============================================================================
*  Name     : CHtmlElement
*  Part of  : CHtmlControl
*  Created  : 09/06/2007 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef HTMLELEMENT_H
#define HTMLELEMENT_H

#include <coecntrl.h>
#include <babitflags.h>

//Forward declaration
class CCoeControl;
class CHtmlControl;

#define KMaxElementIdLength	50
#define KMaxTagNameLength	50

class CHtmlElement : public CBase
{
public:
	virtual ~CHtmlElement();
	
	/* ID or name property of the element.
	 * @return
	*/
	inline const TDesC& Id() const;
	
	inline CHtmlControl* Owner() const;
	inline const TDesC& TagName() const;
	inline CCoeControl* EmbedObject() const;
	
	CHtmlElement* Parent() const;	
	TRect Rect() const;
	TInt Index(const CHtmlElement* aContainer) const;
	CHtmlElement* Element(const TDesC& aId, TInt aIndex = 0) const;
	CHtmlElement* ElementByTag(const TDesC& aTagName, TInt aIndex = 0) const;

	virtual TBool GetProperty(const TDesC& aName, TDes& aBuffer) const = 0;
	TBool GetProperty(const TDesC& aName, TInt& aResult) const;
	
	TBool GetStyle(const TDesC& aStyle, TDes& aBuffer) const;
	TBool GetStyle(const TDesC& aStyle, TInt aBuffer) const;
	
	virtual TBool SetProperty(const TDesC& aName, const TDesC& aValue) = 0;
	TBool SetProperty(const TDesC& aName, TInt aValue);
	
	TBool SetStyle(const TDesC& aStyle, const TDesC& aValue);
	TBool SetStyle(const TDesC& aStyle, TInt aValue);

	virtual void InvokeL(TRefByValue< const TDesC16 > aCommand, ...) = 0;

protected:
	CHtmlElement(CHtmlControl* aOwner);

protected:
	TBuf<KMaxElementIdLength> iId;
	TPtrC iTagName;
	CHtmlControl* iOwner;
	CCoeControl* iEmbedObject;
	HBufC* iRemark;
};

//--inline functions---
inline const TDesC& CHtmlElement::Id() const
{
	return iId;
}

inline CHtmlControl* CHtmlElement::Owner() const
{
	return iOwner;
}

inline const TDesC& CHtmlElement::TagName() const
{
	return iTagName;
}

inline CCoeControl* CHtmlElement::EmbedObject() const
{
	return iEmbedObject;
}
	
#endif
