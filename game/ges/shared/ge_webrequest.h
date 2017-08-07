///////////// Copyright © 2011, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_webrequest.h
// Description:
//      Initializes a web request in an asynchronous thread calling a defined
//      callback when the load is finished for processing
//
// Created On: 09 Oct 2011
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////
#ifndef GE_WEBREQUEST_H
#define GE_WEBREQUEST_H
#pragma once

#include "threadtools.h"
#include "utlbuffer.h"


// typedef void (*WEBRQST_CLBK)( const char *result, const char *error );

// result is data returned from request, error is the potential error thrown.
// internaldata is information specified by the constructor to return to any callbacks.
typedef void (*WEBRQST_CLBK)( const char *result, const char *error, const char *internalData );

class CGEWebRequest : public CThread
{
public:
	CGEWebRequest( const char *addr, WEBRQST_CLBK = NULL, const char *methodData = NULL, const char *internalData = NULL);
	~CGEWebRequest();

	void Destroy();

	bool IsFinished() { return m_bFinished; }

	const char* GetResult() { return (char*) m_Result.Base(); }
	bool		HadError()	{ return m_pError[0] != '\0'; }
	const char* GetError()	{ return m_pError; }

protected:
	static size_t WriteData( void *ptr, size_t size, size_t nmemb, void *userdata );

	virtual int Run();
	virtual void OnExit();

	bool m_bFinished;

	char *m_pAddress;
	char *m_pError;

	char *m_pMethodData;
	char *m_pInternalData;

	CUtlBuffer m_Result;

	WEBRQST_CLBK m_pCallback;
};


class CGETempWebRequest : public CGEWebRequest 
{
public:
	CGETempWebRequest( const char *addr, WEBRQST_CLBK = NULL, const char *methodData = NULL, const char *internalData = NULL);
	DECLARE_CLASS( CGETempWebRequest, CGEWebRequest );

protected:
	virtual void OnExit();
};


class CGEPostWebRequest : public CGEWebRequest 
{
public:
	CGEPostWebRequest( const char *addr, const char *postData = NULL );
	~CGEPostWebRequest();
	DECLARE_CLASS( CGEPostWebRequest, CGEWebRequest );

protected:
	virtual int Run();
	virtual void OnExit();

	char *m_pPostData;
};

#endif