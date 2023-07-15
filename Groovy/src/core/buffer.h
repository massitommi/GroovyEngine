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

	Buffer(const Buffer& other)
		: mData(nullptr), mSize(0)
	{
		copy(other, *this);
	}

	Buffer(Buffer&& tmp)
		: mData(tmp.mData), mSize(tmp.mSize)
	{
		tmp.mData = nullptr;
		tmp.mSize = 0;
	}

	~Buffer()
	{
		free();
	}

	static void copy(const Buffer& from, Buffer& to)
	{
		if (from.mSize)
		{
			to.resize(from.mSize);
			memcpy(to.mData, from.mData, from.mSize);
		}
		else
		{
			to.free();
		}
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
			::free(mData);
		mData = nullptr;
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

	Buffer& operator=(const Buffer& other)
	{
		copy(other, *this);
		return *this;
	}

	Buffer& operator=(Buffer&& other)
	{
		byte* dataToDiscard = mData;
		size_t sizeToDiscard = mSize;
		mData = other.mData;
		mSize = other.mSize;
		other.mData = dataToDiscard;
		other.mSize = sizeToDiscard;
		return *this;
	}

protected:
	byte* mData;
	size_t mSize;
};

class DynamicBuffer
{
public:
	DynamicBuffer()
		: mData(nullptr), mSize(0), mCurrentPtr(nullptr)
	{}

	DynamicBuffer(size_t size)
		: mData(nullptr), mSize(0), mCurrentPtr(nullptr)
	{
		resize(size);
	}

	DynamicBuffer(const DynamicBuffer& other)
		: mData(nullptr), mSize(0), mCurrentPtr(nullptr)
	{
		copy(other, *this);
	}

	DynamicBuffer(DynamicBuffer&& tmp)
		: mData(tmp.mData), mSize(tmp.mSize), mCurrentPtr(tmp.mCurrentPtr)
	{
		tmp.mData = nullptr;
		tmp.mSize = 0;
		tmp.mCurrentPtr = nullptr;
	}

	~DynamicBuffer()
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

		mCurrentPtr = mData;
	}

	static void copy(const DynamicBuffer& from, DynamicBuffer& to)
	{
		size_t used = from.used();
		if (used)
		{
			to.resize(from.mSize);
			to.mCurrentPtr = to.mData + used;
			memcpy(to.mData, from.mData, from.mSize);
		}
		else
		{
			to.free();
		}
	}

	void free()
	{
		if (mData)
			::free(mData);
		mData = nullptr;
		mSize = 0;
		mCurrentPtr = nullptr;
	}

	void* push_bytes(const void* data, size_t sizeBytes)
	{
		check(data);

		size_t bytesUsed = used();
		if (bytesUsed + sizeBytes > mSize)
		{
			resize((bytesUsed + sizeBytes) * 2, true);
			mCurrentPtr = mData + bytesUsed;
		}
		void* ptr = mCurrentPtr;
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

	inline byte* data() const { return mData; }
	inline size_t size() const { return mSize; }
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

	DynamicBuffer& operator=(const DynamicBuffer& other)
	{
		copy(other, *this);
		return *this;
	}

	DynamicBuffer& operator=(DynamicBuffer&& other)
	{
		byte* dataToDiscard = mData;
		size_t sizeToDiscard = mSize;
		byte* currentPtrToDiscard = mCurrentPtr;
		mData = other.mData;
		mSize = other.mSize;
		mCurrentPtr = other.mCurrentPtr;
		other.mData = dataToDiscard;
		other.mSize = sizeToDiscard;
		other.mCurrentPtr = currentPtrToDiscard;
		return *this;
	}

private:
	byte* mData;
	size_t mSize;
	byte* mCurrentPtr;
};

// Warning: push a size_t before tracking
#define DYNAMIC_BUFFER_TRACK(TrackerName, BufferVar) size_t TrackerName = BufferVar.used()
#define DYNAMIC_BUFFER_TRACK_WRITE_RESULT(TrackerName, BufferVar) *(size_t*)(BufferVar.current() - (BufferVar.used() - TrackerName) - sizeof(size_t)) = BufferVar.used() - TrackerName

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

	inline size_t remaining() { return mBytesLeft; }
	inline bool empty() { return mBytesLeft == 0; }

private:
	byte* mCurrentPtr;
	size_t mBytesLeft;
};