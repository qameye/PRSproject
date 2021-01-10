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
#define ALPHA 0.125 //valeur standard d'après Wikipédia
#define BETA 0.25 //valeur standard de Wikipédia
#define TAILLE_BLOC 1494
#define WINDOW_SIZE 300


int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Pour utiliser ce programme, vous devez taper ./serveur1-UDPower <port_serveurUDP>\n");
        exit(-1);
    }
    int portCo = atoi(argv[1]);
    int portData = 5678;

    //SocketServ
    int mySocketServ = socket(AF_INET,SOCK_DGRAM, 0);
       
    //afin de pouvoir réutiliser directement la socket
    int reuse = 1;
    setsockopt(mySocketServ, SOL_SOCKET, SO_REUSEADDR,&reuse, sizeof(reuse)) ;
   
    //detection d'une erreur à la création de la socket
    if (mySocketServ < 0){
        exit(-1);
    }
   
    //remise à 0 de la structure myAddr2 et AddrClUdp
    struct sockaddr_in myAddr2 ;
    struct sockaddr_in AddrClUdp;
    memset((char*)&AddrClUdp, 0, sizeof(AddrClUdp));
    memset((char*)&myAddr2, 0, sizeof(myAddr2)) ;
    myAddr2.sin_family= AF_INET;
    myAddr2.sin_port = htons(portCo);
    myAddr2.sin_addr.s_addr= htonl(INADDR_ANY) ;

    int myBind = bind(mySocketServ, (struct sockaddr*)&myAddr2,sizeof(struct sockaddr_in) );
    if (myBind < 0){
        perror("erreur valeur du bindServeurUDP négative");
        exit(-1);
    }

    char mySendBuffer[500];
    char myReceiveBuffer[500];
    socklen_t len = sizeof(AddrClUdp);
    fd_set mySetSocket;
    //création nouvelle socket pour les données
    
    int mySocketData = socket(AF_INET, SOCK_DGRAM, 0);
    int reuseTer = 1;
    setsockopt(mySocketData, SOL_SOCKET, SO_REUSEADDR,&reuseTer, sizeof(reuseTer));
    if (mySocketData <0)
        exit(-1);

    struct sockaddr_in myAddrData;
    memset((char*)&myAddrData, 0, sizeof(myAddrData));
    myAddrData.sin_family=AF_INET;
    myAddrData.sin_port=htons(portData);
    myAddrData.sin_addr.s_addr=htonl(INADDR_ANY);
    
    while (1)
    {   //initialisation et activation des bons bits     
        FD_ZERO(&mySetSocket);
        //FD_SET(mySocketServ, &mySetSocket);
        FD_SET(mySocketServ, &mySetSocket);        
        int myClient = select(mySocketServ+1, &mySetSocket, NULL, NULL, NULL);
        if(myClient<0){
            perror("erreur fonction select\n\n");
            exit(-1);
        }
        
        if (FD_ISSET(mySocketServ, &mySetSocket)) { //on a lu quelque chose sur la socket mySocketServ2 (UDP)
            int udpClient = mySocketServ;
            recvfrom(udpClient, myReceiveBuffer, sizeof(myReceiveBuffer),0,(struct sockaddr *) &AddrClUdp, &len);
            if(strcmp(myReceiveBuffer,"SYN")!=0){
                perror("3-way handshake mistake at the SYN\n\n");
                continue;
            } else {
                //printf("Message reçu du client %d : %s\n", udpClient, myReceiveBuffer);
            }

            int myBind2 = bind(mySocketData, (struct sockaddr*)&myAddrData,sizeof(struct sockaddr_in) );
            //printf("valeur du bind socket Data: %d\n", myBind3);
            if (myBind2 < 0){
                perror("erreur valeur du binding socketData, la valeur est négative");
                exit(-1);
            }
                        
            //envoi du SYN-ACK avec le num de Port
            strcpy(mySendBuffer, "SYN-ACK");
            char portDataS[10];
            sprintf(portDataS,"%d", portData);
            strcat(mySendBuffer,portDataS);
            //printf("Message envoyé au client de socket %d : %s\n", udpClient, mySendBuffer);
            sendto(udpClient, mySendBuffer, strlen(mySendBuffer), 0, (struct sockaddr *) &AddrClUdp, len); //bien mettre en 1er para le descripteur de la socket de client
            
            //attente réception du ACK
            recvfrom(udpClient, myReceiveBuffer, sizeof(myReceiveBuffer), 0, (struct sockaddr *) &AddrClUdp, &len);
            if(strcmp(myReceiveBuffer,"ACK")!=0){
                perror("3-way handshake mistake at the ACK\n\n");
                continue;
            } else {
                //printf("Message reçu du client %d : %s\n", udpClient, myReceiveBuffer);
            }  
            //printf("la phase de connexion entre le client : %d et le serveur est validée!!!\n\n", udpClient);
            FD_SET(mySocketData, &mySetSocket);
        }

        if(FD_ISSET(mySocketData,&mySetSocket)){ //gestion de la com des données avec 1 client
            int udpData = mySocketData;
            //traiter le recvfrom
            memset(myReceiveBuffer,0, sizeof(myReceiveBuffer));
            recvfrom(udpData, myReceiveBuffer, sizeof(myReceiveBuffer), 0, (struct sockaddr *)&AddrClUdp, &len);
            //printf("taille recue : %d", n);
            //printf("Le fichier demandé par le client est : %s\n\n", myReceiveBuffer);
            //le serveur ouvre le fichier demandé
            FILE * inputFile;
            inputFile = fopen(myReceiveBuffer,"rb");
            if ( inputFile == NULL ) {
                perror( "Cannot open file\n");
            }
            //taille du fichier et création du buffer
            fseek (inputFile, 0, SEEK_END);   // non-portable
            long int size=ftell(inputFile);
            printf("Taille fichier = %ld octets\n",size);
            size_t tailleBloc = TAILLE_BLOC; 
            long int lastAck;
            if(size%tailleBloc!=0){
                lastAck = floor(size/tailleBloc)+1;
            }else{
                lastAck = size/tailleBloc;
            }
            printf("le dernier ACK attendu est: %ld\n",lastAck );
            fseek(inputFile,0, SEEK_SET);
            char bufferSequence[6];
            char bufferSegment[2000]; //buffer pour envoyer le bout de fichier et n° seq
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
            long int offset = 0;
            int tailleFen = WINDOW_SIZE; //trouver la meilleure valeur.
            long int ackRecus[tailleFen];
            long int ackMax = 0;
            long int tailleBlocEC = tailleBloc;
            char myFichierBuffer[tailleBloc*tailleFen];
                    
            //On rentre dans la boucle tant que tous les ACK n'ont pas été reçu
            while(countSeq<=lastAck){  
                  
                //envoi du premier segment de la fenetre avec lancement du chrono
                //printf("\n******SEGMENT*******\n");            
                //création du segment UDP
                memset(myFichierBuffer,0, sizeof(myFichierBuffer));
                fread(myFichierBuffer,1,tailleBloc*tailleFen,inputFile);
                //printf("Nombre d'octets lus dans le fichier:%zu\n",nbOctetsLus);
                memset(bufferSegment,0, sizeof(bufferSegment));
                memset(bufferSequence, 0, sizeof(bufferSequence));
                if(countSeq<=lastAck){
                    tailleBlocEC = tailleBloc;
                    if (countSeq==lastAck){
                        tailleBlocEC = size%tailleBloc;
                        //printf("Taille du dernier bloc: %ld\n",tailleBlocEC);
                    }
                    sprintf(bufferSequence, "%d", countSeq);                        
                    memcpy(bufferSegment, bufferSequence,6);
                    memcpy(bufferSegment+6,myFichierBuffer,tailleBlocEC);
                    sendto(udpData,bufferSegment, tailleBlocEC+6, 0, (struct sockaddr *) &AddrClUdp, len);
                    gettimeofday(&start, NULL);
                    //printf("Segment %s envoyé de %d octets\n",bufferSequence, s);
                    countSeq++;
                    offset = offset + tailleBlocEC;
                }                        
                //envoi des autres segments de la fenêtre
                for(int j = 0; j<tailleFen-1; j++){
                    //printf("\n******SEGMENT*******\n");            
                    //création du segment UDP
                    memset(bufferSegment,0, sizeof(bufferSegment));
                    memset(bufferSequence, 0, sizeof(bufferSequence));
                    if(countSeq<=lastAck){
                        tailleBlocEC = tailleBloc;
                        if (countSeq==lastAck){
                            tailleBlocEC = size%tailleBloc;
                            //printf("Taille du dernier bloc: %ld\n",tailleBlocEC);
                        } 
                        sprintf(bufferSequence, "%d", countSeq);                        
                        memcpy(bufferSegment, bufferSequence,6);
                        memcpy(bufferSegment+6,myFichierBuffer+offset,tailleBlocEC);
                        sendto(udpData,bufferSegment, tailleBlocEC+6, 0, (struct sockaddr *) &AddrClUdp, len);
                        //printf("Segment %s envoyé de %d octets\n",bufferSequence, s);
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
                    //printf("Acquittement reçu str: %s\n", myReceiveBuffer);
                    memset(bufferCheckSeq, 0, sizeof(bufferCheckSeq));
                    memcpy(bufferCheckSeq,myReceiveBuffer+3,6);
                    ackRecus[0] = atoi(bufferCheckSeq);
                    //printf("Acquittement reçu int : %ld\n", ackRecus[0]);
                    //printf("myReceiveBuffer: %d\n",atoi(bufferCheckSeq));
                    gettimeofday(&end, NULL);
                            
                    //calcul RTT par Quentin
                    n_lrtt = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec); // current rtt
					//printf("RTT échantillon déterminé : %ld microsecondes\n",n_lrtt);
                    RTTMoyenLong =((1-ALPHA)*RTTMoyenLong + ALPHA*n_lrtt);
                    VarianceRTT = (1-BETA)*VarianceRTT + BETA * abs(RTTMoyenLong - n_lrtt);
                    //printf("Variance déterminée : %ld\n", VarianceRTT);
                    delayLong = RTTMoyenLong + 4*VarianceRTT;
                    delay.tv_sec = delayLong / 1000000;
					delay.tv_usec = delayLong - (delayLong / 1000000);
                    //printf("Timer mis à jour: %ld secondes %ld microsecondes\n", delay.tv_sec, delay.tv_usec);
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
                        memcpy(bufferCheckSeq,myReceiveBuffer+3,6);
                        ackRecus[k+1]=atoi(bufferCheckSeq);
                        //printf("Acquittement reçu str: %s\n", myReceiveBuffer);
                        //printf("Acquittement reçu int : %ld\n", ackRecus[k+1]);
                    } else{
                        //printf("Un ACK de la fenetre n'a pas été reçu\n");
                    }  
                }
                //affichage du tableau des ACK avant le tri croissant
                for(int s=0; s<tailleFen;s++){
                    //printf("case %d des ack reçus : %d\n", s, ackRecus[s]);
                }
                //récupère la valeur max d'ACK reçu
                for(int i=0;i<tailleFen; i++){
                    if(ackRecus[i]>ackMax){
                        ackMax = ackRecus[i];
                    }
                }                       
                //printf("\nACKMax reçu: %ld\n", ackMax);
                countSeq = ackMax+1; 
                //offset = (countSeq-1)*tailleBloc;
                offset = 0;
                fseek(inputFile,(countSeq-1)*tailleBloc, SEEK_SET);                       
            }
            //printf("\n\nLe client a reçu l'intégralité du fichier demandé\n\n\n");
            memset(bufferSegment,0,sizeof(bufferSegment));
            memcpy(bufferSegment, "FIN",3);
            sendto(udpData,bufferSegment,3, 0, (struct sockaddr *) &AddrClUdp, len);
            fclose(inputFile);
            exit(0);
        }
    }
}
