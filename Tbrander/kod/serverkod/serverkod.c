/      SERVERKOD
int server(DECK card[], PLAYER usr[])
{
    int server_socket, client_socket,consocket=0, i;
    struct sockaddr_in serv, dest;
    char msg[] = "Connected with server.\n";

    // thread and mutex initialization
    THREAD tdata[5]; // this is the threads individual data(struct server_threads)
    pthread_t thread_id[5]  // thread ID given to 5 elements in the array
    for(i = 0; i < 5; i++)
    {
        pthread_mutex_init (&mutex[i], NULL);// initialize mutex i where i: 0..4
    }
    i = 0; // reseting the variable for future use

    //daemonize();

    pid=getpid();
    int listen_socket; // socket used to listen for incoming connections
    socklen_t socksize = sizeof(struct sockaddr_in);
    memset(&serv, 0, sizeof(serv));           // zero the struct before filling the fields
    serv.sin_family = AF_INET;                // set the type of connection to TCP/IP
    serv.sin_addr.s_addr = htonl(INADDR_ANY); // set our address to any interface
    serv.sin_port = htons(PORTNUM);
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    bind(listen_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
    listen(listen_socket, 5);


    printf("Incoming connection from %s - sending welcome\n", inet_ntoa(dest.sin_addr));

    send(consocket, msg, strlen(msg), 0);

    //Main-Accept loop
    while(1) {
        consocket = accept(listen_socket, (struct sockaddr *)&dest, &socksize);
        if(consocket != -1) {  // if accept fails to initialize conection -> return value == -1
            continue;
        }

        //Dealer initilization: The dealer will have a separate thread without the need of an client
        if(i==0){
            tdata[i].tconsocket = consocket;
            tdata[i].nthread = i;
            pthread_create(&thread_id[i], NULL, &serve_client, &tdata[i]);
            i++;
        }
        // Each individual client will be servered by a thread.
        tdata[i].tconsocket = consocket;
        tdata[i].nthread = i;
        pthread_create(&thread_id[i], NULL, &serve_client, &tdata[i]);
        i++;
    }


    // Nu finns de en anslutning, och nedan kan kod för kommunikationen över socketen finnas.
}
    /

void* serve_client (void* parameters) {  //thread_function

    THREAD* p = (THREAD*) parameters;
}

