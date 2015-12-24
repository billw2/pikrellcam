/*
 * MJPEG TCP server
 * Brijesh Singh <brijesh.ksingh@gmail.com>
 *
 * Live preview of the MJPEG Camera Stream. The stream can be viewed directly on browser
 * by accessing http://<YOUR_IP>:8081/mjpeg_live.php or Andriod MJPEG viewer apps
 * (tinycam monitor etc).
 *
 * Eventhough the server is designed to work with single producer and consumer mind but we do
 * not refuse the connection from more than one clients.
 */

#include "pikrellcam.h"

#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#define MAX_WIDTH	1920
#define MAX_HEIGHT	1080
#define MAX_IMAGE_SIZE	(MAX_WIDTH * MAX_HEIGHT * 1.5) /* worst case compression */
#define SERVER_PORT	9999
#define MAX_BUF_SIZE	1024
#define NUM_CIRC_BUFS	2

struct buffer {
	int len;
	char *data;
};

struct client_info {
	int fd;
	struct sockaddr_in sockaddr;
};

static int server_fd;
static int head, tail;
static pthread_t handler_tid;
static struct buffer buffers[NUM_CIRC_BUFS];
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* returned a new buffer, after use the buffer must be free'd with image_buffer_free() */
static struct buffer* client_queue_get()
{
	struct buffer *buf;

	buf = calloc(1, sizeof(*buf));
	if (!buf)
		return NULL;

	buf->data = malloc(MAX_IMAGE_SIZE);
	if (!buf->data) {
		free(buf);
		return NULL;
	}

	pthread_mutex_lock(&mutex);
	if (head == tail) /* queue is empty */
		pthread_cond_wait(&cond, &mutex);
	buf->len = buffers[head].len;
	memcpy(buf->data, buffers[head].data, buf->len);
	head = (head + 1) % NUM_CIRC_BUFS;
	pthread_mutex_unlock(&mutex);

	return buf;
}

static void image_buffer_free(struct buffer *buf)
{
	if (buf) {
		free(buf->data);
		free(buf);
	}
}

static void* handle_client(void *args)
{
	char *data = NULL;
	int i, fd;
	struct client_info *client = args;
	char header[MAX_BUF_SIZE];
	struct buffer *buf = NULL;

	log_printf("new connection from host '%s' on port '%d'\n",
		inet_ntoa(client->sockaddr.sin_addr),
		ntohs(client->sockaddr.sin_port));

	/* We had delayed the circular buffer allocation until atleast one client is
	 * connected to the socket. let allocate the buffer now */
	for (i = 0; i < NUM_CIRC_BUFS; i++) {
		/* if the buffer is already allocated by some other client then share it */
		if (!buffers[i].data) {
			buffers[i].data = malloc(MAX_IMAGE_SIZE);
			if (!buffers[i].data) {
				log_printf("failed to allocate memory %s\n", strerror(errno));
				goto failed;
			}
		}
	}
	
	fd = client->fd;

	while(1) {
		buf = client_queue_get();
		if (!buf)
			goto failed;

		/* send JPEG boundary start header */
		memset(header, '\0', MAX_BUF_SIZE);
		snprintf(header, MAX_BUF_SIZE,
				"\r\n--fooboundary\r\n"
				"Content-type: image/jpeg\r\n\r\n");
		if (send(fd, header, strlen(header), MSG_NOSIGNAL) < 0)
			goto failed;

		/* send image contents */
		if (send(fd, buf->data, buf->len, MSG_NOSIGNAL) < 0)
			goto failed;

		/* we are done with the image buffer */
		image_buffer_free(buf);
	}
failed:
	log_printf("closing connection from host '%s' on port '%d'\n",
		inet_ntoa(client->sockaddr.sin_addr),
		ntohs(client->sockaddr.sin_port));

	image_buffer_free(buf);

	if (data)
		free(data);
	free(client);
	pthread_detach(pthread_self());
	return NULL;
}

static void add_new_connection (int server_fd)
{
	int fd;
	pthread_t tid;
	socklen_t size;
	struct sockaddr_in client;
	struct client_info *c;

	size = sizeof(client);
	fd = accept(server_fd, (struct sockaddr*)&client, &size);
	if (fd < 0) {
		log_printf("failed to accept");
		return;
	}

	c = calloc(1, sizeof(*c));
	if (!c)
		return;

	memcpy(&c->sockaddr, &client, sizeof(struct sockaddr_in));
	c->fd = fd;
	pthread_create(&tid, NULL, handle_client, (void*)c);

	return;
}

static int create_sock (int port)
{
	struct sockaddr_in server;
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		log_printf("socket create failed: %s\n", strerror(errno));
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		log_printf("socket bind failed: %s\n", strerror(errno));
		return 1;
	}

	if (listen(server_fd, 3) < 0) {
		log_printf("socket listen failed: %s\n", strerror(errno));
		return 1;
	}

	log_printf("MJPEG server is listening on port '%d'\n", port);
	return 0;
}

static void* handler(void *unused)
{
	int i;
	fd_set active_fd_set, read_fd_set;

	FD_ZERO(&active_fd_set);
	FD_SET(server_fd, &active_fd_set);

	while(1) {
		/* block until input arrives on one or more active sockets */
		read_fd_set = active_fd_set;
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			log_printf("select failure: %s\n", strerror(errno) );
			goto failed;
		}

		/* service all the socket with input pending */
		for(i=0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET(i, &read_fd_set)) {
				if (i == server_fd)
					add_new_connection(i);
			}
		}
	}

failed:
	pthread_detach(pthread_self());
	return NULL;
}

char* mjpeg_server_queue_get()
{
	return buffers[tail].data;
}

void mjpeg_server_queue_put(char *data, int len)
{
	pthread_mutex_lock(&mutex);
	buffers[tail].len = len;
	tail = (tail + 1) % NUM_CIRC_BUFS;
	if (tail == head) /* queue is full */
		head = head + 1;
	if (head >= NUM_CIRC_BUFS) /* if head reach to end of queue then reset it */
		head = 0;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);
}

int setup_mjpeg_tcp_server()
{
	/* create a server socket */
	if (create_sock(SERVER_PORT))
		return 1;

	/* spwan a thread to accept connections */
	pthread_create(&handler_tid, NULL, handler, NULL);

        return 0;
}

