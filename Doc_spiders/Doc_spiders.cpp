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
#include <algorithm>

#include "spider.h"
#include "visualize.h"

#pragma comment(lib,"ws2_32.lib")

extern std::vector<URL> Ucatch;
extern std::set<URL> Catched;
extern std::vector<std::string> Limit_Host;
extern std::set<Info> MayBe;
extern std::vector<std::string> Search;
extern std::mutex Signal;

int MAX_Spiders = 8;

void show();
void Add_Search_Info();
void Catch_it(Spider spider, const std::string filename);
bool Want_exit();
bool want_exit = FALSE;

int _tmain(int argc, _TCHAR* argv[])
{
	//add in it 
	Limit_Host.push_back("blog");
	Search.push_back("程序员");
	//add info
	show();
	WSADATA wsdata;
	if (WSAStartup(0x0202, &wsdata)!=0){
		printf("初始化失败\n");
		return 0;
	}
	//http://zh.wikipedia.org:80/wiki/Special:Search?search=铁路&go=Go
	char url[MAX_URL_LENGTH + 1] ="http://blog.jobbole.com/";
	URL Url;
	Url = Parse_url(url);
	Ucatch.push_back(Url);
	std::string filename;
	std::thread *threads = new std::thread[MAX_Spiders];
	for (int i = 0; i < MAX_Spiders; i++){
		Spider sp1;
		char temp[50];
		sprintf_s(temp, ".temp/spdier%d.txt", (i + 1));
		filename = temp;
		while (!Ucatch.size()){
			Sleep(10);
		}
		threads[i] = std::thread(Catch_it, sp1, filename);
		threads[i].detach();
	}
	//Only One
	while (!want_exit){
		want_exit = Want_exit();
		Sleep(10);
	}
	printf("Please wait all spiders to stop\n");
	while (MAX_Spiders)
	{
		Sleep(100);
	}
	WSACleanup();

	if (MayBe.empty()){
		printf("Maybe you should change another url\n");
		system("pause");
		return 0;
	}
	visualize();
	system("pause");
	return 0;
}

void show(){
	//... show option in console
	printf("Start Catch-----------g:\n");
	printf("Split key word by \",\"\n");
	printf("Add Keyword-----------ak:key word\n");
	printf("Delete Keyword--------dk:key word\n");
	printf("Add LimitHost---------al:key word\n");
	printf("Delete LimitHost------dl:key word\n");
	printf("Change Spiders--------s:4\n");
	printf("Show Option-----------show\n");
	Add_Search_Info();
}

void Catch_it(Spider spider, const std::string filename){
	ULONG counter = 0;
	//URL Url = *Ucatch.begin();
	//Ucatch.erase(Ucatch.begin());
	//Wait more
	while (!Ucatch.empty() && counter < 100){
		Signal.lock();
		std::vector<URL>::iterator iter = Ucatch.begin() + rand() % Ucatch.size();
		if (iter == Ucatch.end()){
			Signal.unlock();
			break;
		}
		URL Url;
		Url= *(iter);
		Ucatch.erase(iter);
		Signal.unlock();
		if (Is_Catched(Url)){
			continue;
		}
		printf("Catching %s://%s%s\n", Url.protocol, Url.host, Url.path);
		spider.Url = Url;
		if (spider.conn2Peer(Url.host, Url.port) == SOCKET_ERROR){
			printf("Connect %s://%s%s error\n", Url.protocol, Url.host, Url.path);
			continue;
		}
		if (spider.Get_Html(Url, filename.c_str())<=0){
			printf("Fail to get %s://%s%s\n",Url.protocol, Url.host, Url.path);
			continue;
		}
		spider.Parse_Html(filename.c_str());
		Catched.insert(Url);
		counter++;
		//break;
		if (want_exit){
			break;
		}
	}
	MAX_Spiders--;
	remove(filename.c_str());
	printf("Spiders %c Finished\n",filename.c_str()[12]);
	if (!MAX_Spiders){
		want_exit = TRUE;
	}
}

void Add_Search_Info(){
	char Option[MAX_LENGTH];
	while (true)
	{
		std::cin >> Option;
		char *p, *pp;
		p = pp = Option;
		while (*p != ':'&& *p != '\0'){
			p++;
		}
		if (*p == '\0' &&p - pp > 4){
			printf("Wrong Commond\n");
			continue;
		}
		char temp[5];
		strncpy_s(temp, pp, p - pp);
		temp[p - pp] = '\0';
		if (!strcmp(temp, "g")){
			break;
		}
		else if (!strcmp(temp, "ak")){
			pp = ++p;
			while (*(p - 1) != '\0'){
				if (*p == ','|| *p=='\0'){
					if (p - pp > 100){
						printf("key word too long\n");
						break;
					}
					char word[99];
					strncpy_s(word, pp, p - pp);
					std::string temp = word;
					Search.push_back(temp);
					pp = p + 1;
				}
				p++;
			}
			printf("Search Keyword:");
			for (std::vector<std::string>::iterator iter = Search.begin(); iter != Search.end(); iter++){
				printf("%s ", (*iter).c_str());
			}
			printf("\n");
		}
		else if (!strcmp(temp, "dk")){
			pp = ++p;
			while (*(p - 1) != '\0'){
				if (*p == ','|| *p == '\0'){
					if (p - pp > 100){
						printf("key word too long\n");
						break;
					}
					char word[99];
					strncpy_s(word, pp, p - pp);
					std::string temp = word;
					std::vector<std::string>::iterator iter;
					iter = find(Search.begin(), (--Search.end()), temp);
					if (iter != Search.end()){
						Search.erase(iter);
					}
					pp = p + 1;
				}
				p++;
			}
			printf("\nSearch Keyword:");
			for (std::vector<std::string>::iterator iter = Search.begin(); iter != Search.end(); iter++){
				printf("%s ", (*iter).c_str());
			}
			printf("\n");
		}
		else if (!strcmp(temp, "al")){
			pp = ++p;
			while (*(p - 1) != '\0'){
				if (*p == ',' || *p == '\0'){
					if (p - pp > 100){
						printf("key word too long\n");
						break;
					}
					char word[99];
					strncpy_s(word, pp, p - pp);
					std::string temp = word;
					Limit_Host.push_back(temp);
					pp = p + 1;
				}
				p++;
			}
			printf("\nLimit_Host:");
			for (std::vector<std::string>::iterator iter = Limit_Host.begin(); iter != Limit_Host.end(); iter++){
				printf("%s ", (*iter).c_str());
			}
		}
		else if (!strcmp(temp, "dl")){
			pp = ++p;
			while (*(p - 1) != '\0'){
				if (*p == ','|| *p == '\0'){
					if (p - pp > 100){
						printf("key word too long\n");
						break;
					}
					char word[99];
					strncpy_s(word, pp, p - pp);
					std::string temp = word;
					std::vector<std::string>::iterator iter;
					iter = find(Limit_Host.begin(), (--Limit_Host.end()), temp);
					if (iter != Limit_Host.end()){
						Limit_Host.erase(iter);
					}
					pp = p + 1;
				}
				p++;
			}
			printf("\nLimit_Host:");
			for (std::vector<std::string>::iterator iter = Limit_Host.begin(); iter != Limit_Host.end(); iter++){
				printf("%s ", (*iter).c_str());
			}
		}
		else if (!strcmp(temp, "s")){
			pp = ++p;
			char number = *p;
			if (number > '0'&&number < '9'){
				MAX_Spiders = atoi(&number);
			}
			else
			{
				printf("MAX_Spiders is 8\n");
			}
			printf("Spiders:%d", MAX_Spiders);
		}
		else if (!strcmp(temp, "show")){
			printf("Spiders:%d", MAX_Spiders);
			printf("\nLimit_Host:");
			std::vector<std::string>::iterator iter;
			for (iter = Limit_Host.begin(); iter != Limit_Host.end(); iter++){
				printf("%s ", (*iter).c_str());
			}
			printf("\nSearch Keyword:");
			for (iter = Search.begin(); iter != Search.end(); iter++){
				printf("%s ", (*iter).c_str());
			}
			printf("\n");
		}
		else{
			printf("Wrong Commond\n");
		}
	}
	std::string dirName = ".temp";
	if (_access(dirName.c_str(), 0) == -1)
	{
		int i = _mkdir(dirName.c_str());
	}
}

//按键退出
bool Want_exit(){
	if (_kbhit()){   //键盘存在输入
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