/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res, m=0;
    struct termios oldtio,newtio;
    char buf[255], buffer[255]="", buff[5][255];

    if ( (argc < 2) ||
         ((strcmp("/dev/ttyS10", argv[1])!=0) &&
          (strcmp("/dev/ttyS11", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }


    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd < 0) { perror(argv[1]); exit(-1); }

    if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
    */


    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");
    
    while (STOP==FALSE) {       
        res = read(fd,buf,1);  
        
        if(buf[0]==' ')
        {
            strcpy(&buff[m],buffer);
            strcpy(buffer,"");
            m++;    
            continue;
        }
      
        strncat(buffer, buf, 1);
        if (buf[0]=='\0')
        { 
            strcpy(&buff[m],buffer);
            m++;   
            STOP=TRUE;
        }    
    }


    if(strcmp(&buff[0],"5c")!=0)
    {
        printf("0");
        close(fd);
        return -1;
    }

    if(strcmp(&buff[1],"1")!=0)
    {
        printf("1");
        close(fd);
        return -1;
    }

    if(strcmp(&buff[2],"3")!=0)
    {
        printf("2");
        close(fd);
        return -1;
    }

    if(strcmp(&buff[3],"2")!=0)
    {
        printf("3");
        close(fd);
        return -1;
    }

    if(strcmp(&buff[4],"5c")!=0)
    {
        printf("4");
        close(fd);
        return -1;
    }

    unsigned char C= 0x07;
    //unsigned char Bcc1= buff[1]^C;
    sprintf(buffer, "%s %s %x %s %s",buff[0],buff[1],C,buff[3],buff[4]);
    
    for(int i=0; i<m; i++)
    {
        printf("%s\n",buff[i]);    
    }

    buffer[strlen(buffer)+1]='\0';
    write(fd,buffer,255);


    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
