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
        .sin_addr = {.s_addr = (inet_addr("127.0.0.1"))}
    };
    // server_sockaddr.sin_addr.s_addr = htonl(inet_addr("192.168.0.1"));

    if (connect(s, reinterpret_cast<sockaddr *>(&server_sockaddr), sizeof(server_sockaddr)) == -1) {
        error_handle("connect()");
    }

    cout << "---connected---" << endl;

    const int BUF_SIZE = 128;
    char buffer[BUF_SIZE];

    while(1) {
        cout << "Input Q/q to exit." << endl;
        if (!fgets(buffer, BUF_SIZE, stdin))
            error_handle("fgets()");
        if(!strcmp(buffer, "q\n") || !strcmp(buffer, "Q\n"))
            break;
        int len = strlen(buffer);
        int writed_len;
        char *p = buffer;
        while ((writed_len = send(s, p, strlen(buffer), 0)) != -1 && (len -= writed_len) != 0)
            p += writed_len;
        if (writed_len == -1)
            error_handle("send() to socket");
        
        int str_len = read(s, buffer, BUF_SIZE - 1);
        buffer[str_len] = '\0'; ///坑！！！！！
        printf("Message from server : %s\n", buffer);

    }
    cout << "---shutdown---" << endl;
    close(s);
    return 0;
}