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

	void* push_data(const void* data, size_t size)
	{
		if (size + mCurrentPtr > mData)
		{
			size_t prevSize = mCurrentPtr - mData;
			resize((prevSize + size) * 2, true);
			mCurrentPtr = mData + prevSize;
		}
		memcpy(mCurrentPtr, data, size);
		void* dataPtr = mCurrentPtr;
		mCurrentPtr += size;
		return dataPtr;
	}

	template<typename T>
	inline T* push(const T* data, size_t count)
	{
		return (T*)push_data(data, sizeof(T) * count);
	}

	template<typename T>
	inline T* push(const T& data)
	{
		return (T*)push_data(&data, sizeof(T));
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

class BufferView
{
public:
	BufferView(void* data, size_t size)
		: mCurrentPtr((byte*)data), mBytesLeft(size)
	{}

	BufferView(const Buffer& buffer)
		: mCurrentPtr((byte*)buffer.data()), mBytesLeft(buffer.size())
	{}

	byte* seek()
	{
		return mCurrentPtr;
	}

	template<typename T>
	T read()
	{
		check(mBytesLeft >= sizeof(T));
		T val = *(T*)mCurrentPtr;
		mCurrentPtr += sizeof(T);
		mBytesLeft -= sizeof(T);
		return val;
	}

	template<typename T>
	T* read(size_t count)
	{
		check(mBytesLeft >= sizeof(T) * count);
		T* ptr = (T*)mCurrentPtr;
		mCurrentPtr += sizeof(T) * count;
		mBytesLeft -= sizeof(T) * count;
		return ptr;
	}

	byte* read(size_t count)
	{
		check(mBytesLeft >= count);
		byte* ptr = mCurrentPtr;
		mCurrentPtr += count;
		mBytesLeft -= count;
		return ptr;
	}

	void advance(size_t bytes)
	{
		check(mBytesLeft >= bytes);
		mCurrentPtr += bytes;
		mBytesLeft -= bytes;
	}

private:
	byte* mCurrentPtr;
	size_t mBytesLeft;
};