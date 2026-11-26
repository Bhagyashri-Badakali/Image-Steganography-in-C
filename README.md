
# Image Steganography Using C

## Overview
Image Steganography is a technique used to hide secret information inside an image without visibly changing its appearance.

This project implements the Least Significant Bit (LSB) technique to encode and decode secret data within BMP images using C programming.

## Features
- Encode secret data into BMP images.
- Decode and retrieve hidden information.
- Magic string validation.
- Encode and decode secret file extensions.
- Encode and decode secret file sizes.
- File handling and error handling.

## Technologies Used
- Programming Language: C
- Image Format: BMP
- Technique: Least Significant Bit (LSB)
- Platform: Linux / WSL

## Concepts Applied
- File Handling
- Pointers
- Dynamic Memory Allocation
- Bitwise Operators
- Structures
- Modular Programming

## How to Compile

```bash
gcc *.c -o steganography
```

## How to Run

### Encoding
```bash
./steganography -e secret.txt beautiful.bmp stego.bmp
```

### Decoding
```bash
./steganography -d stego.bmp
```

## Project Workflow

1. Read the input BMP image.
2. Validate the input files.
3. Check image capacity.
4. Encode the magic string.
5. Encode the secret file extension and size.
6. Encode secret data using LSB.
7. Decode and retrieve the hidden data.

## Author
Bhagyashri Badakali

## Project Status
Completed
