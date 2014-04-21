 /     SERVERKOD
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
    listen(listen_socket, 4); // a maximum of 4 connections simultaniously can be  made


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
            tdata[0].tconsocket[0] = consocket;
            tdata[0].nthread = 0;
            pthread_create(&thread_id[0], NULL, &serve_client, &tdata[0]);
            i++;
        }
        // Each individual client will be servered by a thread.     // Problem: Dealer och användare1 delar på samma socket
        tdata[0].tconsocket[i] = consocket;
        tdata[0].n_users = i;  // the number of users currently connected must be known to thread[0]/Dealer
        tdata[i].nthread = i;
        pthread_create(&thread_id[i], NULL, &serve_client, &tdata[i]);
        i++;
    }



}
    /

void* serve_client (void* parameters) {  //thread_function

    THREAD* p = (THREAD*) parameters;

    switch(p->nthread) {
        case 0: { // Dealer

            pthread_mutex_lock(&mutex[p->nthread]);
            // The number of users playing must be known to the dealer (n_users), if there are no users -> return to main(?)
            // which user is going to play against the dealer?, other users  must wait and will need to be notified by the dealer
            // Send information to the client -> forward the answer to the specific thread and wait for the threads answer
            // Make Calculations based on the answer from the thread. Compare it with your own values.
            //  Send the necessary values back to the client/clients
            //  Loop
        }
        case 1: { // Threadfunction1/ user1

            pthread_mutex_lock(&mutex[p->nthread]);
            // Stay in a loop and wait for the dealer to make contact.
            // Based  on the information ->(waiting for turn): only make changes to your surroundings and loop, your turn: calculate game vaules,and send to dealer and loop
            // (user has exited): Reset all your values, notify the server that the slot is empty(setting i == p->nthread) and unlock your mutex -> Return to main

        }
        case 2: { // Threadfunction2/ user2

            pthread_mutex_lock(&mutex[p->nthread]);
        }
        case 3: { // Threadfunction3/ user3

            pthread_mutex_lock(&mutex[p->nthread]);
        }
        case 4: { // Threadfunction4/ user4

            pthread_mutex_lock(&mutex[p->nthread]);
        }
        default: { // Maximum number of users has been reached!

        }

    }

}
