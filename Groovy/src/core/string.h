#pragma once
#include <string>

class String
{
public:
	inline String()
		: mStr()
	{}

	inline String(const char* str)
		: mStr(str)
	{}

	inline String(const String& other)
		: mStr(other.mStr)
	{}

	inline String(String&& other)
		: mStr(std::move(other.mStr))
	{}

	inline const char* data() const { return mStr.c_str(); }
	inline char* data() { return mStr._Unchecked_begin(); }
	inline size_t length() const { return mStr.length(); }
	inline size_t size() const { return mStr.size(); }
	inline bool empty() const { return mStr.length() == 0; }
	
	inline String& append(const char* str)
	{
		mStr.append(str);
		return *this;
	}

	inline String& append(const String& other)
	{
		mStr.append(other.mStr);
		return *this;
	}

	inline String& append(const char* str, size_t count)
	{
		mStr.append(str, count);
		return *this;
	}

	inline void resize(size_t size, char c = 0)
	{
		mStr.resize(size, c);
	}

	inline const char* operator*() const { return mStr.c_str(); }

	inline const char operator[](size_t index) const { return mStr[index]; }
	inline char& operator[](size_t index) { return mStr[index]; }

	inline String& operator+=(const char* str) { return append(str); }
	inline String& operator+=(const String& other) { return append(other); }

	String operator+(const char* str)
	{
		String res;
		res.append(data());
		res.append(str);
		return res;
	}

	String operator+(const String& other)
	{
		String res;
		res.append(data());
		res.append(other);
		return res;
	}

	inline String& operator=(const char* str)
	{
		mStr = str;
		return *this;
	}

	inline String& operator=(const String& other)
	{
		mStr = other.mStr;
		return *this;
	}

	inline String& operator=(String&& other)
	{
		mStr = std::move(other.mStr);
		return *this;
	}

private:
	std::string mStr;
};