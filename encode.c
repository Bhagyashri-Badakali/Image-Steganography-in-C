#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "common.h"
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    rewind(fptr_image);
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    rewind(fptr_image);
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{

    char *dot = strchr(argv[2],'.');  //check whether dot is present in secret agr name or not
    if (dot != NULL && dot[1] != '\0')
    {
        encInfo->secret_fname = argv[2]; //storing secret file name in structure
        strcpy(encInfo->extn_secret_file,dot); //storing extension in structure
    }
    else
    {
        printf(RED "Error : Secret file must have an extension\n"RESET);
        return e_failure;
    }

    if(strstr(argv[3], ".bmp")!= NULL)
    {
        encInfo->src_image_fname = argv[3]; //Storing soucre image file name in structure
    }
    else
    {
        printf(RED "Error : Source image must be a .bmp file\n"RESET);
        return e_failure;      
    }

    if(argv[4] != NULL)
    {
        //validate the optional argument
        if(strstr(argv[4],".bmp")!= NULL)
        {
            encInfo->stego_image_fname = argv[4]; //Storing output image file name in structure
        }
        else
        {
        printf(RED "Error : Ouptut image must be a .bmp file\n"RESET);
        return e_failure;             
        }
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp"; //giving default output file name
    }

    return e_success;
}

//Comparing the capacity of image file with  secret file
Status check_capacity(EncodeInfo *encInfo)
{
    uint size_src_image = get_image_size_for_bmp(encInfo->fptr_src_image);
    rewind(encInfo->fptr_src_image); 
    //Move the secret_fptr to the last
    fseek(encInfo->fptr_secret, 0 , SEEK_END);
    encInfo->size_secret_file = ftell(encInfo->fptr_secret);
    rewind(encInfo->fptr_secret);

    //calculating overall data size which is to be encode in output file
    uint data_size = strlen(MAGIC_STRING) + sizeof(int) + strlen(encInfo->extn_secret_file) + sizeof(int) + encInfo->size_secret_file;

    if((data_size * 8) < size_src_image)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
//Copy the bmp header to output image file
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);
    char arr[54];
    if(fread(arr, 54, 1, fptr_src_image) != 1)
    {
        return e_failure;
    }
    if(fwrite(arr, 54, 1, fptr_dest_image) != 1)
    {
        return e_failure;
    }
    return e_success;
}

//Encode string to image
Status encode_string_to_image(char *str, int size, EncodeInfo *encInfo)
{
    
    for(int i = 0; i < size ; i++)
    {
        unsigned char arr[8];
        if(fread(arr, 8, 1, encInfo->fptr_src_image) != 1)
           return e_failure;
        encode_byte_to_lsb(str[i],(char*)arr);
        if(fwrite(arr, 8, 1, encInfo->fptr_stego_image) != 1)
            return e_failure;     
    }
    return e_success;
}

//Encode one byte message data to LSBs of image 8 bytes
void encode_byte_to_lsb(char ch, char *arr)
{
    for(int i = 0; i < 8; i++)
    {
        arr[i] = arr[i] & (~1);
        arr[i] = arr[i] | ((ch &(1 << i)) >> i);
    }
}

//Encode magic string into output image file 
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    if(encode_string_to_image(magic_string,strlen(magic_string),encInfo) == e_success)
    {
        return e_success;
    }
    return e_failure;
}

//Encodes the integer value to output  image file
void encode_int_to_image(int size, char *arr)
{
    for(int i = 0; i < 32; i++)
    {
        char get = (size & (1 << i)) >> i;
        arr[i] = arr[i] & (~1);
        arr[i] = arr[i] | get ;
    }
}

//Encode the size of extension
Status encode_size_extn(EncodeInfo *encInfo)
{
   unsigned char arr[32];
   if(fread(arr, 1, 32, encInfo->fptr_src_image) != 32)
      return e_failure;
    encode_int_to_image(strlen(encInfo->extn_secret_file), (char*)arr);
    if(fwrite(arr, 1, 32, encInfo->fptr_stego_image) != 32)
        return e_failure;
   return e_success;
}

//Encode secret file extension into output image file
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    if(encode_string_to_image(file_extn,strlen(file_extn), encInfo) == e_failure)
    {
      return e_failure;
    }
    return e_success;  
}

//Encode size of seceret data from secret file einto output image file
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    unsigned char arr[32];
    if(fread(arr, 1, 32, encInfo->fptr_src_image) != 32)
       return e_failure;
    encode_int_to_image(file_size, arr);
    if(fwrite(arr, 1, 32, encInfo->fptr_stego_image) != 32)
       return e_failure;
    return e_success;    
}

//Encodes secret data from secret file into output image
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    unsigned char str[encInfo->size_secret_file];
    rewind(encInfo->fptr_secret);
    
    if(fread(str, 1, encInfo->size_secret_file, encInfo->fptr_secret)!= encInfo->size_secret_file)
    {
        return e_failure;
    }

    if(encode_string_to_image((char*)str, encInfo->size_secret_file, encInfo) == e_failure)
      return e_failure;
    
    return e_success;  
}

// copy the remaining pixel data into outputimage file to get proper image after encoding

Status copy_remaining_img_data(FILE *fptr_src , FILE *fptr_dest)
{
    int ch;
    while((ch = fgetc(fptr_src)) != EOF)
    {
        if(fputc(ch , fptr_dest) == EOF)
        {
            return e_failure;
        }
    }
    if(ferror(fptr_src))
       return e_failure;
    return e_success;   
}

//Encoding Procedure
Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        printf(GREEN "\nFiles are opened successfully \n"RESET);
    }
    else
    {
        printf(RED "Error : Files are not opened\n"RESET);
        return e_failure;
    }

    if(check_capacity(encInfo) == e_success)
    {
         printf(GREEN "Yes there is capacity to encode the secret data in image file\n"RESET);
    }
    else
    {
        printf(RED "Error: No capacity to encode the secret data in image file\n"RESET);
        return e_failure;
    }

    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
       printf(GREEN "Successfully encoded the BMP header\n"RESET);
    }
    else
    {
        printf(RED "Error: Failed to copy BMP header\n"RESET);
        return e_failure;
    }

    if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)
    {
       printf(GREEN "\nSuccessfully encoded the magic string\n"RESET);
    }
    else
    {
        printf(RED "\nError: Failed to encode the magic string\n"RESET);
        return e_failure;
    }

    if(encode_size_extn(encInfo) == e_success)
    {
        printf(GREEN"\nSuccessfully encoded the size of extension\n"RESET); 
    }
    else
    {
        printf(RED "\nError: Failed to encode size of extension\n"RESET);
        return e_failure;
    }

    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_success)
    {
       printf(GREEN"\nSuccessfully encoded secret file extension\n"RESET);
    }
    else
    {
        printf(RED"\nError: Failed to encode secret file extension\n"RESET);
        return e_failure;
    }

    if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_success)
    {
       printf(GREEN"\nSuccessfully encoded secret file size\n"RESET);
    }
    else
    {
        printf(RED "\nError: Failed to encode secret file size\n"RESET);
        return e_failure;
    }

    if(encode_secret_file_data(encInfo) == e_success)
    {
        printf(GREEN "\nSuccessfully encoded the secret file data\n"RESET);
    }
    else
    {
        printf(RED "\nError: Failed to encode secret file data\n"RESET);
        return e_failure;
    }

    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
        printf(GREEN "Successfully encoded the remaining image data\n"RESET);
    }
    else
    {
         printf(RED "Error: failed to encode the reamining image data\n"RESET);
         return e_failure;
    }

    // close all the files()
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);
   return e_success;
}


