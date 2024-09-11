# Distributed Peer-to-Peer File Sharing System

## Table of Contents

- [Introduction](#introduction)
- [System Overview](#system-overview)
- [Commands](#commands)
- [How to Run](#how-to-run)
- [Features](#features)
- [Contributing](#contributing)
- [License](#license)

## Introduction

The Distributed File Sharing System is a peer-to-peer file sharing system that allows users to share, download, and manage files within groups. This README provides an overview of the system, its features, and instructions on how to run it.

## System Overview

The system consists of two main components: the Tracker and the Client.

- **Tracker**: Manages group information and file tracking. It facilitates user account management, group creation, and file uploads. Multiple trackers can be set up, allowing for a robust and scalable network.

- **Client**: Users interact with the system through the client application. They can create user accounts, join groups, upload files, and download shared files. The client communicates with the tracker to perform these actions.

## Commands

The system supports various commands that users can execute through the client application:

- **Create User Account**: `create_user <user_id> <passwd>`
- **Login**: `login <user_id> <passwd>`
- **Create Group**: `create_group <group_id>`
- **Join Group**: `join_group <group_id>`
- **Leave Group**: `leave_group <group_id>`
- **List Pending Join Requests**: `list_requests <group_id>`
- **Accept Group Joining Request**: `accept_request <group_id> <user_id>`
- **List All Groups in Network**: `list_groups`
- **List All Sharable Files in Group**: `list_files <group_id>`
- **Upload File**: `upload_file <file_path> <group_id>`
- **Download File**: `download_file <group_id> <file_name> <destination_path>`
- **Logout**: `logout`

## How to Run

Follow these steps to run the Distributed File Sharing System:

1. Start the Tracker:
   - Run Tracker: `./tracker tracker_info.txt tracker_no`
   - `tracker_info.txt` should contain IP and port details of all trackers.
   - To close the Tracker: `quit`

2. Start a Client:
   - Run Client: `./client <IP>:<PORT> tracker_info.txt`
   - `tracker_info.txt` should contain IP and port details of all trackers.

3. Execute commands as a Client to create accounts, join groups, upload/download files, and more.

## Features

- **Group-Based File Sharing**: Users can share and download files within groups.
- **Parallel Downloading**: Downloading files can be done in parallel with support for multiple pieces from multiple peers.
- **Multi-Tracker System**: The system supports multiple trackers for redundancy and scalability.
- **Custom Piece Selection Algorithm**: The system utilizes a custom piece selection algorithm for efficient downloading.

## Contributing

Contributions to this project are welcome. Feel free to open issues or submit pull requests to help improve the system.

