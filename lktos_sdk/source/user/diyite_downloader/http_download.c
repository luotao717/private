#include<stdio.h>
#include<netdb.h> 
#include<stdlib.h> 
#include<errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
 

#define SP_PACKLEN					2048

const char * http_s = "http://";

struct host_info {
	char *allocated;
	const char *path;
	const char *user;
	char       *host;
	int         port;
	int    	is_ftp;
};

static unsigned hex_to_bin(unsigned char c)
{
	unsigned v;

	v = c - '0';
	if (v <= 9)
		return v;
	/* c | 0x20: letters to lower case, non-letters
	 * to (potentially different) non-letters */
	v = (unsigned)(c | 0x20) - 'a';
	if (v <= 5)
		return v + 10;
	return ~0;
/* For testing:
void t(char c) { printf("'%c'(%u) %u\n", c, c, hex_to_bin(c)); }
int main() { t(0x10); t(0x20); t('0'); t('9'); t('A'); t('F'); t('a'); t('f');
t('0'-1); t('9'+1); t('A'-1); t('F'+1); t('a'-1); t('f'+1); return 0; }
*/
}

static char* percent_decode_in_place(char *str, int strict)
{
	/* note that decoded string is always shorter than original */
	char *src = str;
	char *dst = str;
	char c;

	while ((c = *src++) != '\0') {
		unsigned v;

		if (!strict && c == '+') {
			*dst++ = ' ';
			continue;
		}
		if (c != '%') {
			*dst++ = c;
			continue;
		}
		v = hex_to_bin(src[0]);
		if (v > 15) {
 bad_hex:
			if (strict)
				return NULL;
			*dst++ = '%';
			continue;
		}
		v = (v * 16) | hex_to_bin(src[1]);
		if (v > 255)
			goto bad_hex;
		if (strict && (v == '/' || v == '\0')) {
			/* caller takes it as indication of invalid
			 * (dangerous wrt exploits) chars */
			return str + 1;
		}
		*dst++ = v;
		src += 2;
	}
	*dst = '\0';
	return str;
}

static int parse_url(const char *src_url, struct host_info *h)
{
	char *url, *p, *sp;

	free(h->allocated);
	h->allocated = url = strdup(src_url);

	if (strncmp(url, "ftp://", 6) == 0) {
		h->port = 21;
		h->host = url + 6;
		h->is_ftp = 1;
	} else
	if (strncmp(url, "http://", 7) == 0) {
		h->host = url + 7;
 http:
		h->port = 80;
		h->is_ftp = 0;
	} else
	if (!strstr(url, "//")) {
		// GNU wget is user-friendly and falls back to http://
		h->host = url;
		goto http;
	} else {
		printf("not an http or ftp url: %s\n.", url);
		return -1;
	}

	// FYI:
	// "Real" wget 'http://busybox.net?var=a/b' sends this request:
	//   'GET /?var=a/b HTTP 1.0'
	//   and saves 'index.html?var=a%2Fb' (we save 'b')
	// wget 'http://busybox.net?login=john@doe':
	//   request: 'GET /?login=john@doe HTTP/1.0'
	//   saves: 'index.html?login=john@doe' (we save '?login=john@doe')
	// wget 'http://busybox.net#test/test':
	//   request: 'GET / HTTP/1.0'
	//   saves: 'index.html' (we save 'test')
	//
	// We also don't add unique .N suffix if file exists...
	sp = strchr(h->host, '/');
	p = strchr(h->host, '?'); if (!sp || (p && sp > p)) sp = p;
	p = strchr(h->host, '#'); if (!sp || (p && sp > p)) sp = p;
	if (!sp) {
		h->path = "";
	} else if (*sp == '/') {
		*sp = '\0';
		h->path = sp + 1;
	} else { // '#' or '?'
		// http://busybox.net?login=john@doe is a valid URL
		// memmove converts to:
		// http:/busybox.nett?login=john@doe...
		memmove(h->host - 1, h->host, sp - h->host);
		h->host--;
		sp[-1] = '\0';
		h->path = sp;
	}

	// We used to set h->user to NULL here, but this interferes
	// with handling of code 302 ("object was moved")

	sp = strrchr(h->host, '@');
	if (sp != NULL) {
		// URL-decode "user:password" string before base64-encoding:
		// wget http://test:my%20pass@example.com should send
		// Authorization: Basic dGVzdDpteSBwYXNz
		// which decodes to "test:my pass".
		// Standard wget and curl do this too.
		*sp = '\0';
		h->user = percent_decode_in_place(h->host, /*strict:*/ 0);
		h->host = sp + 1;
	}

	sp = h->host;
	return 0;
}

static struct sockaddr_in* str2sockaddr(const char *host, int port)
{
	const char *org_host = host; /* only for error msg */
	const char *cp;
	struct addrinfo hint;
	struct addrinfo *result = NULL, *cur;
	struct sockaddr_in *server_addr = calloc(1, sizeof(struct sockaddr_in));;

	cp = strrchr(host, ':');
	if (cp != NULL) {
		int sz = cp - host + 1;
		host = strncpy(alloca(sz), host, sz);
		cp++; /* skip ':' */
		port = strtoul(cp, NULL, 10);
		if (errno || (unsigned)port > 0xffff) {
			printf("bad port spec '%s'\n", org_host);
			free(server_addr);
			return NULL;
		}
	}

	bzero(server_addr, sizeof(struct sockaddr_in));
	struct in_addr in4;
	if (inet_aton(host, &in4) != 0) {
		server_addr->sin_family = AF_INET;  
		server_addr->sin_addr = in4;   
		server_addr->sin_port = htons(port);
	}
	else
	{
		char ipstr[16] = {0};
		memset(&hint, 0 , sizeof(hint));
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_flags = AI_CANONNAME;
		int rc = getaddrinfo(host, NULL, &hint, &result);
		if (rc || !result) {
			printf("bad address '%s'\n", org_host);
			free(server_addr);
			return NULL;
		}

		for (cur = result; cur != NULL; cur = cur->ai_next) 
		{ 
			struct sockaddr_in *sinp = (struct sockaddr_in *)cur->ai_addr;
			if (inet_ntop(sinp->sin_family, &sinp->sin_addr, ipstr, INET_ADDRSTRLEN) != NULL)
			{		                
				//printf("check server address is %s\n", ipstr);
				
				server_addr->sin_family = AF_INET;  
				server_addr->sin_addr.s_addr = inet_addr(ipstr);   
				server_addr->sin_port = htons(port);
				int sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
				if(sock_fd != -1)
				{
					if(-1 != connect(sock_fd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)))  
					{
						//printf("server %s is ok.\n", ipstr);
						close(sock_fd);
						break;
					}
					close(sock_fd);
				}  
			}
		}
		
		if (result)
			freeaddrinfo(result);
	}

	return server_addr;
}

//---------------------------------------------------------------------------   
   
/*  
该方法获取待下载文件大小  
*/  
int getlength(char *sheader)
{
	char * cont_len_pos;
	unsigned long cont_length;
	const char* header = sheader;
	cont_len_pos =(char *)strstr(header,"Content-Length:");
	cont_len_pos = (char *)strchr(cont_len_pos,' ');
	cont_len_pos++;
	sscanf(cont_len_pos, "%ld", &cont_length);
	return cont_length;   
}   

/*  
该方法判断是否Get成功，不成功就取消下载文件  
*/  
int getstatus(char *sheader){         
	if(strstr(sheader,"HTTP/1.1 200 OK") != NULL)
	{
		return 1;  
	}
	else
	{
		return 0;   
	}
}   

int mystrcmp(char *stringp, const char *delim) 
  {     
  	char *s;     
	int len=strlen(delim);
	int pos = 0;
	if ((s = stringp)== NULL)        
		return (0);    
	while(*s != '\0')
	{
		 
		if(strncmp(s,delim,len) == 0)
		{
			return(pos+len);
		}
		s++;
		pos++;
	}     
	return(0);
}

#define SP_PACKLEN	2048
int http_download(const char * url, const char *fileName) 
{ 
	int ret;
	int sock_fd;
	FILE *f = NULL;
	struct host_info target;
	target.allocated = NULL;
	target.user = NULL;

	parse_url(url, &target);
	//printf("host : %s, port : %d\n", target.host, target.port);

	struct sockaddr_in* server_addr = str2sockaddr(target.host, target.port);
	if(server_addr == NULL)
	{
		printf("server_addr == NULL\n");	
		ret = -1;
		goto end;
	}

	// output file		
	f = fopen(fileName,"w+");
	if (f == NULL)
	{
		printf("http_download  fopen failed fileName = %s\n", fileName);	
		ret = -1;
		goto end;
	}

	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if(sock_fd == -1)
	{ 
		printf("socket error\n"); 
		ret = -1;
		goto end; 
	}

	if(-1 == connect(sock_fd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)))  
	{
		printf("connect fail !\r\n");
		ret = -1;
		goto end; 
	}  
 
	//printf("connect ok !\r\n"); 

	char request[1024]; 
	bzero(&request,1024); 

	sprintf(request,   "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\nHost: %s\r\nConnection:  Keep-Alive\r\n\r\n ", target.path, target.host);             
	//printf("request is %s\n", request);
	int send = write(sock_fd, request,strlen(request));
	if(send == -1)
	{ 
		printf("send error\n"); 
		ret = -1;
		goto end;
	} 

	int length;
	char buffer[SP_PACKLEN]; 
	memset(buffer, 0, SP_PACKLEN);
	fd_set readfds;
	fd_set errfds;
	int maxfd = sock_fd + 1;
	struct timeval timeout={300,0};
	int bwritefile = 0;
	unsigned long content_length = 0, left_length = 0;
	while(1)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&errfds);
		FD_SET(sock_fd, &readfds);
		FD_SET(sock_fd, &errfds);
		switch(select(maxfd, &readfds, NULL, &errfds, &timeout))
		{
			case -1:
			case 0:
				ret = -1;
				goto end;
				break;
			default:
			{
				if (FD_ISSET(sock_fd, &errfds))
				{
					printf("select socket error\n");
					ret = -1;
					goto end;
					break;
				}
				if (FD_ISSET(sock_fd, &readfds))
				{
					memset(buffer, 0, SP_PACKLEN);
					if((length = recv(sock_fd, buffer, SP_PACKLEN, 0)) < 0)   
        				{    
						printf("recv error url=%s\n",url); 
        					ret = -1;
						goto end;   
    					} 
					if (length == 0)
					{
						ret = 0;
						goto end;
					}

					char *header = buffer; 
    					if(!bwritefile && getstatus(header))
					{   
						content_length = getlength(header);
						left_length = content_length;
						char *q = buffer;   
						char *p = strstr(buffer, "\r\n\r\n");   

        					p += 4;
        					fwrite(p,length - (p - q),1,f); 
						bwritefile = 1; 
						left_length -= (length - (p - q));
					}
					else if (bwritefile)
					{
						if (left_length > 0)
						{
							int temp_len = length > left_length ? left_length : length;
							fwrite(buffer, temp_len, 1, f);
							left_length -= temp_len;
						}
						else 
						{
							break;
						}
					}
					else
					{    
						//printf("http error, ret = %s\n",header);
						ret = -1;
						goto end; 
					} 
					break;
				}
			}
		}
	}

	ret = 0; 

end:
	if(server_addr != NULL)
		free(server_addr);
	if(f != NULL)		fclose(f); 
	if(sock_fd != -1)  
		close(sock_fd); 

	return ret; 

} 







