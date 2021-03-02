/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 512

int save_file(char *filename,int sock_cl)
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

int send_file(char *filename,int sock_cl)
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
    if (argc != 3 || strcmp(argv[1], "-p"))
    {
        fprintf(stderr,"Chyba v argumentech\n");
        return -1;
    }

    int my_sck, cl_sck, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char *err = NULL;

    my_sck = socket(AF_INET, SOCK_STREAM, 0);

    if (my_sck < 0)
    {
        fprintf(stderr,"Chyba pri otevirani socketu\n");
        close(my_sck);
        return -1;
    }

    portno = (int) strtol(argv[2],&err,10);
    //49152 až 65535
    if (*err != '\0' || portno < 49152 || portno > 65535)
    {
        fprintf(stderr,"Spatne zadany port\n");
        close(my_sck);
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(my_sck, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr,"Chyba pri praci se socketem\n");
        close(my_sck);
        return -1;
    }

    listen(my_sck,5);
    clilen = sizeof(cli_addr);

    while(1)
    {
        printf("\nCekani na klienta\n");
        cl_sck = accept(my_sck, (struct sockaddr *) &cli_addr, &clilen);

        if (cl_sck < 0)
        {
            fprintf(stderr,"Chyba pri otevirani socketu\n");
            close(my_sck);
        }

        printf("Klient pripojen\n");

        //volani funkci

        char args[BUF_SIZE];
        char name[255];

        bzero(args,BUF_SIZE);
        bzero(name,255);
        read(cl_sck,args,BUF_SIZE);

        if(args[1] == 'w')
        {
            strcat(name, &args[2]);
            printf("Prijem souboru %s\n",name);

            if(save_file(name,cl_sck) != 0)
                fprintf(stderr,"Chyba pri prenosu dat\n");
        }
        else
        {
            strcat(name, &args[2]);
            printf("Odeslani souboru %s\n",name);

            if(send_file(name, cl_sck) != 0)
                fprintf(stderr,"Chyba pri prenosu dat\n");
        }

        printf("Prenos probehl uspesne\nKlient bude odpojen\n");
        close(cl_sck);
    }

    close(my_sck);
    return 0;
}