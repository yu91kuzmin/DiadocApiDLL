// DiadocApi.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include <stdexcept>
#include <iostream> 
#include <fstream>
#include <string>
#include <stdio.h>
#include "curl/curl.h"
#include <cstddef> 
#include "DiadocApi.h"
//include generated proto *.h files
#include "Events/DiadocMessage.PostApi.pb.h"
#include "Events/DiadocMessage.GetApi.pb.h"
#include "Organization.pb.h"
#include "Documents/DocumentList.pb.h"

#pragma comment(lib,"curllib.lib")
#pragma comment(lib,"libprotobuf.lib")
#pragma comment(lib,"libprotobuf-lite.lib")
#pragma comment(lib,"libprotoc.lib")

using namespace std;
using namespace Diadoc::Api::Proto::Events;
using namespace Diadoc::Api::Proto::Documents;
using namespace Diadoc::Api::Proto;

static char errorBuffer[CURL_ERROR_SIZE];
static string buffer;


char* _stdcall DiadocFindAndSaveDocuments(char *DiadocAuthHeader, char *DiadocBoxId, char *DiadocCountDocuments, char *DatBegin, char *DatEnd, char *FilterCategory)
{
	CURL *curl;
	struct curl_slist *headerlist=NULL;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	curl = curl_easy_init();
	string DiadocFindDocumentURL = "https://diadoc-api.kontur.ru/V3/GetDocuments?boxid=";
	DiadocFindDocumentURL += DiadocBoxId;
	DiadocFindDocumentURL += "&filterCategory=";
	DiadocFindDocumentURL += FilterCategory;
	DiadocFindDocumentURL += "&fromDocumentDate=";
	DiadocFindDocumentURL += DatBegin;
	DiadocFindDocumentURL += "&toDocumentDate=";
	DiadocFindDocumentURL += DatEnd;
	if(curl)
		{ 
			headerlist = curl_slist_append(headerlist, "POST https://diadoc-api.kontur.ru/Authenticate HTTP/1.1");
			headerlist = curl_slist_append(headerlist, DiadocAuthHeader);
			curl_easy_setopt(curl, CURLOPT_URL, DiadocFindDocumentURL);
			string execute_result = execute_curl(curl, headerlist);					 
			DocumentList DiadocDocumentsList;
			Document DiadocDocument;
			string DiadocFileName, DiadocMessageId, DiadocEntityId;
			DiadocDocumentsList.ParseFromString(execute_result);
			if (DiadocDocumentsList.IsInitialized() != 0)
			{
				if (DiadocDocumentsList.documents_size() > 0) 
				{
					string PathForXmlFiles = getenv("TEMP");
					PathForXmlFiles += "\\DiadocXmlFiles";
					WCHAR LpPathName[1000];
					swprintf(LpPathName, L"%S", PathForXmlFiles.c_str());
					CreateDirectory(LpPathName, NULL);
				}
				for(int i = 0; i < DiadocDocumentsList.documents_size(); i ++)
				{
					DiadocDocument = DiadocDocumentsList.documents(i);
					DiadocFileName = DiadocDocument.filename();
					DiadocMessageId = DiadocDocument.messageid();
					DiadocEntityId = DiadocDocument.entityid();
					//сохранение файлов
					DiadocGetEntityContent(DiadocAuthHeader, DiadocBoxId, DiadocMessageId, DiadocEntityId, DiadocFileName);
				} 
				if (DiadocDocumentsList.documents_size() == 0)
				{
					DiadocCountDocuments = "Документы для указанного периода не найдены";
					return DiadocCountDocuments;
				}
				else
				{
					DiadocCountDocuments = "1";
					return DiadocCountDocuments;
				}
			}
			else
			{
				const char *tmp_execute = execute_result.c_str();
				const_char_to_char(DiadocCountDocuments, tmp_execute);
				return DiadocCountDocuments;
			}
		}
	curl_easy_cleanup(curl);
}

void DiadocGetEntityContent(char *DiadocAuthHeader, char *DiadocBoxId,string DiadocMessageId, string DiadocEntityId, string DiadocFileName)
{
	CURL *curl;
	struct curl_slist *headerlist=NULL;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	curl = curl_easy_init();
	string UrlGetContent = "https://diadoc-api.kontur.ru/V4/GetEntityContent?boxid=";
	UrlGetContent += DiadocBoxId;
	UrlGetContent += "&messageid=";
	UrlGetContent += DiadocMessageId;
	UrlGetContent += "&entityid=";
	UrlGetContent += DiadocEntityId;
	if(curl)
		{ 
			headerlist = curl_slist_append(headerlist, "POST https://diadoc-api.kontur.ru/Authenticate HTTP/1.1");
			headerlist = curl_slist_append(headerlist, DiadocAuthHeader);
			curl_easy_setopt(curl, CURLOPT_URL, UrlGetContent);
			string execute_result = execute_curl(curl, headerlist);
			string PathForSaveXmlFile = getenv("TEMP");
			PathForSaveXmlFile += "\\DiadocXmlFiles\\";
			PathForSaveXmlFile += DiadocFileName;
			ofstream DiadocXmlFile(PathForSaveXmlFile.c_str(), ios::out | ios::trunc);
			for (int i = 0; i < execute_result.length(); i++)
			{
				if (execute_result[i] == '\r' && execute_result[i+1]== '\n')
				{
					execute_result.replace(i, 1, "");
				}
			}
			DiadocXmlFile << execute_result;	
		}
	curl_easy_cleanup(curl);
}

char* _stdcall DiadocGetMyOrganizations(char *DiadocAuthHeader, char *DiadocBoxId, char *FnsParticipantId)
{
	CURL *curl;
	struct curl_slist *headerlist=NULL;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	curl = curl_easy_init();
	if(curl)
		{ 
			headerlist = curl_slist_append(headerlist, "POST https://diadoc-api.kontur.ru/Authenticate HTTP/1.1");
			headerlist = curl_slist_append(headerlist, DiadocAuthHeader);
			curl_easy_setopt(curl, CURLOPT_URL, "https://diadoc-api.kontur.ru/GetMyOrganizations?outputFormat=protobuf");
			string execute_result = execute_curl(curl, headerlist);		
			OrganizationList DiadocOrgList;	
			Organization DiadocOrg;
			Box DiadocBoxes;
			DiadocOrgList.ParseFromString(execute_result);
			char char_DiadocFnsParticipantId[46]; //размер boxId = 42
			int IsFind = 0; //устанавливается в "1", если равны FnsParticipantId 
			for(int i = 0; i < DiadocOrgList.organizations_size(); i++)
			{
				DiadocOrg = DiadocOrgList.organizations(i);
				string DiadocFnsParticipantId = DiadocOrg.fnsparticipantid();
	 			const char *tmp_DiadocFnsParticipantId = DiadocFnsParticipantId.c_str();
				const_char_to_char(char_DiadocFnsParticipantId, tmp_DiadocFnsParticipantId);
				if (strcmp(char_DiadocFnsParticipantId, FnsParticipantId) == 0)
				{
					const char *tmp_DiadocBoxId = DiadocOrg.boxes(0).boxid().c_str();
					const_char_to_char(DiadocBoxId, tmp_DiadocBoxId);
					IsFind = 1;
					break;
				}
			}
			if (IsFind != 1 &&  DiadocOrgList.organizations_size() != 0)
			{
				DiadocBoxId = "NotFinded";
			}
			if (DiadocOrgList.organizations_size() == 0)
			{
				const char *tmp_execute = execute_result.c_str();
				const_char_to_char(DiadocBoxId, tmp_execute);
			}
			return DiadocBoxId;
		}
	curl_easy_cleanup(curl);
}

char* _stdcall DiadocSendMessage(char *DiadocAuthHeader, char *DiadocXMLFileName, char *DiadocBoxId, char *DiadocXMLRem, char *ReturnRes)
{
	ifstream XMLFile(DiadocXMLFileName, ios::binary );
	string XMLFileString = "";
	char XMLFileChar;
	for(int i = 0; XMLFile.eof() != true; i++){	
		XMLFileChar = XMLFile.get();
		if ( XMLFile.eof() == true) break;
		XMLFileString += XMLFileChar;
	}
	XMLFile.close();
	MessageToPost DiadocMessage;
	{
		DiadocMessage.set_fromboxid(DiadocBoxId);
		//черновик=========================
		DiadocMessage.set_toboxid("");
		DiadocMessage.set_isdraft(1); 
		//черновик=========================	
	}
	XmlDocumentAttachment *DiadocXMLDoc = DiadocMessage.add_invoices();
		SignedContent content;
		SignedContent *tmp_content = &content;
		tmp_content->set_content(XMLFileString.c_str());
		tmp_content->set_signwithtestsignature(true);
		tmp_content->set_signbyattorney(false);
		DiadocXMLDoc->set_allocated_signedcontent(tmp_content);
		DiadocXMLDoc->set_comment(DiadocXMLRem);
	string DiadocMessageToPost;
	DiadocMessage.SerializeToString(&DiadocMessageToPost);
	char hhh[] = "https://diadoc-api.kontur.ru//V3/PostMessage";
	CURL *curl;
	struct curl_slist *headerlist=NULL;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	curl = curl_easy_init();
	if(curl)
		{ 
			headerlist = curl_slist_append(headerlist, "POST https://diadoc-api.kontur.ru/Authenticate HTTP/1.1");
			headerlist = curl_slist_append(headerlist, DiadocAuthHeader);
			curl_easy_setopt(curl, CURLOPT_POST, true);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, DiadocMessageToPost.c_str() );
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, DiadocMessageToPost.length() );
			curl_easy_setopt(curl, CURLOPT_URL, hhh);
			string execute_result = execute_curl(curl, headerlist);
			Message DiadocGetMessage;
			DiadocGetMessage.ParseFromString(execute_result);
			if(DiadocGetMessage.messageid() == "")
			{
				const char *tmp_execute = execute_result.c_str();
				const_char_to_char(ReturnRes, tmp_execute);
				return ReturnRes;
			}
				else
				{
					ReturnRes = "1";
					return ReturnRes;
				}
								
		}
	curl_easy_cleanup(curl); 
}

char* _stdcall DiadocAuth(char *DiadocAuthURL)
{
	CURL *curl;
	struct curl_slist *headerlist=NULL;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	curl = curl_easy_init();
	if(curl)
		{   
			headerlist = curl_slist_append(headerlist, "POST https://diadoc-api.kontur.ru/Authenticate HTTP/1.1");
			headerlist = curl_slist_append(headerlist, "Authorization: DiadocAuth ddauth_api_client_id=comtec-e628f938-1133-449a-96d4-a105a0181e9a");
			curl_easy_setopt(curl, CURLOPT_POST, true);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
			curl_easy_setopt(curl, CURLOPT_URL, DiadocAuthURL);
			string execute_result = execute_curl(curl, headerlist);
			const char *tmp_execute = execute_result.c_str();
			const_char_to_char(DiadocAuthURL, tmp_execute);
		}
	curl_easy_cleanup(curl);
	return DiadocAuthURL;	
}

string execute_curl(CURL *curl, struct curl_slist *headerlist)
{
	string request;
	CURLcode res;
	headerlist = curl_slist_append(headerlist, "Connection: Keep-Alive");
	headerlist = curl_slist_append(headerlist, "Host: diadoc-api.kontur.ru");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	//вывести подробный ответ от сервера
	curl_easy_setopt(curl, CURLOPT_VERBOSE, true);
	curl_easy_setopt(curl, CURLOPT_HEADER, true);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
	res = curl_easy_perform(curl);
	if (res == CURLE_OK) {      
		int k;
		for (int i = int(buffer.length()); i > 0; i--) 
		{
			if (buffer[i-1] == '\n' && buffer[i-2] == '\r' && buffer[i-3] == '\n') 
			{
				k = i;
				break;
			}
		}
		for (k; k < int(buffer.length()); k++) 
		{
			request += buffer[k];
		}
	}
	else  
	{
		request = errorBuffer; 
	}
	return request;
}

static int writer(char *data, size_t size, size_t nmemb, string *buffer)
{
	//переменная - результат, по умолчанию нулевая
	int result = 0;
	//проверяем буфер
	if (buffer != NULL)
	{
	//добавляем к буферу строки из data, в количестве nmemb
	buffer->append(data, size * nmemb);
	//вычисляем объем принятых данных
	result = size * nmemb;
	}
	//вовзращаем результат
	return result;
}

void const_char_to_char(char *str1, const char *str2)
{
	int str2_len = strlen(str2); 
	for(int i = 0; i < str2_len; i++)
	{
		str1[i] = str2[i];
	}
	str1[str2_len] = '\0';
}