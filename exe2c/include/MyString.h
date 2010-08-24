// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//	CMyString.h
#ifndef	CMyString_H
#define	CMyString_H

const int MEMORY_STEP	= 2048;

class MyString
{
	size_t	m_maxlen;
	size_t	m_curlen;
	char *	m_bufptr;
public:
	MyString();
	~MyString();

	void strcat(const char *str);
	void Clear();
	size_t	GetLength();
	const char *	GetString();
	char *	GetWritableString();
};

#endif	//	CMyString_H
