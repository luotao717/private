/*==============================================================================
//
// Project:
//		userapp-ddns module
//
//------------------------------------------------------------------------------
// Description:
//
//          Support for DDNS
==============================================================================*/

//==============================================================================
//                                INCLUDE FILES
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "ddns.h"

extern int SYS_wan_ip;
//==============================================================================
//                                    MACROS
//==============================================================================
#define BUFFER_SIZE		4*1024

//==============================================================================
//                               TYPE DEFINITIONS
//==============================================================================

//==============================================================================
//                               LOCAL VARIABLES
//==============================================================================

int DYNDNS_update_entry(struct userInfo_cxy *info);
static struct service_cxy DYNDNS_service = {
	DYNDNS_NAME,
	DYNDNS_update_entry,
	DYNDNS_DEFAULT_SERVER,
	DYNDNS_DEFAULT_PORT,
	DYNDNS_REQUEST
};

static struct service_cxy QDNS_service = {
	QDNS_NAME,
	DYNDNS_update_entry,
	QDNS_DEFAULT_SERVER,
	QDNS_DEFAULT_PORT,
	QDNS_REQUEST
};

int ORAY_update_entry(struct userInfo_cxy *info);
static struct service_cxy ORAY_service = {
	ORAY_NAME,
	ORAY_update_entry,
	ORAY_DEFAULT_SERVER,
	ORAY_DEFAULT_PORT,
	ORAY_REQUEST
};

#if 0
int M88IP_update_entry(struct userInfo_cxy *info);
static struct service_cxy M88IP_service = {
	M88IP_NAME,
	M88IP_update_entry,
	M88IP_DEFAULT_SERVER,
	M88IP_DEFAULT_PORT,
	M88IP_REQUEST
};
#endif

int TZO_update_entry(struct userInfo_cxy *info);
static struct service_cxy TZO_service = {
  TZO_NAME,           
  TZO_update_entry,   
  TZO_DEFAULT_SERVER, 
  TZO_DEFAULT_PORT,   
  TZO_REQUEST         
};

int NOIP_update_entry(struct userInfo_cxy *info);
static struct service_cxy NOIP_service = {
    NOIP_NAME,           
    NOIP_update_entry,   
    NOIP_DEFAULT_SERVER, 
    NOIP_DEFAULT_PORT,   
    NOIP_REQUEST         
};

int DTDNS_update_entry(struct userInfo_cxy *info);
static struct service_cxy DTDNS_service = {
    DTDNS_NAME,          
    DTDNS_update_entry,  
    DTDNS_DEFAULT_SERVER,
    DTDNS_DEFAULT_PORT,  
    DTDNS_REQUEST        
};

int CHANGEIP_update_entry(struct userInfo_cxy *info);
static struct service_cxy CHANGEIP_service = {
    CHANGEIP_NAME,           
    CHANGEIP_update_entry,   
    CHANGEIP_DEFAULT_SERVER, 
    CHANGEIP_DEFAULT_PORT,   
    CHANGEIP_REQUEST         
};


static struct service_cxy *service_list[]={
	&DYNDNS_service,
	&QDNS_service,
	&ORAY_service,
#if 0
	&M88IP_service,
#endif
	&TZO_service,
	&NOIP_service,
	&DTDNS_service,
	&CHANGEIP_service,
	NULL,
};

//==============================================================================
//                          LOCAL FUNCTION PROTOTYPES
//==============================================================================

//==============================================================================
//                              EXTERNAL FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
// 
//------------------------------------------------------------------------------
void usage(void)
{
	printf("\r\nSample config: yddns -s service domainname username password ip\r\n");
	printf("eg:yddns -s qdns chenfei105105.3322.org chenfei105 18680311896 219.22.66.55\r\n");
	//printf("More config: yddns -m type domainname username password ip mx backmx trytime update_time\r\n");
}
//------------------------------------------------------------------------------
struct service_cxy *find_service(char *name)
{
	struct service_cxy *serv;
	int i;
	
	for(i=0; service_list[i]; i++)
	{
		serv = service_list[i];
		if(strcmp(serv->name, name) == 0)
			return serv;
	}
	return NULL;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int do_connect(int *sock, char *host, unsigned short port)
{
	struct sockaddr_in addr;
	int len;
	int result;
	struct hostent *hostinfo;

	// set up the socket
	if((*sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return(-1);
    }
		
	// get the host address
	hostinfo = gethostbyname(host);  
	
	if(!hostinfo)
	{
		printf("gethostbyname error"); 
		close(*sock);
		return(-1);
    }
   
	addr.sin_family = AF_INET; 
	addr.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
	addr.sin_port = htons(port);

	// connect the socket
	len = sizeof(addr);
	if((result=connect(*sock, (struct sockaddr *)&addr, len)) == -1) 
	{
		perror("connect");
		close(*sock);
		return(-1);
	}

	return 0;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
static void output(int fd, void *buf)
{
	fd_set writefds;
	int max_fd;
	struct timeval tv;
	int ret;

	FD_ZERO(&writefds);
	FD_SET(fd, &writefds);
	max_fd = fd;
	
	tv.tv_sec = 0;
    tv.tv_usec = 20000;          /* timed out every 20 ms */
    
	ret = select(max_fd + 1, NULL, &writefds, NULL, &tv);

	if(ret == -1)
	{
    	printf("[DDNS]: select error\n");
	}
	else if(ret == 0)
	{
		printf("select timeout\n");
	}
	else
	{
		/* if we woke up on client_sockfd do the data passing */
		if(FD_ISSET(fd, &writefds))
		{
			if(send(fd, buf, strlen(buf), 0) == -1)
				printf("error send()ing request\n");
		}
		else
			printf("socket was not exist!!\n");
    }
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int read_input(int fd, void *buf, int len)
{
	fd_set readfds;
	int max_fd;
	struct timeval tv;
	int ret;
	int bread = -1;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	max_fd = fd;
	
	tv.tv_sec = 9;
	tv.tv_usec = 0;          
	ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);

	if(ret == -1)
	{
		printf("[DDNS]: select error\n");
	}
	else if(ret == 0)
	{
		printf("select timeout\n");
	}
	else
	{
		/* if we woke up on client_sockfd do the data passing */
		if(FD_ISSET(fd, &readfds))
		{
			if((bread=recv(fd, buf, len, 0)) == -1)
			{
				printf("error recv()ing reply\n");
			}
		}
		else
			printf("socket was not exist!!\n");
	}

  return(bread);
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int DYNDNS_update_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];

	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("\r\nerror connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(bp, "GET %s?system=dyndns&", info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "hostname=%s&", info->host);
	bp += len;
	
	if(MY_IP)
	{
		len = sprintf(bp, "myip=%s&", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp, "wildcard=%s&", info->wildcard ? "ON" : "OFF");
	bp += len;
 	
	if(*(info->mx) != '\0')
	{
		len = sprintf(bp, "mx=%s&", info->mx);
		bp += len;
	}

	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"Authorization: Basic %s\r\n"
 						"User-Agent: %s\r\n"
 						"Host: %s\r\n\r\n",
 						auth,
						DEVICE_NAME,
						info->service->default_server);
 						
	bp += len;	
	*bp = '\0';	
	
	output(fd, buf);
	printf("\r\nsendmessage:\r\n%s",buf);
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';
	printf("\r\n%s\r\n",buf);
	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		printf("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "\ngood ") != NULL)
		{
			retval = UPDATERES_OK;
		}
		else
		{
			if(strstr(buf, "\nnohost") != NULL)
			{
				printf("invalid hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnotfqdn") != NULL)
			{
				printf("malformed hostname\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\n!yours") != NULL)
			{
				printf("host is not under your control\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nabuse") != NULL)
			{
				printf("host has been blocked for abuse\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnochg") != NULL)
			{
				printf("your IP address has not changed since the last update\n");
				retval = UPDATERES_OK;
			}
			else if(strstr(buf, "\nbadauth") != NULL)
			{
				printf("authentication failure\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadsys") != NULL)
			{
				printf("invalid system parameter\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nbadagent") != NULL)
			{
				printf("this useragent has been blocked\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\nnumhost") != NULL)
			{
				printf("Too many or too few hosts found\n");
				retval = UPDATERES_SHUTDOWN;
			}
			else if(strstr(buf, "\ndnserr") != NULL)
			{
				printf("dyndns internal error\n");
				retval = UPDATERES_ERROR;
			}
			else
			{
				printf("error processing request\n");
				retval = UPDATERES_ERROR;
			}
		}
		break;

	case 401:
		printf("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------

int ORAY_update_entry(struct userInfo_cxy *info)
{
	
	int retval = UPDATERES_ERROR;
	int ret;
	ret = ddns_oray(info->usrname, info->usrpwd, MY_IP_STR, info->host);
	if(ret < 0)
		retval = UPDATERES_ERROR;
	if(ret == 0)
		retval = UPDATERES_OK;	
	if(ret == 1)
		retval = UPDATERES_OK_2;		
	return retval;
}	

#if 0
//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int M88IP_update_entry(struct userInfo_cxy *info)
{
    char *buf, *bp,*temp;
	int bytes, btot, ret, fd, len, p;
	int retval = UPDATERES_ERROR;
	

	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("error connecting to %s:%s\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(buf,	"GET %s?userid=%s&userpwd=%s&ipaddress=%s HTTP/1.1\r\n"
						"HOST: %s\r\n"
 						"User-Agent: %s\r\n"
						"Connection: close\r\n\r\n", 
						info->service->default_request,
						info->usrname,
						info->usrpwd,
						MY_IP_STR,
						info->service->default_server,
						DEVICE_NAME
						);
						
	bp += len;
	*bp = '\0';
	output(fd, buf);        		
       printf("\r\nsend message:\r\n%s\r\n",buf);
	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
	printf("\r\n%s\r\n",buf);
	temp=(char*)strstr(buf,"\r\n\r\n");
	printf("\r\ntemp=%s\r\nooo=%c",temp+4,*(temp+4));
	if(!temp){
	    retval=UPDATERES_ERROR;
	    //printf("Server return msg error:%s",buf);
	    goto error;
	}
	if(strncmp(temp+4,"host",4)==0){
	    retval=UPDATERES_OK;
	}else{
	    //printf("msg:%s\n",buf);
	    retval=UPDATERES_ERROR;
	}
error:
	free(buf);
	return retval;
}
#endif

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int TZO_update_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char *p;

  	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}

	len = sprintf(buf,	"GET %s?TZOName=%s&Email=%s&TZOKey=%s&IPAddress=%s&"
						"HTTP/1.0\r\n"
						"User-Agent: %s-%s %s [%s]\r\n"
						"Host: %s\r\n\r\n",
						info->service->default_request,
						info->host,
						info->usrname,
	            		info->usrpwd,
	            		MY_IP_STR,
	            		DEVICE_NAME, FW_VERSION, FW_OS, AUTHOR,
	            		info->service->default_server);
	
	bp += len;
	
	*bp = '\0';
	output(fd, buf);       		
  
	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
	//printf("TZO_update_entry: buf:%s\n\n", buf);

#if 1	//by chenfei 2012-7-25
	if( (p = strstr(buf, "TZO Name:")) == NULL )
	{
		printf("not find!\n");
		ret = 0;
		goto error;	
	}
	else
	{
		ret = 1;	
	}
	
  	//printf("*************\nret:%d\n", ret);
  	switch(ret)
  	{
  		case TZO_SUCCESS:
  			printf("successfully!\n");
			retval = UPDATERES_OK;
  			break;
  			
  		default:
  			printf("authentication failure!\n");
  			retval = UPDATERES_SHUTDOWN;
			break;
  	}
#else
	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
  	{
	    ret = -1;
  	}  	
	//printf("ret=%d\n", ret);
	switch(ret)
	{
	case -1:
		printf("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		printf("successfully!\n");
		retval = UPDATERES_OK;
		break;

	case 302:
		printf("authentication failure!\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
#endif	

error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int NOIP_update_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char *p;

  	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
  	bp = buf;
  	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("error connecting to %s\n", info->service->default_server);
		retval = UPDATERES_ERROR;
		goto error;
	}
		
	len = sprintf(buf,"GET http://%s/update.php?username=%s&pass=%s&host=%s"
	            "&ip=%s HTTP/1.0\nUser-Agent: %s DUC %s\n\n",
	            info->service->default_request,
	            info->usrname,
	            info->usrpwd,
	            info->host,
	            MY_IP_STR,
	            DEVICE_NAME,
	            FW_VERSION);
  	
  	bp += len;
	
	*bp = '\0';
	output(fd, buf); 
	
  	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
  	//printf("NOIP_update_entry: buf:%s\n", buf);

	if( (p = strstr(buf, "status=")) == NULL )
	{
		printf("not find status!\n");
		goto error;	
	}
	else
	{
		//printf("get string!!!!! p:%s p[7]:%s\n", p, &p[7]);
		ret = atoi(&p[7]);	
	}
		
  	//printf("ret:%d\n", ret);
  	switch(ret)
  	{
  		case NOIP_SUCCESS:
  		case NOIP_ALREADYSET:
  			printf("successfully!\n");
			retval = UPDATERES_OK;
  			break;
  			
  		default:
  			printf("authentication failure!\n");
  			retval = UPDATERES_SHUTDOWN;
			break;
  	}
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int DTDNS_update_entry(struct userInfo_cxy *info)
{	
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp =  buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(buf,"GET %sid=%s&pw=%s&ip=%s HTTP/1.0\r\n"
	            "Host: www.dtdns.com\r\n"
	            "User-Agent: %s\r\n"
	            "\r\n"
	            "Connection: Keep-Alive", 
	            info->service->default_request,
	            info->host,
	            info->usrpwd,
		        MY_IP_STR,
		        DEVICE_NAME);	
		
	bp += len;
	
	*bp = '\0';
	output(fd, buf); 	  	  	
	
  	bp = buf;
  	bytes = 0;
  	btot = 0;  	  	
  	
  	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
  	
  	close(fd);
  	buf[btot] = '\0';
  	
  	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
  	{
	    ret = -1;
  	}  	
  	
	switch(ret)
  	{
    case -1:
    	printf("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

    case 200:
		retval = UPDATERES_OK;
		break;

    case 401:		
      	printf("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

    default:
		retval = UPDATERES_ERROR;
		break;
	}  	
	
error:
	free(buf);
	return retval;
}

//------------------------------------------------------------------------------
// FUNCTION
//
//
// DESCRIPTION
//
//  
// PARAMETERS
//
//  
// RETURN
//
//  
//------------------------------------------------------------------------------
int CHANGEIP_update_entry(struct userInfo_cxy *info)
{
	char *buf, *bp;
	int bytes, btot, ret, fd, len;
	int retval = UPDATERES_ERROR;
	char user[64], auth[64];
	
	len = sprintf(user,"%s:%s", info->usrname , info->usrpwd);
	base64_encode(user, len, auth, 64);
	
	buf = (char *)malloc(BUFFER_SIZE);
	if (buf == 0)
		return UPDATERES_ERROR;
	
	bp = buf;
	
	if(do_connect(&fd, info->service->default_server, info->service->default_port) != 0)
	{
		printf("error connecting to %s:%d\n", info->service->default_server, info->service->default_port);
		retval = UPDATERES_ERROR;
		goto error;
	}
	
	len = sprintf(bp, "GET %s?system=dyndns&", info->service->default_request);
	bp += len;
	
	len = sprintf(bp, "hostname=%s&", info->host);
	bp += len;
	
	if(MY_IP)
	{
		len = sprintf(bp, "myip=%s&", MY_IP_STR);
		bp += len;
	}
	
	len = sprintf(bp, "wildcard=%s&", info->wildcard ? "ON" : "OFF");
	bp += len;
 	
	if(*(info->mx) != '\0')
	{
		len = sprintf(bp, "mx=%s&", info->mx);
		bp += len;
	}
  	
	len = sprintf(bp,	" HTTP/1.0\r\n"
 						"Authorization: Basic %s\r\n"
 						"User-Agent: %s-%s %s [%s]\r\n"
 						"Host: %s\r\n\r\n",
 						auth,
						DEVICE_NAME, FW_VERSION, FW_OS, "",
						info->service->default_server);
 						
	bp += len;
	
	*bp = '\0';
	
	
	output(fd, buf);
	
	bp = buf;
	bytes = 0;
	btot = 0;
	while((bytes=read_input(fd, bp, BUFFER_SIZE-btot)) > 0)
	{
		bp += bytes;
		btot += bytes;
	}
	close(fd);
	buf[btot] = '\0';

	if(sscanf(buf, " HTTP/1.%*c %3d", &ret) != 1)
	{
		ret = -1;
	}

	switch(ret)
	{
	case -1:
		printf("strange server response, are you connecting to the right server?\n");
		retval = UPDATERES_ERROR;
		break;

	case 200:
		if(strstr(buf, "Successful Update!") != NULL)
		{
			retval = UPDATERES_OK;
		}
		else
		{
			retval = UPDATERES_ERROR;
		}
      break;
		break;

	case 401:
		printf("authentication failure\n");
		retval = UPDATERES_SHUTDOWN;
		break;

	default:
		retval = UPDATERES_ERROR;
		break;
	}
	
error:
	free(buf);
	return retval;
}
	
