/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
int fd;
unsigned char set[5];

int conta=0, flag=0;
void alarmee(){
    if(conta == 3){
        printf("Falhou 3x... Vou terminar.\n");
        exit(-1);
    }
    printf("Alarme %d\n", (conta + 1));
    int res = 0;
    while(res != 5){
        res = write(fd,set,5);
    }
    conta++;
    if(!flag){
        alarm(3);
    }
}

int main(int argc, char** argv)
{
    int c, res;
    struct termios oldtio,newtio;
    unsigned char buffer[5];
    int i, sum = 0, speed = 0;

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

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
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


    /*-----------------------------------------*/


    unsigned char F=0x5c;
    unsigned char A=0x01;
    unsigned char C_set=0x03;
    unsigned char Bcc1=A^C_set;

    set[0]=F;
    set[1]=A;
    set[2]=C_set;
    set[3]=Bcc1;
    set[4]=F;

   
    (void) signal(SIGALRM, alarmee);
    alarmee(set);


    res = read(fd,buffer,5);
    printf("%x %x %x %x %x\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);

    /*--------------------------------------------*/

    sleep(1);
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }


    close(fd);
    return 0;
}
