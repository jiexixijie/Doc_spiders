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
extern std::mutex Signal;

int MAX_Spiders;

void Add_Search_Info(const std::string(&limit)[MAX_LENGTH], const std::string(&Keyword)[MAX_LENGTH]);
void Catch_it(Spider spider, const std::string filename);
bool Want_exit();

bool want_exit = FALSE;

int _tmain(int argc, _TCHAR* argv[])
{
	//add in it 
	MAX_Spiders = 1;
	std::string limit[MAX_LENGTH] = { "njupt" };
	std::string Keyword[MAX_LENGTH] = { "教育" };
	//add info
	Add_Search_Info(limit, Keyword);
	WSADATA wsdata;
	WSAStartup(0x0202, &wsdata);
	//http://zh.wikipedia.org:80/wiki/Special:Search?search=铁路&go=Go
	//http://www.njupt.edu.cn/
	char url[MAX_URL_LENGTH + 1] = "http://www.njupt.edu.cn/29/list.htm";
	URL Url;
	Url = Parse_url(url);
	Ucatch.push_back(Url);
	std::string filename = ".temp/spdier1.txt";
	std::thread *threads = new std::thread[MAX_Spiders];

	for (int i = 0; i < MAX_Spiders; i++){
		Spider sp1;
		char temp[50];
		sprintf_s(temp, ".temp/spdier%d.txt", (i + 1));
		filename = temp;
		threads[i] = std::thread(Catch_it, sp1, filename);
		threads[i].detach();
	}
	while (!want_exit)
	{
		Sleep(100);
	}
	//Only One


	//Spider sp1;
	//Catch_it(sp1, filename);

	WSACleanup();
	std::set<Info>::iterator iter = MayBe.begin();

	FILE *final = NULL;
	fopen_s(&final, "result.txt", "w+");
	if (final == NULL){
		printf("write result.txt error\n");
		return 0;
	}
	printf("ALL THE MAYBE HERF:\n");
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
void Catch_it(Spider spider, const std::string filename){
	ULONG counter = 0;
	//URL Url = *Ucatch.begin();
	//Ucatch.erase(Ucatch.begin());
	//Wait more
	while(Ucatch.empty()){
		Sleep(1000);
	}
	while (!Ucatch.empty() && counter < 1000){
		Signal.lock();
		size_t size = Ucatch.size();
		if (size == 0){
			break;
		}
		std::vector<URL>::iterator iter = Ucatch.begin() + rand() % size;
		URL Url = *(iter);
		Ucatch.erase(iter);
		Signal.unlock();
		if (Is_Catched(Url)){
			continue;
		}
		printf("Catching %s://%s:%s\n", Url.protocol, Url.host, Url.path);
		spider.Url = Url;
		if (spider.conn2Peer(Url.host, Url.port) == SOCKET_ERROR){
			printf("Connect %s://%s:%s error\n", Url.protocol, Url.host, Url.path);
			continue;
		}
		if (spider.Get_Html(Url, filename.c_str())<=0){
			printf("Fail to get %s://%s:%s\n",Url.protocol, Url.host, Url.path);
			continue;
		}
		spider.Parse_Html(filename.c_str());
		Catched.insert(Url);
		counter++;
		//break;
		want_exit = Want_exit();
		if (want_exit){
			break;
		}
	}
	printf("EMPTY\n");
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