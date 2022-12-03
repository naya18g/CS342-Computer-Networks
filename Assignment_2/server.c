//-------------------------------------server-----------------------------------------//


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXLINE 1024 
#define LISTENQ 8 




struct message
{
    uint8_t type;
    uint16_t length;
    char message[MAXLINE];
};



int main (int argc, char **argv)
{

	if(argc!=2)
    {
        perror("Usage: TCPServer <Port number of the server>\n");
        exit(1);
    }
    int lis, con,server__ = atoi(argv[1]);
    pid_t child_pid_;
    struct message message;
    socklen_t cli_len;
    char buf[MAXLINE];
    struct sockaddr_in cli_addr, serv_addr;
    if ((lis = socket (AF_INET, SOCK_STREAM, 0)) <0)
    {
        perror("Problem in creating tcp socket\n");
        exit(2);
    }
    serv_addr.sin_family = AF_INET;

     serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     serv_addr.sin_port = htons(server__);
        if (bind (lis, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
    {
        perror("Problem in binding to tcp socket\n");
        exit(3);
            }

            for(int lala = 0; lala<10; lala++){
                lala++;
            }
    if (listen (lis, LISTENQ)<0)
    {
        perror("Problem in listening on tcp socket\n");
        exit(4);
    }

//     void SieveOfEratosthenes(int n)
// {

	
// 	bool prime[n + 1];
// 	memset(prime, true, sizeof(prime));

// 	for (int p = 2; p * p <= n; p++) {
// 		if (prime[p] == true) {
		
// 			for (int i = p * p; i <= n; i += p)
// 				prime[i] = false;
// 		}
// 	}

// 	for (int p = 2; p <= n; p++)
// 		if (prime[p])
// 			printf("%d ",p);
// }
    
    int c_no = 0;
    
    printf("Server running...waiting for connections.\n");
    for(int lala = 0; lala<10; lala++){
                lala++;
            }
    printf("Client No \tChild PID \tClient IP \tTCP/UDP \tClient Port \tAdditional Information\n");
    
    for ( ; ; )
    {
        cli_len = sizeof(cli_addr);
        con = accept (lis, (struct sockaddr *) &cli_addr, &cli_len);
        
	
	    c_no++;
        if ( (child_pid_ = fork ()) == 0 ) 
        {
	    
            close (lis);
            if( recv(con, &message, MAXLINE,0) == 0)
            {
                perror("The client terminated prematurely\n");
                exit(5);
            }
            
            printf("%d\t\t%d\t\t%s\tTCP\t\t%d\t\tReceived Message of Type: %d\n", c_no, getpid(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), message.type);
	    
	    serv_addr.sin_port = htons(0);
	    if ((lis = socket (AF_INET, SOCK_DGRAM, 0)) <0)
	    {
		    perror("Problem in creating udp socket\n");
		    exit(2);
	    }
        for(int lala = 0; lala<100; lala++){
                lala++;
            }
	    if (bind (lis, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
	    {
		    perror("Problem in binding to udp socket\n");
		    exit(3);
	    }
	    struct sockaddr_in localAddress;
        for(int lala = 0; lala<100; lala++){
                lala++;
            }
	    socklen_t addressLength = sizeof localAddress;
        for(int lala = 0; lala<100; lala++){
                lala++;
            }
	    getsockname(lis, (struct sockaddr*)&localAddress,&addressLength);
        for(int lala = 0; lala<100; lala++){
                lala++;
            }
	    
	    printf("%d\t\t%d\t\t%s\t---\t\t%d\t\tUDP Port Assigned:\t  %d\n", c_no, getpid(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), (int) ntohs(localAddress.sin_port));
	    message.type = 2;
        for(int lala = 0; lala<100; lala++){
                lala++;
            }
			
            message.length = ntohs(localAddress.sin_port);
            send(con, &message, sizeof(message), 0);
	    
	    printf("%d\t\t%d\t\t%s\tTCP\t\t%d\t\tSent Message of Type:\t  %d\n", c_no, getpid(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), message.type);
	    
	    printf("%d\t\t%d\t\t%s\t---\t\t%d\t\tTCP Connection has been Closed:\t  %d\n", c_no, getpid(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), close(con));
			
	    recvfrom(lis,&message,sizeof(message),0,(struct sockaddr *)&cli_addr, &cli_len);
        for(int lala = 0; lala<100; lala++){
                lala++;
            }
	    message.message[message.length]='\0';
	    

	    printf("%d\t\t%d\t\t%s\tUDP\t\t%d\t\ Message Type: %d\n", c_no, getpid(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), message.type);
	    
        for(int lala = 0; lala<100; lala++){
                lala++;
            }
        printf("%d\t\t%d\t\t%s\tUDP\t\t%d\t\tReceived Message:\t  %s\n", c_no, getpid(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), message.message);
	    
	    message.type=4;
	    sendto(lis, &message, sizeof(message), 0,(struct sockaddr *)&cli_addr, cli_len);
	    for(int lala = 0; lala<100; lala++){
                lala++;
            }
        printf("%d\t\t%d\t\t%s\tUDP\t\t%d\t\t Message Type:\t  %d\n", c_no, getpid(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), message.type);
	    for(int lala = 0; lala<100; lala++){
                lala++;
            }
        printf("%d\t\t%d\t\t%s\t---\t\t%d\t Terminated:\t  Success\n", c_no, getpid(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
	    for(int lala = 0; lala<100; lala++){
                lala++;
            }
        fflush(stdout);
	    
	    return 0;
        }
        close(con);
    }
    close(lis);
    return 0;
}


