//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// TCP Stream Server for Pikrellcam
// V1.0  2015-12-07
// Thomas Götz
// 
// Goal:
// Live preview of the h264 Full HD Camera Stream
//
// Idea:
// Grab the data being written to circular buffer and send them via tcp server to a listener.
// Maybe there is a better solution, like a further Pipeline which gets the h264 live stream and sends it.
// Maybe also multiple streams possible with different resolution (full, medium, mobile, ...)
//
// use with gst-rtsp-server-1.4.4 on Raspbian Jessie (on Wheezy I didn't get gst-rtsp-server built) or 
// gst-variable-rtsp-server (look for gst-gateworks-apps-master)
// and the following pipeline (!! do-timestamp=true is important !!, blocksize=262144 optional, can be lower)
//
// ./gst-variable-rtsp-server -d 99 -p 8555 -m /stream  
//   -u "(tcpclientsrc port=3000 do-timestamp=true blocksize=262144 
//   ! video/x-h264,stream-format=byte-stream,profile=high 
//   ! h264parse ! rtph264pay name=pay0 pt=96 )" 
//
// then open the stream, e.g. vlc rtsp://your_pi_addr:8555/stream
//
// delay between live and stream is ~ 1 sec.
// CPU load with 1 active full hd stream is ~10% on a PI2 (4 Cores, 100% load is 400% displayed with 'top')
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#include "pikrellcam.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>  

#define SERV_PORT 3000  //port
#define LISTENQ 1       //maximum number of client connections

extern PiKrellCam	pikrellcam;

int listenfd, connfd, num_sent=0;
socklen_t clilen;
struct sockaddr_in cliaddr, servaddr;
long save_fd;
int h264_conn_status=H264_TCP_WAIT_FOR_CONNECT;
  
void setup_h264_tcp_server(void)
{
 //creation of the socket
 listenfd = socket (AF_INET, SOCK_STREAM, 0);

 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 //servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
 servaddr.sin_port = htons(SERV_PORT);
 
 int reuse = 1;
 if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
 {
    perror("Server: setsockopt(SO_REUSEADDR) failed\n");
    log_printf("Server: setsockopt(SO_REUSEADDR) failed\n");
    return;
 }
 
  #ifdef SO_REUSEPORT
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) 
  {
    perror("Server: setsockopt(SO_REUSEPORT) failed\n");
    log_printf("Server: setsockopt(SO_REUSEPORT) failed\n");
    return;
  }
  #endif
 
 if(bind (listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0);
 { 
//    perror("Server: error binding\n");
    log_printf("Server: error binding\n");  
    //return;
 } 

 listen (listenfd, LISTENQ);

 save_fd = fcntl( listenfd, F_GETFL );
 save_fd |= O_NONBLOCK;
 fcntl( listenfd, F_SETFL, save_fd );
 

// fprintf(stderr,"%s\n","Server running...waiting for connections.");
 log_printf("Server running...waiting for connections.\n");  
}


void tcp_poll_connect(void)
{
    if((h264_conn_status == H264_TCP_WAIT_FOR_CONNECT))
    {  
      clilen = sizeof(cliaddr);
      connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
    
      if (connfd <=0)
      {
        if (pikrellcam.debug)
          printf("Poll: Kein Client am Socket ...\n");
      }
      else
      {
        h264_conn_status=H264_TCP_SEND_HEADER; //must send header
        num_sent=0;
//        fprintf (stderr, "Server: connect from host %s, port %u.\n",
//                    inet_ntoa (cliaddr.sin_addr),
//                    ntohs (cliaddr.sin_port));
        log_printf("Server: connect from host %s, port %u.\n",
                    inet_ntoa (cliaddr.sin_addr),
                    ntohs (cliaddr.sin_port));            
      }
    }      
}


void tcp_send_h264_header(void *data, int len)
  {
     if(h264_conn_status != H264_TCP_WAIT_FOR_CONNECT)
      {        
        if(h264_conn_status == H264_TCP_SEND_HEADER) 
        {
          num_sent=send(connfd, data, len, MSG_NOSIGNAL);
          h264_conn_status=H264_TCP_SEND_DATA;
          if (pikrellcam.debug)
            printf("write h264 header:%d \n",len);
          
         if (num_sent < 0 || num_sent !=len) 
          {
            perror("Server: Client connection closed\n");
            log_printf("Server: Client connection closed\n");
            shutdown(connfd, SHUT_RDWR); 
            close(connfd);
            h264_conn_status=H264_TCP_WAIT_FOR_CONNECT;
          }
        }
      }
  }
  
 void tcp_send_h264_data(char * what, void *data, int len)
  {
    if(h264_conn_status != H264_TCP_WAIT_FOR_CONNECT)
    {  
      if(h264_conn_status == H264_TCP_SEND_DATA)
      {
        num_sent=send(connfd, data,len, MSG_NOSIGNAL);
        if (pikrellcam.debug)
          printf("write tcp %s:%d \n",what, len);
        if (num_sent < 0 || num_sent !=len) 
        {
          perror("Server: Client connection closed\n");
          log_printf("Server: Client connection closed\n");
          close(connfd);
          h264_conn_status=H264_TCP_WAIT_FOR_CONNECT;
          return;
        }
      }        
  
    }
  }
  
  
  



