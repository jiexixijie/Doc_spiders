#include "stdafx.h"
#include "spider.h"
#include <assert.h>
#include <fstream>


std::vector<URL> Ucatch;
std::set<URL> Catched;
std::vector<std::string> Limit_Host;
std::set<Info> MayBe;
std::vector<std::string> Search;
std::mutex Signal;


enum Res_State{
	Res_Code,Res_Type
}RState;
enum Parse_State{
	Init=2, Content, Tag, Start_Tag_Head, Start_Tag_Tail, End_Tag_Head, End_Tag_Tail, Doc, Before_Attri, Attri, Head_Value, Value,To_Tail, Parse_Error
};
Parse_State State = Init;

bool Is_Catched(const URL Url){
	std::set<URL>::iterator iter;
	for (iter = Catched.begin(); iter != Catched.end(); iter++){
		if (Url == (*iter)){
			return TRUE;
		}
	}
	//Catched.insert(Url);
	return FALSE;
}

URL Parse_url(char *herf)
{
	URL url;
	memset(&url, 0, sizeof(URL));
	url.port = 80;
	sprintf_s(url.path, "/");
	char *p, *pp;
	p = pp = herf;
	while (*p != '\0'){
		if (*p == '/'){
			if (!*url.protocol){
				if (*(p + 1) == '/'){
					strncpy_s(url.protocol, pp, p - pp - 1); //PROTOCOL
					url.protocol[p - pp - 1] = '\0';
					p++;
					pp = p + 1;
				}
				else{
					sprintf_s(url.protocol, "%s", "http");
				}
			}
			else if (!*url.host){
				strncpy_s(url.host, pp, p - pp);
				url.host[p - pp] = '\0';  //host+port
				pp = p;
			}
		}
		p++;
	}
	strncpy_s(url.path, pp, p - pp);  //PATH
	url.path[p - pp] = '\0';
	p = pp = url.host;
	while (*p != ':'&& *p != '\0'){
		p++;
	}
	if (*p != '\0'){
		char Port[100];
		memset(Port, 0, 100);
		strncpy_s(Port, p + 1, strlen(url.host) - 1 - (p - url.host));
		url.port = atol(Port);
		*p = '\0';
		if (url.port == 443){
			url.port = 80;
		}
	}
	if (url.port == 0){
		int a=0;
	}
	return url;
}

void URL::operator=(const URL &u) {
	memcpy_s(this, sizeof(URL), &u, sizeof(URL));
	int a = 0;
}

bool URL::operator<(const URL &u) const{
	if (strcmp(this->host, u.host) < 0){
		return TRUE;
	}
	else if (strcmp(this->host, u.host) == 0){
		if (strcmp(this->path, u.path) < 0){
			return TRUE;
		}
	}
	return FALSE;
}

bool URL::operator==(const URL &u) const{
	if (strcmp(this->host, u.host) || strcmp(this->path, u.path) || (this->port != u.port)){
		return FALSE;
	}
	return TRUE;
}

bool Info::operator<(const Info &info) const{
	if (strcmp(this->Url.host, info.Url.host) < 0){
		return TRUE;
	}
	else if (strcmp(this->Url.host, info.Url.host) == 0){
		if (strcmp(this->Url.path, info.Url.path) < 0){
			return TRUE;
		}
	}
	return FALSE;
}

void ConvertUtf8ToGBK(char *buffer,ULONG len){
	ULONG wlen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buffer, -1, NULL, 0);
	wchar_t *wszGBK = new wchar_t[len];
	memset(wszGBK, 0, len*sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)buffer, -1, wszGBK, len);
	delete[len] buffer;
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL)+1;
	buffer = new char[len];
	memset(buffer, 0, len);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, buffer, len - 1, NULL, NULL);
	delete[] wszGBK;
}

void ConvertGBKToUtf8(char *buffer, ULONG len){
	ULONG wlen = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buffer, -1, NULL, 0);
	wchar_t *wsUtf8 = new wchar_t[len];
	memset(wsUtf8, 0, len*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)buffer, -1, wsUtf8, len);
	delete[len] buffer;
	len = WideCharToMultiByte(CP_UTF8, 0, wsUtf8, -1, NULL, 0, NULL, NULL) + 1;
	buffer = new char[len];
	memset(buffer, 0, len);
	WideCharToMultiByte(CP_UTF8, 0, wsUtf8, -1, buffer, len - 1, NULL, NULL);
	delete[] wsUtf8;
}

Spider::Spider()
{
	memset(&Url, 0, sizeof(URL));
}

Spider::~Spider()
{
	if (fd){
		closesocket(fd);
	}
}

SOCKET Spider::conn2Peer(ULONG Ipv, int Port){

	fd = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = 0;
	addr.sin_port = 0;
	int i = 0;
	i = bind(fd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
	assert(i != SOCKET_ERROR);  //出错中止

	addr.sin_addr.S_un.S_addr = Ipv;
	addr.sin_port = htons(Port);
	u_long opt = 1;
	::ioctlsocket(fd, FIONBIO, &opt); //opt为非零 设置socket为非阻塞模式
	i = connect(fd, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
	//如果没有立刻连接上
	if (i != NOERROR){
		//超时10s自动断开
		timeval tval = { 10, 0 };
		FD_SET foo;
		//清0,集合为空
		FD_ZERO(&foo); 
		FD_SET(fd, &foo);
		//非阻塞模式中在tval内一旦有信号进行通知,将无信号的从集合中清零
		i = select(1,NULL,&foo,NULL,&tval);  
		//结束  0:超时 -1:出错从集合清零
		if (i == 1)  {
			//如果存在集合中则connet success
			if (FD_ISSET(fd, &foo)) {
				i = NOERROR;
			}
			else{
				i = SOCKET_ERROR;
			}
		}
	}
	if (i != NOERROR)
	{
		//connect fail
		closesocket(fd);
		fd = SOCKET_ERROR;
	}
	opt = 0;
	::ioctlsocket(fd, FIONBIO, &opt);    //设为阻塞模式
	DWORD rtimer = 1000 * 10;
	//设置接收超时
	::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,(const char*)&rtimer,sizeof(DWORD)); 
	return fd;

}

SOCKET Spider::conn2Peer(const char *Host, int Port){
	hostent *p = gethostbyname(Host);
	if (!p){
		printf("Failed to resolve [%s] error number:%d\r\n", Host, WSAGetLastError());
		return SOCKET_ERROR;
	}
	ULONG Ipv = *(ULONG*)p->h_addr_list[0];
	return conn2Peer(Ipv, Port);
}

int Spider::Get_Html(const URL url,const char *filename){
	char html[MAX_LENGTH];
	char buffer[8192];
	int i;
	int length = 0;
	if (url.port != 80){
		i = sprintf_s(buffer, "GET %s HTTP/1.1\r\n"
			"Host: %s:%d\r\n"
			"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko\r\n"
			"Accept-Type: */*\r\n"
			"Connection: close\r\n"
			"\r\n", url.path, url.host, url.port);
	}
	else{
		i = sprintf_s(buffer, "GET %s HTTP/1.1\r\n"
			"Host: %s\r\n"
			"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko\r\n"
			"Accept-Type: */*\r\n"
			"Connection: close\r\n"
			"\r\n", url.path, url.host);
	}
	i = send(fd, buffer, i, 0);
	if (i < 0){
		//printf("send error\n");
		return 0;
	}
	Url = url;
	FILE *fp = NULL;
	fopen_s(&fp, filename, "wb+");
	if (NULL == fp)
	{

		printf("error");
		return -1;

	}
	do{
		i = recv(fd, html, MAX_LENGTH - 1, 0);
		if (i<0){
			printf("recv error ");
			fclose(fp);
			return -1;
		}
		html[i] = '\0';
		length += i;
		fwrite(html, i, 1, fp);
		memset(html, 0, MAX_LENGTH);
	} while (i>0);
	fclose(fp);
	return 1;
}

int Spider::Get_Info(char *buffer,ULONG flen){
	//HTTP/1.1 200 OK
	//get response
	/*char res[4] = "0";
	strncpy_s(res, buffer + 9, 3);*/
	char *p, *pp;
	std::string Type, Code;
	p = pp = buffer;
	RState = Res_Code;
	while (*p != '<' && *p != '\0'){
		switch (RState)
		{
		case Res_Code:
			if (*p == '\r'&&*(p + 1) == '\n'){
				char temp[4];
				strncpy_s(temp, pp + 9, 3);
				Type = "HTTP";
				Code = temp;
				Res_Headers.insert(make_pair(Type, Code));
				pp = p + 2;
				RState = Res_Type;
			}
			break;
		case Res_Type:
			//Content-Type: text/html
			if (*p == ':'&&*(p + 1) == ' '){
				char *temp = new char[p - pp + 1];
				memcpy(temp, pp, p - pp);
				temp[p - pp] = '\0';
				Type = temp;
				pp = p + 2;
			}
			else if (*p == '\r'&&*(p + 1) == '\n'){
				char *temp = new char[p - pp + 1];
				memcpy(temp, pp, p - pp);
				temp[p - pp] = '\0';
				Code = temp;
				pp = p + 2;
				Res_Headers.insert(make_pair(Type, Code));
				if (*pp == '\r'&&*(pp + 1) == '\n'){
					//end:/r/n/r/n
					p = pp + 2;
					break;
				}
			}
			break;
		default:
			break;
		}
		p++;
	}
	//pdf,mp3,xls ....
	if (Res_Headers["Content-Type"].find("application") != std::string::npos){
		printf("This Html maybe pdf/mp3/xls...\n");
		return -2;
	}
	if (Res_Headers["Content-Encoding"] != "gzip"){
		ConvertUtf8ToGBK(buffer, flen + 1);
	}
	//判断大小是否超过100K
	if (atoi(Res_Headers["Content-Length"].c_str()) > 100 * 1024 * 1024){
		printf("Can't receive so large Html!\n");
		return -1;
	}
	return atoi(Res_Headers["HTTP"].c_str());
	//默认为UTF-8
}

bool Spider::Is_Limit_Host(const char *host){
	//为空任意爬取
	if (Limit_Host.empty()){
		return FALSE;
	}
	std::string temp = host;
	std::vector<std::string>::iterator result;
	for (result = Limit_Host.begin(); result != Limit_Host.end(); result++){
		if (temp.find(*result) != std::string::npos){
			return TRUE;
		}
	}
	return FALSE;
}

bool Spider::Is_Want_Con(const char *content){
	//为空表示ALL
	std::string temp = content;
	for (std::vector<std::string>::iterator iter = Search.begin(); iter != Search.end(); iter++){
		if (temp.find(*iter) != std::string::npos){
			return TRUE;
		}
	}
	return FALSE;
}

void Spider::Start_Parse(char *p){
	memset(url, 0, MAX_LENGTH);
	memset(content, 0, MAX_LENGTH);
	char *pp = p;
	char attri[MAX_LENGTH] = "\0";
	char tag[MAX_LENGTH] = "\0";
	//char *attri,*url;
	State = Init;
	while (*p != '\0'){
		switch (State)
		{
		case Init:
			if (*p == '<'){
				State = Tag;
			}
			else{
				State = Content;
				pp = p;
			}
			break;
		case Content:
			//GET CONTETNT;
			if (*p == '<' ){
				State = Tag;
				if (content[0] == 'H'){
					strncpy_s(content, pp, p - pp);
				}
			}
			break;
		case Tag:
			if (*p == '!'){
				State = Doc;
			}
			else if (*p == '/'){
				State = End_Tag_Head;
			}
			else if ((*p >= 'A' && (*p) <= 'Z') || (*p >= 'a' && (*p) <= 'z') || (*p >= '0' && (*p) <= '9')){
				pp = p;
				State = Start_Tag_Head;
			}
			else{
				State = Content;
			}
			break;
		case Start_Tag_Head:
			if (*p == ' '){
				strncpy_s(tag, pp, p - pp);
				char a[] = "a";
				if (!strcmp(a, tag)){
					State = Before_Attri;
				}
				else{
					State=To_Tail;
				}
			}
			else if (*p == '>'){
				State = Start_Tag_Tail;
			}
			break;
		case Start_Tag_Tail:
			if (*p == '\r' || *p == '\n' || *p == ' '){
				//delete \r\r\n
				//p++;
				//break;
			}
			else if (*p == '<'){
				State = Tag;
			}
			else{
				State = Content;
				pp = p;
			}
			break;
		case End_Tag_Head:
			if (*p == '>'){
				if (content[0] != '\0'){
					if (url[0] != 'h'){
						char temp[MAX_LENGTH];
						strcpy_s(temp, url);
						sprintf_s(url, "%s://%s%s", Url.protocol, Url.host, temp);
					}
					if (content[0] == 'H'){
						//content is null
						content[0] = '\0';
						State = End_Tag_Tail;
						break;
					}
					URL temp = Parse_url(url);
					//std::string w_except[10] = { /*"pdf",*/ "mp3", "xls", "zip", "doc", "docx","jpg"};
					//std::string mytest = url;
					//for (int i = 0; i < 10; i++){
					//	if (w_except[i].empty()){
					//		break;
					//	}
					//	if (mytest.find(w_except[i]) != std::string::npos){
					//		*(temp.protocol) = '\0';
					//		State = End_Tag_Tail;
					//		break;
					//	}
					//}
					Signal.lock();
					//合法host,将url加入待爬取队列
					if (Is_Limit_Host(temp.host)){
						Ucatch.push_back(temp);
						//判断content是否为要搜索的内容
						if (Is_Want_Con(content)){
							//contetn为空
							Info want;
							memset(&want, 0, sizeof(0));
							want.Url = temp;
							want.Content = content;
							Print_Content(want);
							MayBe.insert(want);
						}
					}
					Signal.unlock();
					//printf("%s:%s\n", url,content);
					//add in map
					memset(url, 0, MAX_LENGTH);
					memset(content, 0, MAX_LENGTH);
				}
				State = End_Tag_Tail;
			}
			break;
		case End_Tag_Tail:
			if (*p == '\r' || *p == '\n'){
				//delete \r\r\n
				//p++;
				//break;
			}
			else{
				State = Init;
				p--;
			}
			break;
		case Doc:
			if (*p == '>'){
				State = End_Tag_Tail;
			}
			break;
		case Before_Attri:
			if ((*p >= 'A' && (*p) <= 'Z') || (*p >= 'a' && (*p) <= 'z') || (*p >= '0' && (*p) <= '9')){
				State = Attri;
				pp = p;
			}
			break;
		case Attri:
			if (*p == '='){
				strncpy_s(attri, pp, p - pp);
				char temp[] = "href";
				if (!strcmp(temp, attri)){
					url[0] = 'H';
				}
				memset(attri, 0, p - pp);
				State = Head_Value;
			}
			break;
		case Head_Value:
			if (*p == '\'' || *p == '"'){
				if (url[0] == 'H'){
					pp = p + 1;
				}
				State = Value;
			}
			break;
		case Value:
			if (*p == '"' || *p == '\''){
				if (url[0] == 'H'){
					strncpy_s(url, pp, p - pp);
					content[0] = 'H';
					State = To_Tail;
				}
				else if (*(p + 1) == ' '){ 
					State = Before_Attri;
				}
				else if (*(p + 1) == '/'){
					State = End_Tag_Head;
					p++;
				}
				else if (*(p + 1) == '>'){
					State = Start_Tag_Tail;
				}
			}
			break;
		case To_Tail:
			if (*p == '>'){
				State = Start_Tag_Tail;
			}
			break;
		case Parse_Error:
			*p = '\0';
			break;
		default:
			break;
		}
		p++;
	}
}

int Spider::Parse_Html(const char *filename){
	FILE *fp = NULL;
	fopen_s(&fp, filename, "rb+");
	if (fp == NULL){
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	ULONG flen = ftell(fp);
	if (flen > 1024*100*100)
	{
		fclose(fp);
		return -1;
	}
	char *buffer = new char[flen+1];
	memset(buffer, 0, flen + 1);
	fseek(fp, 0, SEEK_SET);
	fread(buffer, flen, 1, fp);
	fclose(fp);
	if (int res = Get_Info(buffer,flen) > 300){
		//fclose(fp);
		//return res;
		closesocket(fd);
		return res;
	}
	char *p = buffer;
	Start_Parse(p);
	closesocket(fd);
	//std::set<URL>::iterator iter;
	//for (iter = Ucatch.begin(); iter != Ucatch.end(); iter++){
	//	printf("%s://%s:%s\n", (*iter).protocol, (*iter).host, (*iter).path);
	//}
	return 0;
}

void Spider::Print_Content(const Info info){
	int size = 0;
	for (std::set<Info>::iterator iter = MayBe.begin(); iter != MayBe.end(); iter++){
		if ((*iter).Url == info.Url){
			break;
		}
		size++;
	}
	if (size == MayBe.size()){
		printf("MAYBE:%s://%s%s:%s\n", info.Url.protocol,info.Url.host,info.Url.path, info.Content.c_str());
	}
}