#pragma once

#include "coreminimal.h"

class Buffer
{
public:
	Buffer()
		: mData(nullptr), mSize(0)
	{
	}

	Buffer(size_t size)
		: mData(nullptr), mSize(0)
	{
		resize(size);
	}

	Buffer(const Buffer& other) = delete;

	~Buffer()
	{
		free();
	}

	void resize(size_t size, bool copyOldData = false)
	{
		byte* newData = (byte*)malloc(size);
		
		if (copyOldData && mData)
		{
			size_t cpyLen = size > mSize ? mSize : size;
			memcpy(newData, mData, cpyLen);
		}

		if (mData)
		{
			::free(mData);
		}

		mData = newData;
		mSize = size;
	}

	void free()
	{
		if (mData)
		{
			::free(mData);
			mData = nullptr;
		}
		mSize = 0;
	}

	inline byte* data() { return mData; }
	inline const byte* data() const { return mData; }
	inline size_t size() const { return mSize; }

	template<typename T>
	inline T* as()
	{
		return (T*)mData;
	}

	template<typename T>
	inline const T* as() const
	{
		return (T*)mData;
	}

	inline byte* operator*() { return mData; }
	inline const byte* operator*() const { return mData; }

	inline operator bool() const { return mData; }

	Buffer& operator=(const Buffer& other) = delete;

protected:
	byte* mData;
	size_t mSize;
};

class DynamicBuffer : public Buffer
{
public:
	DynamicBuffer()
		: Buffer(), mCurrentPtr(nullptr)
	{}

	DynamicBuffer(size_t size)
		: Buffer(size), mCurrentPtr(nullptr)
	{}

	void push(void* data, size_t size)
	{
		if (size + mCurrentPtr > mData)
		{
			resize((size + mCurrentPtr - mData) * 2, true);
		}
		memcpy(mCurrentPtr + size, data, size);
		mCurrentPtr += size;
	}

	template<typename T>
	inline void push(T* data, size_t count)
	{
		push(data, sizeof(T) * count);
	}

	inline void pop(size_t size)
	{
		check(mCurrentPtr - size >= mData);
		mCurrentPtr -= size;
	}

	inline size_t used() const { return mCurrentPtr - mData; }
	inline byte* current() { return mCurrentPtr; }
	inline const byte* current() const { mCurrentPtr; }

	template<typename T>
	inline const T* current() const
	{
		return (T*)current();
	}

	template<typename T>
	inline T* current()
	{
		return (T*)current();
	}

private:
	byte* mCurrentPtr;
};