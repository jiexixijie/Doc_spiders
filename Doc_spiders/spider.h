#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <Windows.h>
#include <set>
#include <vector>
#include <map>
#include <algorithm>

#define MAX_URL_LENGTH (8192-1)
#define MAX_LENGTH 8192



//std::set<URL>::iterator iqwe = MayBe.begin();

struct URL
{
	char protocol[100];
	char host[200];
	ULONG port;
	char path[MAX_URL_LENGTH];
	void operator = (const URL &u);
	bool operator < (const URL &u) const;
	bool operator == (const URL &u) const;
};

struct Info
{
	URL Url;
	std::string Content;
	bool operator < (const Info &info) const;
};



URL Parse_url(char *herf);
void ConvertUtf8ToGBK(char *buffer, ULONG len);
void ConvertGBKToUtf8(char *buffer, ULONG len);


class Spider
{
public:
	Spider();
	~Spider();
	//just connect
	SOCKET conn2Peer(ULONG Ipv, int Port);
	SOCKET conn2Peer(const char *Host,int Port); 
	int Get_Html(const URL url,const char *filename);
	int Parse_Html(const char *filename);
	void Print_Content(const Info info);
	URL Url;

private:
	SOCKET fd;
	SOCKADDR_IN addr;
	char url[MAX_LENGTH] ;
	char content[MAX_LENGTH];
	std::map<std::string, std::string> Res_Headers;
	//return response
	int Get_Info(char *buffer, ULONG flen);
	void Start_Parse(char *p);
	//防止url越界爬取其他
	//in the limit host
	bool Is_Limit_Host(const char *host);
	//search right content
	bool Is_Want_Con(const char *content);
};

//is catched url
bool Is_Catched(const URL Url);