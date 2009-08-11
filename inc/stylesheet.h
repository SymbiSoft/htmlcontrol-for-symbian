#ifndef HCSTYLESHEET_H
#define HCSTYLESHEET_H

#include "htmlcontrol.hrh"
#include "style.h"

class THcSelector
{
public:
	enum TFakeClass
	{
		EFCNone,
		EFCHover,
		EFCLink,
		EFCFocus,
		EFCDown,
		EFCError
	};
	TBuf<10> iTag;
	TBuf<30> iClass;
	TInt iFakeClass;
	TInt iAttribute;
	
	void Zero();
	TBool operator==(const THcSelector& aSelector) const;
	TInt Priority() const;
	TBool Select(const TDesC& aTag, const TDesC& aClass, TInt aFakeClass, TInt aAttribute) const;
};

class CHcStyleSheet : public CBase
{
public:
	CHcStyleSheet();
	virtual ~CHcStyleSheet();
	
	void AddL(const THcSelector& aSelector, const TDesC& aSource);
	void Remove(const THcSelector& aSelector);
	void Clear();
	void Sort();
	
	TBool GetStyle(const TDesC& aTag, const TDesC& aClass, TInt aFakeClass, TInt aAttribute, CHcStyle& aStyle) const;

	void Assign(const CHcStyleSheet& aSource);
	
	inline TUint16 Version() const;

private:
	struct THCHcStyleSheetItem
	{
		THcSelector iSelector;
		CHcStyle* iStyle;
	};
	RArray<THCHcStyleSheetItem> iItems;
	TUint16 iVersion;

	static TInt CompareSelector(const THCHcStyleSheetItem & a, const THCHcStyleSheetItem & b);
};

inline TUint16 CHcStyleSheet::Version() const
{
	return iVersion;
}

#endif
