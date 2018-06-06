#include "stdafx.h"
#include "visualize.h"
#include "spider.h"
#include <fstream>



extern std::set<Info> MayBe;

void Show_Console(){
	std::set<Info>::iterator iter;
	FILE *final = NULL;
	fopen_s(&final, "result.txt", "w+");
	if (final == NULL){
		printf("write result.txt error\n");
		return ;
	}
	printf("ALL THE MAYBE HERF:\n");
	for (iter = MayBe.begin(); iter != MayBe.end(); iter++){
		char temp[MAX_LENGTH];
		sprintf_s(temp, "%s://%s:%s:%s\n", (*iter).Url.protocol, (*iter).Url.host, (*iter).Url.path, (*iter).Content.c_str());
		fwrite(temp, strlen(temp), 1, final);
		printf("%s://%s:%s:%s\n", (*iter).Url.protocol, (*iter).Url.host, (*iter).Url.path, (*iter).Content.c_str());
	}
	fclose(final);
}

void visualize(){
	//Load to buffer
	FILE *final = NULL;
	fopen_s(&final, "assets/Template", "rb+");
	if (final == NULL){
		printf("Can't open assets/Template\n");
		Show_Console();
		return;
	}
	fseek(final, 0, SEEK_END);
	ULONG flen = ftell(final);
	char *buffer = new char[flen + 1];
	memset(buffer, 0, flen + 1);
	fseek(final, 0, SEEK_SET);
	fread(buffer, flen, 1, final);
	fclose(final);
	FILE *vis = NULL;
	fopen_s(&vis, "index.html", "wb+");
	if (vis == NULL){
		printf("Can't create index.html\n");
		return;
	}
	//<!--insert here--!>
	//find the pos to write html
	char *pos = Get_Pos(buffer);
	fwrite(buffer, pos - buffer, 1, vis);
	char end[MAX_LENGTH];
	sprintf_s(end,"\n\r</thead></table>\n</div>\n</div>\n</div>\n</div>\n</div>\n</body>\n</html>\0");
	std::set<Info>::iterator iter;
	int i = 0;
	for (iter = MayBe.begin(); iter != MayBe.end(); iter++){
		i++;
		char Content[MAX_LENGTH];
		strcpy_s(Content,(*iter).Content.c_str());
		char url[MAX_LENGTH];
		sprintf_s(url, "%s://%s%s\0", (*iter).Url.protocol, (*iter).Url.host, (*iter).Url.path);
		ULONG len = strlen(Content) + strlen(url);
		char write[MAX_LENGTH*2 + 400];
		len=sprintf_s(write, "\n\r<tr>\n\r\r<td>%d</td>\n\r\r<td>%s</td>\n\r\r<td><a href = \"%s\" target = \"blank\" style = \"text-decoration: none; color: #ccc;\">%s</a></td>\n\r</tr>\0",i,Content, url, url);
		fwrite(write, len, 1, vis);
	}
	ULONG len = strlen(end);
	fwrite(end, len, 1, vis);
	fclose(vis);
	ShellExecute(NULL, _T("open"), _T("index.html"), NULL, NULL, SW_SHOW);
}


char* Get_Pos(char *buffer){
	char *p, *pp;
	p = pp = buffer;
	while (*p!='\0')
	{
		if (*p == '<'&&*(p+1)=='!'){
			pp = p;
			while (*p != '>'){
				p++;
			}
			char *temp = new char[p - pp + 2];
			memcpy(temp, pp, p - pp + 1);
			temp[p - pp + 1] = '\0';
			if (!strcmp(temp, "<!--insert here--!>")){
				break;
			}
		}
		p++;
	}
	while (*p!='<'){
		p++;
	}
	return (p + 1);
}