#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>
#define MAX_CLIENT 3
#define max(a,b) (a>=b?a:b)
#define ALPHA 0.125 //valeur standard d'après Wikipédia
#define BETA 0.25 //valeur standard de Wikipédia


int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Pour utiliser ce programme, vous devez taper ./server <port_serveurUDP>\n");
        exit(-1);
    }
    int portBis = atoi(argv[1]);
    int portData = 5678;

    //SocketServ2
    int mySocketServ2 = socket(AF_INET,SOCK_DGRAM, 0);
       
    //afin de pouvoir réutiliser directement la socket
    int reuseBis = 1;
    setsockopt(mySocketServ2, SOL_SOCKET, SO_REUSEADDR,&reuseBis, sizeof(reuseBis)) ;
   
    //detection d'une erreur à la création de la socket
    if (mySocketServ2 < 0){
        exit(-1);
    }    
    printf("Valeur du descripteur de la socketServeurUDP : %d\n", mySocketServ2);  
   
    //remise à 0 de la structure myAddr2 et AddrClUdp
    struct sockaddr_in myAddr2 ;
    struct sockaddr_in AddrClUdp;
    memset((char*)&AddrClUdp, 0, sizeof(AddrClUdp));
    memset((char*)&myAddr2, 0, sizeof(myAddr2)) ;
    myAddr2.sin_family= AF_INET;
    //htons()=>conversion du format machine entier court vers le format réseau
    //htonl()=>conversion du format machine entier long vers le format réseau
    myAddr2.sin_port = htons(portBis);
    myAddr2.sin_addr.s_addr= htonl(INADDR_ANY) ;

    int myBind2 = bind(mySocketServ2, (struct sockaddr*)&myAddr2,sizeof(struct sockaddr_in) );
    printf("valeur du bindServeurUDP: %d\n", myBind2);
    if (myBind2 < 0){
        perror("erreur valeur du bindServeurUDP négative");
        exit(-1);
    }
    printf("le serveur fonctionne bien \n\n");

    char mySendBuffer[500];
    char myReceiveBuffer[500];
    socklen_t len = sizeof(AddrClUdp);
    fd_set mySetSocket;
    /* int sockBoard[10];
    for(int i = 0; i<10; i++){
        sockBoard[i]=0;
    } */
    //création nouvelle socket pour les données
    int mySocketData = socket(AF_INET, SOCK_DGRAM, 0);
    int reuseTer = 1;
    setsockopt(mySocketData, SOL_SOCKET, SO_REUSEADDR,&reuseTer, sizeof(reuseTer));
    if (mySocketData <0)
        exit(-1);
    printf("Valeur du descripteur de la socket Data d'UDP : %d\n", mySocketData);

    struct sockaddr_in myAddrData;
    memset((char*)&myAddrData, 0, sizeof(myAddrData));
    myAddrData.sin_family=AF_INET;
    myAddrData.sin_port=htons(portData);
    myAddrData.sin_addr.s_addr=htonl(INADDR_ANY);
    while (1)
    {   //initialisation et activation des bons bits     
        FD_ZERO(&mySetSocket);
        //FD_SET(mySocketServ, &mySetSocket);
        FD_SET(mySocketServ2, &mySetSocket);
        /* for (int i = 0; i < 10; i++){
            if(sockBoard[i]!=0){
                FD_SET(sockBoard[i], &mySetSocket);
            }
        } */

       /*  int nbMax = 0;        
        for (int i=0; i<10; i++){
            if(sockBoard[i]>nbMax)
                nbMax = sockBoard[i]; //trouver le plus grand descripteur de fichier
        }
        nbMax = max(nbMax, mySocketServ2); */
        //printf("nbMax : %d\n", nbMax);
        
        int myClient = select(mySocketServ2+1, &mySetSocket, NULL, NULL, NULL); //attente d'une connexion sur 1 des sockets
        printf("Valeur de select() : %d\n\n", myClient);
        if(myClient<0){
            perror("erreur fonction select\n\n");
            exit(-1);
        }
        
        if (FD_ISSET(mySocketServ2, &mySetSocket)) { //on a lu quelque chose sur la socket mySocketServ2 (UDP)
            int udpClient = mySocketServ2;
            recvfrom(udpClient, myReceiveBuffer, sizeof(myReceiveBuffer),0,(struct sockaddr *) &AddrClUdp, &len);
            if(strcmp(myReceiveBuffer,"SYN")!=0){
                perror("3-way handshake mistake at the SYN\n\n");
                continue;
            } else {
                printf("Message reçu du client %d : %s\n", udpClient, myReceiveBuffer);
            }

            int myBind3 = bind(mySocketData, (struct sockaddr*)&myAddrData,sizeof(struct sockaddr_in) );
            printf("valeur du bind socket Data: %d\n", myBind3);
            if (myBind3 < 0){
                perror("erreur valeur du binding socketData, la valeur est négative");
                exit(-1);
            }
                        
            //envoi du SYN-ACK avec le num de Port
            strcpy(mySendBuffer, "SYN-ACK");
            char portDataS[10];
            sprintf(portDataS,"%d", portData);
            strcat(mySendBuffer,portDataS);
            printf("Message envoyé au client de socket %d : %s\n", udpClient, mySendBuffer);
            sendto(udpClient, mySendBuffer, strlen(mySendBuffer), 0, (struct sockaddr *) &AddrClUdp, len); //bien mettre en 1er para le descripteur de la socket de client
            
            //attente réception du ACK
            recvfrom(udpClient, myReceiveBuffer, sizeof(myReceiveBuffer), 0, (struct sockaddr *) &AddrClUdp, &len);
            if(strcmp(myReceiveBuffer,"ACK")!=0){
                perror("3-way handshake mistake at the ACK\n\n");
                continue;
            } else {
                printf("Message reçu du client %d : %s\n", udpClient, myReceiveBuffer);
            }  
            printf("la phase de connexion entre le client : %d et le serveur est validée!!!\n\n", udpClient);
                        
            /* int index =0;
            for(int i =0; i<10; i++){
                if(sockBoard[i]!=0)
                    index++;
            }
            printf("index de la première case libre de sockBoard: %d\n\n", index);
            sockBoard[index]=mySocketData; */
            FD_SET(mySocketData, &mySetSocket);

        }

        //for (int i = 0; i < 10; i++){ 
            //if(sockBoard[i]!=0){
        if(FD_ISSET(mySocketData,&mySetSocket)){ //gestion de la com des données avec 1 client
            int udpData = mySocketData;
            //traiter le recvfrom
            memset(myReceiveBuffer,0, sizeof(myReceiveBuffer));
            recvfrom(udpData, myReceiveBuffer, sizeof(myReceiveBuffer), 0, (struct sockaddr *)&AddrClUdp, &len);
            //printf("taille recue : %d", n);
            printf("Le fichier demandé par le client est : %s\n\n", myReceiveBuffer);
            //le serveur ouvre le fichier demandé
            FILE * inputFile;
            inputFile = fopen(myReceiveBuffer,"rb");
            if ( inputFile == NULL ) {
                perror( "Cannot open file\n");
            }
            //taille du fichier et création du buffer
            fseek (inputFile, 0, SEEK_END);   // non-portable
            int size=ftell(inputFile);
            printf("Taille fichier = %d octets\n",size);
            char myFichierBuffer[size];
            size_t tailleBloc = 500; 
            int lastAck;
            if(size%tailleBloc!=0){
                lastAck = floor(size/tailleBloc)+1;
            }else{
                lastAck = size/tailleBloc;
            }
            printf("le dernier ACK attendu est: %d\n",lastAck );
            fseek(inputFile,0, SEEK_SET);
            char bufferSequence[6];
            char bufferSegment[1000]; //buffer pour envoyer le bout de fichier et n° seq
            char bufferCheckSeq[6];
            int countSeq = 1;
            fd_set setCurrentClient;
            long RTTMoyenLong;
            long delayLong;
            long VarianceRTT;
            struct timeval delay;
            struct timeval start;
            struct timeval end;     
            VarianceRTT = 0; //initialisée random                    
            long n_lrtt = 0; //mycurrentrtt
            RTTMoyenLong= 11000; //sur la base de ce qui a été observé en local
            delayLong = RTTMoyenLong + 4*VarianceRTT;
            delay.tv_sec = delayLong /1000000;
            delay.tv_usec = delayLong - (delayLong/1000000);
        
            //le serveur lit l'intégralité du fichier dans un buffer
            size_t nbOctetsLus = fread(myFichierBuffer,1,size,inputFile);
            //4 paramètres dont le nb octets qu'on veut lire
            //descripteur du fichier, taille du buffer, nbOctet
            //fread retourne le nombre de bloc qui sera fait avce le nb octet défini
            printf("Nombre d'octets lus dans le fichier:%zu\n",nbOctetsLus);
            //int curseur = ftell(inputFile);
            //printf("Le curseur est à la position : %d\n", curseur);
            int offset = 0;
            int tailleFen = 500; //trouver la meilleure valeur.
            int ackRecus[tailleFen];
            int ackMax = 0;
            int tailleBlocEC = tailleBloc;
                    
            //On rentre dans la boucle tant que tous les ACK n'ont pas été reçu
            while(countSeq<=lastAck){    
                //envoi du premier segment de la fenetre avec lancement du chrono
                printf("\n******SEGMENT*******\n");            
                //création du segment UDP
                memset(bufferSegment,0, sizeof(bufferSegment));
                memset(bufferSequence, 0, sizeof(bufferSequence));
                if(countSeq<=lastAck){
                    tailleBlocEC = tailleBloc;
                    if (countSeq==lastAck){
                        tailleBlocEC = size%tailleBloc;
                        printf("Taille du dernier bloc: %d\n",tailleBlocEC);
                    }
                    sprintf(bufferSequence, "%d", countSeq);                        
                    memcpy(bufferSegment, bufferSequence,6);
                    memcpy(bufferSegment+6,myFichierBuffer+offset,tailleBlocEC);
                    int s = sendto(udpData,bufferSegment, tailleBlocEC+6, 0, (struct sockaddr *) &AddrClUdp, len);
                    gettimeofday(&start, NULL);
                    printf("Segment %s envoyé de %d octets\n",bufferSequence, s);
                    countSeq++;
                    offset = offset + tailleBlocEC;
                }                        
                //envoi des autres segments de la fenêtre
                for(int j = 0; j<tailleFen-1; j++){
                    printf("\n******SEGMENT*******\n");            
                    //création du segment UDP
                    memset(bufferSegment,0, sizeof(bufferSegment));
                    memset(bufferSequence, 0, sizeof(bufferSequence));
                    if(countSeq<=lastAck){
                        tailleBlocEC = tailleBloc;
                        if (countSeq==lastAck){
                            tailleBlocEC = size%tailleBloc;
                            printf("Taille du dernier bloc: %d\n",tailleBlocEC);
                        } 
                        sprintf(bufferSequence, "%d", countSeq);                        
                        memcpy(bufferSegment, bufferSequence,6);
                        memcpy(bufferSegment+6,myFichierBuffer+offset,tailleBlocEC);
                        int s = sendto(udpData,bufferSegment, tailleBlocEC+6, 0, (struct sockaddr *) &AddrClUdp, len);
                        printf("Segment %s envoyé de %d octets\n",bufferSequence, s);
                        countSeq++;
                        offset = offset + tailleBlocEC;
                    }  
                }
                        
                //GESTION RECEPTION DES ACK
                memset(ackRecus, 0, sizeof(ackRecus)); 

                FD_ZERO(&setCurrentClient);
                FD_SET(udpData, &setCurrentClient);
                select(udpData+1, &setCurrentClient,NULL,NULL,&delay);
                if(FD_ISSET(udpData,&setCurrentClient)){
                //le serveur attend un acquittement pour le morceau de fichier envoyé
                //recvfrom()
                    memset(myReceiveBuffer,0, sizeof(myReceiveBuffer));
                    recvfrom(udpData, myReceiveBuffer, sizeof(myReceiveBuffer), 0, (struct sockaddr *)&AddrClUdp, &len);
                    printf("Acquittement reçu : %s\n", myReceiveBuffer);
                    memset(bufferCheckSeq, 0, sizeof(bufferCheckSeq));
                    memcpy(bufferCheckSeq,myReceiveBuffer+4,6);
                    ackRecus[0] = atoi(bufferCheckSeq);
                    printf("myReceiveBuffer: %d\n",atoi(bufferCheckSeq));
                    gettimeofday(&end, NULL);
                            
                    //calcul RTT par Quentin
                    n_lrtt = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec); // current rtt
					printf("RTT échantillon déterminé : %ld microsecondes\n",n_lrtt);
                    RTTMoyenLong =((1-ALPHA)*RTTMoyenLong + ALPHA*n_lrtt);
                    VarianceRTT = (1-BETA)*VarianceRTT + BETA * abs(RTTMoyenLong - n_lrtt);
                    printf("Variance déterminée : %ld\n", VarianceRTT);
                    delayLong = RTTMoyenLong + 4*VarianceRTT;
                    delay.tv_sec = delayLong / 1000000;
					delay.tv_usec = delayLong - (delayLong / 1000000);
                    printf("Timer mis à jour: %ld secondes %ld microsecondes\n", delay.tv_sec, delay.tv_usec);
                } else{
                    //RTT par Quentin
                    delay.tv_sec = delayLong/1000000;
                    delay.tv_usec = delayLong;
                }   
                //réception des tailleFen-1 autres ACK
                for(int k=0; k<tailleFen-1; k++){
                    FD_ZERO(&setCurrentClient);
                    FD_SET(udpData, &setCurrentClient);
                    select(udpData+1, &setCurrentClient,NULL,NULL,&delay);
                    if(FD_ISSET(udpData,&setCurrentClient)){
                    //le serveur attend un acquittement pour le morceau de fichier envoyé
                    //recvfrom()
                        memset(myReceiveBuffer,0, sizeof(myReceiveBuffer));
                        recvfrom(udpData, myReceiveBuffer, sizeof(myReceiveBuffer), 0, (struct sockaddr *)&AddrClUdp, &len);
                        memset(bufferCheckSeq, 0, sizeof(bufferCheckSeq));
                        memcpy(bufferCheckSeq,myReceiveBuffer+4,6);
                        ackRecus[k+1]=atoi(bufferCheckSeq);
                        printf("Acquittement reçu : %s\n", myReceiveBuffer);
                    } else{
                        printf("Un ACK de la fenetre n'a pas été reçu\n");
                        offset = (atoi(bufferSequence)-1)*tailleBloc; 
                    }  
                }
                //affichage du tableau des ACK avant le tri croissant
                for(int s=0; s<tailleFen;s++){
                    printf("case %d des ack reçus : %d\n", s, ackRecus[s]);
                }
                //récupère la valeur max d'ACK reçu
                for(int i=0;i<tailleFen; i++){
                    if(ackRecus[i]>ackMax){
                        ackMax = ackRecus[i];
                    }
                }                       
                printf("\nACKMax reçu: %d\n", ackMax);
                countSeq = ackMax+1; 
                offset = (countSeq-1)*tailleBloc;                       
            }
            printf("\n\nLe client a reçu l'intégralité du fichier demandé\n\n\n");
            memset(bufferSegment,0,sizeof(bufferSegment));
            memcpy(bufferSegment, "FIN",3);
            sendto(udpData,bufferSegment,3, 0, (struct sockaddr *) &AddrClUdp, len);
            fclose(inputFile);
            //sockBoard[i]=0;
        }

    }           
            
        //}
}

//}
