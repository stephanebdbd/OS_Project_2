# Image Comparison Application (Client-Server)

**Course:** INFO-F201 - Systèmes d'exploitation (Operating Systems)
**University:** Université Libre de Bruxelles (ULB)
**Academic Year:** 2023-2024
**Project:** 2 (Client-Server Architecture with Threads & Sockets)

## 📜 Description

This project is an evolution of the first Operating Systems project. The goal was to transform a standalone image comparison application into a **Client-Server architecture**.

The application allows a client to send an image (BMP format) to a server. The server compares this image against a database of images using **Perceptual Hashing (pHash)** to find the most similar match and returns the result to the client.

The implementation makes extensive use of **multithreading**, **sockets**, and **synchronization mechanisms** (mutexes, semaphores) to handle multiple clients concurrently and perform efficient image processing.

## ✨ Features

### Client Side

* **Image Reading:** Reads BMP images from a local path provided via `stdin`.
* **Size Validation:** Ensures the image does not exceed 20KB.
* **Socket Communication:** Connects to the server, sends the image data, and waits for the result.
* **Input Validation:** Verifies that the total length of file paths does not exceed 999 characters.

### Server Side

* **Concurrent Client Handling:** Capable of handling up to **1000 simultaneous clients** using threads and semaphores.
* **Image Database:** Loads and indexes images from a specified directory using a custom file listing implementation.
* **Multithreaded Comparison:** Uses **3 worker threads** to compare the received image against the database in parallel for faster performance.
* **Synchronization:** Uses **mutexes** to protect critical sections during image comparison and **semaphores** to limit active client connections.
* **Perceptual Hashing:** Calculates a unique hash for images to determine similarity (Hamming distance).

### Signal Handling

* **SIGINT:** Gracefully shuts down the server or client.
* **SIGPIPE:** Handles broken pipe errors (e.g., if a client disconnects unexpectedly) by sending an interruption request.

## 🛠️ Technical Implementation

### Architecture

* **Language:** C
* **Communication:** TCP Sockets
* **Concurrency:** POSIX Threads (`pthread`)
* **Synchronization:** Mutexes (`pthread_mutex`) and Semaphores (`sem_t`)

### Key Functions

* **Client:** `LectureImageBMP`, `clientListener`
* **Server:** `create_socket`, `connectToClient`, `serveClient`, `compare_image`, `getPictures`, `SignalHandler`

## 🚀 Usage

### 1. Compilation

Compile both the client and server using the provided Makefile (or manual `gcc` commands).

```bash
make

```

### 2. Start the Server

Run the server, specifying the port and the directory containing the image database.

```bash
./server <port> <image_directory>

```

### 3. Run the Client

Run the client, specifying the server address and port. Then, input the path to the BMP image you want to compare.

```bash
./client <server_address> <port>
Enter image path: /path/to/image.bmp

```

## 👥 Authors

* **Haytam Benouda**
* **Chris Badi Budu**
* **Marouane Annaim**
