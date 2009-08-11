#include <eikenv.h>
#include <coemain.h>

#include "htmlcontrol.hrh"

#ifdef __SERIES60__
#include <aknappui.h>
#include <aknutils.h>
#endif

#ifdef __UIQ__
#include <QUiConfigClient.h>
#endif

#include "HtmlCtlUtils.h"
#include "htmlctlenv.h"
#include "htmlparser.h"
#include "style.h"
#include "utils.h"

TBool HtmlCtlUtils::IsOrientationLandscape() 
{
#ifdef __SERIES60_3_ONWARDS__
	TSize size = HtmlCtlUtils::ScreenSize(EOrientationCurrent);
	return size.iWidth>size.iHeight;
#endif
	
#ifdef __SERIES60_3_DOWNWARDS__
	return EFalse;
#endif
	
#ifdef __UIQ__
	TQikScreenMode sm = CQUiConfigClient::Static().CurrentConfig().ScreenMode();
	return sm==EQikUiConfigLandscape || sm==EQikUiConfigSmallLandscape;
#endif
}

#ifdef __SERIES60__
TBool HtmlCtlUtils::IsAppForeground()
{
	return ((CAknAppUi*)CEikonEnv::Static()->AppUi())->IsForeground();
}
#else
TBool HtmlCtlUtils::IsAppForeground()
{
	CCoeEnv* env = CCoeEnv::Static();
	return env->WsSession().GetFocusWindowGroup() == env->RootWin().Identifier();
}
#endif

CFont* HtmlCtlUtils::CreateFontL(const TDesC& aStyle)
{
	THcTextStyle textStyle;
	HtmlParser::ParseTextStylesL(aStyle, textStyle);
	return textStyle.CreateFont();
}

TRgb HtmlCtlUtils::ConvertHtmlColorToRgb(const TDesC& aSource)
{
	THcColor color;
	HtmlParser::ParseColor(aSource, color);
	return color.Rgb();
}

void HtmlCtlUtils::ConvertRgbToHtmlColor(const TRgb& aRgb, TDes& aBuf)
{
	HtmlParser::GetRgb(aRgb, aBuf);
}

TVersion HtmlCtlUtils::PlatformVersion()
{
	return CHtmlCtlEnv::Static()->PlatformVersion();
}

TSize HtmlCtlUtils::ScreenSize(TScreenOrientation aOrientation)
{
	TSize size;
#ifdef __UIQ__
	size = CEikonEnv::Static()->ScreenDevice()->SizeInPixels();
#else

#ifdef __SERIES60_3_ONWARDS__
	AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EScreen, size);
#else
	size = TSize(176,208);
#endif
	
#endif
	
	if(aOrientation==EOrientationPortrait)
	{
		if(size.iWidth>size.iHeight)
		{
			TInt t = size.iHeight;
			size.iHeight = size.iWidth;
			size.iWidth = t;
		}
	}
	else if(aOrientation==EOrientationLandscape)
	{
		if(size.iWidth<size.iHeight)
		{
			TInt t = size.iHeight;
			size.iHeight = size.iWidth;
			size.iWidth = t;
		}
	}
	
	return size;
}

HBufC* HtmlCtlUtils::DecodeL(const TDesC& aSource)
{
	return HcUtils::DecodeHttpCharactersL(aSource);
}

HBufC8* HtmlCtlUtils::DecodeL(const TDesC8& aSource)
{
	return HcUtils::DecodeHttpCharactersL(aSource);
}

HBufC* HtmlCtlUtils::EncodeL(const TDesC& aSource)
{
	return HcUtils::EncodeHttpCharactersL(aSource);
}

HBufC8* HtmlCtlUtils::EncodeL(const TDesC8& aSource)
{
	return HcUtils::EncodeHttpCharactersL(aSource);
}


