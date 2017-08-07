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

#include "cbase.h"
#include "ge_webrequest.h"

#pragma warning( disable : 4005 )
#include "curl.h"
#pragma warning( default : 4005 )

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CGEWebRequest::CGEWebRequest( const char *addr, WEBRQST_CLBK callback /*=NULL*/, const char *methodData /*=NULL*/, const char *internalData /*=NULL*/ )
{	
	m_pAddress = new char[Q_strlen(addr)+1];
	Q_strcpy( m_pAddress, addr );

	m_pError = new char[CURL_ERROR_SIZE+1];
	m_pError[0] = '\0';

	if (methodData)
	{
		m_pMethodData = new char[Q_strlen(methodData) + 1];
		Q_strcpy(m_pMethodData, methodData);
	}
	else
		m_pMethodData = NULL;

	if (internalData)
	{
		m_pInternalData = new char[Q_strlen(internalData) + 1];
		Q_strcpy(m_pInternalData, internalData);
	}
	else
		m_pInternalData = NULL;

	m_bFinished = false;

	m_pCallback = callback;

	// Start the thread
	SetName( "GEWebRequest" );
	Start();
}

CGEWebRequest::~CGEWebRequest()
{
	// Stop the thread
	if ( IsAlive() )
	{
		Terminate();
		Join();
	}

	// Clear the buffers
	delete [] m_pAddress;
	delete [] m_pError;
	delete [] m_pInternalData;
	delete [] m_pMethodData;

	m_Result.Purge();
}

void CGEWebRequest::Destroy()
{
	delete this;
}

size_t CGEWebRequest::WriteData( void *ptr, size_t size, size_t nmemb, void *userdata )
{
	CUtlBuffer *result = (CUtlBuffer*) userdata;
	result->PutString( (char*)ptr );
	return size * nmemb;
}

int CGEWebRequest::Run()
{
	CURL *curl;
	CURLcode res;
	//struct curl_slist *headerList = NULL;


	curl = curl_easy_init();
	if ( curl )
	{
		char *cleanMethodData = NULL;
		char *urlToUse = m_pAddress;
		char addressBuffer[256];

		if (m_pMethodData)
		{
			char *cleanMethodData = curl_easy_escape(curl, m_pMethodData, Q_strlen(m_pMethodData));
			Q_snprintf(addressBuffer, sizeof(addressBuffer), "%s%s", m_pAddress, cleanMethodData);

			urlToUse = addressBuffer;
		}

		//Warning("Final URL of %s\n", urlToUse);

		curl_easy_setopt(curl, CURLOPT_URL, urlToUse);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &m_Result);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &CGEWebRequest::WriteData);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, m_pError);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
		
		res = curl_easy_perform( curl );
		/*
		if (res != CURLE_OK ) //&& res != CURLE_HTTP_RETURNED_ERROR
		{
			//Q_strncpy( m_pError, curl_easy_strerror(res), CURL_ERROR_SIZE );
			Warning( "Threw curl error of %s on request to %s\n", m_pError, m_pAddress);
		}*/

		curl_free( cleanMethodData );
		curl_easy_cleanup( curl );
	}
	else
	{
		Q_strncpy( m_pError, "Failed to start CURL", CURL_ERROR_SIZE );
	}

	m_bFinished = true;
	return 0;
}

void CGEWebRequest::OnExit()
{
	if ( m_pCallback )
		m_pCallback( (char*) m_Result.Base(), m_pError, m_pInternalData );
}


// ----------------------TempWebRequest----------------------
// For the web requests that should retire right after getting their information to us.

CGETempWebRequest::CGETempWebRequest(const char *addr, WEBRQST_CLBK callback /*=NULL*/, const char *internalData /*=NULL*/, const char *methodData /*=NULL*/):CGEWebRequest( addr, callback, methodData, internalData)
{
}

void CGETempWebRequest::OnExit()
{
	CGEWebRequest::OnExit();
	delete this;
}



// ----------------------PostWebRequest----------------------
// For the web requests that post information rather than query it.

CGEPostWebRequest::CGEPostWebRequest( const char *addr, const char *postData ) : CGEWebRequest( addr, NULL )
{
	if (postData)
	{
		m_pPostData = new char[Q_strlen(postData) + 1];
		Q_strcpy(m_pPostData, postData);
	}
	else
		m_pPostData = NULL;
}

CGEPostWebRequest::~CGEPostWebRequest()
{
	delete [] m_pPostData;
}

int CGEPostWebRequest::Run()
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if ( curl )
	{
		char *proccessedData = NULL;
		char messageBuffer[512];

		const char *encryptedData = m_pPostData;

		proccessedData = curl_easy_escape(curl, encryptedData, Q_strlen(encryptedData));
		Q_snprintf( messageBuffer, sizeof(messageBuffer), "data: %s", proccessedData );

		struct curl_slist *sDataList=NULL;
		sDataList = curl_slist_append(sDataList, messageBuffer);

		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, sDataList);
		curl_easy_setopt(curl, CURLOPT_URL, m_pAddress);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, m_pError ); 
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

		res = curl_easy_perform( curl );
		/*if (res != CURLE_OK ) //&& res != CURLE_HTTP_RETURNED_ERROR
		{
			Warning( "Threw curl error of %s on request to %s\n", m_pError, m_pAddress);
		}*/

		curl_free( proccessedData );
		curl_easy_cleanup( curl );
	}
	else
	{
		Q_strncpy( m_pError, "Failed to start CURL", CURL_ERROR_SIZE );
	}

	m_bFinished = true;
	return 0;
}

void CGEPostWebRequest::OnExit()
{
	delete this; // We've posted our data, nothing more for us to do.
}