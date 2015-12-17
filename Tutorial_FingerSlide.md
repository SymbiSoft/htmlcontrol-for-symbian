在带触摸屏手机上，htmlcontrol除了通常的点击和拖动外，还支持两种特别的滑动操作。

## 1.左右滑动 ##
当手指（或笔，下同）从左水平向右滑动，或从右水平向左滑动，将触发THtmlCtlEvent::ESlideLeft或THtmlCtlEvent::ESlideRight事件。
用户可以响应这两种事件进行处理，例如翻页等。

## 2.上下滑动 ##
在带有滚动条的区域，默认情况下，我们需要拖动滚动条的滑块进行上下滑动。当页面较长，滑块较小时，长时间的拖动可能使用户感到疲劳。
htmlcontrol支持直接拖动内容区，只需简单设置一个参数即可：

在HTML中设置：
```
<div style='overflow:scroll' scroll-on-slide='true'></div>
```

或者动态设置：
```
//e是带有滚动条的DIV element指针
e->SetProperty(KHStrScrollOnSlide, ETrue);
```

滑动操作示意图

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-06-05_134526.jpg](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/2009-06-05_134526.jpg)