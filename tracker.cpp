#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sstream>  
#include <vector> 
#include <pthread.h> 
#include <map>


char decideUser[1024];
char decidePassword[1024];
int choice;
int serverConn;
bool serverShutdownRequested = false;
std::vector<int> clientSockets;
std::map<std::string,std::string> registeredUsers;

struct ClientInfo{
    char username[1024];
    char password[1024];

};
struct Group {
    std::string name;
    std::string owner;
    std::vector<std::string> members;
};

struct FileDetails {
    int clientConn;
    int total_fileSize;
    int uploaded_fileSize;
    int curr_fileSize;
    std::string file_path;
    int chunk_number;
    int startByte;
    int endByte;
};

std::vector<Group> groups;
std::map<std::string, std::vector<std::string>> pendingJoins;
std::map<std::pair<std::string,std::string>, std::vector<FileDetails>> fileSharedDeatils;

int transferFile(const char* fileFullPAth, const char* destinationPath) {
    char buffer[512];
    int totalBytesReceived = 0;
    
    int sourceFd = open(fileFullPAth, O_RDONLY);
    if (sourceFd == -1) {
        perror("file opening error");
        return 1;
    }
    
    int destFd = open(destinationPath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (destFd == -1) {
        perror("file opening or creating error");
        close(sourceFd);
        return 1;
    }
    
    ssize_t bytesRead;

    while ((bytesRead = read(sourceFd, buffer, sizeof(buffer))) > 0) {
        ssize_t bytesWritten = write(destFd, buffer, bytesRead);
        if (bytesWritten == -1) {
            perror("file write error");
            close(sourceFd);
            close(destFd);
            return 1;
        }
        totalBytesReceived += bytesWritten;
        std::cout << "\rBytes Received: " << totalBytesReceived << std::flush;
    }

    if (bytesRead == -1) {
        perror("file reading error");
        close(sourceFd);
        close(destFd);
        return 1;
    }

    std::cout << std::endl;
    std::cout << "File received successfully and saved at: " << destinationPath << std::endl;
    close(sourceFd);
    close(destFd);
    return 0;
}

void handleClientDisconnection(int clientConn) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);
    if (getpeername(clientConn, (struct sockaddr*)&clientAddress, &clientAddressLen) == 0) {
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);
        int clientPort = ntohs(clientAddress.sin_port);

        std::cout << "Client disconnected from IP: " << clientIP << ", Port: " << clientPort << std::endl;
    }

    // Close the client socket
    close(clientConn);
}

void* clientHandler(void* arg) {
    int clientConn = *(int*)arg;
    ClientInfo clientInfo;
    while (true) {
        ssize_t receivedBytes = recv(clientConn, &choice, sizeof(choice), 0);

        if (receivedBytes <= 0) {
            perror("Receive error");
            break;
        }

        if (choice == 1000) {
            handleClientDisconnection(clientConn);
            serverShutdownRequested = true;
            close(serverConn);
            exit(0);
        }

        //login system
        if (choice == 1) {
            char response1[1024];
            char response2[1024];
            memset(response1, 0, sizeof(response1));
            memset(response2, 0, sizeof(response2));

            // Receive the username and password from the client
            ssize_t receivedUser = recv(clientConn, response1, sizeof(response1), 0);
            ssize_t receivedPass = recv(clientConn, response2, sizeof(response2), 0);

            if (receivedUser <= 0 || receivedPass <= 0) {
                std::cerr << "Receive failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }
            
            // Implement your login logic here
            if (registeredUsers.find(response1) != registeredUsers.end() && registeredUsers[response1] == response2) {
                strncpy(clientInfo.username, response1, sizeof(clientInfo.username));
                strncpy(clientInfo.password, response2, sizeof(clientInfo.password));
                
                send(clientConn, "Login Successful", sizeof("Login Successful"), 0);
            } else {
                send(clientConn, "Login Failed", sizeof("Login Failed"), 0);
            }
        }



        //registeration 
        if (choice == 2) {
            char decide[1024];
            memset(decideUser, 0, sizeof(decideUser));
            memset(decidePassword,0,sizeof(decidePassword));
            ssize_t receivedBytes1= recv(clientConn, decideUser, sizeof(decideUser), 0);
            ssize_t receivedBytes2= recv(clientConn, decidePassword, sizeof(decidePassword), 0);
            if (receivedBytes1 <= 0 || receivedBytes2<=0) {
                std::cerr << "Receiving of username and password failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }
            if (registeredUsers.find(decideUser) == registeredUsers.end()) {
                // Register the new user
                registeredUsers[decideUser] = decidePassword;

                send(clientConn, "Registration Successful", sizeof("Registration Successful"), 0);
            } else {
                send(clientConn, "Registration Failed: Username already exists", sizeof("Registration Failed: Username already exists"), 0);
            }

        }

        //creating a group
        if(choice == 3){
            if (clientInfo.username[0] == '\0' || clientInfo.password[0] == '\0') {
                send(clientConn, "Please Login first.", sizeof("Please Login first."), 0);
            }
            else{
            char groupName[1024];
            memset(groupName, 0, sizeof(groupName));
            
            // Receive the group name from the client
            ssize_t receivedGroup = recv(clientConn, groupName, sizeof(groupName), 0);
            
            if (receivedGroup <= 0) {
                std::cerr << "Receive group name failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }

            // Check if the group name already exists
            bool groupExists = false;
            for (const Group& existingGroup : groups) {
                if (existingGroup.name == groupName) {
                    groupExists = true;
                    break;
                }
            }

            if(groupExists == true){
                send(clientConn, "Group name already exists. Choose a different name.", sizeof("Group name already exists. Choose a different name."), 0);
            }


            else{
            Group newGroup;
            newGroup.name = groupName;
            newGroup.owner = std::string(clientInfo.username);

            
            //inserting full group into vector of groups
            groups.push_back(newGroup);

            char response[1024];
            snprintf(response, sizeof(response), "Group %s created successfully", groupName);

            // Send the response to the client
            ssize_t sentResponse = send(clientConn, response, strlen(response), 0);

            if (sentResponse <= 0) {
                std::cerr << "Sending response failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }
            }
            }

            

        }

        // Leaving a group
        if (choice == 4) {
            if (clientInfo.username[0] == '\0' || clientInfo.password[0] == '\0') {
                send(clientConn, "Please Login first.", sizeof("Please Login first."), 0);
            }
            else{
            char groupName[1024];
            memset(groupName, 0, sizeof(groupName));
            ssize_t receivedGroupName = recv(clientConn, groupName, sizeof(groupName), 0);
            if (receivedGroupName <= 0) {
               std::cerr << "Receiving the command failed." << std::endl;
               handleClientDisconnection(clientConn);
               return NULL;
            }

            auto groupIteration = groups.begin();
            bool groupFound = false;

            while (groupIteration != groups.end()) {
                if (groupIteration->name == groupName) {
                    bool userFound = false;

                    // Check if the user is the owner of the group
                    if (groupIteration->owner == clientInfo.username) {
                    // Transfer ownership to another member (if any)
                        if (groupIteration->members.size() > 1) {
                            groupIteration->owner = groupIteration->members[0]; // Assign ownership to the first member
                        } 
                        else {
                            groupIteration = groups.erase(groupIteration);
                        }
                    } 
                else {
                    auto it = std::find(groupIteration->members.begin(), groupIteration->members.end(), clientInfo.username);
                    if (it != groupIteration->members.end()) {
                        groupIteration->members.erase(it);
                    }
                }

                groupFound = true;
                break;
                } 
                else {
                    ++groupIteration;
                }
            }

            if (groupFound) {
                send(clientConn, "You have left the group.", sizeof("You have left the group."), 0);
            } else {
                send(clientConn, "Group not found.", sizeof("Group not found."), 0);
            }
            }
        }


        //fetching list all the groups in server
        if(choice == 5){
            if (clientInfo.username[0] == '\0' || clientInfo.password[0] == '\0') {
                send(clientConn, "Please Login first.", sizeof("Please Login first."), 0);
            }
            else{
            char response[1024];
            memset(response,0,sizeof(response));
            std::string groupList = "";
            for(const Group& group : groups){
                groupList += group.name + " ";
            }
            strncpy(response, groupList.c_str(), sizeof(response));

            if(groupList.length() == 0){
                send(clientConn, "Currently no group exists.", sizeof("Currently no group exists."), 0);
            }
            else{
                // Send the response to the client
                ssize_t sentBytes = send(clientConn, response, strlen(response), 0);

                if (sentBytes <= 0) {
                    std::cerr << "Sending group list failed." << std::endl;
                    handleClientDisconnection(clientConn);
                    return NULL;
                }
            }
            }
        }

        //waiting for join at server in list of pending 
        if(choice == 7){
            if (clientInfo.username[0] == '\0' || clientInfo.password[0] == '\0') {
                send(clientConn, "Please Login first.", sizeof("Please Login first."), 0);
            }
            else{
            char groupName[1024];
            memset(groupName,0,sizeof(groupName));
            ssize_t receivedGroupName = recv(clientConn,groupName, sizeof(groupName), 0);
            if (receivedGroupName <= 0) {
                std::cerr << "Receiving the command failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }
            bool alreadyMember = false;
            if (pendingJoins.find(clientInfo.username) != pendingJoins.end()) {
                for (const std::string& group : pendingJoins[clientInfo.username]) {
                    if (group == groupName) {
                        alreadyMember = true;
                        break;
                    }
                }
            }
            if(alreadyMember == true){
                ssize_t sentBytes = send(clientConn, "You are already a member of this group.", sizeof("You are already a member of this group."), 0);
                if (sentBytes <= 0) {
                    std::cerr << "Sending pending join requests failed." << std::endl;
                    handleClientDisconnection(clientConn);
                    return NULL;
                }
            }
            else{
                pendingJoins[clientInfo.username].push_back(std::string(groupName));
                ssize_t sentBytes = send(clientConn, "Waiting for server to accept request.", sizeof("Waiting for server to accept request."), 0);
                if (sentBytes <= 0) {
                    std::cerr << "Sending pending join requests failed." << std::endl;
                    handleClientDisconnection(clientConn);
                    return NULL;
                }
            }
            }

        }

        //list all the pending joins at server end
        if(choice == 8){
            if (clientInfo.username[0] == '\0' || clientInfo.password[0] == '\0') {
                send(clientConn, "Please Login first.", sizeof("Please Login first."), 0);
            }
            else{
            char groupName[1024];
            memset(groupName,0,sizeof(groupName));
            ssize_t receivedGroupName = recv(clientConn,groupName, sizeof(groupName), 0);
            if (receivedGroupName <= 0) {
                std::cerr << "Receiving the command failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }

            std::vector<std::string> pendingRequests;

            for (const auto& entry : pendingJoins) {
                const std::vector<std::string>& groups = entry.second;
                for (const std::string& group : groups) {
                    if (group == groupName) {
                        pendingRequests.push_back(entry.first);
                    }
                }
            }

            //if pendingRequest is empty

            if(pendingRequests.size() == 0){
                 send(clientConn, "Currently this group has no pending requests.", sizeof("Currently this group has no pending requests."), 0);
            }

            else {
                // Send the list of pending join requests to the client
                std::string pendingList = "";
                for (const std::string& request : pendingRequests) {
                    pendingList += request + ","; 
                }

                ssize_t sentBytes = send(clientConn, pendingList.c_str(), pendingList.size(), 0);

                if (sentBytes <= 0) {
                    std::cerr << "Sending pending join requests failed." << std::endl;
                    handleClientDisconnection(clientConn);
                    return NULL;
                }
            }
            }

        }

        //accepting a joing a group provide username and groupname
        if(choice == 6){
            if (clientInfo.username[0] == '\0' || clientInfo.password[0] == '\0') {
                send(clientConn, "Please Login first.", sizeof("Please Login first."), 0);
            }
            else{
            char groupName[1024];
            char username[1024];
            memset(groupName,0,sizeof(groupName));
            memset(username,0,sizeof(username));
            
            ssize_t receivedGroupName = recv(clientConn,groupName, sizeof(groupName), 0);
            ssize_t receivedUsername = recv(clientConn, username, sizeof(username), 0);

            if (receivedGroupName <= 0 || receivedUsername <= 0) {
                std::cerr << "Receiving the command failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }
            auto groupIteration = groups.begin();
            bool groupFound=false;
            bool alreadyMember = false;
            while (groupIteration != groups.end()) {
                if (groupIteration->name == groupName) {
                    //join a group
                    groupFound = true;
                    // Check if the user is already a member of the group
                    auto it = std::find(groupIteration->members.begin(), groupIteration->members.end(), username);
                    if (it != groupIteration->members.end()) {
                        alreadyMember = true;
                        std::cout<<"naa chala tha "<<std::endl;
                    }
                    else{
                        groupIteration->members.push_back(std::string(username));
                        if(pendingJoins.find(username) != pendingJoins.end()){
                            //now find that particular group
                            if(pendingJoins[username].size() == 1){
                                pendingJoins.erase(username);
                            }
                            else{
                                auto it = std::find(pendingJoins[username].begin(), pendingJoins[username].end(), groupName);
                                if(it != pendingJoins[username].end()){
                                    pendingJoins[username].erase(it);
                                }
                            }
                        }

                    }
                    break;
                }
                else {
                    ++groupIteration;
                }
            }


            if (groupFound) {
                if (alreadyMember == true) {
                    send(clientConn, "You are already a member of the group.", sizeof("You are already a member of the group."), 0);
                } else { 
                    send(clientConn, "Group joined successfully.", sizeof("Group joined successfully."), 0);
                }
            } else {
                send(clientConn, "Group not found.", sizeof("Group not found."), 0);
            }
            }

        }

    // Upload the file
    if (choice == 12) {
    FileDetails details;
    bool alreadyUploaded = false;

    char filePathName[1024];
    char groupName[1024];
    memset(groupName, 0, sizeof(groupName));
    memset(filePathName, 0, sizeof(filePathName));

    ssize_t receivedFilePathName = recv(clientConn, filePathName, sizeof(filePathName), 0);
    ssize_t receivedGroupName = recv(clientConn, groupName, sizeof(groupName), 0);
   

    if (receivedGroupName <= 0 || receivedFilePathName <= 0) {
        std::cerr << "Receiving the command failed." << std::endl;
        handleClientDisconnection(clientConn);
        return NULL;
    }
    bool isGroupPresent=false;
    for (const Group& group : groups) {
        if (group.name == groupName) {
           isGroupPresent = true;  // Found a group with the specified name
        }
    }
    
    // Receive the clientConn
    ssize_t receivedClientConn = recv(clientConn, &details.clientConn, sizeof(details.clientConn), 0);
    if (receivedClientConn <= 0) {
        perror("Receive clientConn error");
        close(clientConn);
        return NULL;
    }

    // Receive the total_fileSize
    ssize_t receivedTotalFileSize = recv(clientConn, &details.total_fileSize, sizeof(details.total_fileSize), 0);
    if (receivedTotalFileSize <= 0) {
        perror("Receive total_fileSize error");
        close(clientConn);
        return NULL;
    }

    // Receive the uploaded_fileSize
    ssize_t receivedUploadedFileSize = recv(clientConn, &details.uploaded_fileSize, sizeof(details.uploaded_fileSize), 0);
    if (receivedUploadedFileSize <= 0) {
        perror("Receive uploaded_fileSize error");
        close(clientConn);
        return NULL;
    }

    // Receive the curr_fileSize
    ssize_t receivedCurrFileSize = recv(clientConn, &details.curr_fileSize, sizeof(details.curr_fileSize), 0);
    if (receivedCurrFileSize <= 0) {
        perror("Receive curr_fileSize error");
        close(clientConn);
        return NULL;
    }

    // Receive the file_path
    char file_path_buffer[1024];
    ssize_t receivedFilePath = recv(clientConn, file_path_buffer, sizeof(file_path_buffer), 0);
    if (receivedFilePath <= 0) {
        perror("Receive file_path error");
        close(clientConn);
        return NULL;
    }
    details.file_path = std::string(file_path_buffer);

    // Receive the chunk_number
    ssize_t receivedChunkNumber = recv(clientConn, &details.chunk_number, sizeof(details.chunk_number), 0);
    if (receivedChunkNumber <= 0) {
        perror("Receive chunk_number error");
        close(clientConn);
        return NULL;
    }

    // Receive the chunk_number
    ssize_t receivedStartByte = recv(clientConn, &details.startByte, sizeof(details.startByte), 0);
    if (receivedStartByte <= 0) {
        perror("Receive chunk_number error");
        close(clientConn);
        return NULL;
    }

    // Receive the chunk_number
    ssize_t receivedEndByte = recv(clientConn, &details.endByte, sizeof(details.endByte), 0);
    if (receivedEndByte <= 0) {
        perror("Receive chunk_number error");
        close(clientConn);
        return NULL;
    }

    // Acknowledge the reception of details
    if (details.total_fileSize < details.uploaded_fileSize) {
        send(clientConn, "File was already uploaded", sizeof("File was already uploaded"), 0);
    } else {
        std::pair<std::string, std::string> key = std::make_pair(groupName, std::to_string(clientConn));

        // Check if the same file is already uploaded by the client in the same group
        if (fileSharedDeatils.find(key) != fileSharedDeatils.end()) {
            const std::vector<FileDetails>& existingFiles = fileSharedDeatils[key];
            
            for (const FileDetails& existingDetails : existingFiles) {
                if (existingDetails.file_path == details.file_path) {
                    alreadyUploaded = true;
                    break;
                }
            }

            
        }
        if(isGroupPresent == false) {
        ssize_t sentAck = send(clientConn, "Group Not Present to upload a file", sizeof("Group Not Present to upload a file"), 0);
        if (sentAck <= 0) {
            perror("Send error");
            close(clientConn);
            return NULL;
        }
        }
        else{
        if (alreadyUploaded) {
            send(clientConn, "File was already uploaded", sizeof("File was already uploaded"), 0);
        }
        else{
        ssize_t sentAck = send(clientConn, "File uploaded successfully", sizeof("File uploaded successfully"), 0);
        if (sentAck <= 0) {
            perror("Send error");
            close(clientConn);
            return NULL;
        }

        // Store the file details in your server's data structure (std::map)
        if (fileSharedDeatils.find(key) == fileSharedDeatils.end()) {
            std::vector<FileDetails> info;
            info.push_back(details);
            fileSharedDeatils[key] = info;
            std::cout<<fileSharedDeatils.size()<<std::endl;
        } else {
            fileSharedDeatils[key].push_back(details);
        }
        }
        }
    }
}

        //list all sharable files
        if(choice == 21){
            char groupName[1024];
            memset(groupName, 0, sizeof(groupName));
            ssize_t receivedGroupName = recv(clientConn, groupName, sizeof(groupName), 0);
    
            if (receivedGroupName <= 0) {
                std::cerr << "Receiving the command failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }
            bool isGroupPresent=false;
    for (const Group& group : groups) {
        if (group.name == groupName) {
           isGroupPresent = true;  // Found a group with the specified name
        }
    }

            // Iterate through the fileSharedDeatils map to find files in the same group
            bool found=false;
            std::string fileDescription="Files: ";
for (const auto& entry : fileSharedDeatils) {
    const std::pair<std::string, std::string>& key = entry.first;
    const std::vector<FileDetails>& groupFiles = entry.second;
    if (key.first == std::string(groupName)) {
        // Send the list of files to the client for this group
        for (const FileDetails& file : groupFiles) {
            found = true;
            fileDescription +=  file.file_path + '\n';
        }
    }
}

if(isGroupPresent == false) {
        ssize_t sentAck = send(clientConn, "Group Not Present only", sizeof("Group Not Present only"), 0);
        if (sentAck <= 0) {
            perror("Send error");
            close(clientConn);
            return NULL;
        }
        }
        else{
if(found == true){
    ssize_t sentFileDescription = send(clientConn, fileDescription.c_str(), fileDescription.length(), 0);
    if (sentFileDescription <= 0) {
        std::cerr << "Sending file description failed." << std::endl;
        close(clientConn);
        return NULL;
    }
}
// If no files were found for the group, send an appropriate message
if(found == false){
    ssize_t sentNoFilesMessage = send(clientConn, "No files found in group",sizeof("No files found in group"), 0);
    if (sentNoFilesMessage <= 0) {
        std::cerr << "Sending message failed." << std::endl;
        close(clientConn);
        return NULL;
    }
}
        }


        }


        //logging out from everywhere->client
        if(choice == 100){
            if (clientInfo.username[0] == '\0' || clientInfo.password[0] == '\0') {
                send(clientConn, "Please Login first.", sizeof("Please Login first."), 0);
            }
            else{
            memset(clientInfo.username,0,sizeof(clientInfo.username));
            memset(clientInfo.password,0,sizeof(clientInfo.password));
            send(clientConn, "Logout successfully.", sizeof("Logout successfully."), 0);
            }
        }

        //now sending all info to asking client
        if(choice == 1998){
            char groupName[1024];
            char fileName[1024];
            char destinationPath[1024];
            memset(groupName, 0, sizeof(groupName));
            memset(fileName, 0, sizeof(fileName));
            memset(destinationPath, 0, sizeof(destinationPath));
            

            ssize_t receivedGroupName = recv(clientConn, groupName, sizeof(groupName), 0);
            ssize_t receivedFileName = recv(clientConn, fileName, sizeof(fileName), 0);
            ssize_t receivedDestinationPath = recv(clientConn, destinationPath, sizeof(destinationPath), 0);
   

            if (receivedGroupName <= 0 || receivedFileName <= 0 || receivedDestinationPath <= 0) {
                std::cerr << "Receiving the command failed." << std::endl;
                handleClientDisconnection(clientConn);
                return NULL;
            }
             bool found=false;
             std::string message = "";
            for (const auto& pair : fileSharedDeatils) {
        const std::pair<std::string, std::string>& key = pair.first;
        std::string groupName = key.first;  // Extract the groupName
           
        if (groupName == groupName) {
            const std::vector<FileDetails>& detailsList = pair.second;
        
            for (const FileDetails& details : detailsList) {
                // Check if the file_path contains the given fileName
                if (details.file_path.find(fileName) != std::string::npos) {
                    found = true;
                   
                    std::string clientConn = key.second; 
                   
                    message +=  "Found in group: " + groupName + '\n';
                    message += "Matching filePath: " + details.file_path + '\n';
                     // Calling write_file function to download the file
                     int clientConnInt = std::stoi(clientConn);
                int result = transferFile(fileName,destinationPath);
                if (result == 0) {
                    message += "File downloaded successfully";
                } else {
                    message += "File download failed";
                }
                }
            }
        }
            }
            if(found == false){
             ssize_t sentNoFilesMessage = send(clientConn, "Either Invalid group or invalid file. File Not Present",sizeof("Either Invalid group or invalid file. File Not Present"), 0);
             if (sentNoFilesMessage <= 0) {
               std::cerr << "Sending message failed." << std::endl;
               close(clientConn);
                return NULL;
             }
          }
          else{
            ssize_t sentNoFilesMessage = send(clientConn, message.c_str(), message.length(), 0);
             if (sentNoFilesMessage <= 0) {
               std::cerr << "Sending message failed." << std::endl;
               close(clientConn);
                return NULL;
             }
          }

        }

    }

    handleClientDisconnection(clientConn);
    return NULL;
}





int main(int argc, char* argv[]) {
    serverConn = socket(AF_INET, SOCK_STREAM, 0);
    if (serverConn < 0) {
        perror("socket error");
        return 1;
    }

    struct sockaddr_in binding;
    binding.sin_family = AF_INET;
    binding.sin_port = htons(3006);
    binding.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(binding.sin_zero), 0, 8);

    int res = bind(serverConn, (struct sockaddr*)&binding, sizeof(binding));
    if (res < 0) {
        perror("failed to bind to localhost");
        return 1;
    }

    int lfd = listen(serverConn, 5);
    if (lfd == -1) {
        perror("server listening error");
        return 1;
    }

    std::cout << "server is listening" << std::endl;

    while (true) {
        struct sockaddr_in clientBinding;
        socklen_t clen = sizeof(clientBinding);
        int clientConn = accept(serverConn, (struct sockaddr*)&clientBinding, &clen);

        if (clientConn == -1) {
            if (serverShutdownRequested) {
                break;
            }
             
            perror("server accepting error");
            return 1;
        }
        clientSockets.push_back(clientConn);
        
        // Print information about the new client
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientBinding.sin_addr, clientIP, INET_ADDRSTRLEN);
        int clientPort = ntohs(clientBinding.sin_port);
        std::cout << "New client connected from " << clientIP << ", Port: " << clientPort << std::endl;

        // Create a new thread for each client connection
        pthread_t thread;
        if (pthread_create(&thread, NULL, clientHandler, &clientConn) != 0) {
            perror("pthread_create error");
            return 1;
        }
        if (serverShutdownRequested) {
            break; // Exit the loop if a shutdown is requested
        }


        // Detach the thread to prevent resource leaks
        pthread_detach(thread);
    }

    for (int clientSocket : clientSockets) {
        close(clientSocket);
    }

    close(serverConn);
    return 0;
}