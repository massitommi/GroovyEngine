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

	void resize(size_t size)
	{
		byte* newData = (byte*)malloc(size);
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

	inline byte* operator*() { return mData; }
	inline const byte* operator*() const { return mData; }

	inline operator bool() const { return mData; }

	Buffer& operator=(const Buffer& other) = delete;

private:
	byte* mData;
	size_t mSize;
};