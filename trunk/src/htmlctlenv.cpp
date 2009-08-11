#include <coeutils.h>
#include <f32file.h>
#include <eikenv.h>
#include <eikappui.h>

#include "htmlcontrol.hrh"

#ifdef __SERIES60__
#include <aknutils.h>

#endif

#ifdef __UIQ__
#include <QikMediaFileFolderUtils.h>
#include <QikUiConfigDataTypes.h>
#include <QikUiConfigDataTypes.h>
#endif

#include "htmlctlenv.h"
#include "htmlparser.h"
#include "htmlctlutils.h"
#include "measurestatus.h"
#include "imagepool.h"
#include "scrollbar.h"
#include "utils.h"
#include "writablebitmap.h"

static const TInt BASE_FONT_SIZE1[] = {9, 11,13,15,17,19,24};
static const TInt BASE_FONT_SIZE2[] = {10,12,14,16,18,22,26};
static const TInt BASE_FONT_SIZE3[] = {16,20,22,24,28,32,38};

#define KHtmlCtlEnvUid TUid::Uid(0x080F046C)
CHtmlCtlEnv* CHtmlCtlEnv::Static()
{
	CHtmlCtlEnv* instance = static_cast<CHtmlCtlEnv*> ( CCoeEnv::Static( KHtmlCtlEnvUid ) );

	if ( !instance )
	{
		instance = new ( ELeave ) CHtmlCtlEnv;
		instance->ConstructL();
	}

  	return instance;
}
	
CHtmlCtlEnv::CHtmlCtlEnv() : CCoeStatic(KHtmlCtlEnvUid, -1)
{
}

CHtmlCtlEnv::~CHtmlCtlEnv()
{
	iObjectFactories.Close();
	iDestroyMonitors.Close();
	
	delete iSearchPaths;
	delete iStyleSheet;
	delete iFontFamilies;
	delete iHtmlParser;
	delete iMeasureStatus;
	delete iScrollbarDrawer;
	delete iImagePool;
	for(TUint i=0;i<sizeof(iHelperBitmap)/sizeof(CWritableBitmap*);i++)
		delete iHelperBitmap[i];

#ifdef TOUCH_FEEDBACK_SUPPORT
	if(iTouchFeedback && iOwnTouchFeedbackInstance)
		iTouchFeedback->DestroyInstance();
#endif
}

void CHtmlCtlEnv::ConstructL()
{
#ifdef __SERIES60__
	_LIT(KProductIDFile, "Series60v*.sis");
	_LIT(KROMInstallDir, "z:\\system\\install\\");
	
	TFindFile ff( CEikonEnv::Static()->FsSession() );
	CDir* result;
	if( ff.FindWildByDir( KProductIDFile, KROMInstallDir, result ) == KErrNone) 
	{
		CleanupStack::PushL( result );
		User::LeaveIfError( result->Sort( ESortByName|EDescending ) );
		iPlatformVersion.iMajor = (*result)[0].iName[9] - '0';
		iPlatformVersion.iMinor = (*result)[0].iName[11] - '0';
		CleanupStack::PopAndDestroy(); // result
	}
#endif

	iSearchPaths = new CDesCArrayFlat(2);
    TFileName temp;
#ifdef __SYMBIAN_9_ONWARDS__
	CEikonEnv::Static()->FsSession().PrivatePath(temp);
	temp.Insert(0, _L("c:"));
	iSearchPaths->AppendL(temp);
	temp[0] = 'e';
	iSearchPaths->AppendL(temp);
#ifdef __UIQ__
	temp[0] = 'd';
	iSearchPaths->AppendL(temp);
#endif
#else
	_LIT(KTemp, "1.txt");
	temp.Copy(KTemp);
	CompleteWithAppPath(temp);
	temp.SetLength(temp.Length()-5);
	temp[0] = 'c';
	iSearchPaths->AppendL(temp);
	temp[0] = 'e';
	iSearchPaths->AppendL(temp);
#endif

	iImagePool = CHcImagePool::NewL();
	iFontFamilies = new CDesCArrayFlat(5);
	CWsScreenDevice* screen = CEikonEnv::Static()->ScreenDevice();
	int n = screen->NumTypefaces();
	TTypefaceSupport ts;
	for(TInt i=0;i<n;i++)
	{
		screen->TypefaceSupport(ts, i);
		iFontFamilies->AppendL(ts.iTypeface.iName);
	}
	
	TSize size = HtmlCtlUtils::ScreenSize(EOrientationLandscape);
	if(size.iWidth<320)
		iBaseFontSize = &BASE_FONT_SIZE1[0];
	else if(size.iWidth<640)
		iBaseFontSize = &BASE_FONT_SIZE2[0];
	else
		iBaseFontSize = &BASE_FONT_SIZE3[0];

    iHtmlParser = CHtmlParser::NewL();
	iMeasureStatus = CHcMeasureStatus::NewL();
    iScrollbarDrawer = CHcScrollbarDrawer::NewL();

    iStyleSheet = new (ELeave) CHcStyleSheet();
    {
		_LIT(KCSS, "body {  background-color:#FFFFFF; color:#000000; font-size:medium; width:100%; height:100%; overflow:hidden; } \
					.blockquote { background-color:#eeeeee; clear:both; width:100%} \
					object {clear:both } \
					input { background-color:#FFFFFF; color:#000000; border:1 solid #7F9DB9; margin:3; }");
    	HtmlParser::ParseStyleSheetL(KCSS, *iStyleSheet);
    }
    {
    	_LIT(KCSS, "input[type=button] { background-color:#eeeeee; border:1 outset #eeeeee}\
					input[type=button]:focus { margin:-2; border:2 solid #FF9E39} \
					input[type=button]:down { border:1 inset #eeeeee }");
    	HtmlParser::ParseStyleSheetL(KCSS, *iStyleSheet);
    }
    {
		_LIT(KCSS, "input[type=checkbox] {color: #008000} \
					input[type=checkbox]:focus {margin:-2; border:2 solid #FF9E39}");
    	HtmlParser::ParseStyleSheetL(KCSS, *iStyleSheet);
    }
    {
    	_LIT(KCSS, "input[type=radio] { color: #008000; filter:corner(9999,9999)} \
					input[type=radio]:focus { margin:-2; border:2 solid #FF9E39}");
    	HtmlParser::ParseStyleSheetL(KCSS, *iStyleSheet);
    }
    {
    	_LIT(KCSS, "input[type=text]:focus { margin:-2; border:2 solid #FF9E39}");
    	HtmlParser::ParseStyleSheetL(KCSS, *iStyleSheet);
    }
    {
    	_LIT(KCSS, "textarea { background-color:#FFFFFF; color:#000000; border:1 solid #7F9DB9; margin:3; clear:both} \
					textarea:focus { margin:-2; border:2 solid #FF9E39}");
    	HtmlParser::ParseStyleSheetL(KCSS, *iStyleSheet);
    }
    {
    	_LIT(KCSS, "select {  background-color:#FFFFFF; color:#000000; border:1 solid #7F9DB9; margin:3; font-size:medium; } \
					select:focus { margin:-2; border:2 solid #FF9E39 } \
					.selectbutton {background-color:#C5D3FC; color:#4D6185; border:1 solid #7AC4EA; filter:corner(2,2); background-pattern:diamond-cross-hatch #7AC4EA}");
    	HtmlParser::ParseStyleSheetL(KCSS, *iStyleSheet);
    }
    {
    	_LIT(KCSS, "a:link { color: #0000FF; text-decoration: underline} \
					a:hover { color: #000000; text-decoration: underline; background-color:#A4A2FF}");
    	HtmlParser::ParseStyleSheetL(KCSS, *iStyleSheet);
    }

#ifdef TOUCH_FEEDBACK_SUPPORT
	iTouchFeedback = MTouchFeedback::Instance();
	if(!iTouchFeedback)
	{
		iTouchFeedback = MTouchFeedback::CreateInstanceL();
		iOwnTouchFeedbackInstance = ETrue;
	}
#endif
}

void CHtmlCtlEnv::AddSearchPathL(const TDesC& aPath)
{
	if(aPath.Length()==0)
		return;

	iSearchPaths->InsertL(0, aPath);
}

TBool CHtmlCtlEnv::ResolvePath(TDes& aPath) const
{
	if(aPath.Locate(':')!=KErrNotFound)
		return ConeUtils::FileExists(aPath);
	
	TFileName fileName;
	for(TInt i=0;i<Static()->iSearchPaths->Count();i++) 
	{
		fileName.Zero();
		fileName.Append((*Static()->iSearchPaths)[i]);
		fileName.Append(aPath);
		HcUtils::StringReplace(fileName, '/', '\\');
		
		if(ConeUtils::FileExists(fileName)) 
		{
			aPath.Copy(fileName);
			return ETrue;
		}
	}
	
	return EFalse;
}

void CHtmlCtlEnv::AddObjectFactory(NewHtmlObjectL aFactory)
{
	if(iObjectFactories.Find(aFactory)==KErrNotFound)
		iObjectFactories.Append(aFactory);
}

CCoeControl* CHtmlCtlEnv::CreateObjectL(const TDesC& aClassId, const TDesC& aMimeType, const TDesC& aData, 
		CHtmlControl& aParent, CHtmlElement& aElement)
{
	for(TInt i=iObjectFactories.Count()-1;i>=0;i--)
	{
		CCoeControl* control = (*iObjectFactories[i])(aClassId, aMimeType, aData, aParent, aElement);
		if(control)
			return control;
	}
	
	return NULL;
}

void CHtmlCtlEnv::AddDestroyMoinitor(CHtmlControlImpl* aObject)
{
	TInt pos = iDestroyMonitors.Find(aObject);
	if(pos==KErrNotFound)
		iDestroyMonitors.Append(aObject);
}
	
TBool CHtmlCtlEnv::RemoveDestroyMonitor(CHtmlControlImpl* aObject)
{
	TInt pos = iDestroyMonitors.Find(aObject);
	if(pos!=KErrNotFound)
	{
		iDestroyMonitors.Remove(pos);
		return ETrue;
	}
	else
		return EFalse;
}

CWritableBitmap* CHtmlCtlEnv::HelperBitmap(TInt aIndex, const TSize& aSize) 
{
	if(!iHelperBitmap[aIndex] 
	    || iHelperBitmap[aIndex]->SizeInPixels().iWidth<aSize.iWidth
	    || iHelperBitmap[aIndex]->SizeInPixels().iHeight<aSize.iHeight) 
	{
		delete iHelperBitmap[aIndex];
		iHelperBitmap[aIndex] = NULL;
		
		iHelperBitmap[aIndex] = CWritableBitmap::NewL(aSize, aIndex==0?EGray2:(aIndex==1?EGray256:EColor16M));
	}

	return iHelperBitmap[aIndex];
}

