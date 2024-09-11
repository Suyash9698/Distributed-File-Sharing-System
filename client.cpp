#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <thread> 
#include<map>
#include <sys/stat.h>

struct FileDetails {
    int clientConn;
    int total_fileSize;
    int uploaded_fileSize;
    int curr_fileSize;
    char file_path[1024];
    int chunk_number;
    int startByte;
    int endByte;
};

std::map<std::pair<std::string, std::string>, FileDetails> fileDetailsMap;

void sendFileDetails(int clientSocket, const FileDetails& details) {
    // Serialize the FileDetails structure to a buffer
    char buffer[sizeof(FileDetails)];
    memcpy(buffer, &details, sizeof(FileDetails));

    // Send the serialized data to the server
    ssize_t sentBytes = send(clientSocket, buffer, sizeof(FileDetails), 0);
    if (sentBytes <= 0) {
        perror("Send error");
        close(clientSocket);
    }
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Please provide 3 necessary arguments." << std::endl;
        return 1;
    }
    int clientConn = socket(AF_INET, SOCK_STREAM, 0);
    if (clientConn == -1) {
        perror("Client Connection Error");
        return 1;
    }

    // Extract IP address and port number from command line arguments
    std::string ipAddressStr = argv[1];
    std::string portStr = argv[2];

    struct sockaddr_in serverBinding;
    serverBinding.sin_family = AF_INET;

   
    if (inet_pton(AF_INET, argv[1], &(serverBinding.sin_addr)) <= 0) {
        std::cerr << "Invalid IP address." << std::endl;
        return 1;
    }

    
    int port;
    try {
        port = std::stoi(argv[2]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid port number." << std::endl;
        return 1;
    }
    serverBinding.sin_port = htons(port);

    // //const char* filename = "suyash.txt";
    // struct sockaddr_in serverBinding;
    // serverBinding.sin_family = AF_INET;
    // serverBinding.sin_port = htons(3006);
    // serverBinding.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientConn, (struct sockaddr*)&serverBinding, sizeof(serverBinding)) == -1) {
        perror("client connect error");
        return 1;
    }

    std::cout << "Server is connected" << std::endl;
    bool check = false;
    while(true){
    
    int num;

    std::string input;
    std::cout<< "$ " << std::flush;
    
    // Read the entire line of input
    std::getline(std::cin, input);
    
    char* inputStr = const_cast<char*>(input.c_str()); // Convert string to char array
    
    const char* delimiter = " ";
    
    char* token = strtok(inputStr, delimiter);
    std::vector<std::string> commands;

    while (token != nullptr) {
        commands.push_back(token);
        token = strtok(nullptr, delimiter);
    }
    

    int arc = commands.size(); 
    char* arv[arc]; 

    for (int i = 0; i < arc; ++i) {
        arv[i] = const_cast<char*>(commands[i].c_str());
    }

    std::string command = arv[0];

    if(command == "quit"){
        num = 1000;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }
        std::cout<<"logout successfully"<<std::endl;
        close(clientConn);
        return 0;
    }

    if (command == "login") {
        if(arc != 3){
            std::cout<<"Please provide other 2 arguments"<<std::endl;
        }
        else{
        num = 1;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }
        // if(argc != 3){
        //     std::cout<<"Insufficient number of arguments. Please provide 3 arguments"<<std::endl;
        //     return 1;
        // }

        char response1[1024];
        char response2[1024];
        memset(response1, 0, sizeof(response1));
        memset(response2, 0, sizeof(response2));

        std::string arv1 = arv[1];
        std::string arv2 = arv[2];

        // Copy the contents of arv1 to response1
        strncpy(response1, arv1.c_str(), sizeof(response1) - 1);
        response1[sizeof(response1) - 1] = '\0';  // Null-terminate the string

        // Copy the contents of arv2 to response2
        strncpy(response2, arv2.c_str(), sizeof(response2) - 1);
        response2[sizeof(response2) - 1] = '\0';  // Null-terminate the string


        
        ssize_t sentBytes1 = send(clientConn, response1, sizeof(response1), 0);
        ssize_t sentBytes2 = send(clientConn, response2, sizeof(response2), 0);

        if (sentBytes1 <= 0 || sentBytes2 <= 0) {
            std::cerr << "Credentials sending failed." << std::endl;
            close(clientConn);
            return 1;
        }

        char response[1024];
        memset(response, 0, sizeof(response));
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        std::cout << response << std::endl;
        }
}


    if (command == "create_user") {
        if(arc != 3){
            std::cout<<"Please provide other 2 arguments"<<std::endl;
        }
        else{
        num = 2;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }
        // if(argc != 3){
        //     std::cout<<"Insufficient number of arguments. Please provide 3 arguments"<<std::endl;
        //     return 1;
        // }
        char response1[1024];
        char response2[1024];
        
        std::string arv1 = arv[1];
        std::string arv2 = arv[2];

        // Copy the contents of arv1 to response1
        strncpy(response1, arv1.c_str(), sizeof(response1) - 1);
        response1[sizeof(response1) - 1] = '\0';  // Null-terminate the string

        // Copy the contents of arv2 to response2
        strncpy(response2, arv2.c_str(), sizeof(response2) - 1);
        response2[sizeof(response2) - 1] = '\0';  // Null-terminate the string

        ssize_t sentUser = send(clientConn, response1, sizeof(response1), 0);
        ssize_t sentPass = send(clientConn, response2, sizeof(response2), 0);
        if (sentUser <= 0 || sentPass <= 0) {
                perror("Send error");
                close(clientConn);
                return 1;
        }

        char response[1024];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        // Print the message from server
        std::cout << response << std::endl;
        }
    }

    //creating a group
    if(command == "create_group"){
        if(arc != 2){
            std::cout<<"Please provide other 1 argument"<<std::endl;
        }
        else{
        num = 3;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }
        char groupName[1024];

        std::string arv1 = arv[1];

        // Copy the contents of arv1 to response1
        strncpy(groupName, arv1.c_str(), sizeof(groupName) - 1);
        groupName[sizeof(groupName) - 1] = '\0';  // Null-terminate the string

        ssize_t sentGroupName = send(clientConn, groupName, sizeof(groupName), 0);

        if (sentGroupName <= 0) {
            std::cerr << "Sending group information failed." << std::endl;
            close(clientConn);
            return 1;
        }

        char response[1024];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        // Print the message from server
        std::cout << response << std::endl;
        }
    }


    //leaving a group
    if(command == "leave_group"){
        if(arc != 2){
            std::cout<<"Please provide other 1 arguments"<<std::endl;
        }
        else{
        num = 4;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }

        char groupToBeDeleted[1024];

        std::string arv1 = arv[1];

        // Copy the contents of arv1 to response1
        strncpy(groupToBeDeleted, arv1.c_str(), sizeof(groupToBeDeleted) - 1);
        groupToBeDeleted[sizeof(groupToBeDeleted) - 1] = '\0';  // Null-terminate the string

        ssize_t sentGroupDeleted = send(clientConn, groupToBeDeleted, sizeof(groupToBeDeleted), 0);
       
        if(sentGroupDeleted <= 0){
            std::cerr << "Sending Group Deletion failed." << std::endl;
            close(clientConn);
            return 1;
        }

        char response[1024];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        // Print the message from server
        std::cout << response << std::endl;
        }
    }

    //joining a group and put it in waiting list
    if(command == "join_group"){
        if(arc != 2){
            std::cout<<"Please provide other 1 arguments"<<std::endl;
        }
        else{
        num = 7;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }

        char groupToBeJoined[1024];

        std::string arv1 = arv[1];

        // Copy the contents of arv1 to response1
        strncpy(groupToBeJoined, arv1.c_str(), sizeof(groupToBeJoined) - 1);
        groupToBeJoined[sizeof(groupToBeJoined) - 1] = '\0';  // Null-terminate the string


        ssize_t sentGroupJoined = send(clientConn, groupToBeJoined, sizeof(groupToBeJoined), 0);
       
        if(sentGroupJoined <= 0){
            std::cerr << "Sending Group Deletion failed." << std::endl;
            close(clientConn);
            return 1;
        }

        char response[1024];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        // Print the message from server
        std::cout << response << std::endl;
        }

    }


    //to fetch list of group
    if(command == "list_groups"){

        num = 5;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }
        char response[1024];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        // Print the received list of groups (response) here
        std::cout << response << std::endl;


    }

    //joining a group
    if(command == "accept_request"){
        if(arc != 3){
            std::cout<<"Please provide other 2 arguments"<<std::endl;
        }
        else{
        num = 6;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }

        char groupToBeAccepted[1024];
        std::string arv1 = arv[1];
        std::string arv2 = arv[2];

        // Copy the contents of arv1 to response1
        strncpy(groupToBeAccepted, arv1.c_str(), sizeof(groupToBeAccepted) - 1);
        groupToBeAccepted[sizeof(groupToBeAccepted) - 1] = '\0';  // Null-terminate the string



        char userToBeAccepted[1024];

        // Copy the contents of arv2 to response2
        strncpy(userToBeAccepted, arv2.c_str(), sizeof(userToBeAccepted) - 1);
        userToBeAccepted[sizeof(userToBeAccepted) - 1] = '\0';  // Null-terminate the string


        ssize_t sentGroupAccepted = send(clientConn, groupToBeAccepted, sizeof(groupToBeAccepted), 0);
        
        ssize_t sentUserAccepted = send(clientConn, userToBeAccepted, sizeof(userToBeAccepted), 0);
       
        if(sentGroupAccepted <= 0 || sentUserAccepted <= 0){
            std::cerr << "Sending Group Acceptance failed." << std::endl;
            close(clientConn);
            return 1;
        }

        char response[1024];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        // Print the message from server
        std::cout << response << std::endl;
        }

        
    }

    //printing the list of pending joining list
    if(command == "list_requests"){
        if(arc != 2){
            std::cout<<"Please provide other 1 arguments"<<std::endl;
        }
        else{
        num = 8;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }
        char groupName[1024];

        std::string arv1 = arv[1];
        // Copy the contents of arv2 to response2
        strncpy(groupName, arv1.c_str(), sizeof(groupName) - 1);
        groupName[sizeof(groupName) - 1] = '\0';  // Null-terminate the string


        ssize_t sentGroup = send(clientConn, groupName, sizeof(groupName), 0);
       
        if(sentGroup <= 0){
            std::cerr << "Sending Group for listing failed." << std::endl;
            close(clientConn);
            return 1;
        }

        char response[1024];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        // Print the message from server
        std::cout << response << std::endl;
        }

    }

    //uploading vision
    //uploading vision
if (command == "upload_file") {
    if(arc != 3){
        std::cout<<"Please provide other 2 arguments"<<std::endl;
    }
    else{
    num = 12;
    ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
    if (sentBytes <= 0) {
        perror("Send error");
        close(clientConn);
        return 1;
    }
    char filePath[1024];
    char groupName[1024];

    // Get the file path from the command
    std::string arv1 = arv[1];
    memset(filePath, 0, sizeof(filePath));
    strncpy(filePath, arv1.c_str(), sizeof(filePath) - 1);
    filePath[sizeof(filePath) - 1] = '\0';  // Null-terminate the string

    std::string arv2 = arv[2];
    memset(groupName, 0, sizeof(groupName));
    strncpy(groupName, arv2.c_str(), sizeof(groupName) - 1);
    groupName[sizeof(groupName) - 1] = '\0';  // Null-terminate the string

    ssize_t sentFilePath = send(clientConn, filePath, sizeof(filePath), 0);
    ssize_t sentGroup = send(clientConn, groupName, sizeof(groupName), 0);
       
    if(sentGroup <= 0 || sentFilePath <= 0){
        std::cerr << "Sending Group and file failed." << std::endl;
        close(clientConn);
        return 1;
    }


    // Check if the file exists
    struct stat fileInfo;
    FileDetails details;
    std::pair<std::string, std::string> key = std::make_pair(groupName, filePath);
    if (stat(filePath, &fileInfo) == 0) {
        // Check if the file details exist in the map
        if (fileDetailsMap.find(key) != fileDetailsMap.end()) {
            // Retrieve the existing FileDetails
            details = fileDetailsMap[key];

            // Update the details based on the new upload
            int chunkSize = std::min(512, details.total_fileSize - details.uploaded_fileSize);
            details.uploaded_fileSize += chunkSize;
            details.curr_fileSize = chunkSize;
            details.chunk_number++;
            details.startByte = details.uploaded_fileSize;

            details.endByte = details.uploaded_fileSize + chunkSize;

        } else {
            // Initialize a new FileDetails struct
            details.clientConn = clientConn;
            details.total_fileSize = fileInfo.st_size;
            details.uploaded_fileSize = 0;
            details.curr_fileSize = 0;
            memset(details.file_path, 0, sizeof(details.file_path));
            strncpy(details.file_path, filePath, sizeof(details.file_path) - 1);
            details.file_path[sizeof(details.file_path) - 1] = '\0';  // Null-terminate the string
            details.chunk_number = 0;
            details.startByte = 0;
            details.endByte =0;
        }

        // Send each element individually
        ssize_t sentClientConn = send(clientConn, &details.clientConn, sizeof(details.clientConn), 0);
        ssize_t sentTotalFileSize = send(clientConn, &details.total_fileSize, sizeof(details.total_fileSize), 0);
        ssize_t sentUploadedFileSize = send(clientConn, &details.uploaded_fileSize, sizeof(details.uploaded_fileSize), 0);
        ssize_t sentCurrFileSize = send(clientConn, &details.curr_fileSize, sizeof(details.curr_fileSize), 0);
        ssize_t sentFilePath = send(clientConn, details.file_path, sizeof(details.file_path), 0);
        ssize_t sentChunkNumber = send(clientConn, &details.chunk_number, sizeof(details.chunk_number), 0);
        ssize_t sentStartByte = send(clientConn, &details.startByte, sizeof(details.startByte), 0);
        ssize_t sentEndByte = send(clientConn, &details.endByte, sizeof(details.endByte), 0);

        if (sentClientConn <= 0 || sentTotalFileSize <= 0 || sentUploadedFileSize <= 0 ||
            sentCurrFileSize <= 0 || sentFilePath <= 0 || sentChunkNumber <= 0 || sentStartByte <= 0 || sentEndByte <= 0) {
            std::cout << "Error sending file details" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Error getting file size." << std::endl;
        return 1;
    }

    // Receive the response from the server
    char response[1024];
    memset(response, 0, sizeof(response));
    ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

    if (receivedBytes <= 0) {
        std::cerr << "Receive failed." << std::endl;
        close(clientConn);
        return 1;
    }

    fileDetailsMap[key] = details;
    // Print the message from the server
    std::cout << response << std::endl;
    }
}


    //list all sharable files within a group
    if(command == "list_files"){
         if(arc != 2){
        std::cout<<"Please provide other 1 arguments"<<std::endl;
    }
    else{
        num = 21;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }
        char groupName[1024];

        // Get the group name from the command
        std::string arv1 = arv[1];
        memset(groupName, 0, sizeof(groupName));
        strncpy(groupName, arv1.c_str(), sizeof(groupName) - 1);
        groupName[sizeof(groupName) - 1] = '\0';  // Null-terminate the string

        ssize_t sentGroup = send(clientConn, groupName, sizeof(groupName), 0);
       
        if(sentGroup <= 0){
            std::cerr << "Sending Group failed." << std::endl;
            close(clientConn);
            return 1;
        }
         // Receive the response from the server
        char response[1024];
        memset(response, 0, sizeof(response));
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
           std::cerr << "Receive failed." << std::endl;
           close(clientConn);
           return 1;
        }

        // Print the message from the server
        std::cout << response << std::endl;



    }

    }    

    if(command == "download_file"){
         if(arc != 4){
        std::cout<<"Please provide other 3 arguments"<<std::endl;
    }
    else{
        num = 1998;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }

        char groupName[1024];
        char fileName[1024];
        char destinationPath[1024];

        // Get the group name from the command
        std::string arv1 = arv[1];
        memset(groupName, 0, sizeof(groupName));
        strncpy(groupName, arv1.c_str(), sizeof(groupName) - 1);
        groupName[sizeof(groupName) - 1] = '\0';  // Null-terminate the string

        ssize_t sentGroup = send(clientConn, groupName, sizeof(groupName), 0);

        std::string arv2 = arv[2];
        memset(fileName, 0, sizeof(fileName));
        strncpy(fileName, arv2.c_str(), sizeof(fileName) - 1);
        fileName[sizeof(fileName) - 1] = '\0';  // Null-terminate the string

        ssize_t sentFileName = send(clientConn, fileName, sizeof(fileName), 0);

        std::string arv3 = arv[3];
        memset(destinationPath, 0, sizeof(destinationPath));
        strncpy(destinationPath, arv3.c_str(), sizeof(destinationPath) - 1);
        destinationPath[sizeof(destinationPath) - 1] = '\0';  // Null-terminate the string

        ssize_t sentDestinationPath = send(clientConn, destinationPath, sizeof(destinationPath), 0);
       
        if(sentGroup <= 0 || sentFileName <= 0 || sentDestinationPath <= 0){
            std::cerr << "Sending Download failed." << std::endl;
            close(clientConn);
            return 1;
        }

          // Receive the response from the server
        char response[1024];
        memset(response, 0, sizeof(response));
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
           std::cerr << "Receive failed." << std::endl;
           close(clientConn);
           return 1;
        }

        // Print the message from the server
        std::cout << response << std::endl;

    }

    }

    //logout from all stuffs
    if(command == "logout"){
        num = 100;
        ssize_t sentBytes = send(clientConn, &num, sizeof(num), 0);
        if (sentBytes <= 0) {
            perror("Send error");
            close(clientConn);
            return 1;
        }

        char response[1024];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t receivedBytes = recv(clientConn, response, sizeof(response), 0);

        if (receivedBytes <= 0) {
            std::cerr << "Receive failed." << std::endl;
            close(clientConn);
            return 1;
        }

        // Print the message from server
        std::cout << response << std::endl;
        
    }
    std::cout<<std::endl; 
    
}
    // int fd = open(filename, O_RDONLY);
    // if (fd == -1) {
    //     perror("File Opening Error");
    //     return 1;
    // }
    // send_file(fd, clientConn);

    close(clientConn);
    return 0;
}