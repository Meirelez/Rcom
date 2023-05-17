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
    int fd,c, res, m=0, i=0;
    struct termios oldtio,newtio;
    unsigned char buff[5];
    char buffer[255];

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


    /*-------------------------------------------------*/

 /*   while (STOP==FALSE) {
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
    }   */

    int success=0;
    res = read(fd,buff,5);
    printf("%x %x %x %x %x\n",buff[0],buff[1],buff[2],buff[3],buff[4]);

    typedef enum{
        START,
        FLAG_RCV,
        A_RCV,
        C_RCV,
        BCC_OK,
        STOP,
    } stateNames;

    stateNames currentState = START;

    while(1)
    {
        if(success)
            break;

        switch (currentState) {

            case START:
                if (buff[i]==0x5c) {
                    currentState = FLAG_RCV;
                    i++;
                }
            break;

            case FLAG_RCV:
                if (buff[i]==0x01) {
                    currentState = A_RCV;
                    i++;
                }

                else if (buff[i]!=0x5c) {
                    currentState = START;
                    i++;
                }

                else{
                    currentState=FLAG_RCV;
                    i++;
                }
            break;

            case A_RCV:
                if (buff[i]==0x03) {
                    currentState = C_RCV;
                    i++;
                }

                else if (buff[i]==0x5c) {
                    currentState = FLAG_RCV;
                    i++;
                }

                else{
                    currentState = START;
                    i++;
                }
            break;

            case C_RCV:
                if (buff[i]==(buff[i-1]^buff[i-2])) {
                    currentState = BCC_OK;
                    i++;
                }

                else if (buff[i]==0x5c) {
                    currentState = FLAG_RCV;
                    i++;
                }

                else{
                    currentState = START;
                    i++;
                }
            break;

            case BCC_OK:
                if (buff[i]==0x5c) {
                    currentState = STOP;
                }

                else {
                    currentState = START;
                    i++;
                }
            break;

            case STOP:
                success=1;
            break;
        }
    }

    printf("ola %d\n",success);

    if(success)
    {
        unsigned char ua[5];
        unsigned char C= 0x07;
        unsigned char Bcc1= buff[1]^C;

        ua[0]=buff[0];
        ua[1]=buff[1];
        ua[2]=C;
        ua[3]=Bcc1;
        ua[4]=buff[4];

        res = write(fd,ua,5);
    }
    /*---------------------------------------------------------*/

    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
