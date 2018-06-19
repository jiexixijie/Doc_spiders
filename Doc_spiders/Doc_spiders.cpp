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
extern std::set<std::string> Limit_Host;
extern std::set<Info> MayBe;
extern std::set<std::string> Search;
extern std::mutex Signal;

int MAX_Spiders = 8;
//http://zh.wikipedia.org:80/wiki/Special:Search?search=铁路&go=Go
char url[MAX_URL_LENGTH + 1] = "http://www.njupt.edu.cn/";

void show();
void Add_Search_Info();
void Catch_it(Spider spider, const std::string filename);
bool Want_exit();
bool want_exit = FALSE;

int _tmain(int argc, _TCHAR* argv[])
{
	//add in it 
	Limit_Host.insert("njupt");
	Search.insert("研究生");
	std::string filename;
	std::thread *threads = new std::thread[MAX_Spiders];
	//add info and show option
	show();
	//start catch
	WSADATA wsdata;
	if (WSAStartup(0x0202, &wsdata)!=0){
		printf("初始化失败\n");
		goto _ToEnd;
	}
	URL Url;
	Url = Parse_url(url);
	Ucatch.push_back(Url);
	for (int i = 0; i < MAX_Spiders; i++){
		Spider sp1;
		char temp[50];
		sprintf_s(temp, ".temp/spdier%d.txt", (i + 1));
		filename = temp;
		while (!Ucatch.size()){
			if (want_exit){
				MAX_Spiders = 0;
				break;
			}
			Sleep(10);
		}
		//多线程
		threads[i] = std::thread(Catch_it, sp1, filename);
		threads[i].detach();
	}
	//Only One
	while (!want_exit){
		want_exit = Want_exit();
		Sleep(10);
	}
	printf("Please wait all spiders to stop\n");
	while (MAX_Spiders > 0){
		Sleep(100);
	}
	WSACleanup();
	if (MayBe.empty()){
		printf("Maybe you should change another url\n");
		goto _ToEnd;
	}
	//show result in the html
	visualize();
_ToEnd:
	system("pause");
	return 0;
}

void show(){
	//... show option in console
	printf("Start Catch-----------g\n");
	printf("Split key word by \",\"\n");
	printf("Add Keyword-----------ak:key word\n");
	printf("Delete Keyword--------dk:key word\n");
	printf("Add LimitHost---------al:key word\n");
	printf("Delete LimitHost------dl:key word\n");
	printf("Change Spiders--------s:4\n");
	printf("Change StartUrl-------u:url\n");
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
		Signal.lock();
		Catched.insert(Url);
		Signal.unlock();
		counter++;
		//break;
		if (want_exit){
			break;
		}
	}
	MAX_Spiders--;
	remove(filename.c_str());
	printf("Spiders %c Finished\n",filename.c_str()[12]);
	want_exit = TRUE;
}

void Add_Search_Info(){
	char Option[MAX_LENGTH];
	while (true)
	{
		std::cin.getline(Option, MAX_LENGTH);
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
		if (!strcmp(temp, "g") || !strcmp(temp, "G")){
			break;
		}
		//add keyword
		else if (!strcmp(temp, "ak")){
			pp = ++p;
			while (*(p - 1) != '\0'){
				if (*p == ','|| *p=='\0'){
					if (p - pp > 100){
						printf("key word too long\n");
						break;
					}
					//keyword is Null
					else if (p == pp){
						break;
					}
					char word[99];
					strncpy_s(word, pp, p - pp);
					std::string temp = word;
					Search.insert(temp);
					pp = p + 1;
				}
				p++;
			}
			printf("Search Keyword:");
			for (std::set<std::string>::iterator iter = Search.begin(); iter != Search.end(); iter++){
				printf("%s  ", (*iter).c_str());
			}
			printf("\n");
		}
		//delete keyword
		else if (!strcmp(temp, "dk")){
			pp = ++p;
			while (*(p - 1) != '\0'){
				if (*p == ','|| *p == '\0'){
					if (p - pp > 100){
						printf("key word too long\n");
						break;
					}
					else if (p == pp){
						break;
					}
					char word[99];
					strncpy_s(word, pp, p - pp);
					std::string temp = word;
					std::set<std::string>::iterator iter;
					for (iter = Search.begin(); iter != Search.end(); iter++){
						if (*iter == word){
							Search.erase(iter);
							break;
						}
					}
					pp = p + 1;
				}
				p++;
			}
			printf("Search Keyword:");
			for (std::set<std::string>::iterator iter = Search.begin(); iter != Search.end(); iter++){
				printf("%s  ", (*iter).c_str());
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
					else if (p == pp){
						break;
					}
					char word[99];
					strncpy_s(word, pp, p - pp);
					std::string temp = word;
					Limit_Host.insert(temp);
					pp = p + 1;
				}
				p++;
			}
			printf("Limit_Host:");
			for (std::set<std::string>::iterator iter = Limit_Host.begin(); iter != Limit_Host.end(); iter++){
				printf("%s  ", (*iter).c_str());
			}
			printf("\n");
		}
		else if (!strcmp(temp, "dl")){
			pp = ++p;
			while (*(p - 1) != '\0'){
				if (*p == ','|| *p == '\0'){
					if (p - pp > 100){
						printf("key word too long\n");
						break;
					}
					else if (p == pp){
						break;
					}
					char word[99];
					strncpy_s(word, pp, p - pp);
					std::string temp = word;
					for (std::set<std::string>::iterator iter = Limit_Host.begin(); iter != Limit_Host.end(); iter++){
						if (*iter == temp){
							Limit_Host.erase(iter);
							break;
						}
					}
					pp = p + 1;
				}
				p++;
			}
			printf("Limit_Host:");
			for (std::set<std::string>::iterator iter = Limit_Host.begin(); iter != Limit_Host.end(); iter++){
				printf("%s  ", (*iter).c_str());
			}
			printf("\n");
		}
		else if (!strcmp(temp, "s")){
			pp = ++p;
			std::string temp = p;
			if (temp.length() > 1 || atoi(temp.c_str())<1 || atoi(temp.c_str())>8){
				printf("MAX_Spiders is 8\n");
				break;
			}
			else{
				MAX_Spiders = atoi(temp.c_str());
				printf("Spiders:%d\n", MAX_Spiders);
			}
		}
		else if (!strcmp(temp, "u")){
			pp = ++p;
			std::string temp = pp;
			if (temp.length() > MAX_LENGTH){
				printf("url too long\n");
			}
			else
			{
				memset(url, 0, MAX_LENGTH + 1);
				strcpy_s(url, temp.c_str());
				printf("url:%s\n", url);
			}
		}
		else if (!strcmp(temp, "show")){
			printf("Start url:%s\n", url);
			printf("Spiders:%d\n", MAX_Spiders);
			printf("Limit_Host:");
			std::set<std::string>::iterator iter;
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