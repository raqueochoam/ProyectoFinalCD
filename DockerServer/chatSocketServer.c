#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>	// it is required to call signal handler functions
#include <unistd.h>  // it is required to close the socket descriptor
#include <sys/shm.h>

#define  DIRSIZE   2048      /* longitud maxima parametro entrada/salida */
#define  msgSIZE   2048      /* longitud maxima parametro entrada/salida */
#define  PUERTO   5001	     /* numero puerto arbitrario */

#define MAX_USERS 100
#define MAX_GROUPS 200
#define MAX_MSGS 2048

struct groupUser {
    char username[40];
    int role;
};

struct group {
    char groupName[40];
    struct groupUser groupUsers[MAX_USERS];
    int numUsers;
    char groupPendUsers[MAX_USERS][30];
    int numPendUsers;
    char groupConv[MAX_MSGS][100];
    int numMssgs;
};

struct user {
    char username[40];
    char password[40];
    char groups[MAX_GROUPS][40];
    int numUserGroups;
};

typedef struct {
    struct user usersCredentials[MAX_USERS];
    int numUsers;
    struct group groups[MAX_GROUPS];
    int numGroups;
} SharedData;


int                  sd, sd_actual;  /* descriptores de sockets */
int                  addrlen;        /* longitud direcciones */
struct sockaddr_in   sind, pin;      /* direcciones sockets cliente u servidor */


/*  procedimiento de aborte del servidor, si llega una senal SIGINT */
/* ( <ctrl> <c> ) se cierra el socket y se aborta el programa       */
void aborta_handler(int sig){
   printf("....abortando el proceso servidor %d\n",sig);
   close(sd);  
   close(sd_actual); 
   exit(1);
}

void decipherString(char* text) {
    int length = strlen(text);

    for (int i = 0; i < length; i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            text[i] = 'z' - (text[i] - 'a');
        }
    }
}

int main(){
    printf("Socket Server Start\n");

    // Create shared memory segment
    int shmId = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmId == -1) {
        perror("shmget");
        exit(1);
    }
    // Attach shared memory segment
    SharedData* sharedData = shmat(shmId, NULL, 0);
    if (sharedData == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    // Initialize shared data
    sharedData->numGroups = 0;
    sharedData->numUsers = 0;

    //Read Users File to get usernames and passwords
    FILE *usersTxt = fopen("users.txt", "r");
    if (usersTxt == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    char lineUserData[40];
    int lineCount = 0;
    while (fgets(lineUserData, sizeof(lineUserData), usersTxt) != NULL) {
        lineUserData[strcspn(lineUserData, "\n")] = '\0'; // Remove trailing newline character
        
        if (lineCount % 2 == 0) {
            // Read username
            strcpy(sharedData->usersCredentials[sharedData->numUsers].username, lineUserData);
            sharedData->usersCredentials[sharedData->numUsers].username[strlen(sharedData->usersCredentials[sharedData->numUsers].username)] = '\0';
        } else {
            // Read password
            strcpy(sharedData->usersCredentials[sharedData->numUsers].password, lineUserData);
            sharedData->usersCredentials[sharedData->numUsers].password[strlen(sharedData->usersCredentials[sharedData->numUsers].password)] = '\0';
            printf("%s %s\n", sharedData->usersCredentials[sharedData->numUsers].username, sharedData->usersCredentials[sharedData->numUsers].password);
            sharedData->usersCredentials[sharedData->numUsers].numUserGroups = 0;
            sharedData->numUsers++;
            
            if (sharedData->numUsers >= MAX_USERS) {
                printf("Maximum number of users reached.\n");
                break;
            }
        }
        lineCount++;
    }
    fclose(usersTxt);
    //
    // Read Users Groups
    for (int i = 0; i < sharedData->numUsers; i++)
    {
        char txtFile[40];
        strcpy(txtFile, sharedData->usersCredentials[i].username);
        strcat(txtFile, ".groups");
        FILE *file = fopen(txtFile, "r");
        if (file == NULL) {
            printf("Error opening file.\n");
            return 1;
        }

        char userGroupLine[40];

        while (fgets(userGroupLine, sizeof(userGroupLine), file) != NULL) {
            userGroupLine[strcspn(userGroupLine, "\n")] = '\0'; // Remove trailing newline character
            
            strcpy(sharedData->usersCredentials[i].groups[sharedData->usersCredentials[i].numUserGroups], userGroupLine);
            sharedData->usersCredentials[i].groups[sharedData->usersCredentials[i].numUserGroups][strlen(sharedData->usersCredentials[i].groups[sharedData->usersCredentials[i].numUserGroups])] = '\0';

            sharedData->usersCredentials[i].numUserGroups++;
            
            if (sharedData->usersCredentials[i].numUserGroups >= MAX_GROUPS) {
                printf("Maximum number of groups reached.\n");
                break;
            }
        }
        fclose(file);
    }
    //Read Groups File to get groups
    FILE *groupsTxt = fopen("groups.txt", "r");
    if (groupsTxt == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    char groupNameLine[40];
    while (fgets(groupNameLine, sizeof(groupNameLine), groupsTxt) != NULL) {
        groupNameLine[strcspn(groupNameLine, "\n")] = '\0'; // Remove trailing newline character
        
        strcpy(sharedData->groups[sharedData->numGroups].groupName, groupNameLine);
        sharedData->groups[sharedData->numGroups].groupName[strlen(sharedData->groups[sharedData->numGroups].groupName)] = '\0';
        sharedData->groups[sharedData->numGroups].numMssgs = 0;
        sharedData->groups[sharedData->numGroups].numUsers = 0;
        sharedData->groups[sharedData->numGroups].numPendUsers = 0;
        sharedData->numGroups++;
    }
    fclose(groupsTxt);
    //
    // Read Groups Data (users, pendusers, conv)
    for (int i = 0; i < sharedData->numGroups; i++)
    {
        // Group Conv
        char groupconvTxtName[40];
        strcpy(groupconvTxtName, sharedData->groups[i].groupName);
        strcat(groupconvTxtName, ".conv");
        FILE *groupconvTxt = fopen(groupconvTxtName, "r");
        if (groupconvTxt == NULL) {
            printf("Error opening file.\n");
            return 1;
        }
        char mssgLine[100];
        while (fgets(mssgLine, sizeof(mssgLine), groupconvTxt) != NULL) {
            mssgLine[strcspn(mssgLine, "\n")] = '\0'; // Remove trailing newline character           
            strcpy(sharedData->groups[i].groupConv[sharedData->groups[i].numMssgs], mssgLine);
            sharedData->groups[i].groupConv[sharedData->groups[i].numMssgs][strlen(sharedData->groups[i].groupConv[sharedData->groups[i].numMssgs])] = '\0';
            sharedData->groups[i].numMssgs++;
            if (sharedData->groups[i].numMssgs >= MAX_MSGS) {
                printf("Maximum number of messages reached.\n");
                break;
            }
        }
        fclose(groupconvTxt);
        // Group Users
        char groupusersTxtName[40];
        strcpy(groupusersTxtName, sharedData->groups[i].groupName);
        strcat(groupusersTxtName, ".users");
        FILE *groupusersTxt = fopen(groupusersTxtName, "r");
        if (groupusersTxt == NULL) {
            printf("Error opening file.\n");
            return 1;
        }
        char groupUserLine[40];
        char groupUsernameLine[35];
        char groupUserRoleLine[10];
        while (fgets(groupUserLine, sizeof(groupUserLine), groupusersTxt) != NULL) {
            groupUserLine[strcspn(groupUserLine, "\n")] = '\0'; // Remove trailing newline character
            char *token;
            // Get the first token
            token = strtok(groupUserLine, " ");
            strcpy(groupUsernameLine,token);
            token = strtok(groupUserLine, " ");
            strcpy(groupUserRoleLine,token);
            strcpy(sharedData->groups[i].groupUsers[sharedData->groups[i].numUsers].username, groupUsernameLine);
            sharedData->groups[i].groupUsers[sharedData->groups[i].numUsers].role = atoi(groupUserRoleLine);
            sharedData->groups[i].groupUsers[sharedData->groups[i].numUsers].username[strlen(sharedData->groups[i].groupUsers[sharedData->groups[i].numUsers].username)] = '\0';
            sharedData->groups[i].numUsers++;
            if (sharedData->groups[i].numUsers >= MAX_USERS) {
                printf("Maximum number of users reached.\n");
                break;
            }
        }
        fclose(groupusersTxt);
        //
        // Group Pending Users
        char grouppendusersTxtName[40];
        strcpy(grouppendusersTxtName, sharedData->groups[i].groupName);
        strcat(grouppendusersTxtName, ".pendusers");
        FILE *grouppendusersTxt = fopen(grouppendusersTxtName, "r");
        if (grouppendusersTxt == NULL) {
            printf("Error opening file.\n");
            return 1;
        }
        char groupPendUserLine[40];
        while (fgets(groupPendUserLine, sizeof(groupPendUserLine), grouppendusersTxt) != NULL) {
            groupPendUserLine[strcspn(groupPendUserLine, "\n")] = '\0'; // Remove trailing newline character
            strcpy(sharedData->groups[i].groupPendUsers[sharedData->groups[i].numPendUsers], groupPendUserLine);
            sharedData->groups[i].groupPendUsers[sharedData->groups[i].numPendUsers][strlen(sharedData->groups[i].groupPendUsers[sharedData->groups[i].numPendUsers])] = '\0';
            sharedData->groups[i].numPendUsers++;          
            if (sharedData->groups[i].numPendUsers >= MAX_USERS) {
                printf("Maximum number of pending users reached.\n");
                break;
            }
        }
        fclose(grouppendusersTxt);
        //

    }
    //
	char  dir[DIRSIZE];	     /* parametro entrada y salida */
	/*
	When the user presses <Ctrl + C>, the aborta_handler function will be called, 
	and such a message will be printed. 
	Note that the signal function returns SIG_ERR if it is unable to set the 
	signal handler, executing line 54.
	*/	
    if(signal(SIGINT, aborta_handler) == SIG_ERR){
        perror("Could not set signal handler");
        return 1;
    }
    //signal(SIGINT, aborta);      /* activando la senal SIGINT */

    /* obtencion de un socket tipo internet */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

    /* asignar direcciones en la estructura de direcciones */
	sind.sin_family = AF_INET;
	sind.sin_addr.s_addr = INADDR_ANY;   /* INADDR_ANY=0x000000 = yo mismo */
	sind.sin_port = htons(PUERTO);       /*  convirtiendo a formato red */

    /* asociando el socket al numero de puerto */
	if (bind(sd, (struct sockaddr *)&sind, sizeof(sind)) == -1) {
		perror("bind");
		exit(1);
	}

    /* ponerse a escuchar a traves del socket */
	if (listen(sd, 5) == -1) {
		perror("listen");
		exit(1);
	}
    pid_t child_pid = 1;
    while (1)
    {
        if (child_pid != 0){
            /* esperando que un cliente solicite un servicio */
            sd_actual = accept(sd, (struct sockaddr *)&pin, &addrlen);
            if (sd_actual == -1) {
                perror("accept");
                exit(1);
            }else{
                printf("New Client Connected\n");
            }
            child_pid = fork();
        }
        if (child_pid == 0)
        {            
            // Attach to the shared memory segment
            sharedData = shmat(shmId, NULL, 0);
            if (sharedData == (void*)-1) {
                perror("shmat");
                exit(1);
            }

            char event[msgSIZE];
            int bytesReceived = recv(sd_actual, event, sizeof(event), 0);
            if (bytesReceived == -1) {
                printf("Error Disconnection\n");
                perror("recv");
                exit(1);
            }
            event[bytesReceived] = '\0';
            char *eventData[50];
            int dataNum = 0;
            char* data;
            data = strtok(event, "\n");
            while (data != NULL) {
                eventData[dataNum] = malloc(strlen(data) + 1);  // Allocate memory for the string
                // Copy the string to the allocated memory
                strcpy(eventData[dataNum], data);
                dataNum++;
                data = strtok(NULL, "\n");
            }
            char action[50];
            strcpy(action, eventData[0]);
            printf("%s\n",action);
            if(!strcmp(action,"test")){
                int sent;
                char retMsg[msgSIZE];
                strcpy(retMsg, "connected");
                sent = send(sd_actual, retMsg, strlen(retMsg), 0);
                if ( sent == -1) {
                    printf("%s\n",retMsg);
                    perror("send");
                    exit(1);
                }
                printf("Client Disconnected\n");
                close(sd_actual);
                exit(1);
            }else if(!strcmp(action,"login")){
                char username[21];
                strcpy(username, eventData[1]);
                decipherString(username);
                char password[21];
                strcpy(password, eventData[2]);
                decipherString(password);
                int correctLogin = 0;
                int sent;
                char retMsg[msgSIZE];
                for (int i = 0; i < sharedData->numUsers; i++)
                {
                    if (strcmp(sharedData->usersCredentials[i].username,username)==0)
                    {
                        if (strcmp(sharedData->usersCredentials[i].password,password)==0)
                        {
                            correctLogin = 1;
                        }
                        break;
                    }
                    
                }
                printf("Login: %d\n",correctLogin);  
                if (correctLogin)
                {
                    strcpy(retMsg, "1");
                }else{
                    strcpy(retMsg,"0");
                }             
                sent = send(sd_actual, retMsg, strlen(retMsg), 0);
                if ( sent == -1) {
                    printf("%s\n",retMsg);
                    perror("send");
                    exit(1);
                }
                printf("Client Disconnected\n");
                // Detach from the shared memory segment
                if (shmdt(sharedData) == -1) {
                    perror("shmdt");
                    exit(1);
                }
                close(sd_actual);
                exit(1);
            }else if(!strcmp(action,"register")){
                char username[21];
                strcpy(username, eventData[1]);
                decipherString(username);
                char password[21];
                strcpy(password, eventData[2]);
                decipherString(password);
                int correctRegister = 1;
                int sent;
                char retMsg[msgSIZE];
                for (int i = 0; i < sizeof(sharedData->numUsers); i++)
                {
                    if(strcmp(sharedData->usersCredentials[i].username,username)==0){
                        correctRegister = 0;
                        break;
                    }
                }
                if (correctRegister)
                {
                    strcpy(sharedData->usersCredentials[sharedData->numUsers].username, username);
                    strcpy(sharedData->usersCredentials[sharedData->numUsers].password, password);
                    sharedData->numUsers++;
                    // Add Username and Password to users.txt
                    FILE *archivo;
                    archivo = fopen("users.txt", "a"); // Abre el archivo en modo de escritura
                    if (archivo == NULL) {
                        printf("No se pudo crear el archivo.\n");
                        return 1;
                    }
                    // Check if the last character of the file is a newline character
                    fseek(archivo, -1, SEEK_END);
                    char lastChar;
                    if (ftell(archivo) > 0) {
                        lastChar = fgetc(archivo);
                    } else {
                        lastChar = '\n'; // If file is empty, set lastChar to null character
                    }
                    char newUsername[30];
                    strcpy(newUsername, username);
                    strcat(newUsername, "\n");
                    fprintf(archivo, "%s", newUsername);
                    char newPassword[30];
                    strcpy(newPassword, password);
                    strcat(newPassword, "\n");
                    fprintf(archivo, "%s", newPassword);
                    fclose(archivo); // Close the file
                    //
                    // Create Username.groups to save each group the user joins
                    FILE *archivo2;
                    char userGroupsTxt[40];
                    strcpy(userGroupsTxt, username);
                    strcat(userGroupsTxt, ".groups");
                    archivo2 = fopen(userGroupsTxt, "w"); // Abre el archivo en modo de escritura
                    if (archivo2 == NULL) {
                        printf("No se pudo crear el archivo.\n");
                        return 1;
                    }
                    fclose(archivo2); // Close the file
                    //
                    strcpy(retMsg, "1");
                }else{
                    strcpy(retMsg,"0");
                }             
                sent = send(sd_actual, retMsg, strlen(retMsg), 0);
                if ( sent == -1) {
                    printf("%s\n",retMsg);
                    perror("send");
                    exit(1);
                }
                printf("Client Disconnected\n");
                // Detach from the shared memory segment
                if (shmdt(sharedData) == -1) {
                    perror("shmdt");
                    exit(1);
                }
                close(sd_actual);
                exit(1);
            }else if(!strcmp(action,"create")){
                char groupName[30];
                strcpy(groupName, eventData[1]);
                decipherString(groupName);
                char username[30];
                strcpy(username, eventData[2]);
                decipherString(username);
                int groupCreated = 0;
                int sent;
                char retMsg[msgSIZE];
                for (int i = 0; i < sharedData->numGroups; i++)
                {
                    printf("%s %s\n",sharedData->groups[i].groupName, groupName);
                    if(strcmp(sharedData->groups[i].groupName,groupName)==0){
                        groupCreated = 1;
                        break;
                    }
                }
                if (groupCreated)
                {
                    strcpy(retMsg, "0");
                }else{
                    strcpy(retMsg,"1");
                    //
                    strcpy(sharedData->groups[sharedData->numGroups].groupName,groupName);
                    sharedData->groups[sharedData->numGroups].numUsers = 0;
                    sharedData->groups[sharedData->numGroups].numPendUsers = 0;
                    sharedData->groups[sharedData->numGroups].numMssgs = 0;
                    // Create groupname.conv conversation
                    FILE *groupconvTxt;
                    char convTxt[30];
                    strcpy(convTxt, groupName);
                    strcat(convTxt, ".conv");
                    groupconvTxt = fopen(convTxt, "w"); // Abre el archivo en modo de escritura
                    if (groupconvTxt == NULL) {
                        printf("No se pudo crear el archivo.\n");
                        return 1;
                    }
                    fclose(groupconvTxt); // Cierra el archivo
                    printf("Archivo conv creado exitosamente.\n");
                    // Create groupname.user users and write username as 'username #', # = {0:member, 1:admin, 2:creator}
                    FILE *groupusersTxt;
                    char usersTxt[30];
                    strcpy(usersTxt, groupName);
                    strcat(usersTxt, ".users");
                    groupusersTxt = fopen(usersTxt, "w");
                    if (groupusersTxt == NULL) {
                        printf("No se pudo crear el archivo.\n");
                        return 1;
                    }
                    // Write content to the file
                    char groupCreator[30];
                    strcpy(groupCreator, username);
                    strcat(groupCreator, " 2\n");
                    fprintf(groupusersTxt, "%s", groupCreator);
                    fclose(groupusersTxt); // Cierra el archivo
                    printf("Archivo users creado exitosamente.\n");
                    //
                    strcpy(sharedData->groups[sharedData->numGroups].groupUsers[sharedData->groups[sharedData->numGroups].numUsers].username,groupCreator);
                    sharedData->groups[sharedData->numGroups].groupUsers[sharedData->groups[sharedData->numGroups].numUsers].username[strlen(sharedData->groups[sharedData->numGroups].groupUsers[sharedData->groups[sharedData->numGroups].numUsers].username)] = '\0';
                    sharedData->groups[sharedData->numGroups].groupUsers[sharedData->groups[sharedData->numGroups].numUsers].role = 2;
                    sharedData->groups[sharedData->numGroups].numUsers++;
                    //
                    //
                    // Add groupname to groups.txt
                    FILE *groupsTxt;
                    groupsTxt = fopen("groups.txt", "a"); // Abre el archivo en modo de escritura
                    if (groupsTxt == NULL) {
                        printf("No se pudo crear el archivo.\n");
                        return 1;
                    }
                    // Check if the last character of the file is a newline character
                    fseek(groupsTxt, -1, SEEK_END);
                    char lastChar;
                    if (ftell(groupsTxt) > 0) {
                        lastChar = fgetc(groupsTxt);
                    } else {
                        lastChar = '\n'; // If file is empty, set lastChar to null character
                    }
                    char newGroup[30];
                    strcpy(newGroup, groupName);
                    strcat(newGroup, "\n");
                    fprintf(groupsTxt, "%s", newGroup);
                    fclose(groupsTxt); // Close the file
                    //
                    sharedData->numGroups++;
                    // Create username.penduser pending users file
                    FILE *grouppendusersTxt;
                    char pendUsers[30];
                    strcpy(pendUsers, groupName);
                    strcat(pendUsers, ".pendusers");
                    grouppendusersTxt = fopen(pendUsers, "w");
                    if (grouppendusersTxt == NULL) {
                        printf("No se pudo abrir el archivo.\n");
                        return 1;
                    }
                    fclose(grouppendusersTxt); // Close the file
                    // Add groupname to username.groups // Save group in user.groups file
                    FILE *usergroupsTxt;
                    char userGroups[30];
                    strcpy(userGroups, username);
                    strcat(userGroups, ".groups");
                    usergroupsTxt = fopen(userGroups, "a"); // Open the file in append mode
                    if (usergroupsTxt == NULL) {
                        printf("No se pudo abrir el archivo.\n");
                        return 1;
                    }
                    // Check if the last character of the file is a newline character
                    fseek(usergroupsTxt, -1, SEEK_END);
                    char lastChar2;
                    if (ftell(usergroupsTxt) > 0) {
                        lastChar2 = fgetc(usergroupsTxt);
                    } else {
                        lastChar2 = '\n'; // If file is empty, set lastChar to null character
                    }
                    char userGroup[30];
                    strcpy(userGroup, groupName);
                    strcat(userGroup,"\n");
                    fprintf(usergroupsTxt, "%s", userGroup);
                    fclose(usergroupsTxt); // Close the file
                    //
                    for (int i = 0; i < sharedData->numUsers; i++)
                    {
                        if (strcmp(sharedData->usersCredentials[i].username, username)==0)
                        {
                            strcpy(sharedData->usersCredentials[i].groups[sharedData->usersCredentials[i].numUserGroups], groupName);
                            sharedData->usersCredentials[i].numUserGroups++;
                            break;
                        }
                        
                    }
                    
                    //
                }           
                sent = send(sd_actual, retMsg, strlen(retMsg), 0);
                if ( sent == -1) {
                    printf("%s\n",retMsg);
                    perror("send");
                    exit(1);
                }
                printf("Client Disconnected\n");
                // Detach from the shared memory segment
                if (shmdt(sharedData) == -1) {
                    perror("shmdt");
                    exit(1);
                }
                close(sd_actual);
                exit(1);
            }else if(!strcmp(action,"join")){
                char groupName[30];
                strcpy(groupName, eventData[1]);
                decipherString(groupName);
                char username[30];
                strcpy(username, eventData[2]);
                decipherString(username);
                int joinedGroup = 0;
                int sent;
                char retMsg[msgSIZE];
                for (int i = 0; i < sharedData->numGroups; i++)
                {
                    printf("%s %s %d\n",sharedData->groups[i].groupName,groupName, sharedData->groups[i].numPendUsers);
                    if(strcmp(sharedData->groups[i].groupName,groupName)==0){
                        joinedGroup = 1;
                        for (int j = 0; j < sharedData->groups[i].numPendUsers; j++)
                        {
                            printf("%s %s\n",sharedData->groups[i].groupPendUsers[j], username);
                            if(strcmp(sharedData->groups[i].groupPendUsers[j],username)==0){
                                joinedGroup = 2;
                                break;
                            }
                        }
                        for (int j = 0; j < sharedData->groups[i].numUsers; j++)
                        {
                            printf("%s %s\n",sharedData->groups[i].groupUsers[j].username, username);
                            if(strcmp(sharedData->groups[i].groupUsers[j].username,username)==0){
                                joinedGroup = 3;
                                break;
                            }
                        }
                        break;
                    }
                }
                if (joinedGroup == 1)
                {
                    strcpy(retMsg, "1");
                    FILE *archivo;
                    char pendUsersTxt[30];
                    strcpy(pendUsersTxt, groupName);
                    strcat(pendUsersTxt, ".pendusers");
                    archivo = fopen(pendUsersTxt, "a"); // Abre el archivo en modo de escritura
                    if (archivo == NULL) {
                        printf("No se pudo crear el archivo.\n");
                        return 1;
                    }
                    // Check if the last character of the file is a newline character
                    fseek(archivo, -1, SEEK_END);
                    char lastChar;
                    if (ftell(archivo) > 0) {
                        lastChar = fgetc(archivo);
                    } else {
                        lastChar = '\n'; // If file is empty, set lastChar to null character
                    }
                    char newPendUser[30];
                    strcpy(newPendUser, username);
                    strcat(newPendUser, "\n");
                    fprintf(archivo, "%s", newPendUser);
                    fclose(archivo); // Close the file
                    printf("Join Group Request Send\n");
                    //
                    for (int i = 0; i < sharedData->numGroups; i++)
                    {
                        if(strcmp(sharedData->groups[i].groupName,groupName)==0){
                            strcpy(sharedData->groups[i].groupPendUsers[sharedData->groups[i].numPendUsers],username);
                            sharedData->groups[i].numPendUsers++;
                            break;
                        }
                    }
                    //
                    //
                }else{
                    char retNumber[10];
                    sprintf(retNumber, "%d", joinedGroup);
                    strcpy(retMsg,retNumber);
                }           
                sent = send(sd_actual, retMsg, strlen(retMsg), 0);
                if ( sent == -1) {
                    printf("%s\n",retMsg);
                    perror("send");
                    exit(1);
                }
                printf("Client Disconnected\n");
                // Detach from the shared memory segment
                if (shmdt(sharedData) == -1) {
                    perror("shmdt");
                    exit(1);
                }
                close(sd_actual);
                exit(1);
            }
        }
        
    }

	/* cerrar los dos sockets */
	close(sd_actual);  
	close(sd);
    // Detach from the shared memory segment
    if (shmdt(sharedData) == -1) {
        perror("shmdt");
        exit(1);
    }
	printf("Conexion cerrada\n");
	return 0;
}