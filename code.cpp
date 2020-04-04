#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<fstream>
#include<string>
#include<queue>
#include<vector>
#include<thread>
#include<sstream>	//alternate of itoa&&atoi

#define useport 10000
#define bufsize 4096
#define strrecvsize 100000
#define counter_step 4096
const char myip[]="127.0.0.1"; 
using namespace std;
/* **NULL reference** created to handle elegantly */
string NULLstring;
ofstream NULLofstream;

int my_hex2dec(char* l,char* r);
int make_clientfd(string& host,string& port);
int make_clientfd(const char* host,int port);
void dlhttpfile(string& req,string& host,string& port,string& dest,string& filename);
void recv_write(int s,string& dest,ofstream& fout);
void listen_fork(int);
void doehentai(string& req);
string itoa(int a);


int main()
{
	int s_listen;
	if (((s_listen=socket(AF_INET,SOCK_STREAM,0))<0))
		printf("wrong socket\n");
	
	int bReuseaddr=1;
	if (setsockopt(s_listen,SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(bReuseaddr))<0)
		printf("setsockpot wrong\n");
		
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	//server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	server_addr.sin_addr.s_addr=inet_addr("localhost");
	server_addr.sin_port=htons(useport);
	if (bind(s_listen,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
	{
		printf("wrong bind\n");
		perror("bind");
	}
	if (listen(s_listen,1000)<0)
	{
		printf("wrong listen\n");
		perror("listen");
	}
	
	int connfd;
	while ((connfd=accept(s_listen,NULL,NULL))>0)
	{
		printf("new acc!now new thread..\n");
		listen_fork(connfd);
		printf("wait for next client...\n");
	}
return 0;
}

void listen_fork(int connfd)
{
		string strrecv;
		int len,recv_size=0;
		char tbuf[bufsize];
		while ((len=recv(connfd,tbuf,bufsize-1,0))>0)
		{
			printf("%d\n",len);
			strrecv+=string(tbuf,len);
			recv_size+=len;
		}
		printf("server receive:\n%s\n",strrecv.c_str());
		printf("over\n");
		/*dl HTTP file*/
		if (strrecv.find("GET")!=string::npos){
			close(connfd);
			doehentai(strrecv);
		}
}

void doehentai(string& req)
{
	/*mainpage source code in utf-8*/
	string MPsour,hostname,sbhostname,port,sbport,imgURL;
	hostname="e-hentai.org";
	port="80";
	dlhttpfile(req,hostname,port,MPsour,NULLstring);
	/*get title first,make directory*/
	int lbound=MPsour.find("<title>")+strlen("<title>"),rbound=MPsour.find("- E-Hentai Galleries</title>");
	string booktitle=MPsour.substr(lbound,rbound-lbound);
	lbound=booktitle.find("[");rbound=booktitle.find("]");
	string author=booktitle.substr(lbound,rbound-lbound);
	booktitle=booktitle.substr(0,lbound)+booktitle.substr(rbound+1,booktitle.size()-rbound-1);
	lbound=author.find("(");
	if (lbound!=string::npos){
		rbound=author.find(")",lbound);
		author=author.substr(lbound,rbound-lbound+1);
	}
	booktitle+=author;
	printf("book title:%s\npress any key to download\n",booktitle.c_str());
	getchar();
	string syscomm=("mkdir \'"+booktitle+"\'");
	printf("syscomm:%s\n",syscomm.c_str());
	
	if (system(syscomm.c_str())==-1){
		printf("directory already exist\n");
		return;
	}
	printf("/////////////////////////////\n%s\n",MPsour.c_str());
	
	/*extract all subpage URL*/
	string subpatt="https://e-hentai.org/s/";
	int cnt=0;
	for (int it=MPsour.find(subpatt);it!=string::npos;it+=subpatt.size(),it=MPsour.find(subpatt,it))
	{
		/*left/right bound of URL*/
		int lbound=it+strlen("https://e-hentai.org");
		int rbound=MPsour.find("\"",lbound);
		//printf("%d %d\n\n",lbound,rbound);
		string sbURL=MPsour.substr(lbound,rbound-lbound);
		
		printf("////////////////////////////////////\nsbURL:\n%s\n",sbURL.c_str());
		
		string request="GET "+sbURL+" HTTP/1.1\r\nHost:e-hentai.org\r\n\r\n";
		string sbsour;
		/*get subpage source code*/
		dlhttpfile(request,hostname,port,sbsour,NULLstring);
		
		printf("///////////////////////////////////\nsbsour:%s",sbsour.c_str());
		
		/*extract img URL and download*/
		/*first host name*/
		lbound=sbsour.find("<img id=\"img\" src=\"http://");
		lbound+=strlen("<img id=\"img\" src=\"http://");
		rbound=sbsour.find(":",lbound);
		sbhostname=sbsour.substr(lbound,rbound-lbound);
		/*now port*/
		lbound=rbound+strlen(":");			//utf-8,for portability
		rbound=sbsour.find("/h",lbound);
		sbport=sbsour.substr(lbound,rbound-lbound);
		/*last one,img URL*/
		lbound=rbound;
		rbound=sbsour.find("\"",lbound);
		imgURL=sbsour.substr(lbound,rbound-lbound);
		
		printf("hostname:%s\nport:%s\nimgURL:%s\n",sbhostname.c_str(),sbport.c_str(),imgURL.c_str());
		
		/*download*/
		request="GET "+imgURL+" HTTP/1.1\r\nHost:"+sbhostname+"\r\n\r\n";
		string filename=booktitle+"/"+itoa(++cnt)+".jpg";
		dlhttpfile(request,sbhostname,sbport,NULLstring,filename);
	}
	printf("download over,%d files done\n",cnt);
}

string itoa(int a)
{
	stringstream ss;
	string ret;
	ss<<a;
	ss>>ret;
	return ret;
}

/*download to string or file*/
void dlhttpfile(string& req,string& host,string& port,string& dest,string& filename)
{
	int len;
	/*connect to e-hentai host*/
	int clientfd=make_clientfd(host,port);
//	int clientfd=make_clientfd("e-hentai.org",80);
	send(clientfd,req.c_str(),req.size(),0);
	if (filename==NULLstring){
		recv_write(clientfd,dest,NULLofstream);
		printf("HTTP to string ok\n");
		return;
	}
	ofstream fout(filename,ios::binary);
	recv_write(clientfd,NULLstring,fout);
	fout.close();
	close(clientfd);
	printf("HTTPfile ok\n");
}

//todo 
void recv_write(int s,string& dest,ofstream& fout)
{
	int len,recved=0,counter=0;
	char buf[bufsize];
	memset(buf,0,bufsize-1);
	/*write to string,must be sourcepage code*/
	if (fout==NULLofstream){
	 	len=recv(s,buf,bufsize-1,0);
	 	char* numl=strstr(buf,"\r\n\r\n")+strlen("\r\n\r\n"),*numr=strstr(numl,"\r\n");
	 	string header(buf,numr);
	 	printf("////////////////////////////\nheader:%s",header.c_str());
	 	int chunklen=my_hex2dec(numl,numr);
		printf("chunk length:%d\n",chunklen);
	 	len=len-(numr-buf)-strlen("\r\n");
	 	string temp(numr+strlen("\r\n"),len);
	 	dest+=temp;
	 	recved+=len;
	 	chunklen-=len;

		while (chunklen>0)
		{
			len=recv(s,buf,bufsize-1,0);
			recved+=len;
			chunklen-=len;
			string tempstr=string(buf,len);
			dest+=tempstr;
			if (recved>counter){
				printf("%d\n",recved);
				counter+=counter_step;
			}
		}
	} 
	/*write to file,need to erase header*/
	else{
			len=recv(s,buf,bufsize-1,0);
			char* w_p=strstr(buf,"\r\n\r\n")+strlen("\r\n\r\n");
			recved+=len-(w_p-buf);
			fout.write(w_p,recved);
			while ((len=recv(s,buf,bufsize-1,0))>0)
			{
				recved+=len;
				fout.write(buf,len);
				if (recved>counter){
					printf("%d\n",recved);
					counter+=counter_step;
				}
			}
		}
	printf("recv:%d\n",recved);
}

int my_hex2dec(char* l,char* r)
{
	int ret=0;
	if (r-l>31)	printf("warning:out of int range\n");
	for (char* i=l;i!=r;i++)
	{
		ret*=16;
		char now=*i;
		int temp;
		if (now>='0'&&now<='9')	temp=now-'0';
		else					temp=now-'a'+10;
		ret+=temp;
	}
	return ret;
}

int make_clientfd(const char* host,int port)
{
	int s=socket(AF_INET,SOCK_STREAM,0);
	if (s<0)	printf("socket wrong\n");
	struct sockaddr_in sa;
	memset(&sa,0,sizeof(sa));
	sa.sin_family=AF_INET;
	sa.sin_addr.s_addr=inet_addr(host);
	sa.sin_port=htons(port);
	
	if (connect(s,(sockaddr*)&sa,sizeof(sockaddr_in))<0){
		printf("connect fail\n");
		perror("connect");
		return -1;
	}
	printf("connect ok");
	return s;
}


int make_clientfd(string& host,string& port)
{
	int s;
	if ((s=socket(AF_INET,SOCK_STREAM,0))<0)	printf("wrong socket\n");
	struct addrinfo hints;
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_protocol=0;
	hints.ai_flags=0;
	struct addrinfo* padif=NULL,*curr;
	
	printf("getaddrinfo now\n");
	if (getaddrinfo(host.c_str(),port.c_str(),&hints,&padif)!=0)
		printf("getaddrinfo failed\n");
	printf("getaddrinfo ok\n");
	if (padif==NULL)	printf("get host fail\n");
	struct sockaddr_in* sa=((struct sockaddr_in*)(padif->ai_addr));
	int hostip=sa->sin_addr.s_addr;
	char ipbuf[16]="\0";
	inet_ntop(AF_INET,&hostip,ipbuf,16);
	for (curr = padif; curr != NULL; curr = curr->ai_next)
	{
		struct sockaddr_in* sa = ((struct sockaddr_in*)(curr->ai_addr));
		int hostip = sa->sin_addr.s_addr;
		char ipbuf[16] = "\0";
		inet_ntop(AF_INET, &hostip, ipbuf, 16);
		printf("ip resolved:%s\n", ipbuf);
		if (connect(s, (sockaddr*)sa, sizeof(sockaddr_in))>=0) {
			printf("connect ok!\n");
			break;
		}
	}
	if (curr==NULL)	printf("wrong connect!\n");
	
	freeaddrinfo(padif);
	return s;							
}
