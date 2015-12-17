﻿#summary Tutorial 如何使用htmlcontrol实现界面过渡效果

下面以一个例子说明如何使用htmlcontrol实现界面过渡效果。htmlcontrol内置支持了平移、淡入淡出、平移、缩放的过渡效果，而调用的方法非常简单，只是将原来的RefreshAndDraw改为Transition的Perform。

先看最终要达到的效果

![http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/slide.gif](http://htmlcontrol-for-symbian.googlecode.com/svn/wiki/images/slide.gif)

开始时先显示第一个图片，当用户按左键或右键时，显示下一个图片。在图片切换过程中，显示一个平移的效果。

定义一个设定页面内容的方法
```
void SetPageContent(TInt index)
{
	if(index==0)
	{
		_LIT(KHtml, "<div style='width:80%;height:80%;border:1;margin:auto auto;padding:auto auto'><img src='images/e1.jpg'><br>This is page1</div>");
		iControl->ClearContent();
		iControl->AppendContentL(KHtml);
	}
	else
	{
		_LIT(KHtml, "<div style='width:80%;height:80%;border:1;margin:auto auto;padding:auto auto'><img src='images/e2.jpg'><br>This is page2</div>");
		iControl->ClearContent();
		iControl->AppendContentL(KHtml);
	}
}
```


首先显示第一个图片
```
iIndex = 0;
SetPageContent(0);
iControl->RefreshAndDraw();
```

然后响应左右按键进行切换
```
TKeyResponse OfferKeyEventL (const TKeyEvent &aKeyEvent, TEventCode aType) 
{
	if(aKeyEvent.iCode==EKeyLeftArrow)
	{
		iIndex--;
		if(iIndex<0)
			iIndex = 1;
		SetPageContent(iIndex);
		iControl->Transition()->Perform(iControl->Body(), MTransition::ESlideRight, 600*1000, 5, &TEasingCircular::EaseOut);
	}
	else if(aKeyEvent.iCode==EKeyRightArrow)
	{
		iIndex++;
		if(iIndex>1)
			iIndex = 0;
		SetPageContent(iIndex);
		iControl->Transition()->Perform(iControl->Body(), MTransition::ESlideLeft, 600*1000, 5, &TEasingCircular::EaseOut);
	}
	else
		return iControl->OfferKeyEventL(aKeyEvent, aType);
}
```

这里关键点代码在Perform方法，第一个参数指定了需要进行过渡效果的元素；

第二个是过渡效果，这里使用了ESlideRight和ESlideLeft，也就是左右平移的效果；

第三个是效果的持续时间，这里选择了一个适当的值600毫秒；

第四个是效果的持续帧数，帧数越多，动画越细腻，但占用手机的CPU越多；

第五个是运动的物理特性，这里使用了TEasingCircular::EaseOut，则平移运动将先加速然后再减速至0，这种比匀速移动更真实。