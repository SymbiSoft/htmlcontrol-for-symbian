#ifndef HCSTACK_H
#define HCSTACK_H

template <class T>
class RHcStack
{
public:
	void Close() 
	{
		iArray.Close();
	}
	
	void Push(const T& e) 
	{
		iArray.Append(e);
	}
	
	T Pop() 
	{
		TInt i=iArray.Count()-1;
		T ret = iArray[i];
		iArray.Remove(i);
		return ret;
	}
	
	void PopAndDestroy()
	{
		T t = Pop();
		delete t;
	}
	
	TInt Count() const
	{
		return iArray.Count();
	}
	
	T Top() const
	{
		return iArray[iArray.Count()-1];
	}
	
	void SetTop(const T& e)
	{
		iArray[iArray.Count()-1] = e;
	}
	
	void Reset() 
	{
		iArray.Reset();
	}
	
	void ResetAndDestroy()
	{
		for(TInt i=0;i<iArray.Count();i++) 
		{
			T t = iArray[i];
			delete t;
		}
		iArray.Reset();
	}
	
private:
	RArray<T> iArray;
};

#endif
