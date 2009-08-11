#include <utf.h>
#include <coeutils.h>
#include <eikenv.h>
#include <gdi.h>

#include "htmlcontrol.hrh"

#ifdef __SERIES60__
#include <aknutils.h>
#endif

#ifdef __UIQ__
#include <DeviceKeys.h>
#endif

#include "controlimpl.h"
#include "htmlctlenv.h"
#include "imagepool.h"
#include "writablebitmap.h"
#include "utils.h"

TUint32 HcUtils::StrToInt(const TDesC& aSource, TRadix aRadix)
{
	_LIT(KHexPrefix, "0x");
	_LIT(KTrue, "true");
	_LIT(KFalse, "false");
	
	if(aSource.CompareF(KTrue)==0)
		return 1;
	else if(aSource.CompareF(KFalse)==0)
		return 0;
	else if(aSource.Left(2).CompareF(KHexPrefix)==0)
	{
		TLex lex = aSource.Mid(2);
		TUint32 ret = 0;
		lex.Val(ret, EHex);
		
		return ret;	
	}
	else if(aRadix==EHex)
	{
		TLex lex = aSource;
		TUint32 ret = 0;
		lex.Val(ret, EHex);
		
		return ret;		
	}
	else 
	{
		TLex lex = aSource;
		TInt ret = 0;
		lex.Val(ret);
		
		return ret;	
	}
}

TBool HcUtils::StrToBool(const TDesC& aSource) 
{
	_LIT(K1, "1");
	_LIT(KTrue, "true");
	
	return aSource.Length()==0 || aSource.CompareF(KTrue)==0 || aSource.CompareF(K1)==0;
}

void HcUtils::Trim(TPtrC& aDest)
{
	TInt start = 0;
	while(start<aDest.Length()) 
	{
		TChar c = aDest[start];
		if(!c.IsSpace())
			break;
		
		start++;
	}
	
	if(start>=aDest.Length()) 
	{
		aDest.Set(KNullDesC);
		return;
	}
	
	TInt end = aDest.Length()-1;
	while(end>=0) 
	{
		TChar c = aDest[end];
		if(!c.IsSpace())
			break;
		
		end--;
	}
	
	aDest.Set(aDest.Mid(start, end-start+1));
}

void HcUtils::TrimRedundant(TDes& aDest)
{
	TInt writePos = 0;
	TInt first = 1;
	TInt len = aDest.Length();
	TUint16 c;
	
	for(TInt i=0;i<len;i++) 
	{
		c = aDest[i];
		if(c==' ' || c=='\t' || c==0x0B || c==0x0C)
		{
			if(first) 
			{
				aDest[writePos++] = ' ';
				first = 0;
			}
		}
		else if(c=='\r')
		{
			//ignore
		}
		else if(c=='\n')
		{
			if(first==1)
				first++;
			else if(first==2)
			{
				aDest[writePos++] = ' ';
				first = 0;
			}
		}
		else 
		{
			first = 1;
			aDest[writePos++] = c;
		}
	}
	if(writePos==1 && aDest[0]==' ')
		aDest.Zero();
	else
		aDest.SetLength(writePos);
}

TBool HcUtils::IsEmptyText(const TDesC& aSource)
{
	TInt len = aSource.Length();
	TUint16 c;
	
	for(TInt i=0;i<len;i++) 
	{
		c = aSource[i];
		if(!(c==' ' || c=='\t' || c=='\n' || c=='\r' || c==0x0B || c==0x0C || c==0xA0))
		{
			return EFalse;
		}
	}
	
	return ETrue;
}

void HcUtils::Intersection(TRect& aRect, const TRect& aOtherRect)
{
	aRect.Intersection(aOtherRect);
	if(aRect.iBr.iX<aRect.iTl.iX || aRect.iBr.iY<aRect.iTl.iY)
		aRect = TRect();
}

TBool HcUtils::Intersects(const TRect& aRect, const TRect& aOtherRect) 
{
	TRect rect = aRect;
	Intersection(rect, aOtherRect);
	return !rect.IsEmpty();
}

void HcUtils::ShrinkRect(TRect& aRect, const TMargins& aMargins)
{
	aRect.iTl.iX += aMargins.iLeft;
	aRect.iBr.iX -= aMargins.iRight;
	if(aRect.iBr.iX<aRect.iTl.iX)
		aRect.iBr.iX = aRect.iTl.iX;

	aRect.iTl.iY += aMargins.iTop;
	aRect.iBr.iY -= aMargins.iBottom;	
	if(aRect.iBr.iY<aRect.iTl.iY)
		aRect.iBr.iY = aRect.iTl.iY;
}

void HcUtils::GrowRect(TRect& aRect, const TMargins& aMargins)
{
	aRect.iTl.iX -= aMargins.iLeft;
	aRect.iBr.iX += aMargins.iRight;
	aRect.iTl.iY -= aMargins.iTop;
	aRect.iBr.iY += aMargins.iBottom;
}

HBufC* HcUtils::StringReplaceL(const TDesC& aSource, const TDesC& aTarget, const TDesC& aReplacement)
{
	HBufC* ret = HBufC::NewL(aSource.Length());
	TPtr dest = ret->Des();
	TInt pos1 = 0, pos2 = 0;
	do
	{
		pos2 = aSource.Mid(pos1).Find(aTarget);
		if(pos2==KErrNotFound)
		{
			TInt wantLength = aSource.Mid(pos1).Length();
			if(dest.Length() + wantLength > dest.MaxLength()) 
			{
				TInt length = dest.Length();
				ret = ret->ReAllocL(length + wantLength + 50);
				dest.Set(ret->Des());
				dest.SetLength(length);
			}
		
			dest.Append(aSource.Mid(pos1));
			break;
		}
		
		TInt wantLength = aSource.Mid(pos1, pos2).Length() + aReplacement.Length();
		if(dest.Length() + wantLength > dest.MaxLength()) 
		{
			TInt length = dest.Length();
			ret = ret->ReAllocL(length + wantLength + 50);
			dest.Set(ret->Des());
			dest.SetLength(length);
		}
		
		dest.Append(aSource.Mid(pos1, pos2));
		dest.Append(aReplacement);
		pos1 = pos1 + pos2 + aTarget.Length();
	}while(ETrue);
	
	return ret;
}

void HcUtils::StringReplace(const TDesC& aSource, const TDesC& aTarget, const TDesC& aReplacement, TDes& aDest)
{
	aDest.Zero();
	TInt pos1 = 0, pos2 = 0;
	do
	{
		pos2 = aSource.Mid(pos1).Find(aTarget);
		if(pos2==KErrNotFound)
		{
			aDest.Append(aSource.Mid(pos1));
			break;
		}
		aDest.Append(aSource.Mid(pos1, pos2));
		aDest.Append(aReplacement);
		pos1 = pos1 + pos2 + aTarget.Length();
	}while(ETrue);
}

void HcUtils::StringReplace(TDes& aTarget, TChar aSrcChar, TChar aDestChar)
{
	TInt pos1 = 0, pos2 = 0;
	while(pos1<aTarget.Length()
		&& (pos2=aTarget.Mid(pos1).Locate(aSrcChar))!=KErrNotFound) 
	{
		pos2 += pos1;
		if(aDestChar!=0) 
		{
			aTarget[pos2] = aDestChar;
			pos1 = pos2 + 1;
		}
		else 
		{
			aTarget.Delete(pos2, 1);
			pos1 = pos2;
		}	
	}
}

TUint16 HcUtils::GetEntityUnicode(const TDesC& aEntity)
{
	TUint16 u = 0;
	switch(aEntity[0])
	{
	case 'a':
	case 'A':
		if(aEntity.Compare(_L("amp"))==0)
			u = 38;
		else if(aEntity.Compare(_L("apos"))==0)
			u = 39;
		else if(aEntity.Compare(_L("acute"))==0)
			u = 180;
		else if(aEntity.Compare(_L("Agrave"))==0)
			u = 192;
		else if(aEntity.Compare(_L("Aacute"))==0)
			u = 193;
		else if(aEntity.Compare(_L("Acirc"))==0)
			u = 194;
		else if(aEntity.Compare(_L("Atilde"))==0)
			u = 195;
		else if(aEntity.Compare(_L("Auml"))==0)
			u = 196;
		else if(aEntity.Compare(_L("Aring"))==0)
			u = 197;
		else if(aEntity.Compare(_L("AElig"))==0)
			u = 198;
		else if(aEntity.Compare(_L("agrave"))==0)
			u = 224;
		else if(aEntity.Compare(_L("aacute"))==0)
			u = 225;
		else if(aEntity.Compare(_L("acirc"))==0)
			u = 226;
		else if(aEntity.Compare(_L("atilde"))==0)
			u = 227;
		else if(aEntity.Compare(_L("auml"))==0)
			u = 228;
		else if(aEntity.Compare(_L("aring"))==0)
			u = 229;
		else if(aEntity.Compare(_L("aelig"))==0)
			u = 230;
		else if(aEntity.Compare(_L("Alpha"))==0)
			u = 913;
		else if(aEntity.Compare(_L("alpha"))==0)
			u = 945;
		else if(aEntity.Compare(_L("alefsym"))==0)
			u = 8501;
		else if(aEntity.Compare(_L("ang"))==0)
			u = 8736;
		else if(aEntity.Compare(_L("and"))==0)
			u = 8743;
		else if(aEntity.Compare(_L("asymp"))==0)
			u = 8776;
		break;
	case 'b':
	case 'B':
		if(aEntity.Compare(_L("brvbar"))==0)
			u = 166;
		else if(aEntity.Compare(_L("Beta"))==0)
			u = 914;
		else if(aEntity.Compare(_L("beta"))==0)
			u = 946;
		else if(aEntity.Compare(_L("bdquo"))==0)
			u = 8222;
		else if(aEntity.Compare(_L("bull"))==0)
			u = 8226;
		break;
	case 'c':
	case 'C':
		if(aEntity.Compare(_L("cent"))==0)
			u = 162;
		else if(aEntity.Compare(_L("curren"))==0)
			u = 164;
		else if(aEntity.Compare(_L("copy"))==0)
			u = 169;
		else if(aEntity.Compare(_L("cedil"))==0)
			u = 184;
		else if(aEntity.Compare(_L("Ccedil"))==0)
			u = 199;
		else if(aEntity.Compare(_L("ccedil"))==0)
			u = 231;
		else if(aEntity.Compare(_L("circ"))==0)
			u = 710;
		else if(aEntity.Compare(_L("Chi"))==0)
			u = 935;
		else if(aEntity.Compare(_L("chi"))==0)
			u = 967;
		else if(aEntity.Compare(_L("crarr"))==0)
			u = 8629;
		else if(aEntity.Compare(_L("cap"))==0)
			u = 8745;
		else if(aEntity.Compare(_L("cup"))==0)
			u = 8746;
		else if(aEntity.Compare(_L("cong"))==0)
			u = 8773;
		else if(aEntity.Compare(_L("clubs"))==0)
			u = 9827;
		break;
	case 'd':
	case 'D':
		if(aEntity.Compare(_L("deg"))==0)
			u = 176;
		else if(aEntity.Compare(_L("divide"))==0)
			u = 247;
		else if(aEntity.Compare(_L("Delta"))==0)
			u = 916;
		else if(aEntity.Compare(_L("delta"))==0)
			u = 948;
		else if(aEntity.Compare(_L("dagger"))==0)
			u = 8224;
		else if(aEntity.Compare(_L("Dagger"))==0)
			u = 8225;
		else if(aEntity.Compare(_L("darr"))==0)
			u = 8595;
		else if(aEntity.Compare(_L("dArr"))==0)
			u = 8659;
		else if(aEntity.Compare(_L("diams"))==0)
			u = 9830;
		break;
	case 'e':
	case 'E':
		if(aEntity.Compare(_L("Egrave"))==0)
			u = 200;
		else if(aEntity.Compare(_L("Eacute"))==0)
			u = 201;
		else if(aEntity.Compare(_L("Ecirc"))==0)
			u = 202;
		else if(aEntity.Compare(_L("Euml"))==0)
			u = 203;
		else if(aEntity.Compare(_L("ETH"))==0)
			u = 208;
		else if(aEntity.Compare(_L("egrave"))==0)
			u = 232;
		else if(aEntity.Compare(_L("eacute"))==0)
			u = 233;
		else if(aEntity.Compare(_L("ecirc"))==0)
			u = 234;
		else if(aEntity.Compare(_L("euml"))==0)
			u = 235;
		else if(aEntity.Compare(_L("eth"))==0)
			u = 240;
		else if(aEntity.Compare(_L("Epsilon"))==0)
			u = 917;
		else if(aEntity.Compare(_L("Eta"))==0)
			u = 919;
		else if(aEntity.Compare(_L("epsilon"))==0)
			u = 949;
		else if(aEntity.Compare(_L("eta"))==0)
			u = 951;
		else if(aEntity.Compare(_L("ensp"))==0)
			u = 8194;
		else if(aEntity.Compare(_L("emsp"))==0)
			u = 8195;
		else if(aEntity.Compare(_L("euro"))==0)
			u = 8364;
		else if(aEntity.Compare(_L("exist"))==0)
			u = 8707;
		else if(aEntity.Compare(_L("empty"))==0)
			u = 8709;
		else if(aEntity.Compare(_L("equiv"))==0)
			u = 8801;
		break;
	case 'f':
	case 'F':
		if(aEntity.Compare(_L("frac14"))==0)
			u = 188;
		else if(aEntity.Compare(_L("frac12"))==0)
			u = 189;
		else if(aEntity.Compare(_L("frac34"))==0)
			u = 190;
		else if(aEntity.Compare(_L("fnof"))==0)
			u = 402;
		else if(aEntity.Compare(_L("frasl"))==0)
			u = 8260;
		else if(aEntity.Compare(_L("forall"))==0)
			u = 8704;
		break;
	case 'g':
	case 'G':
		if(aEntity.Compare(_L("gt"))==0)
			u = 62;
		else if(aEntity.Compare(_L("Gamma"))==0)
			u = 915;
		else if(aEntity.Compare(_L("gamma"))==0)
			u = 947;
		else if(aEntity.Compare(_L("ge"))==0)
			u = 8805;
		break;
	case 'h':
	case 'H':
		if(aEntity.Compare(_L("hellip"))==0)
			u = 8230;
		else if(aEntity.Compare(_L("harr"))==0)
			u = 8596;
		else if(aEntity.Compare(_L("hArr"))==0)
			u = 8660;
		else if(aEntity.Compare(_L("hearts"))==0)
			u = 9829;
		break;
	case 'i':
	case 'I':
		if(aEntity.Compare(_L("iexcl"))==0)
			u = 161;
		else if(aEntity.Compare(_L("iquest"))==0)
			u = 191;
		else if(aEntity.Compare(_L("Igrave"))==0)
			u = 204;
		else if(aEntity.Compare(_L("Iacute"))==0)
			u = 205;
		else if(aEntity.Compare(_L("Icirc"))==0)
			u = 206;
		else if(aEntity.Compare(_L("Iuml"))==0)
			u = 207;
		else if(aEntity.Compare(_L("igrave"))==0)
			u = 236;
		else if(aEntity.Compare(_L("iacute"))==0)
			u = 237;
		else if(aEntity.Compare(_L("icirc"))==0)
			u = 238;
		else if(aEntity.Compare(_L("iuml"))==0)
			u = 239;
		else if(aEntity.Compare(_L("Iota"))==0)
			u = 921;
		else if(aEntity.Compare(_L("iota"))==0)
			u = 953;
		else if(aEntity.Compare(_L("image"))==0)
			u = 8465;
		else if(aEntity.Compare(_L("isin"))==0)
			u = 8712;
		else if(aEntity.Compare(_L("infin"))==0)
			u = 8734;
		else if(aEntity.Compare(_L("int"))==0)
			u = 8747;
		break;
	case 'j':
	case 'J':
		break;
	case 'k':
	case 'K':
		if(aEntity.Compare(_L("Kappa"))==0)
			u = 922;
		else if(aEntity.Compare(_L("kappa"))==0)
			u = 954;
		break;
	case 'l':
	case 'L':
		if(aEntity.Compare(_L("lt"))==0)
			u = 60;
		else if(aEntity.Compare(_L("laquo"))==0)
			u = 171;
		else if(aEntity.Compare(_L("Lambda"))==0)
			u = 923;
		else if(aEntity.Compare(_L("lambda"))==0)
			u = 955;
		else if(aEntity.Compare(_L("lrm"))==0)
			u = 8206;
		else if(aEntity.Compare(_L("lsquo"))==0)
			u = 8216;
		else if(aEntity.Compare(_L("ldquo"))==0)
			u = 8220;
		else if(aEntity.Compare(_L("lsaquo"))==0)
			u = 8249;
		else if(aEntity.Compare(_L("larr"))==0)
			u = 8592;
		else if(aEntity.Compare(_L("lArr"))==0)
			u = 8656;
		else if(aEntity.Compare(_L("lowast"))==0)
			u = 8727;
		else if(aEntity.Compare(_L("le"))==0)
			u = 8804;
		else if(aEntity.Compare(_L("lceil"))==0)
			u = 8968;
		else if(aEntity.Compare(_L("lfloor"))==0)
			u = 8970;
		else if(aEntity.Compare(_L("lang"))==0)
			u = 9001;
		else if(aEntity.Compare(_L("loz"))==0)
			u = 9674;
		break;
	case 'm':
	case 'M':
		if(aEntity.Compare(_L("macr"))==0)
			u = 175;
		else if(aEntity.Compare(_L("micro"))==0)
			u = 181;
		else if(aEntity.Compare(_L("middot"))==0)
			u = 183;
		else if(aEntity.Compare(_L("Mu"))==0)
			u = 924;
		else if(aEntity.Compare(_L("mu"))==0)
			u = 956;
		else if(aEntity.Compare(_L("mdash"))==0)
			u = 8212;
		else if(aEntity.Compare(_L("minus"))==0)
			u = 8722;
		break;
	case 'n':
	case 'N':
		if(aEntity.Compare(_L("nbsp"))==0)
			u = 32;
		else if(aEntity.Compare(_L("not"))==0)
			u = 172;
		else if(aEntity.Compare(_L("Ntilde"))==0)
			u = 209;
		else if(aEntity.Compare(_L("ntilde"))==0)
			u = 241;
		else if(aEntity.Compare(_L("Nu"))==0)
			u = 925;
		else if(aEntity.Compare(_L("nu"))==0)
			u = 957;
		else if(aEntity.Compare(_L("ndash"))==0)
			u = 8211;
		else if(aEntity.Compare(_L("nabla"))==0)
			u = 8711;
		else if(aEntity.Compare(_L("notin"))==0)
			u = 8713;
		else if(aEntity.Compare(_L("ni"))==0)
			u = 8715;
		else if(aEntity.Compare(_L("ne"))==0)
			u = 8800;
		else if(aEntity.Compare(_L("nsub"))==0)
			u = 8836;
		break;
	case 'o':
	case 'O':
		if(aEntity.Compare(_L("ordf"))==0)
			u = 170;
		else if(aEntity.Compare(_L("ordm"))==0)
			u = 186;
		else if(aEntity.Compare(_L("Ograve"))==0)
			u = 210;
		else if(aEntity.Compare(_L("Oacute"))==0)
			u = 211;
		else if(aEntity.Compare(_L("Ocirc"))==0)
			u = 212;
		else if(aEntity.Compare(_L("Otilde"))==0)
			u = 213;
		else if(aEntity.Compare(_L("Ouml"))==0)
			u = 214;
		else if(aEntity.Compare(_L("Oslash"))==0)
			u = 216;
		else if(aEntity.Compare(_L("ograve"))==0)
			u = 242;
		else if(aEntity.Compare(_L("oacute"))==0)
			u = 243;
		else if(aEntity.Compare(_L("ocirc"))==0)
			u = 244;
		else if(aEntity.Compare(_L("otilde"))==0)
			u = 245;
		else if(aEntity.Compare(_L("ouml"))==0)
			u = 246;
		else if(aEntity.Compare(_L("oslash"))==0)
			u = 248;
		else if(aEntity.Compare(_L("OElig"))==0)
			u = 338;
		else if(aEntity.Compare(_L("oelig"))==0)
			u = 339;
		else if(aEntity.Compare(_L("Omicron"))==0)
			u = 927;
		else if(aEntity.Compare(_L("Omega"))==0)
			u = 937;
		else if(aEntity.Compare(_L("omicron"))==0)
			u = 959;
		else if(aEntity.Compare(_L("omega"))==0)
			u = 969;
		else if(aEntity.Compare(_L("oline"))==0)
			u = 8254;
		else if(aEntity.Compare(_L("or"))==0)
			u = 8744;
		else if(aEntity.Compare(_L("oplus"))==0)
			u = 8853;
		else if(aEntity.Compare(_L("otimes"))==0)
			u = 8855;
		break;
	case 'p':
	case 'P':
		if(aEntity.Compare(_L("pound"))==0)
			u = 163;
		else if(aEntity.Compare(_L("plusmn"))==0)
			u = 177;
		else if(aEntity.Compare(_L("para"))==0)
			u = 182;
		else if(aEntity.Compare(_L("Pi"))==0)
			u = 928;
		else if(aEntity.Compare(_L("Phi"))==0)
			u = 934;
		else if(aEntity.Compare(_L("Psi"))==0)
			u = 936;
		else if(aEntity.Compare(_L("pi"))==0)
			u = 960;
		else if(aEntity.Compare(_L("phi"))==0)
			u = 966;
		else if(aEntity.Compare(_L("psi"))==0)
			u = 968;
		else if(aEntity.Compare(_L("piv"))==0)
			u = 982;
		else if(aEntity.Compare(_L("permil"))==0)
			u = 8240;
		else if(aEntity.Compare(_L("prime"))==0)
			u = 8242;
		else if(aEntity.Compare(_L("Prime"))==0)
			u = 8243;
		else if(aEntity.Compare(_L("part"))==0)
			u = 8706;
		else if(aEntity.Compare(_L("prod"))==0)
			u = 8719;
		else if(aEntity.Compare(_L("prop"))==0)
			u = 8733;
		else if(aEntity.Compare(_L("perp"))==0)
			u = 8869;
		break;
	case 'q':
	case 'Q':
		if(aEntity.Compare(_L("quot"))==0)
			u = 34;
		break;
	case 'r':
	case 'R':
		if(aEntity.Compare(_L("reg"))==0)
			u = 174;
		else if(aEntity.Compare(_L("raquo"))==0)
			u = 187;
		else if(aEntity.Compare(_L("Rho"))==0)
			u = 929;
		else if(aEntity.Compare(_L("rho"))==0)
			u = 961;
		else if(aEntity.Compare(_L("rlm"))==0)
			u = 8207;
		else if(aEntity.Compare(_L("rsquo"))==0)
			u = 8217;
		else if(aEntity.Compare(_L("rdquo"))==0)
			u = 8221;
		else if(aEntity.Compare(_L("rsaquo"))==0)
			u = 8250;
		else if(aEntity.Compare(_L("real"))==0)
			u = 8476;
		else if(aEntity.Compare(_L("rarr"))==0)
			u = 8594;
		else if(aEntity.Compare(_L("rArr"))==0)
			u = 8658;
		else if(aEntity.Compare(_L("radic"))==0)
			u = 8730;
		else if(aEntity.Compare(_L("rceil"))==0)
			u = 8969;
		else if(aEntity.Compare(_L("rfloor"))==0)
			u = 8971;
		else if(aEntity.Compare(_L("rang"))==0)
			u = 9002;
		break;
	case 's':
	case 'S':
		if(aEntity.Compare(_L("sect"))==0)
			u = 167;
		else if(aEntity.Compare(_L("shy"))==0)
			u = 173;
		else if(aEntity.Compare(_L("sup2"))==0)
			u = 178;
		else if(aEntity.Compare(_L("sup3"))==0)
			u = 179;
		else if(aEntity.Compare(_L("sup1"))==0)
			u = 185;
		else if(aEntity.Compare(_L("szlig"))==0)
			u = 223;
		else if(aEntity.Compare(_L("Scaron"))==0)
			u = 352;
		else if(aEntity.Compare(_L("scaron"))==0)
			u = 353;
		else if(aEntity.Compare(_L("Sigma"))==0)
			u = 931;
		else if(aEntity.Compare(_L("sigmaf"))==0)
			u = 962;
		else if(aEntity.Compare(_L("sigma"))==0)
			u = 963;
		else if(aEntity.Compare(_L("sbquo"))==0)
			u = 8218;
		else if(aEntity.Compare(_L("sum"))==0)
			u = 8721;
		else if(aEntity.Compare(_L("sim"))==0)
			u = 8764;
		else if(aEntity.Compare(_L("sub"))==0)
			u = 8834;
		else if(aEntity.Compare(_L("sup"))==0)
			u = 8835;
		else if(aEntity.Compare(_L("sube"))==0)
			u = 8838;
		else if(aEntity.Compare(_L("supe"))==0)
			u = 8839;
		else if(aEntity.Compare(_L("sdot"))==0)
			u = 8901;
		else if(aEntity.Compare(_L("spades"))==0)
			u = 9824;
		break;
	case 't':
	case 'T':
		if(aEntity.Compare(_L("times"))==0)
			u = 215;
		else if(aEntity.Compare(_L("THORN"))==0)
			u = 222;
		else if(aEntity.Compare(_L("thorn"))==0)
			u = 254;
		else if(aEntity.Compare(_L("tilde"))==0)
			u = 732;
		else if(aEntity.Compare(_L("Theta"))==0)
			u = 920;
		else if(aEntity.Compare(_L("Tau"))==0)
			u = 932;
		else if(aEntity.Compare(_L("theta"))==0)
			u = 952;
		else if(aEntity.Compare(_L("tau"))==0)
			u = 964;
		else if(aEntity.Compare(_L("thetasym"))==0)
			u = 977;
		else if(aEntity.Compare(_L("thinsp"))==0)
			u = 8201;
		else if(aEntity.Compare(_L("trade"))==0)
			u = 8482;
		else if(aEntity.Compare(_L("there4"))==0)
			u = 8756;
		break;
	case 'u':
	case 'U':
		if(aEntity.Compare(_L("uml"))==0)
			u = 168;
		else if(aEntity.Compare(_L("Ugrave"))==0)
			u = 217;
		else if(aEntity.Compare(_L("Uacute"))==0)
			u = 218;
		else if(aEntity.Compare(_L("Ucirc"))==0)
			u = 219;
		else if(aEntity.Compare(_L("Uuml"))==0)
			u = 220;
		else if(aEntity.Compare(_L("ugrave"))==0)
			u = 249;
		else if(aEntity.Compare(_L("uacute"))==0)
			u = 250;
		else if(aEntity.Compare(_L("ucirc"))==0)
			u = 251;
		else if(aEntity.Compare(_L("uuml"))==0)
			u = 252;
		else if(aEntity.Compare(_L("Upsilon"))==0)
			u = 933;
		else if(aEntity.Compare(_L("upsilon"))==0)
			u = 965;
		else if(aEntity.Compare(_L("upsih"))==0)
			u = 978;
		else if(aEntity.Compare(_L("uarr"))==0)
			u = 8593;
		else if(aEntity.Compare(_L("uArr"))==0)
			u = 8657;
		break;
	case 'v':
	case 'V':
		break;
	case 'w':
	case 'W':
		if(aEntity.Compare(_L("weierp"))==0)
			u = 8472;
		break;
	case 'x':
	case 'X':
		if(aEntity.Compare(_L("Xi"))==0)
			u = 926;
		else if(aEntity.Compare(_L("xi"))==0)
			u = 958;
		break;
	case 'y':
	case 'Y':
		if(aEntity.Compare(_L("yen"))==0)
			u = 165;
		else if(aEntity.Compare(_L("Yacute"))==0)
			u = 221;
		else if(aEntity.Compare(_L("yacute"))==0)
			u = 253;
		else if(aEntity.Compare(_L("yuml"))==0)
			u = 255;
		else if(aEntity.Compare(_L("Yuml"))==0)
			u = 376;
		break;
	case 'z':
	case 'Z':
		if(aEntity.Compare(_L("Zeta"))==0)
			u = 918;
		else if(aEntity.Compare(_L("zeta"))==0)
			u = 950;
		else if(aEntity.Compare(_L("zwnj"))==0)
			u = 8204;
		else if(aEntity.Compare(_L("zwj"))==0)
			u = 8205;
		break;
	}
	
	return u;
}

HBufC* HcUtils::DecodeHttpCharactersL(const TDesC& aSource)
{
	TInt len = aSource.Length();
	HBufC* ret = HBufC::NewL(len);
	TPtr ptr = ret->Des();
	TInt pos1 = 0, pos2 = 0;

	do
	{
		pos2 = aSource.Mid(pos1).Locate('&');
		if(pos2==KErrNotFound)
		{
			ptr.Append(aSource.Mid(pos1));
			break;
		}
		ptr.Append(aSource.Mid(pos1, pos2));
		pos1 = pos1 + pos2 + 1;
		pos2 = pos1;
		TInt end = Min(len, pos2+10);
		for(;pos2<end;pos2++) 
		{
			if(aSource[pos2]==';')
				break;
		}
		if(pos2<end && pos2>pos1)
		{
			TPtrC entity = aSource.Mid(pos1, pos2-pos1);
			if(entity[0]=='#') 
			{
				if(entity.Length()>1) 
				{
					TUint16 u = '?';
					if(entity[1]=='x') 
					{
						TLex lex(entity.Mid(2));
						lex.Val(u, EHex);						
					}
					else 
					{
						TLex lex(entity.Mid(1));
						lex.Val(u, EDecimal);
					}
					ptr.Append(u);
					pos1 = pos2 + 1;					
				}
				else
					ptr.Append('&');
			}
			else 
			{
				TUint16 u = GetEntityUnicode(entity);
				if(u>0) 
				{
					ptr.Append(u);
					pos1 = pos2 + 1;
				}
				else 
					ptr.Append('&');
			}
		}
		else 
		{
			ptr.Append('&');
		}
	}while(ETrue);
	
	return ret;
}

HBufC8* HcUtils::DecodeHttpCharactersL(const TDesC8& aSource)
{
	TInt len = aSource.Length();
	HBufC8* ret = HBufC8::NewL(len);
	TPtr8 ptr = ret->Des();
	TInt pos1 = 0, pos2 = 0;
	TBuf<10> entity;
	TBuf8<5> entity8;
	do
	{
		pos2 = aSource.Mid(pos1).Locate('&');
		if(pos2==KErrNotFound)
		{
			ptr.Append(aSource.Mid(pos1));
			break;
		}
		ptr.Append(aSource.Mid(pos1, pos2));
		pos1 = pos1 + pos2 + 1;
		pos2 = pos1;
		TInt end = Min(len, pos2+10);
		for(;pos2<end;pos2++) 
		{
			if(aSource[pos2]==';')
				break;
		}
		if(pos2<end && pos2>pos1)
		{
			entity.Copy(aSource.Mid(pos1, pos2-pos1));
			if(entity[0]=='#') 
			{
				if(entity.Length()>1) 
				{
					TUint16 u = '?';
					if(entity[1]=='x') 
					{
						TLex lex(entity.Mid(2));
						lex.Val(u, EHex);						
					}
					else 
					{
						TLex lex(entity.Mid(1));
						lex.Val(u, EDecimal);
					}
					entity.Copy(&u, 1);
					CnvUtfConverter::ConvertFromUnicodeToUtf8(entity8, entity);
					ptr.Append(entity8);
					pos1 = pos2 + 1;					
				}
				else
					ptr.Append('&');
			}
			else 
			{
				TUint16 u = GetEntityUnicode(entity);
				if(u>0) 
				{
					entity.Copy(&u, 1);
					CnvUtfConverter::ConvertFromUnicodeToUtf8(entity8, entity);
					ptr.Append(entity8);
					pos1 = pos2 + 1;
				}
				else
					ptr.Append('&');
			}
		}
		else 
		{
			ptr.Append('&');
		}
	}while(ETrue);
	
	return ret;
}

HBufC* HcUtils::EncodeHttpCharactersL(const TDesC& aSource)
{
	_LIT(KEscape1, "&amp;");
	_LIT(KEscape2, "&lt;");
	_LIT(KEscape3, "&gt;");
	
	TInt len = aSource.Length();
	HBufC* buf = HBufC::NewL(len);
	TPtr ptr = buf->Des();

	for(TInt i=0;i<len;i++) 
	{
		TUint16 c = aSource[i];
		TInt wantLength;
		if(c=='&' || c=='<' || c=='>')
			wantLength = 6;
		else
			wantLength = 1;
		
		if(ptr.Length() + wantLength > ptr.MaxLength()) 
		{
			TInt length = ptr.Length();
			buf = buf->ReAllocL(length + wantLength + 50);			
			ptr.Set(buf->Des());
			ptr.SetLength(length);
		}
		
		if(c=='&')
			ptr.Append(KEscape1);
		else if(c=='<')
			ptr.Append(KEscape2);
		else if(c=='>')
			ptr.Append(KEscape3);
		else
			ptr.Append(c);
	}
	return buf;
}


HBufC8* HcUtils::EncodeHttpCharactersL(const TDesC8& aSource)
{
	_LIT8(KEscape1, "&amp;");
	_LIT8(KEscape2, "&lt;");
	_LIT8(KEscape3, "&gt;");

	TInt len = aSource.Length();
	HBufC8* buf = HBufC8::NewL(len);
	TPtr8 ptr = buf->Des();

	for(TInt i=0;i<len;i++) 
	{
		TUint16 c = aSource[i];
		TInt wantLength;
		if(c=='&' || c=='<' || c=='>')
			wantLength = 6;
		else
			wantLength = 1;
		
		if(ptr.Length() + wantLength > ptr.MaxLength()) 
		{
			TInt length = ptr.Length();
			buf = buf->ReAllocL(length + wantLength + 50);			
			ptr.Set(buf->Des());
			ptr.SetLength(length);
		}
		
		if(c=='&')
			ptr.Append(KEscape1);
		else if(c=='<')
			ptr.Append(KEscape2);
		else if(c=='>')
			ptr.Append(KEscape3);
		else
			ptr.Append(c);
	}
	return buf;
}

TBool HcUtils::StringSplit(const TDesC& aSource, TInt& aOffset, TChar aSeperator, TPtrC& aRet)
{
	if(aOffset==KErrNotFound || aOffset >= aSource.Length()) 
		return EFalse;

	do 
	{
		TInt pos = aSource.Mid(aOffset).Locate(aSeperator);
		if(pos==KErrNotFound) 
		{
			aRet.Set(aSource.Mid(aOffset));
			aOffset = aSource.Length();
			return ETrue;
		}
		
		if(pos!=0) 
		{
			aRet.Set(aSource.Mid(aOffset, pos));
			aOffset += pos + 1;
			return ETrue;
		}
		aOffset++;
	}
	while(aOffset<aSource.Length());
	
	return EFalse;
}

//tag type:0 - start tag, 1 - end tag, 2 - empty content tag
TInt HcUtils::EnumTag(const TDesC& aSource, TInt aOffset, TDes& aTagName, TInt& aLength, TInt& aTagType)
{
	TInt sourceLen = aSource.Length();
	if(aOffset>=sourceLen)
		return KErrNotFound;
	
	TInt pos;
	aTagType = 0;
	while((pos = aSource.Mid(aOffset).Locate('<'))!=KErrNotFound)
	{
		pos += aOffset;
		aOffset = pos;
		pos++;
		
		if(pos==sourceLen) 
			return KErrNotFound;

		if(aSource[pos]=='/') 
		{
			pos++;
			aTagType = 1;
		}
		else if(aSource[pos]=='!')
		{//<!-- -->
			_LIT(KCommentStart, "--");
			_LIT(KCommentEnd, "-->");
			_LIT(KCDataStart, "[CDATA[");
			_LIT(KCDataEnd, "]]>");
			
			if(pos+2<sourceLen && aSource.Mid(pos+1, 2).Compare(KCommentStart)==0) //comment
			{
				pos = aSource.Mid(aOffset).Find(KCommentEnd);
				if(pos==KErrNotFound)
					return KErrNotFound;
				
				aLength = pos + 3;
				aTagName.Copy(aSource.Mid(aOffset+1, 3));				
				return aOffset;
			}
			else if(pos+7<sourceLen && aSource.Mid(pos+1, 7).Compare(KCDataStart)==0) //cdata
			{
				aOffset = aSource.Mid(pos).Find(KCDataEnd);
				if(aOffset==KErrNotFound)
					return KErrNotFound;
				
				aOffset += pos + 3;
				if(aOffset>=sourceLen)
					return KErrNotFound;
				
				continue;
			}
		}
		
		for(;pos<sourceLen;pos++) 
		{
			TChar c(aSource[pos]);
			if(c.IsSpace() || c == '>' || c == '/') 
				break;
		}
		if(pos==sourceLen)
			return KErrNotFound;
		
		aTagName.Copy(aSource.Mid(aOffset+1, pos-aOffset-1).Left(aTagName.MaxLength()));
		if(aTagName.Length()>0 && aTagName[0]=='/')
			aTagName.Delete(0,1);
		
		TBool inattr = EFalse;
		TInt possibleEnd = -1;
		for(;pos<sourceLen;pos++) 
		{
			TChar c(aSource[pos]);
			if(c=='"') 
				inattr = !inattr;
			
			if(c=='>') 
			{
				if(!inattr) 
				{
					possibleEnd = -1;
					break;
				}

				possibleEnd = pos;
			}
			else if(c=='<')
				break;
		}
		if(possibleEnd!=-1)
			pos = possibleEnd;
		
		if(pos==sourceLen)
			return KErrNotFound;

		aLength = pos - aOffset + 1;
		if(aSource[pos - 1] == '/')
			aTagType = 2;
		else if(aTagType!=1)
			aTagType = 0;
		return aOffset;
	}
	
	return KErrNotFound;		
}

TInt HcUtils::EnumAttribute(const TDesC& aSource, TInt aOffset, TDes& aAttrName, TInt& aAttrLength, TInt& aValueStart, TInt& aValueLength)
{
	TInt sourceLen = aSource.Length();
	TBuf<50> attrNameBuf;
	TInt attrStart = -1, attrEnd = -1;
	TBool waitValue = EFalse;
	TInt i=aOffset;
	if(i<sourceLen && aSource[i]=='<') 
	{
		for(;i<sourceLen;i++)
		{
			TChar c(aSource[i]);
			if(c.IsSpace() || c == '>' || c == '/') 
				break;	
		}
	}
	
	for(;i<sourceLen;i++) 
	{
		TChar c(aSource[i]);
		if(c == '=')
		{
			TInt valueStart = i+1;
			TInt valueEnd = -1;
			TBool started = EFalse, singleQuoted = EFalse, doubleQuoted = EFalse;
			for(TInt j=i+1;j<sourceLen;j++)
			{
				TChar c(aSource[j]);
				if(c.IsSpace()) 
				{
					if(started && !(singleQuoted || doubleQuoted)) 
					{
						attrEnd = j-1;
						valueEnd = attrEnd;
						break;
					}
				}
				else if(c == '>')
				{
					//if(!(singleQuoted || doubleQuoted)) 
					{
						attrEnd = j-1;
						valueEnd = attrEnd;
						break;
					}
				}
				else if(c == '"') 
				{
					if(started) 
					{
						if(!singleQuoted) 
						{
							attrEnd = j;
							valueEnd = j-1;
							break;
						}
					}
					else 
					{
						started = ETrue;
						doubleQuoted = ETrue;
						valueStart = j+1;
					}
				}
				else if(c == '\'')
				{
					if(started) 
					{
						if(!doubleQuoted) 
						{
							attrEnd = j;
							valueEnd = j-1;
							break;	
						}
					}
					else 
					{
						started = ETrue;
						singleQuoted = ETrue;
						valueStart = j+1;
					}
				}
				else if(!started)
				{
					started = ETrue;
					valueStart = j;
				}
			}
			
			if(attrEnd!=-1) 
			{
				aAttrName.Copy(attrNameBuf);
				aValueStart = valueStart;
				aValueLength = valueEnd - valueStart + 1;
				aAttrLength = attrEnd - attrStart + 1;
				return attrStart;
			}
			else
				break;
		}
		else if(!c.IsSpace()) 
		{
			if(waitValue || c=='/' || c=='>') 
			{
				if(attrNameBuf.Length()>0) 
				{
					aAttrName.Copy(attrNameBuf);
					aValueStart = attrStart + aAttrName.Length();
					aValueLength = 0;
					aAttrLength = i - attrStart;
					return attrStart;
				}
				
				waitValue = EFalse;
				attrNameBuf.Zero();
			}
			
			if(attrNameBuf.Length()==0) 
			{
				if(c!='/')
					attrStart = i;
				else
					continue;
			}
			
			if(attrNameBuf.Length()==attrNameBuf.MaxLength())
				attrNameBuf.Zero();
			attrNameBuf.Append(c);
		}
		else 
		{
			if(attrNameBuf.Length()>0)
				waitValue = ETrue;
		}
	}

	
	return KErrNotFound;	
}

TInt HcUtils::GetAttributeValue(const TDesC& aSource, const TDesC& aAttrName, TPtrC& aDest)
{
	TBuf<50> buf;
	TInt pos = 0;
	TInt attrLength, valueStart, valueLength;
	while(ETrue)
	{
		pos = EnumAttribute(aSource, pos, buf, attrLength, valueStart, valueLength);
		if(pos==KErrNotFound)
			break;
		
		if(buf.CompareF(aAttrName)==0) 
		{
			aDest.Set(aSource.Mid(valueStart, valueLength));
			return pos;
		}
		
		pos += attrLength;
	}
	
	return KErrNotFound;	
}

TInt HcUtils::GetAttributeValue(const TDesC& aSource, const TDesC& aAttrName, TDes& aDest)
{
	TPtrC ptr;
	TInt ret = GetAttributeValue(aSource, aAttrName, ptr);
	if(ret!=KErrNotFound)
		aDest.Copy(ptr.Left(aDest.MaxLength()));

	return ret;
}

#ifdef __SERIES60__
void HcUtils::WrapTextL(const TDesC& aSource, TInt aFirstLineWidth, TInt aOtherLineWidth, const CFont& aFont, CArrayFix<TPtrC>& aWrappedArray)
{
	if(aFirstLineWidth==aOtherLineWidth)
	{
		AknTextUtils::WrapToArrayL(aSource, aOtherLineWidth, aFont, aWrappedArray);
	}
	else
	{
		CArrayFix<TInt>* lineWidths = new (ELeave)CArrayFixFlat<TInt>(2);
		CleanupStack::PushL(lineWidths);
		lineWidths->AppendL(aFirstLineWidth);
		lineWidths->AppendL(aOtherLineWidth);
		AknTextUtils::WrapToArrayL(aSource, *lineWidths, aFont, aWrappedArray);
		if(aWrappedArray.Count()>1)
		{
			TPtrC firstLine = aWrappedArray[0];
			TInt pos = aWrappedArray[1].Ptr() - aSource.Ptr();
			if(pos + aWrappedArray[1].Length() < aSource.Length())
			{
				AknTextUtils::WrapToArrayL(aSource.Mid(pos), aOtherLineWidth, aFont, aWrappedArray);
				aWrappedArray.InsertL(0, firstLine);
			}
		}
		CleanupStack::PopAndDestroy(lineWidths);
	}
	
	if(aWrappedArray.Count()>0)
	{
		TPtrC lastLine = aWrappedArray[aWrappedArray.Count()-1];
		TInt pos = lastLine.Ptr() - aSource.Ptr();
		aWrappedArray.Delete(aWrappedArray.Count()-1);
		lastLine.Set(aSource.Mid(pos));
		aWrappedArray.AppendL(lastLine);
	}
}
#else
#define IS_ASCII(c) ( (c)>='a' && (c)<='z' || (c)>='A' && (c)<='Z' || (c)>='0' && (c)<='9' )
void HcUtils::WrapTextL(const TDesC& aSource, TInt aFirstLineWidth, TInt aOtherLineWidth, const CFont& aFont, CArrayFix<TPtrC>& aWrappedArray)
{
	TInt total = aSource.Length();
	if(total==0)
		return;
	
	TInt cnt = aFont.TextCount(aSource, aFirstLineWidth);
	if(cnt==0)
		cnt = 1;
	if(cnt<total)
	{
		TUint16 c = aSource[cnt];
		if(IS_ASCII(c))
		{
			c = aSource[cnt-1];
			if(IS_ASCII(c))
			{
				TInt i = cnt-2;
				for(;i>=0;i--)
				{
					c = aSource[i];
					if(!IS_ASCII(c))
						break;
				}
				if(i>0)
					cnt = i + 1;
			}
		}
	}

	aWrappedArray.AppendL(aSource.Mid(0, cnt));
	if(cnt==total)
		return;
	
	TInt pos = cnt;
	while(ETrue)
	{
		cnt = aFont.TextCount(aSource.Mid(pos), aOtherLineWidth);
		if(cnt==0)
			cnt = 1;
		if(pos+cnt<total)
		{
			TUint16 c = aSource[pos+cnt];
			if(IS_ASCII(c))
			{
				c = aSource[pos+cnt-1];
				if(IS_ASCII(c))
				{
					TInt i = pos+cnt-2;
					for(;i>=pos;i--)
					{
						c = aSource[i];
						if(!IS_ASCII(c))
							break;
					}
					if(i>pos)
						cnt = i + 1 - pos;
				}
			}
		}
			
		aWrappedArray.AppendL(aSource.Mid(pos, cnt));
		pos+=cnt;
		if(pos>=total)
			break;
	}
}
#endif

TUint HcUtils::TranslateKey(TUint aKeyCode)
{
#ifdef __SERIES60__
	if(aKeyCode==EKeyDevice3)
		return EKeyEnter;
	else
		return aKeyCode;
#endif
	
#ifdef __UIQ__
	switch(aKeyCode) 
	{
		case EDeviceKeyFourWayLeft:
			return EKeyLeftArrow;

		case EDeviceKeyFourWayRight:
			return EKeyRightArrow;
			
		case EDeviceKeyFourWayUp:
		case EDeviceKeyTwoWayUp:
			return EKeyUpArrow;

		case EDeviceKeyFourWayDown:
		case EDeviceKeyTwoWayDown:
			return EKeyDownArrow;

		case EDeviceKeyAction:
			return EKeyEnter;
			
		default:
			return aKeyCode;
	}
#endif
}

TInt  HcUtils::TranslateStdKey(TInt aStdKeyCode)
{
#ifdef __SERIES60__
	if(aStdKeyCode==EStdKeyDevice3)
		return EStdKeyEnter;
	else
		return aStdKeyCode;
#endif
	
#ifdef __UIQ__
	switch(aStdKeyCode) 
	{
		case EStdDeviceKeyFourWayLeft:
			return EStdKeyLeftArrow;

		case EStdDeviceKeyFourWayRight:
			return EStdKeyRightArrow;
			
		case EStdDeviceKeyFourWayUp:
		case EStdDeviceKeyTwoWayUp:
			return EStdKeyUpArrow;

		case EStdDeviceKeyFourWayDown:
		case EStdDeviceKeyTwoWayDown:
			return EStdKeyDownArrow;

		case EStdDeviceKeyAction:
			return EStdKeyEnter;
			
		default:
			return aStdKeyCode;
	}
#endif	
}

void HcUtils::DrawBackgroundAndBorders(CHtmlControl& aControl, CBitmapContext& aGc, const TRect& aRect, const CHcStyle& aStyle) 
{
	if(aRect.Height()<=0)
		return;
	
	TRgb backColor = KRgbWhite;	
	THcDrawImageParams params;
	if(aStyle.IsSet(CHcStyle::ECorner))
		params.iCorner = aStyle.iCorner;
	if( aStyle.IsSet(CHcStyle::EOpacity))
		params.iOpacity = aStyle.iOpacity;
	if(aStyle.IsSet(CHcStyle::EScale9Grid))
		params.iScale9Grid = aStyle.iScale9Grid;
	params.iOwningControl = &aControl;
	
	if(aStyle.IsSet(CHcStyle::EBackColor) && !aStyle.iBackColor.IsNone())
	{
		backColor = aStyle.iBackColor.Rgb();
	#ifdef __SYMBIAN_9_ONWARDS__
		backColor.SetAlpha(params.iOpacity);
	#endif
		
		aGc.SetPenStyle(CGraphicsContext::ENullPen);
		if(aStyle.IsSet(CHcStyle::EBackPattern) 
				&& aStyle.iBackPattern.iStyle!=CGraphicsContext::ESolidBrush)
		{
			aGc.SetBrushStyle(aStyle.iBackPattern.iStyle);
			aGc.SetPenStyle(CGraphicsContext::ESolidPen);
			aGc.SetPenColor(aStyle.iBackPattern.iColor);
		}
		else
			aGc.SetBrushStyle(CGraphicsContext::ESolidBrush);
		aGc.SetBrushColor(backColor);

		if(!aStyle.IsSet(CHcStyle::ECorner)) 
			aGc.DrawRect(aRect);
		else if(aStyle.iCorner.iWidth==9999)
			aGc.DrawEllipse(aRect);
		else
			aGc.DrawRoundRect(aRect, aStyle.iCorner);
	}
	#ifdef __SYMBIAN_9_ONWARDS__
	else
		backColor.SetAlpha(params.iOpacity);
	#endif
	
	if(aStyle.IsSet(CHcStyle::EBackImage)) 
	{
		CHcImage* image = aStyle.BackgroundImage();
		if(image && !image->IsError()) 
		{
		 	if(image->IsLoaded()) 
		 	{
		 		THcLocationType type = image->Location().iType;
		 		if(type==ELTSkin || type==ELTFrame || type==ELTGradient)  
		 		{
		 			image->Draw(aGc, aRect, params);
		 		}
		 		else
		 		{
					if(aStyle.IsSet(CHcStyle::EBackPosition)) 
					{
						if(!aStyle.iBackPosition.iStretch) 
						{
							TPoint pos;
							
							if(aStyle.iBackPosition.iAlign==ECenter)
								pos.iX += (aRect.Width() -  image->Size().iWidth)/2;
							else if(aStyle.iBackPosition.iAlign==ERight)
								pos.iX = aRect.iBr.iX - image->Size().iWidth;
							else
								pos.iX = aStyle.iBackPosition.iX.GetRealValue(aRect.Width(), 0);

							if(aStyle.iBackPosition.iVAlign==EVCenter)
								pos.iY += (aRect.Height() - image->Size().iHeight)/2;
							else if(aStyle.iBackPosition.iVAlign==EVBottom)
								pos.iY = aRect.iBr.iY - image->Size().iHeight;
							else
								pos.iY = aStyle.iBackPosition.iY.GetRealValue(aRect.Height(), 0);
							
							image->Draw(aGc, aRect.iTl + pos, params);
						}
						else
							image->Draw(aGc, aRect, params);
					}
					else if(aStyle.IsSet(CHcStyle::EBackRepeatX) || aStyle.IsSet(CHcStyle::EBackRepeatY))
					{
						TPoint pos(0,0);
						while(ETrue)
						{
							image->Draw(aGc, aRect.iTl + pos, params);
							if(aStyle.IsSet(CHcStyle::EBackRepeatX)) 
							{
								pos.iX += image->Size().iWidth;
								if(pos.iX>=aRect.Width() + image->Size().iWidth) 
								{
									if(!aStyle.IsSet(CHcStyle::EBackRepeatY))
										break;
									
									pos.iX = 0;
									pos.iY += image->Size().iHeight;
									if(pos.iY>=aRect.Height() + image->Size().iHeight)
										break;
								}	
							}
							else
							{
								pos.iY +=  image->Size().iHeight;
								if(pos.iY>=aRect.Height() + image->Size().iHeight)
									break;
							}
						}
					}
					else 
						image->Draw(aGc, aRect.iTl, params);
		 		}

		 	}
			else
		 		image->AddLoadedEventSubscriber(aControl.Impl());
		}
	}

	if(params.iCorner.iWidth!=0 || params.iCorner.iHeight!=0) 
	{
		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderTop) 
				&& aStyle.iBorders.iTop.iStyle!=THcBorder::ENone) 
		{
			aGc.SetPenStyle(CGraphicsContext::ESolidPen);
			aGc.SetPenSize(TSize(aStyle.iBorders.iTop.iWidth, aStyle.iBorders.iTop.iWidth));
			aGc.SetPenColor(aStyle.iBorders.iTop.iColor);
			aGc.SetBrushStyle(CGraphicsContext::ENullBrush);
				
			TRect rect = aRect;
			if(aStyle.iBorders.iTop.iWidth%2==0)
				rect.iBr.iX -= 1;
			
			if(aStyle.iCorner.iWidth==9999)
				aGc.DrawEllipse(rect);
			else
				aGc.DrawRoundRect(rect, aStyle.iCorner);
		}
		return;
	}
	
	if(!aStyle.iBorders.IsEmpty()) 
	{
		aGc.SetBrushStyle(CGraphicsContext::ENullBrush );
		
		TInt x1 = aRect.iTl.iX, x2 = aRect.iBr.iX;
		TInt y1 = aRect.iTl.iY, y2 = aRect.iBr.iY;

		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderTop)) 
			y1 += (aStyle.iBorders.iTop.iWidth - 1)/2;
		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderRight)) 
			x2 -= (aStyle.iBorders.iRight.iWidth)/2 + 1;
		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderBottom))
			y2 -= (aStyle.iBorders.iBottom.iWidth)/2 + 1;
		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderLeft))
			x1 += (aStyle.iBorders.iLeft.iWidth - 1)/2;
		
		TPoint p(x1, y1);
		aGc.MoveTo(p);
		p = TPoint(x2,y1);
		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderTop))
			DrawBorder(aGc, aStyle.iBorders.iTop,  p, backColor, 0);
		aGc.MoveTo(p);

		p = TPoint(x2,y2);
		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderRight)) 
			DrawBorder(aGc, aStyle.iBorders.iRight, p, backColor, 1);
		aGc.MoveTo(p);
		
		p = TPoint(x1,y2);
		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderBottom))
			DrawBorder(aGc, aStyle.iBorders.iBottom, p, backColor, 2);
		aGc.MoveTo(p);
		
		p = TPoint(x1,y1);
		if(aStyle.iBorders.iMask.IsSet(THcBorders::EBorderLeft))
			DrawBorder(aGc, aStyle.iBorders.iLeft, p, backColor, 3);		
	}
}

void HcUtils::DrawBorder(CBitmapContext& aGc, const THcBorder& aBorder, const TPoint& p, const TRgb& /*aBackColor*/, TInt aPosition)
{	
	if(aBorder.iStyle==THcBorder::ENone) 
		aGc.SetPenStyle(CGraphicsContext::ENullPen);
	else if(aBorder.iStyle==THcBorder::EDotted)
		aGc.SetPenStyle(CGraphicsContext::EDottedPen);
	else if(aBorder.iStyle==THcBorder::EDashed)
		aGc.SetPenStyle(CGraphicsContext::EDashedPen);
	else
		aGc.SetPenStyle(CGraphicsContext::ESolidPen);

	if(aBorder.iStyle==THcBorder::EOutset) 
	{
		if(aPosition==1 || aPosition==2)
			aGc.SetPenColor(ColorUtils::RgbDarkerColor(aBorder.iColor, aGc.Device()->DisplayMode()));
		else
			aGc.SetPenColor(ColorUtils::RgbLighterColor(aBorder.iColor, aGc.Device()->DisplayMode()));
	}
	else if(aBorder.iStyle==THcBorder::EInset)
	{
		if(aPosition==0 || aPosition==3)
			aGc.SetPenColor(ColorUtils::RgbDarkerColor(aBorder.iColor, aGc.Device()->DisplayMode()));
		else
			aGc.SetPenColor(ColorUtils::RgbLighterColor(aBorder.iColor, aGc.Device()->DisplayMode()));
	}
	else
		aGc.SetPenColor(aBorder.iColor);
	aGc.SetPenSize(TSize(aBorder.iWidth, aBorder.iWidth));

	aGc.DrawLineTo(p);
}

void HcUtils::PrepareGcForTextDrawing(CBitmapContext& aGc, const THcTextStyle& aStyle)
{
	if(aStyle.IsSet(THcTextStyle::EUnderline))
		aGc.SetUnderlineStyle(EUnderlineOn);
	else
		aGc.SetUnderlineStyle(EUnderlineOff);
	
	if(aStyle.IsSet(THcTextStyle::ELineThrough))
		aGc.SetStrikethroughStyle(EStrikethroughOn);
	else
		aGc.SetStrikethroughStyle(EStrikethroughOff);
	
	aGc.SetPenStyle(CBitmapContext::ESolidPen);
	if(aStyle.IsSet(THcTextStyle::EColor))
	{
		TRgb rgb = aStyle.iColor.Rgb();
		aGc.SetPenColor(rgb);
	}
	else
		aGc.SetPenColor(KRgbBlack);
	aGc.SetPenSize(TSize(1,1));
	
	if(aStyle.IsSet(THcTextStyle::EBackColor)) 
	{
		aGc.SetBrushStyle(CGraphicsContext::ESolidBrush);
		aGc.SetBrushColor(aStyle.iBackColor.Rgb());		
	}
	else
		aGc.SetBrushStyle(CGraphicsContext::ENullBrush );
}

void HcUtils::DrawBitmapMasked(CBitmapContext& aGc, const TRect &aDestRect, const CFbsBitmap *aBitmap, const TRect &aSourceRect, const CFbsBitmap *aMaskBitmap, TBool aInvertMask)
{
#ifdef __SYMBIAN_9_ONWARDS__
	aGc.DrawBitmapMasked(aDestRect, aBitmap, aSourceRect, aMaskBitmap, aInvertMask);
#else
	CWritableBitmap* helper1 = CHtmlCtlEnv::Static()->HelperBitmap(2, aDestRect.Size());
	CWritableBitmap* helper2 = CHtmlCtlEnv::Static()->HelperBitmap(0, aDestRect.Size());
	
	TRect rect(TPoint(0,0), aDestRect.Size());
	helper1->Gc().SetBrushStyle(CGraphicsContext::ENullBrush);
	helper1->Gc().DrawBitmap(rect, aBitmap, aSourceRect);
	helper2->Gc().SetBrushStyle(CGraphicsContext::ENullBrush);
	helper2->Gc().DrawBitmap(rect, aMaskBitmap, aSourceRect);
	aGc.BitBltMasked(aDestRect.iTl, helper1, rect, helper2, aInvertMask);
#endif
}

void HcUtils::ConvertToGray256(CFbsBitGc& aGc, const TRect &aRect)
{
	CWritableBitmap* helper = CHtmlCtlEnv::Static()->HelperBitmap(1, aRect.Size());
	helper->Gc().BitBlt(TPoint(0,0), aGc, aRect);
	TRgb rgbs[2] = { KRgbWhite, TRgb(0xE4EBEB) };
	helper->Gc().MapColors(TRect(TPoint(0,0), aRect.Size()), &rgbs[0]);
	aGc.BitBlt(aRect.iTl, helper->Gc(), TRect(TPoint(0,0), aRect.Size()));
}

#define sourceLeftTop (*aSourceRects)
#define sourceRightTop (*(aSourceRects+1))
#define sourceLeftBot (*(aSourceRects+2))
#define sourceRightBot (*(aSourceRects+3))

#define destLeftTop (*aDestRects)
#define destRightTop (*(aDestRects+1))
#define destLeftBot (*(aDestRects+2))
#define destRightBot (*(aDestRects+3))
void HcUtils::GetScale9GridRects(const TSize& aSourceSize, const TRect& aDestRect, const TRect& aScale9GridRect, 
		TRect* aSourceRects, TRect* aDestRects)
{
	TInt midW = aDestRect.Width() - aScale9GridRect.iTl.iX - (aSourceSize.iWidth - aScale9GridRect.iBr.iX);
	TInt midH = aDestRect.Height() - aScale9GridRect.iTl.iY - (aSourceSize.iHeight - aScale9GridRect.iBr.iY);

	sourceLeftTop = TRect(TPoint(0,0),aScale9GridRect.iTl);
	sourceRightTop = TRect(TPoint(aScale9GridRect.iBr.iX,0),TPoint(aSourceSize.iWidth, aScale9GridRect.iTl.iY));
	sourceLeftBot = TRect(TPoint(0, aScale9GridRect.iBr.iY), TPoint(aScale9GridRect.iTl.iX, aSourceSize.iHeight));
	sourceRightBot = TRect(aScale9GridRect.iBr, TPoint(aSourceSize.iWidth, aSourceSize.iHeight));
	
	if(midW<0)
	{
		sourceLeftTop.iBr.iX += midW/2;
		sourceRightTop.iTl.iX -= midW/2;
		sourceLeftBot.iBr.iX += midW/2;
		sourceRightBot.iTl.iX -= midW/2;
	}
	
	if(midH<0)
	{
		sourceLeftTop.iBr.iY += midH/2;
		sourceRightTop.iBr.iY += midH/2;
		sourceLeftBot.iTl.iY -= midH/2;
		sourceRightBot.iTl.iY -= midH/2;
	}
	
	destLeftTop = TRect(aDestRect.iTl, sourceLeftTop.Size());
	destRightTop = TRect(TPoint(aDestRect.iBr.iX - sourceRightTop.Width(), aDestRect.iTl.iY), sourceRightTop.Size());
	destLeftBot = TRect(TPoint(aDestRect.iTl.iX, aDestRect.iBr.iY - sourceLeftBot.Height()), sourceLeftBot.Size());
	destRightBot = TRect(TPoint(aDestRect.iBr.iX - sourceRightBot.Width(), aDestRect.iBr.iY - sourceRightBot.Height()), sourceRightBot.Size());
}

