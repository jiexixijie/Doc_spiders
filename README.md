Doc_spiders

to catch something which include key word from web (http)

0x00 重要函数概要

- Parse_url(char *herf)：解析URL
- conn2Peer(const char *Host,int Port)：连接（三次握手）
- Get_Html(const URL url,const char *filename)：获取HTML文件
- Get_Info(char *buffer, ULONG flen)：获取HTTP头信息
- Parse_Html(const char *filename)：解析HTML

0x01 使用方法

- Start Catch--------------g                        开始扫描
- Split key word by ","                           使用逗号隔开关键词
- Add Keyword-----------ak:key word    添加关键词：搜索内容中包含该关键词的链接
- Delete Keyword--------dk:key word    删除关键词
- Add LimitHost----------al:key word     添加主机限制：限制扫描的host范围  
- Delete LimitHost------dl:key word     删除主机限制
- Change Spiders--------s:4(default)     默认4线程（最大8线程）
- Change StartUrl-------u:urlShow       设置Start URL
- Option--------------------show                显示已设置的内容

运行过程中输入“e”，停止爬取，并调用默认浏览器显示爬取结果（使用Bootstrap 3编写html）
