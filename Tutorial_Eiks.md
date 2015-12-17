本文介绍如何用htmlcontrol实现一些基本的控件。

## Label ##
Label就是显示小量的文本，这个是很简单的，例如
```
_LIT(KHtml, "I'm a label.");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_103458.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_103458.jpg)

当然，你可能希望改变它的字体或者颜色，
```
_LIT(KHtml, "<font size='30' color='#ff0000'><b>I'm a label.</b></font>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_103530.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_103530.jpg)

或者添加边框和背景色，
```
_LIT(KHtml, "<div style='border:1 outset #333333;background-color:#cccccc'><font size='30' color='#ff0000'><b>I'm a label.</b></font></div>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_103853.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_103853.jpg)

## Edit ##
htmlcontrol封装了CEikEdwin，创建一个输入框变得很简单，例如
```
_LIT(KHtml, "<input type='text'>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_104121.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_104121.jpg)

你可能希望改变它的大小、边框和背景色，
```
_LIT(KHtml, "<input type='text' style='width:60%; border:1 solid #000000; background-color:#00ffff'>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_104505.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_104505.jpg)

或者希望创建一个多行的输入框，
```
_LIT(KHtml, "<textarea></textarea>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_104718.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_104718.jpg)

更时髦点的应该加上下划线，
```
_LIT(KHtml, "<textarea drawlines='true'></textarea>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_104835.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_104835.jpg)

样式也是想改就改的，比如说实现一个透明的输入框，
```
_LIT(KHtml, "<body style='background-image:images/2009-02-24_173611.jpg'><textarea draw-lines='true' style='background-color:none;color:#ff0000'></textarea></body>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105400.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105400.jpg)

## Image ##
htmlcontrol支持几乎所有格式的图片，例如
```
_LIT(KHtml, "<img src='images/N97_main.jpg'>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105555.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105555.jpg)

缩放是随意的，
```
_LIT(KHtml, "<img src='images/N97_main.jpg' width='100' height='100'>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105718.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105718.jpg)

支持GIF动画的播放，
```
_LIT(KHtml, "<img src='images/2.gif'>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105824.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105824.jpg)

GIF也可以缩放着播放，
```
_LIT(KHtml, "<img src='images/2.gif' width='200' height='200'>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105940.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_105940.jpg)

## Button ##
可以使用`<`input type='button'`>`实现一个传统的button，不过这里不讨论这种简单的方式。我们看一种复杂的button。
```
_LIT(KHtml, "<body style='padding:auto auto'><div focusing='true' style='width:100;height:30;border:1 inset #333333;background-color:#eeeeee;padding:auto auto'>Button</div></body>");
```
![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_110423.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_110423.jpg)

focusing='true'说明这个DIV是可以获得键盘焦点的（在触摸屏上则可以点击）。但作为Button，它还需在获得焦点时提示用户，也就是
```
_LIT(KHtml, "<body style='padding:auto auto'><div focusing='true' style='width:100;height:30;border:1 inset #333333;background-color:#eeeeee;padding:auto auto' focus-style='border:2 solid #0000ff; padding:-3'>Button</div></body>");
```
这里使用了focus-style定义获得焦点后的样式。

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_115639.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_115639.jpg)

下面是一个带图片的两个Button的例子（更简洁的写法可以使用样式表）
```
_LIT(KHtml, "<body style='padding:auto auto'>"
	"<div focusing='true' style='width:100;height:30;border:1 inset #333333;background-color:#eeeeee;padding:auto auto' focus-style='border:2 solid #0000ff; padding:-3'><img src='images/apply.png' width='20' height='20'>Button1</div><br><br>"
	"<div focusing='true' style='width:100;height:30;border:1 inset #333333;background-color:#eeeeee;padding:auto auto' focus-style='border:2 solid #0000ff; padding:-3'><img src='images/cancel.png' width='20' height='20'>Button2</div>"
	"</body>");
```

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_120924.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-05-26_120924.jpg)