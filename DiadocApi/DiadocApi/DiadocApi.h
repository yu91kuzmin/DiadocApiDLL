#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA
#ifndef __DIADOCAPI_H
#define __TDIADOCAPI_H
  
#ifdef DIADOCAPI_EXPORTS
#define DllExport  __declspec(dllexport) 
#else
#define DllExport  __declspec(dllimport)
#endif 

using namespace std;
//получение токена 
DllExport char* _stdcall DiadocAuth(char *DiadocAuthURL);
DllExport char* _stdcall DiadocSendMessage(char *DiadocAuthHeader, char *DiadocXMLFileName, char *DiadocBoxId, char *DiadocXMLRem);
DllExport char* _stdcall DiadocGetMyOrganizations(char *DiadocAuthHeader, char *DiadocBoxId, char *FnsParticipantId);
DllExport char* _stdcall DiadocFindAndSaveDocuments(char *DiadocAuthHeader, char *DiadocBoxId, char *DiadocCountDocuments, char *DatBegin, char *DatEnd, char *FilterCategory);

//выполнение HTTPS запроса
string execute_curl(CURL *curl, struct curl_slist *headerlist);

//получение ответа на запрос
static int writer(char *data, size_t size, size_t nmemb, string *buffer);

//сохранение XML файлов
void DiadocGetEntityContent(char *DiadocAuthHeader, char *DiadocBoxId,string DiadocMessageId, string DiadocEntityId, string DiadocFileName);

void const_char_to_char(char *str1, const char *str2);


#endif 
#undef AFX_DATA
#define AFX_DATA