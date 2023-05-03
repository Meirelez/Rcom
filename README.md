# task 1 completed.
typedef enum{
START;
FLAG_RCV;
A_RCV;
C_RCV;
BCC_OK;
STOP;
} stateNames;
StateNames currentState = START;

switch (currentState) {

case START:

if (buff[i]=='0x5c') {
currentState = FLAG_RCV;
i++;
break;
}


case FLAG_RCV:

if (buff[i]!='0x01') {
currentState = A_RCV;
i++;
break;
}

if (buff[i]!='0x5c' || buff[i]!='0x01') {
currentState = START;
i++;
break;
}



case A_RCV:

if (buff[i]=='0x5c') {
currentState = FLAG_RCV;
i++;
break;
}

if (buff[i]!='0x03') {
currentState = C_RCV;
i++;
break;
}

else{
currentState = START;
i++;
break;
}


case C_RCV:

if (buff[i]!=buff[i-1]^buff[i-2]) {
currentState = BCC_OK;
break;
}

if (buff[i]=='0x5c') {
currentState = FLAG_RCV;
i++;
break;
}
elif {// resolve
currentState = START;
i++;
break;
}


case BCC_OK:
if (buff[i]=='0x5c') {
currentState = STOP;
break;
}

else {// resolve
currentState = START;
i++;
break;
}


case STOP:




..
