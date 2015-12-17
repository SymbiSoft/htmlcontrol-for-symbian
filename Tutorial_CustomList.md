这篇文章介绍了如何利用htmlcontrol控件实现一个完全自定义外观的列表控件，包括背景、项目高度、字体、颜色等等。

头文件：
```
#include <html\htmlctlevent.h>
#include <html\lirenderer.h>
 
class CHtmlControl;
class CAppUi : public CAknAppUi, MHtmlCtlEventObserver, MListItemRenderer
{
public:
 
private:
  // Functions from base classes
  void HandleResourceChangeL(TInt aType);
 
  void HandleHtmlCtlEventL(const THtmlCtlEvent& aEvent);
 
  void RenderListItemL(CHtmlElement* aElement, TInt aIndex);
	
private:
  CHtmlControl* iControl;
  CHtmlElement* iList;
};
```

源文件：
```
iControl = CHtmlControl::NewL(NULL);
iControl->SetMopParent(this);
iControl->SetRect(ClientRect());
iControl->SetEventObserver(this);
iControl->ActivateL();
AddToStackL(iControl);

//设置样式表
_LIT(KStyleSheet, ".list_bg{ background-image: skin#1000; color:skin#5}"
  ".item {}"
  ".focus-item { background-image:frame#1400;  filter:corner(3,3); color:skin#9 }"
);
iControl->AddStyleSheetL(KStyleSheet);

//创建列表
_LIT(KHtml, "<div id='list' class='list_bg' style='width:100%;height:100%;overflow:scroll'></div>");
iControl->AppendContentL(KHtml);
_LIT(KIdList, "list");
iList = iControl->Element(KIdList);
iList->InvokeL(KHStrCreateList, (MListItemRenderer*)this);

//设置列表项模板
_LIT(KItemHtml, 
  "<div id='item' style='width:100%; height:22;' class='item' focus-class='focus-item'>"
      "<div style='width:20;height:100%;border-right:1 dotted;padding:auto auto'><img src='' width='16' height='16'></div>"
      "<div id='title' style='padding:auto 0 auto 2; width:auto; height:100%' line-wrap='scroll'></div>"
  "</div>");
iList->SetProperty(KHStrListItemHtml, KItemHtml);  

//设置当列表为空时显示的内容
_LIT(KEmptyHtml, "<p align='center'>(No Data)</p>");
iList->SetProperty(KHStrListEmptyHtml, KEmptyHtml);

//设置列表项目数量
iList->SetProperty(KHStrListItemCount, 20);

//更新并重画控件
iControl->RefreshAndDraw();
```

由于创建的列表是虚拟列表，也就是列表控件并不管理数据，而是由回调函数完成从数据到界面的渲染，因此当列表项目非常多时，仍然具有较高的性能。
```
void CAppUi::RenderListItemL(CHtmlElement* aElement, TInt aIndex)
{
  //在这里，完成对aElement的渲染，你可以任意改变它的样式和属性。
 
  _LIT(KIdTitle, "title");
  CHtmlElement* icon = aElement->ElementByTag(KHStrImg);
  CHtmlElement* title = aElement->Element(KIdTitle);
 
  icon->SetProperty(KHStrSrc, ...);
  title->SetProperty(KHStrInnerText, ...);
  if(...)
    title->SetStyle(KHStrFontWeight, KHStrBold);
  else
    title->SetProperty(KHStrFontWeight, KNullDesC);
}
void CAppUi::HandleHtmlCtlEventL(const THtmlCtlEvent& aEvent)
{
  if(aEvent.iType==THtmlCtlEvent::EOnClick && aEvent.iElement->Id().Compare(KIdItem)==0)
  {
     //TODO: 响应单击事件
     TInt itemIndex;
     iList->GetProperty(KHStrListSelectedIndex, itemIndex);
 
     ...
  }
}
void CAppUi::HandleResourceChangeL(TInt aType)
{
  CAknAppUi::HandleResourceChangeL(aType);
	
  //当手机主题发生改变时，通知所有控件
  HtmlCtlLib::ReportResourceChange(aType);
	
  //当Orientation发生改变时，通知控件
  if(aType==KEikDynamicLayoutVariantSwitch)
  {
     iControl->SetRect(ClientRect());
     iControl->Refresh();
  }
}
```

如果要更改列表中某项，你需要告诉控件更新，然后控件就会重新调用回调函数进行渲染。例如
```
//通知需要更新第10个数据项
iList->InvokeL(KHStrUpdate, 10);
iControl->RefreshAndDraw();
```
使用iList->InvodeL(KHStrUpdateAll)可以更新整个列表，通常用于重新设置了列表项数量。

在上面的代码片段中，完成的效果为：

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-23_105102.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-23_105102.jpg)

它使用了系统的默认皮肤，这里，发生作用的代码片段为
```
_LIT(KStyleSheet, ".list_bg{ background-image: skin#1000; color:skin#5}"
    ".item {}"
    ".focus-item { background-image:frame#1400;  filter:corner(3,3); color:skin#9 }"
  );
iControl->AddStyleSheetL(KStyleSheet);
```

现在我们来改变它的皮肤，例如：
```
//Set stylesheet
_LIT(KStyleSheet, ".list_bg{ background-image: swbkgnd.mbm; color:#000000; }"
    ".item {  }"
    ".focus-item { background-color:#FFCC00; border:1 solid #FFCC00; filter:alpha(opacity=30);filter:corner(5,5) }"
);
iControl->ClearStyleSheet();
iControl->AddStyleSheetL(KStyleSheet);
```

效果图

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-03-04_144240.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-03-04_144240.jpg)

这里使用了自定义的图片做列表的背景图，选中列表项的效果是一个带圆角和透明效果的背景。

另一套示范的皮肤：
```
//Set stylesheet
  _LIT(KStyleSheet, ".list_bg{ background-image: swbkgnd.mbm; color:#000000; }"
    ".item { border-bottom:1 solid #DDDDDD }"
    ".focus-item { { color:#FFFFFF; background-image:gradient#4199F5,286AE9 }"
  );
  iControl->ClearStyleSheet();
  iControl->AddStyleSheetL(KStyleSheet);
```

效果图

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-03-04_144321.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-03-04_144321.jpg)

这里的选中列表项效果是一个渐变色的背景。而且，每个列表的底部都增加了分隔线。

这个列表可以自动支持S60第五版，但由于屏幕大小不一致，这里还要做些处理，关键是设置列表项模板那里需要改变列表项的大小。有两种方法，第一种是运行时决定：
```
//VGA_ONWARDS的定义在htmlctlutils.h里
  if(VGA_ONWARDS) //640xN的分辨率
  {
    _LIT(KItemHtml, 
      "<div id='item' style='width:100%; height:40;' class='item' focus-class='focus-item'>"
          "<div style='width:20;height:100%;border-right:1 dotted;padding:auto auto'><img src='' width='32' height='32'></div>"
          "<div id='title' style='padding:auto 0 auto 2; width:auto; height:100%' line-wrap='scroll'></div>"
      "</div>");
    iList->SetProperty(KHStrListItemHtml, KItemHtml);  
  }
  else
  {
    _LIT(KItemHtml, 
       "<div id='item' style='width:100%; height:22;' class='item' focus-class='focus-item'>"
          "<div style='width:20;height:100%;border-right:1 dotted;padding:auto auto'><img src='' width='16' height='16'></div>"
          "<div id='title' style='padding:auto 0 auto 2; width:auto; height:100%' line-wrap='scroll'></div>"
      "</div>");
    iList->SetProperty(KHStrListItemHtml, KItemHtml);  
  }
```

另一种是编译时确定，例如：
```
#ifdef __S60_50__
#define ITEM_HEIGHT 40
#define IMAGE_SIZE 32
#else
#define ITEM_HEIGHT 22
#define IMAGE_SIZE 16
#endif
 
  _LIT(KItemHtml, 
     "<div id='item' style='width:100%; height:"MAKESTR(ITEM_HEIGHT)";' class='item' focus-class='focus-item'>"
        "<div style='width:20;height:100%;border-right:1 dotted;padding:auto auto'>
           <img src='' width='"MAKESTR(IMAGE_SIZE)"' height='"MAKESTR(IMAGE_SIZE)"'></div>"
        "<div id='title' style='padding:auto 0 auto 2; width:auto; height:100%' line-wrap='scroll'></div>"
    "</div>");
  iList->SetProperty(KHStrListItemHtml, KItemHtml);
```

效果图为：
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-23_105809.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-02-23_105809.jpg)