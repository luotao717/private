#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "nvram.h"



int main(int argc, char **argv)
{

	int pingFlag=-1;
	sleep(40);
	while(1)
	{		
		
  	pingFlag=system("ping www.qq.com");
  	if( 0 != pingFlag)
  	{
  		system("gpio l 18 0 4000 0 1 4000");
  	}
 	 	else
 	 	{
  		system("gpio l 18 4000 0 1 0 4000");
  	}
  	sleep(30);
	}	
	
	return 0;

}
