#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

//Read and validate input stego image file and output secret filename
Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo)
{
    //check if the input file extension is a valid BMP format
    if(strstr(argv[2],".bmp") != NULL)
    {
        decInfo->stego_file = argv[2];
    }
    else
    {
        printf(RED "\nError: Input image must be a .bmp file\n"RESET);
        return e_failure;
    }

    //check if user passed optional arg for output secret file
    if(argv[3] != NULL)
    {
          strtok(argv[3],".");
          strcpy(decInfo->secret_file,argv[3]);
    }
    else
    {
        strcpy(decInfo->secret_file,"decoded");   //Default ouput file name
    }
    return e_success;
}

//opens the input stego BMP image in binary read mode
Status open_output_image_file(DecodeInfo *decInfo)
{
     decInfo->fp_stego = fopen(decInfo->stego_file,"rb"); //opens stego image file
     if(decInfo->fp_stego == NULL)
     {
        printf(RED "\nError: Unable to open the file %s\n" RESET,decInfo->stego_file);
        return e_failure;
     }
     return e_success;   //return success if file opened
}

//Skips first 54 bytes BMP header 
Status skip_header(FILE *fp_stego)
{
    fseek(fp_stego , 54, SEEK_SET);
    return e_success;
}

//decode one byte of message from the LSBs of 8 image byte
char decode_byte_from_lsb(unsigned char *arr)
{
    char ch = 0,ch1;
    for(int i = 0 ; i < 8 ; i++)
    {
        ch1 = arr[i] & 1 ;
        ch = ch | ch1 << i;
    }
    return ch;
}

//decode string from stego image
Status decode_string_from_image(int size , DecodeInfo *decInfo , char *arr)
{
    int i;
    char buffer[8];  //to store pixel data
    for(i = 0 ; i < size ; i++)
    {
        //Read 8 bytes from stego image
         if(fread(buffer , 1, 8,decInfo->fp_stego) != 8)
         {
            printf(RED "\nError: Unable to read string\n"RESET);
            return e_failure;
         }
         //Decode one char from 8 byte data
         arr[i] = decode_byte_from_lsb(buffer);
         
    }
   arr[i] = '\0';
   return e_success; 
}

//Extracts and verifiesthe magicstring from stego file
Status decode_magic_string(DecodeInfo *decInfo)
{
    if(decode_string_from_image(strlen(MAGIC_STRING),decInfo,decInfo->magicstring) == e_failure)
    {
         return e_failure;
    }

    //vallidate magic string
    if(strcmp(decInfo->magicstring,"#*") == 0)
    {
        printf(GREEN "\nMagic string is matched successfully\n"RESET);
        return e_success;
    }
    else
    {
        printf(RED "\nError: Magic string mismatched\n"RESET);
        return e_failure;  
    }
}

//integer from the LSBs of 32 image bytes
int decode_int_from_lsb(unsigned char *arr)
{
    int ch = 0,ch1;
    for(int i = 0 ; i < 32 ; i++)
    {
        ch1 = arr[i] & 1;
        ch = ch | (ch1 << i);
    }
    return ch;
}

//decode size
Status decode_size(long int *data , FILE *fp_stego)
{
    int size;
    char arr[32];  //to read 32 image bytes
    if(fread(arr,1,32,fp_stego) != 32)
    {
        return e_failure;
    }
    size = decode_int_from_lsb(arr);
    *data = size;
    return e_success;
}

//decode the size of extension
Status decode_size_of_extn(DecodeInfo *decInfo)
{
    if(decode_size(&(decInfo->size_extn) , decInfo->fp_stego) == e_failure)
    {
        return e_failure;
    }
    return e_success;
}

//Decodes the extension string name from the stego image
Status decode_extension(DecodeInfo *decInfo)
{
    if(decode_string_from_image(decInfo->size_extn , decInfo , decInfo->extension) == e_failure)
    {
        printf(RED "\nError: Unable to read extension string\n"RESET);
        return e_failure;
    }
    strcat(decInfo->secret_file,decInfo->extension);
    return e_success;
}

//Decodes the size of the data
Status decode_size_of_data(DecodeInfo *decInfo)
{
    if(decode_size(&(decInfo->size_data),decInfo->fp_stego) == e_failure)
    {
        return e_failure;
    }
    return e_success;
}

//open the secret file
Status open_secret_file(DecodeInfo *decInfo)
{
    decInfo->fptr_secret = fopen(decInfo->secret_file,"wb");
    if(decInfo->fptr_secret == NULL)
    {
        printf(RED "\nError: Unable to create secret file\n"RESET);
        return e_failure;
        
    }
    return e_success;
}

//Decodes the secret data intosecret file
Status decode_secret_data(DecodeInfo *decInfo)
{
   if(open_secret_file(decInfo) == e_failure)
   {
      printf(RED "\nError: unable to open the file to store secret data\n"RESET);
      return e_failure;
   }
   printf(GREEN "\nSuccessfully opened the file to store secret data\n"RESET); 
   char *data = (char*)malloc(decInfo->size_data + 1);

   if(data == NULL)
   {
    printf(RED "\nError: memory is not allocated\n"RESET);
    fclose(decInfo->fptr_secret);
    return e_failure;
   }
   if(decode_string_from_image(decInfo->size_data , decInfo , data) == e_failure)
   {
      printf(RED "\nError: Unable to decode secret data from image\n"RESET);
      free(data);
      fclose(decInfo->fptr_secret);
      return e_failure;
   }
   if(fwrite(data , 1 , decInfo->size_data , decInfo->fptr_secret) != decInfo->size_data)
   {
     printf(RED "\nError: Unable to write data to secret file\n"RESET);
     free(data);
     fclose(decInfo->fptr_secret);
     return e_failure;
   }
   free(data);
   return e_success;
}

//Decoding process
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_output_image_file(decInfo) == e_success)
    {
       printf(GREEN "\nSuccessfully opened the input image file\n"RESET);
    }
    else
    {
       printf(RED "\nError: Unable to open the input stego image file\n"RESET);   
       return e_failure;
    }

    if(skip_header(decInfo->fp_stego) == e_success)
    {
       printf(GREEN"\nSuccessfully skip the header\n"RESET);
    }
    else
    {
       printf(RED "\nError: Unable to read and skip the header\n" RESET);   
       return e_failure;
    } 

    if(decode_magic_string(decInfo) == e_success)
    {
       printf(GREEN "\nSuccessfully decoded the magic string\n"RESET);
    }
    else
    {
       printf(RED "\nError: Unable to decode the magic string\n"RESET);   
       return e_failure;
    }  
    
    if(decode_size_of_extn(decInfo) == e_success)
    {
       printf(GREEN "\nSuccessfully decoded the size of the extension\n"RESET);
    }
    else
    {
       printf(RED "\nError: Unable to decode the size of the extension\n"RESET);   
       return e_failure;
    }   

    if(decode_extension(decInfo) == e_success)
    {
       printf(GREEN "\nSuccessfully decoded the extension\n"RESET);
    }
    else
    {
       printf(RED "\nError: Unable to decode the extension\n"RESET);   
       return e_failure;
    }

    if(decode_size_of_data(decInfo) == e_success)
    {
       printf(GREEN "\nSuccessfully decoded the size of secret data\n"RESET);
    }
    else
    {
       printf(RED "\nError: Unable to decode the size of the secret data\n"RESET);   
       return e_failure;
    } 
    
    if(decode_secret_data(decInfo) == e_success)
    {
       printf(GREEN "\nSuccessfully decoded the secret data\n"RESET);
    }
    else
    {
       printf(RED "\nError: Unable to decode the secret data\n"RESET);   
       return e_failure;
    }    
    fclose(decInfo->fp_stego);
    fclose(decInfo->fptr_secret);
    return e_success;
}