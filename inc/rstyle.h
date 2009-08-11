#ifndef RHCSTYLE_H
#define RHCSTYLE_H

#include "htmlcontrol.hrh"
#include "style.h"

class RHcAbstractStyle
{
public:
	RHcAbstractStyle();
	void SetClass(const TDesC& aClass);
	inline const TDesC& Class() const;
	void SetStyleL(const TDesC& aSource);
	void SetSingleStyleL(const TDesC& aName, const TDesC& aValue);
	void GetSingleStyle(const TDesC& aName, TDes& aBuf) const;
	TBool IsInvisibleStyle() const;
	
	void SetTag(const TDesC& aTag);
	inline const TDesC& Tag() const;
	void SetFakeClass(TInt aFakeClass);
	inline TInt FakeClass() const;
	void SetAttribute(TInt aAttribute);
	inline TInt Attribute() const;
	
	virtual TBool Update(CHtmlControlImpl* aControl);
	void CopyL(const RHcAbstractStyle& aStyle);

	inline TBool IsNull() const;
	inline CHcStyle& Style() const;
	void Close();
	
protected:
	HBufC* iClass;
	TPtrC iTag;
	TInt iFakeClass;
	TInt iAttribute;
	CHcStyle* iFragmentStyle;
	CHcStyle* iStyle;
	TUint32 iVersion;
};

class RHcStyle : public RHcAbstractStyle
{
public:
	RHcStyle();	
};


class RHcLightStyle : public RHcAbstractStyle
{
public:
	TBool Update(CHtmlControlImpl* aControl);
};

inline const TDesC& RHcAbstractStyle::Class() const
{
	return *iClass;
}

inline TBool RHcAbstractStyle::IsNull() const
{
	return !iStyle;
}

inline CHcStyle& RHcAbstractStyle::Style() const
{
	return *iStyle;
}

inline const TDesC& RHcAbstractStyle::Tag() const
{
	return iTag;
}

inline TInt RHcAbstractStyle::FakeClass() const
{
	return iFakeClass;
}

inline TInt RHcAbstractStyle::Attribute() const
{
	return iAttribute;
}

#endif
