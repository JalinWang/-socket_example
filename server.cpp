#include <iostream>

#include <cstring>

extern "C" {
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <unistd.h>
}

using namespace std;
const int kPort = 8000;
// const int kPort = 80;

void error_handle(const char *func_name)
{
    // perror("error on: ");
    fprintf(stderr, "error in %s : \n", func_name);
    fprintf(stderr, "errno: %d, %s\n", errno, strerror(errno));
    // fputc('\n', stderr);
    exit(1);
}

int main()
{
    // int s = socket(AF_INET, SOCK_STREAM, 0); // choose protocol auto
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in server_sockaddr = {
        .sin_family = AF_INET,
        .sin_port = htons(kPort),
        // .sin_addr = {.s_addr = htonl(0x01020304)}
        .sin_addr = {.s_addr = htonl(INADDR_ANY)}
    };

    // len is required when sockaddr is moving to kernel space 
    // asmlinkage long sys_bind(int fd, struct sockaddr __user *umyaddr, int addrlen) 
    // move_addr_to_kernel(umyaddr, addrlen, &address);
    if (bind(s, reinterpret_cast<sockaddr *>(&server_sockaddr), sizeof(server_sockaddr)) == -1) {
        error_handle("bind()");
    }

    // 第二个参数：连接请求的队列长度，如果为6，表示队列中最多同时有6个连接请求。
    // 这里需要注意的是，listen()函数不会阻塞，它主要做的事情为，将该套接字和套接字对应的连接队列长度告诉 Linux 内核，然后，listen()函数就结束。
    if (listen(s, 6) == -1) {
        error_handle("listen()");
    }

    cout << "---start listening---" << endl;

    sockaddr_in client_sockaddr;
    socklen_t client_sockaddr_size;

    const int BUF_SIZE = 128;
    char buffer[BUF_SIZE];

    for (int i = 0; i < 3; ++i) {
        int client_socket_fd = accept(s, reinterpret_cast<sockaddr *>(&client_sockaddr), &client_sockaddr_size);
        if (client_socket_fd == -1) {
            error_handle("accept()");
        }
        cout << "\nclient id: " << i << endl;

        int len;
        cout << "Message from client:" << endl;
        while ((len = read(client_socket_fd, buffer, BUF_SIZE - 1)) != 0) {
            if (len == -1)
                error_handle("read() from socket");
            buffer[len] = '\0'; ///坑！！！！！
            cout << buffer;
            len = sprintf(buffer, "recievde %d bytes", len);
            
            ///坑！！！！！
            //但是循环过程中的buf参数和nbytes参数是我们自己来更新的，
            //也就是说，网络编程中写函数是不负责将全部数据写完之后再返回的，说不定中途就返回了！
            int writed_len;
            char *p = buffer;
            while ((writed_len = write(client_socket_fd, p, len)) != -1 && (len -= writed_len) != 0)
                p += writed_len;
            if (writed_len == -1)
                error_handle("write() to socket");
        }
        cout << endl;
        printf("client %d disconnected\n", i);
        close(client_socket_fd);
    }
    cout << "---stop listening---" << endl;
    close(s);
    return 0;
}