#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUF_SIZE 512

int read_file(char *filename,int sock_cl)
{
    FILE *fr = fopen(filename, "w"); //vytvoreni souboru

    if (fr == NULL)
    {
        fprintf(stderr,"Chyba pri otevirani souboru\n");
        return -1;
    }

    char buff[BUF_SIZE];
    int recived = 0;
    int written = 0;

    bzero(buff,BUF_SIZE);

    while ((recived=recv(sock_cl, buff, BUF_SIZE,0)) > 0)
    {
        written = fwrite(buff, sizeof(char), recived, fr);

        if(written != recived)
        {
            fprintf(stderr,"Chyba pri zapisu do souboru\n");
            return  -1;
        }

        bzero(buff,BUF_SIZE);
    }

    if (recived < 0)
    {
        fprintf(stderr,"Chyba pri prenosu\n");
        return  -1;
    }

    fclose(fr);

    return  0;
}

int write_file(char *filename,int sock_cl)
{
    FILE *fr = fopen(filename, "r"); //otevreni souboru

    if (fr == NULL)
    {
        fprintf(stderr,"Chyba pri otevirani souboru\n");
        return -1;
    }

    long file_size;
    char buff[BUF_SIZE];
    int send_total = 0;

    fseek(fr, 0L, SEEK_END);
    file_size = ftell(fr);
    fseek(fr, 0L, SEEK_SET);
    bzero(buff,BUF_SIZE);

    while (send_total < file_size)
    {
        long send_now = 0;
        long char_read = 0;

        char_read = fread(buff, sizeof(char), BUF_SIZE, fr);

        if ((send_now = send(sock_cl,buff, char_read,0)) < 0)
        {
            fprintf(stderr,"Chyba pri odesilani souboru\n");
            return -1;
        }

        send_total += send_now;
        bzero(buff, BUF_SIZE);
    }

    fclose(fr);

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc != 7 || strcmp(argv[1], "-h") || strcmp(argv[3], "-p")
       || !(!strcmp(argv[5], "-r") || !strcmp(argv[5], "-w")))
    {
        fprintf(stderr,"Chyba v argumentech\n");
        return -1;
    }

    int my_sck, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *err;

    port = (int) strtol(argv[4],&err,10);

    if (*err != '\0' || port < 49152 || port > 65535)
    {
        fprintf(stderr,"Spatne zadany port\n");
        return -1;
    }

    my_sck = socket(AF_INET, SOCK_STREAM, 0);

    if (my_sck < 0)
    {
        fprintf(stderr,"Chyba pri otevirani socketu\n");
        close(my_sck);
        return -1;
    }
    server = gethostbyname(argv[2]);

    if (server == NULL) {
        fprintf(stderr,"Chybne zadany server\n");
        close(my_sck);
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(my_sck,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        fprintf(stderr,"Chyba pri spojeni se serverem\n");
        close(my_sck);
        return -1;
    }

    //volani funkci

    char args[BUF_SIZE];
    bzero(args,BUF_SIZE);

    if (!strcmp(argv[5], "-r"))
    {
        sprintf(args,"-r");
        sprintf(args + strlen(args),argv[6]);

        if (write(my_sck,args,BUF_SIZE) < 0)
        {
            fprintf(stderr,"Chyba pri komunikaci se serverem");
            close(my_sck);
            return -1;
        }

        read_file(argv[6],my_sck);
    }
    else
    {
        sprintf(args,"-w");
        sprintf(args + strlen(args),argv[6]);

        if (write(my_sck,args,BUF_SIZE) < 0)
        {
            fprintf(stderr,"Chyba pri komunikaci se serverem");
            close(my_sck);
            return -1;
        }

        write_file(argv[6],my_sck);
    }

    close(my_sck);
    return 0;
}