/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#define SBUFSIZE 1000
#define NTHREADS 100

typedef struct {
    int *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
}sbuf_t;
sbuf_t sbuf;
static sem_t mutex;

typedef struct stock{
    int ID;
    int left_stock;
    int price;
    int readcnt;
    struct stock* left;
    struct stock* right;
    sem_t mutex, w;
}Stock;
Stock *root = NULL;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);
void *thread(void *vargp);
static void init_echo_cnt(void);
void check_client(int connfd);
//void echo(int connfd);

/*stock functions*/
Stock* insertstock(Stock *node, int ID, int left_stock, int price);
void readstocks();
void writestocks(Stock *node, FILE *wfp);
Stock* searchstock(Stock *node, int ID);
void showstock(Stock *node, char* stock_info, int connfd);
void buystock(int ID, int cnt, int connfd);
void sellstock(int ID, int cnt, int connfd);
void freestocks(Stock* node);
void signal_handler(int sign);

int main(int argc, char **argv) 
{
    Signal(SIGINT, signal_handler);
     
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    pthread_t tid;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(0);
    }
    readstocks();

    listenfd = Open_listenfd(argv[1]);
    sbuf_init(&sbuf, SBUFSIZE);
    for (int i = 0; i < NTHREADS; i++)
        Pthread_create(&tid, NULL, thread, NULL);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage); 
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        //printf("1\n"); //확인용
        sbuf_insert(&sbuf, connfd);
    }
    FILE *wfp = fopen("stock.txt", "w");
    if (wfp == NULL) {
        perror("Error : open file\n");
        exit(0);
    }
    writestocks(root, wfp);
    fclose(wfp);

    exit(0);
}

void *thread(void *vargp) {
    Pthread_detach(pthread_self());
    while (1) {
        //printf("1\n"); //확인용
        int connfd = sbuf_remove(&sbuf);
        check_client(connfd);
        Close(connfd);
    }
}

static void init_echo_cnt(void) {
    Sem_init(&mutex, 0 , 1);
}

void check_client(int connfd) {
    int n;
    char buf[MAXLINE];
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    char cmd[10];

    Pthread_once(&once, init_echo_cnt);
    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        int ID, cnt;
        char stock_info[MAXLINE];
        P(&mutex);
        printf("server received %d bytes\n", n);
        V(&mutex);
        //printf("1\n"); //확인용
        sscanf(buf, "%s %d %d", cmd, &ID, &cnt);
        if (!strcmp(cmd, "show")) {
            stock_info[0] = '\0';
            showstock(root, stock_info, connfd);
            stock_info[strlen(stock_info)] = '\0';
            Rio_writen(connfd, stock_info, MAXLINE);
        }
        else if (!strcmp(cmd, "exit")) {
            Rio_writen(connfd, buf, strlen(buf));
            break;
        }
        else if (!strcmp(cmd, "buy")) {
            buystock(ID, cnt, connfd);
        }
        else if (!strcmp(cmd, "sell")) {
            sellstock(ID, cnt, connfd);
        }
    }
    FILE *wfp = fopen("stock.txt", "w");
    if (wfp == NULL) {
        perror("Error : open file\n");
        exit(0);
    }
    writestocks(root, wfp);
    fclose(wfp);
}

void sbuf_init(sbuf_t *sp, int n) {
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    Sem_init(&sp->mutex, 0, 1);
    Sem_init(&sp->slots, 0, n);
    Sem_init(&sp->items, 0, 0);
}

void sbuf_deinit(sbuf_t *sp) {
    Free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item) {
    P(&sp->slots);
    P(&sp->mutex);
    sp->buf[(++sp->rear)%(sp->n)] = item;
    V(&sp->mutex);
    V(&sp->items);
}

int sbuf_remove(sbuf_t *sp) {
    int item;
    P(&sp->items);
    P(&sp->mutex);
    item = sp->buf[(++sp->front)%(sp->n)];
    V(&sp->mutex);
    P(&sp->slots);
    return item;
}

Stock* insertstock(Stock *node, int ID, int left_stock, int price) {
    if (node == NULL) {
        Stock *newnode = (Stock*)malloc(sizeof(Stock));
        newnode->ID = ID;
        newnode->left_stock = left_stock;
        newnode->price = price;
        newnode->readcnt = 0;
        newnode->left = NULL; newnode->right = NULL;
        Sem_init(&(newnode->mutex), 0, 1);
        Sem_init(&(newnode->w), 0, 1);
        return newnode;
    }
    if (ID < node->ID) 
        node->left = insertstock(node->left, ID, left_stock, price);
    else if (ID > node->ID) 
        node->right = insertstock(node->right, ID, left_stock, price);
    return node;
}

void readstocks() {
    FILE *fp = fopen("stock.txt", "r");
    if (fp == NULL) {
        perror("Error : open file\n");
        exit(0);
    }
    int ID, left_stock, price;
    while (fscanf(fp, "%d %d %d", &ID, &left_stock, &price) != EOF) {
        root = insertstock(root, ID, left_stock, price);
    }
    fclose(fp);
}
void writestocks(Stock *node, FILE *wfp) {
    if (node == NULL) return ;
    writestocks(node->left, wfp);
    fprintf(wfp, "%d %d %d\n", node->ID, node->left_stock, node->price);
    writestocks(node->right, wfp);
}

Stock* searchstock(Stock *node, int ID) {
    //if (node == NULL) return node;
    if (node == NULL || node->ID == ID) {
        return node;
    }
    if (ID < node->ID) 
        return searchstock(node->left, ID);
    return searchstock(node->right, ID);
}

void showstock(Stock *node, char* stock_info, int connfd) {
    if (node == NULL) return;
    char buffer[MAXLINE];
    showstock(node->left, stock_info, connfd);
    P(&node->mutex);
    node->readcnt++;
    if (node->readcnt == 1)
        P(&node->w);
    V(&node->mutex);

    //P(&node->w);
    sprintf(buffer, "%d %d %d\n", node->ID, node->left_stock, node->price);
    strcat(stock_info, buffer);
    //V(&node->w);
    
    P(&node->mutex);
    node->readcnt--;
    if (node->readcnt == 0)
        V(&node->w);
    V(&node->mutex);
    showstock(node->right, stock_info, connfd);
}

void buystock(int ID, int cnt, int connfd) {
    Stock *stock = searchstock(root, ID);
    char buffer[MAXLINE];
    if (!stock) return;
    if (stock->left_stock < cnt) {
        strcpy(buffer, "Not enough left stocks\n");
        Rio_writen(connfd, buffer, MAXLINE);
    }
    else {
        P(&stock->w);
        stock->left_stock -= cnt;
        V(&stock->w);
        strcpy(buffer, "[buy] success\n");
        Rio_writen(connfd, buffer, MAXLINE);
    }
}

void sellstock(int ID, int cnt, int connfd) {
    Stock *stock = searchstock(root, ID);
    char buffer[MAXLINE];
    if (!stock) return;
    P(&stock->w);
    stock->left_stock += cnt;
    V(&stock->w);
    strcpy(buffer, "[sell] success\n");
    Rio_writen(connfd, buffer, MAXLINE);
}

void freestocks(Stock *node) {
    if (node == NULL) return;
    freestocks(node->left);
    free(node);
    freestocks(node->right);
}

void signal_handler(int sign) {
    freestocks(root);
    exit(0);
}   


/* $end stockserverimain */
