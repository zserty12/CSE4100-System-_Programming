/* $begin stockserverimain */
#include "csapp.h"

typedef struct {
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];
    rio_t clientrio[FD_SETSIZE];
}pool;

typedef struct stock{
    int ID;
    int left_stock;
    int price;
    //int readcnt;
    struct stock* left;
    struct stock* right;
}Stock;
Stock *root = NULL;
//char stock_info[MAXLINE];

void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients(pool *p);
void echo(int connfd);
int client_cnt = 0;

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
//void print(Stock *node); 

int main(int argc, char **argv) 
{
    Signal(SIGINT, signal_handler);

    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    static pool pool;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
    }
    readstocks();

    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);
    while (1) {
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &pool.ready_set)) {
            clientlen = sizeof(struct sockaddr_storage); 
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                        client_port, MAXLINE, 0);
            printf("Connected to (%s %s)\n", client_hostname, client_port);
            add_client(connfd, &pool);
        }
        check_clients(&pool);
        if (client_cnt == 0) {
            FILE *wfp = fopen("stock.txt", "w");
            if (wfp == NULL) {
                perror("Error : open file\n");
                exit(0);
            }
            writestocks(root, wfp);
            fclose(wfp);
        }
    }
    exit(0);
}

void init_pool(int listenfd, pool *p) {
    int i;
    p->maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        p->clientfd[i] = -1;
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p) {
    int i;
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++) {
        if (p->clientfd[i] < 0) {
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->clientrio[i], connfd);
            FD_SET(connfd, &p->read_set);
            if (connfd > p->maxfd)
                p->maxfd = connfd;
            if (i > p->maxi)
                p->maxi = i;
            client_cnt++;
            //printf("%d\n",client_cnt);
            break;
        }    
    }
    if (i == FD_SETSIZE)
        app_error("add_client error : Too many clients");
}

void check_clients(pool *p) {
    int i, connfd, n;
    char buf[MAXLINE];
    char cmd[10];
    rio_t rio;
    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
        connfd = p->clientfd[i];
        rio = p->clientrio[i];
        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) {
            p->nready--;
            if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
                int ID, cnt;
                char stock_info[MAXLINE];
                printf("server received %d bytes\n", n);
                sscanf(buf, "%s %d %d", cmd, &ID, &cnt);
                if (!strcmp(cmd, "show")) {
                    stock_info[0] = '\0';
                    showstock(root, stock_info, connfd);
                    stock_info[strlen(stock_info)] = '\0';
                    Rio_writen(connfd, stock_info, MAXLINE);
                }
                else if (!strcmp(cmd, "exit")) {
                    Rio_writen(connfd, buf, strlen(buf));
                }
                else if (!strcmp(cmd, "buy")) {
                    buystock(ID, cnt, connfd);
                }
                else if (!strcmp(cmd, "sell")) {
                    sellstock(ID, cnt, connfd);
                }
            }
            else {
                Close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
                client_cnt--;
                //printf("%d\n",client_cnt);
            }
        }
    } 
}

Stock* insertstock(Stock *node, int ID, int left_stock, int price) {
    if (node == NULL) {
        Stock *newnode = (Stock*)malloc(sizeof(Stock));
        newnode->ID = ID;
        newnode->left_stock = left_stock;
        newnode->price = price;
        newnode->left = NULL; newnode->right = NULL;
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
    if (node == NULL || node->ID == ID) 
        return node;
    if (ID < node->ID) 
        return searchstock(node->left, ID);
    return searchstock(node->right, ID);
}

void showstock(Stock *node, char* stock_info, int connfd) {
    if (node == NULL) return;
    char buffer[MAXLINE];
    showstock(node->left, stock_info, connfd);
    sprintf(buffer, "%d %d %d\n", node->ID, node->left_stock, node->price);
    strcat(stock_info, buffer);
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
        stock->left_stock -= cnt;
        strcpy(buffer, "[buy] success\n");
        Rio_writen(connfd, buffer, MAXLINE);
    }
}

void sellstock(int ID, int cnt, int connfd) {
    Stock *stock = searchstock(root, ID);
    char buffer[MAXLINE];
    if (!stock) return;
    stock->left_stock += cnt;
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
