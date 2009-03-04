/*
* ============================================================================
*  Name     : MTransition
*  Part of  : CHtmlControl
*  Created  : 03/01/2009 by ytom 
*  Version  : 1.0
*  Copyright: ytom(gzytom@gmail.com)
* ============================================================================
*/

#ifndef TRANSITION_H
#define TRANSITION_H

#include <e32base.h>

class CHtmlElement;

/**
 * Easing allows your animation to speed up or slow down between keyframes.
	@param t Specifies the current time, between 0 and duration inclusive.
	@param b Specifies the initial value of the animation property.
	@param c Specifies the total change in the animation property.
	@param d Specifies the duration of the motion.
	@return he value of the interpolated property at the specified time.
	*/
typedef TReal (*EasingFunction)(TReal t, TReal b, TReal c, TReal d);

/**
 * The Linear class defines easing functions to implement non-accelerated motion with HtmlControl transitions.
 * Its methods all produce the same effect, a constant motion. The various names easeIn, easeOut and so on, are provided in the interest of polymorphism.
 */
class TEasingLinear
{
public:
	/**
	 *  The easeNone() method defines a constant motion with no acceleration.
	 */
	static TReal EaseNone(TReal t, TReal b, TReal c, TReal d);
};

/**
 * The Circular class defines three easing functions to implement motion with HtmlControl transitions. 
 * The acceleration of motion for a Circular easing equation produces an abrupt change in velocity.
 */
class TEasingCircular
{
public:
	/**
	 * The easeIn() method starts motion from zero velocity and then accelerates motion as it executes.
	 */
	static TReal EaseIn(TReal t, TReal b, TReal c, TReal d);
	/**
	 * The easeInOut() method combines the motion of the easeIn() and easeOut() methods to start the motion from a zero velocity, accelerate motion, then decelerate to a zero velocity.
	 */
	static TReal EaseOut(TReal t, TReal b, TReal c, TReal d);
	/**
	 * The easeOut() method starts motion fast and then decelerates motion to a zero velocity as it executes.
	 */
	static TReal EaseInOut(TReal t, TReal b, TReal c, TReal d);
};

/**
 * The Back class defines three easing functions to implement motion with HtmlControl transitions.
 */
class TEasingBack
{
public:
	/**
	 * The easeIn() method starts the motion by backtracking and then reversing direction and moving toward the target.
	 */
	static TReal EaseIn(TReal t, TReal b, TReal c, TReal d);
	/**
	 * The easeOut() method starts the motion by moving towards the target, overshooting it slightly, and then reversing direction back toward the target.
	 */
	static TReal EaseOut(TReal t, TReal b, TReal c, TReal d);
	/**
	 *  The easeInOut() method combines the motion of the easeIn() and easeOut() methods to start the motion by backtracking, then reversing direction and moving toward the target, overshooting the target slightly, reversing direction again, and then moving back toward the target.
	 */
	static TReal EaseInOut(TReal t, TReal b, TReal c, TReal d);
};

/**
 * Transition class
 */
class MTransition
{
public:
	enum TTransitionType
	{
		//new content slides from right to left.
		ESlideLeft,
		
		//new content slides from left to right.
		ESlideRight,
		
		//new content slides from bottom to top.
		ESlideUp,
		
		//new content slides from top to bottom.
		ESlideDown,
		
		//new content fades in.
		EFade,
		
		//element fly from old position to new position.
		EFly,
		
		//element change its size from old size to new size.
		EScale,
		
		//combie the effect of EFly and EScale
		EFlyAndScale
	};
	
	/**
	 * Perform the transition on specific element.
	 * @param aElement target element.
	 * @param aType TTransitionType enums.
	 * @param aDuration the total time of the motion.
	 * @param aFrames frames count of the motion.
	 * @param aEasingFunc EasingFunction
	 */
	virtual void Perform(CHtmlElement* aElement, 
			TTransitionType aType, 
			TTimeIntervalMicroSeconds32 aDuration, 
			TInt aFrames,
			EasingFunction aEasingFunc=&TEasingLinear::EaseNone) = 0;
};

#endif
