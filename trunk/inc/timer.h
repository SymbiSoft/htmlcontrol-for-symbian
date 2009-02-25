#ifndef HCTIMER_H
#define HCTIMER_H

template <class T>
class CHcTimer : public CTimer
{
public:
	static CHcTimer<T>* NewL(T& aOwner, TInt aIndex=0);
	
protected:
	CHcTimer(T& aOwner, TInt aIndex);
	void RunL();
	TInt RunError(TInt aError);
	
private:
	T& iOwner;
	TInt iIndex;
};

template <class T>
CHcTimer<T>* CHcTimer<T>::NewL(T& aOwner, TInt aIndex) 
{
	CHcTimer<T>* self = new (ELeave)CHcTimer<T>(aOwner, aIndex);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
}

template <class T>
CHcTimer<T>::CHcTimer(T& aOwner, TInt aIndex):CTimer(EPriorityStandard),iOwner(aOwner), iIndex(aIndex)
{
	CActiveScheduler::Add(this);
}

template <class T>
void CHcTimer<T>::RunL()
{
	iOwner.OnTimerL(iStatus.Int(), iIndex);
}

template <class T>
TInt CHcTimer<T>::RunError(TInt)
{
	return KErrNone;
}

#endif
