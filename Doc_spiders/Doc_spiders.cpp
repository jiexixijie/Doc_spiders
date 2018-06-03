// Doc_spiders.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <conio.h>  
#include "spider.h"

#pragma comment(lib,"ws2_32.lib")

extern std::vector<URL> Ucatch;
extern std::set<URL> Catched;
extern std::vector<std::string> Limit_Host;
extern std::set<Info> MayBe;
extern std::vector<std::string> Search;

void Add_Search_Info(const std::string(&limit)[MAX_LENGTH], const std::string(&Keyword)[MAX_LENGTH]);
void Catch(Spider spider,const char *filename);
bool Want_exit();

bool want_exit = FALSE;

int _tmain(int argc, _TCHAR* argv[])
{
	//add in it 
	std::string limit[MAX_LENGTH] = { "jobbole" };
	std::string Keyword[MAX_LENGTH] = { "Java" };
	Add_Search_Info(limit, Keyword);
	//Limit_Host.push_back(limit);

	WSADATA wsdata;
	WSAStartup(0x0202, &wsdata);
	//http://zh.wikipedia.org:80/wiki/Special:Search?search=铁路&go=Go
	char url[MAX_URL_LENGTH + 1] = "http://blog.jobbole.com/70907/";
	URL Url;
	Url = Parse_url(url);
	Ucatch.push_back(Url);
	//Only One
	char filename[] = ".temp/spdier1.txt";
	Spider sp1;
	Catch(sp1, filename);


	WSACleanup();
	std::set<Info>::iterator iter = MayBe.begin();

	FILE *final = NULL;
	fopen_s(&final, "result.txt", "w+");
	if (final == NULL){
		printf("write result.txt error\n");
		return 0;
	}
	printf("MAYBE\n");
	for (iter = MayBe.begin(); iter != MayBe.end(); iter++){

		char temp[MAX_LENGTH] ;
		sprintf_s(temp, "%s://%s:%s:%s\n", (*iter).Url.protocol, (*iter).Url.host, (*iter).Url.path, (*iter).Content.c_str());
		fwrite(temp, strlen(temp), 1, final);
		printf("%s://%s:%s:%s\n", (*iter).Url.protocol, (*iter).Url.host, (*iter).Url.path, (*iter).Content.c_str());
	}
	fclose(final);

	system("pause");
	return 0;
}
void Catch(Spider spider,const char *filename){
	ULONG counter = 0;
	//URL Url = *Ucatch.begin();
	//Ucatch.erase(Ucatch.begin());
	while (!Ucatch.empty() && counter < 100){
		size_t size = Ucatch.size();
		std::vector<URL>::iterator iter = Ucatch.begin() + rand() % size;
		URL Url = *(iter);
		Ucatch.erase(iter);
		if (Is_Catched(Url)){
			continue;
		}
		printf("Catching %s://%s:%s\n", Url.protocol, Url.host, Url.path);
		spider.Url = Url;
		if (spider.conn2Peer(Url.host, Url.port) == SOCKET_ERROR){
			printf("connect %s://%s:%s error\n", Url.protocol, Url.host, Url.path);
			continue;
		}
		if (spider.Get_Html(Url, filename)<=0){
			printf("fail to get %s://%s:%s\n",Url.protocol, Url.host, Url.path);
			continue;
		}
		spider.Parse_Html(filename);
		Catched.insert(Url);
		counter++;
		if (counter > 50){
			int a = 100;
		}
		//break;
	}
	printf("has catched %d urls\n", counter);
}

void Add_Search_Info(const std::string(&limit)[MAX_LENGTH], const std::string(&Keyword)[MAX_LENGTH]){
	//Limit_Host
	for (int i = 0; i < MAX_LENGTH; i++){
		if (limit[i].empty()){
			break;
		}
		Limit_Host.push_back(limit[i]);
	}
	for (int i = 0; i < MAX_LENGTH; i++){
		if (Keyword[i].empty()){
			break;
		}
		Search.push_back(Keyword[i]);
	}
	std::string dirName = ".temp";
	if (_access(dirName.c_str(), 0) == -1)
	{
		int i = _mkdir(dirName.c_str());
	}
}

bool Want_exit(){
	if (_kbhit())  //键盘存在输入
	{
		char key = _getch();
		if (key == 'E' || key == 'e')
		{
			return true;
		}
		else
		{
			printf("Input E to exit now!\r\n");
		}
	}
	return false;
}
