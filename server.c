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


int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Pour utiliser ce programme, vous devez taper ./server <port_serveurUDP>\n");
        exit(-1);
    }
    int portBis = atoi(argv[1]);
    /* if(portBis<1000||portBis>9999){
        printf("<port_serveurUDP> doit être compris entre 1000 et 9999\n");
        exit(-1);
    } */
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
        exit(-1);}
    
    printf("le serveur fonctionne bien \n\n");

    char mySendBuffer[500];
    char myReceiveBuffer[500];
    socklen_t len = sizeof(AddrClUdp);
    fd_set mySetSocket;
    int sockBoard[10];
    for(int i = 0; i<10; i++){
        sockBoard[i]=0;
    }

    while (1)
    {   //initialisation et activation des bons bits     
        FD_ZERO(&mySetSocket);
        //FD_SET(mySocketServ, &mySetSocket);
        FD_SET(mySocketServ2, &mySetSocket);
        for (int i = 0; i < 10; i++){
            if(sockBoard[i]!=0){
                FD_SET(sockBoard[i], &mySetSocket);
            }
        }

        int nbMax = 0;        
        for (int i=0; i<10; i++){
            if(sockBoard[i]>nbMax)
                nbMax = sockBoard[i]; //trouver le plus grand descripteur de fichier
        }
        nbMax = max(nbMax, mySocketServ2);
        printf("nbMax : %d\n", nbMax);
        
        int myClient = select(nbMax+1, &mySetSocket, NULL, NULL, NULL); //attente d'une connexion sur 1 des sockets
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
                        
            int index =0;
            for(int i =0; i<10; i++){
                if(sockBoard[i]!=0)
                    index++;
            }
            printf("index de la première case libre de sockBoard: %d\n\n", index);
            sockBoard[index]=mySocketData;
        }

        for (int i = 0; i < 10; i++){ 
            if(sockBoard[i]!=0){
                if(FD_ISSET(sockBoard[i],&mySetSocket)){ //gestion de la com des données avec 1 client
                    int udpData = sockBoard[i];
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
                        //exit( 0 );
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
                    //int nbBlocsLus = 0;
                    fseek(inputFile,0, SEEK_SET);
                    char bufferSequence[6];
                    char bufferSegment[1000]; //buffer pour envoyer le bout de fichier et n° seq
                    char bufferCheckSeq[6];
                    int countSeq = 1;
                    //int countAck = 1;
                    fd_set setCurrentClient;
                    struct timeval myTimer;
                    struct timeval t1;
                    struct timeval t2;     
                    struct timeval RTT;
                    RTT.tv_sec = 0;
                    RTT.tv_usec = 5000; //déterminer une valeur plus judicieuse

                    
                    //On rentre dans la boucle tant que tous les ACK n'ont pas été reçu
                    while(countSeq<=lastAck){ 
                        myTimer.tv_sec = 0;
                        myTimer.tv_usec = 4*RTT.tv_usec; // à optimiser
                        printf("\n******SEGMENT*******\n");
                        //le serveur lit le fichier dans un buffer
                        size_t nbOctetsLus = fread(myFichierBuffer,1,tailleBloc,inputFile);
                        printf("Nombre d'octets lus dans le fichier:%zu\n",nbOctetsLus);
                        int curseur = ftell(inputFile);
                        printf("Le curseur est à la position : %d\n", curseur);
                                               
                        //4 paramètres dont le nb octets qu'on veut lire
                        //descripteur du fichier, taille du buffer, nbOctet
                        //fread retourne le nombre de bloc qui sera fait avce le nb octet défini
                        //création du segment UDP
                        memset(bufferSegment,0, sizeof(bufferSegment));
                        memset(bufferSequence, 0, sizeof(bufferSequence));
                        sprintf(bufferSequence, "%d", countSeq);
                        
                        memcpy(bufferSegment, bufferSequence,6);
                        memcpy(bufferSegment+6,myFichierBuffer,nbOctetsLus);
                        //le serveur envoie le morceau de fichier lu
                        int s = sendto(udpData,bufferSegment, nbOctetsLus+6, 0, (struct sockaddr *) &AddrClUdp, len);
                        gettimeofday(&t1, NULL);
                        printf("Nombre d'octets envoyés : %d\n",s);
                        //initialisation et activation des bons bits     
                        FD_ZERO(&setCurrentClient);
                        //FD_SET(mySocketServ, &mySetSocket);
                        FD_SET(udpData, &setCurrentClient);
                        select(udpData+1, &setCurrentClient,NULL,NULL,&myTimer);
                        if(FD_ISSET(udpData,&setCurrentClient)){
                            //le serveur attend un acquittement pour le morceau de fichier envoyé
                            //recvfrom()
                            memset(myReceiveBuffer,0, sizeof(myReceiveBuffer));
                            recvfrom(udpData, myReceiveBuffer, sizeof(myReceiveBuffer), 0, (struct sockaddr *)&AddrClUdp, &len);
                            printf("l'acquittement reçu par le serveur est : %s\n", myReceiveBuffer);
                            gettimeofday(&t2, NULL);
                            RTT.tv_usec = abs(t2.tv_usec - t1.tv_usec);  //Pq le RTT devient négatif par moment?
                            printf("Le nouveau RTT déterminé est: %ld\n",RTT.tv_usec);                            
                            memset(bufferCheckSeq, 0, sizeof(bufferCheckSeq));
                            memcpy(bufferCheckSeq,myReceiveBuffer+3,6);
                            countSeq = atoi(bufferCheckSeq) +1; //ou countSeq++
                            //fseek(inputFile,0,(countSeq-1)*tailleBloc); 
                            printf("countSeq: %d\n",countSeq);
                        } else{
                            printf("Le segment %d va être retransmis, timeout!\n",atoi(bufferSequence));
                            printf("valeur fseek: %ld\n",(atoi(bufferSequence)-1)*tailleBloc);
                            //ATTENTION FAUX: int f = fseek(inputFile, 0, (atoi(bufferSequence)-1)*tailleBloc);
                            int f = fseek(inputFile, (atoi(bufferSequence)-1)*tailleBloc,SEEK_SET);
                            if(f<0)
                                perror("erreur fseek\n");
                            printf("curseur : %ld\n",ftell(inputFile));
                            RTT.tv_usec = 5000;
                            //


                        }     
                        //idée: faire un tableau VRAI/FAUX des acquittements pour chacun des bouts de chacun                  
                        
                        //incrémenter  le compteur de suivi des ACK

                    }
                    printf("\n\nLe client a reçu l'intégralité du fichier demandé\n\n\n");
                    memset(bufferSegment,0,sizeof(bufferSegment));
                    memcpy(bufferSegment, "FIN",3);
                    sendto(udpData,bufferSegment,3, 0, (struct sockaddr *) &AddrClUdp, len);
                    fclose(inputFile);
                    sockBoard[i]=0;
                }

            }           
            
        }
        
    
    }

}
