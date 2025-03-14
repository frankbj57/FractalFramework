#include <iostream>
using namespace std;

template < typename myType >
class CircularBuffer
{
public:
	CircularBuffer(int = 10);
	CircularBuffer &insert(const myType &);
	myType meanValue() const;
	void print() const;
	~CircularBuffer()
	{
		delete[] arrayPtr_;
	};
	void clear();

private:
	myType *arrayPtr_;
	int size_;
	int newestIndex_;
	bool cleared;
};

template < typename myType >
CircularBuffer<myType>::CircularBuffer(int s)
{
	size_ = (s > 0 ? s : 10);

	arrayPtr_ = new myType[size_];

	newestIndex_ = -1;

	clear();
}

template < typename myType >
inline CircularBuffer<myType> & CircularBuffer<myType>::insert(const myType &v)
{
	if (cleared)
	{
		newestIndex_ = -1;

		// Initialize to given value
		for (int i = 0; i < size_; i++)
		{
			arrayPtr_[i] = v;  // Default constructor also valid for int and other base types
		}

		cleared = false;
	}
	else
	{
		(++newestIndex_) %= size_;
		arrayPtr_[newestIndex_] = v;
	}

	return *this;
}

template < typename myType >
inline myType CircularBuffer<myType>::meanValue() const
{
	myType sum = myType();  // Default constructor also valid for int and other base types

	for (int i = 0; i < size_; i++)
	{
		sum += arrayPtr_[i];
	}

	return sum / size_;

}

template < typename myType >
inline void CircularBuffer<myType>::print() const
{
	for (int i = 0; i < size_; i++)
	{
		 cout << arrayPtr_[(newestIndex_-i+size_) % size_] << endl;
	}

}

template<typename myType>
inline void CircularBuffer<myType>::clear()
{
	newestIndex_ = -1;

	// Initialize to default value
	for (int i = 0; i < size_; i++)
	{
		arrayPtr_[i] = myType();  // Default constructor also valid for int and other base types
	}

	cleared = true;
}
