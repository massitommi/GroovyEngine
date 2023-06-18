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
		if (!size)
		{
			free();
			return;
		}

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

	void* push_bytes(const void* data, size_t sizeBytes)
	{
		size_t bytesUsed = used();
		void* ptr = mCurrentPtr;
		if (bytesUsed + sizeBytes > mSize)
		{
			resize((bytesUsed + sizeBytes) * 2, true);
			ptr = mCurrentPtr = mData + bytesUsed;
		}
		memcpy(ptr, data, sizeBytes);
		mCurrentPtr += sizeBytes;
		return ptr;
	}

	template<typename T>
	void* push(const T& val)
	{
		return push_bytes(&val, sizeof(T));
	}

	template<typename T>
	void* push(const T* data, uint32 count)
	{
		return push_bytes(data, (sizeof(T) * count));
	}

	template<>
	void* push(const std::string& str)
	{
		return push_bytes(str.c_str(), str.length() + 1);
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

	BufferView(const DynamicBuffer& buffer)
		: mCurrentPtr((byte*)buffer.data()), mBytesLeft(buffer.used())
	{}

	byte* seek()
	{
		return mCurrentPtr;
	}

	template<typename T>
	T read()
	{
		T val = *(T*)mCurrentPtr;
		advance(sizeof(T) * 1);
		return val;
	}

	template<typename T>
	T* read(uint32 count)
	{
		T* ptr = (T*)mCurrentPtr;
		advance(sizeof(T) * count);
		return ptr;
	}

	byte* read(size_t count)
	{
		byte* ptr = mCurrentPtr;
		advance(count);
		return ptr;
	}

	template<>
	std::string read<std::string>()
	{
		std::string str((char*)mCurrentPtr);
		advance(str.length() + 1);
		return str;
	}

	void advance(size_t bytes)
	{
		check(mBytesLeft >= bytes);
		mCurrentPtr += bytes;
		mBytesLeft -= bytes;
	}

	size_t remaining() { return mBytesLeft; }

private:
	byte* mCurrentPtr;
	size_t mBytesLeft;
};