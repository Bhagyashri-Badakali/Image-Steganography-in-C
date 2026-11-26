#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

    /* check command-line argument to check operation type is it encode or decode or invalid
        Operation type returns e_encode for '-e', e_decode for '-d' and for invalid type e_unsupported */

OperationType check_operation_type(char *argv[])
{
    // Compare the first argument agrv[1]
    if(strcmp(argv[1] ,"-e") == 0)
        return e_encode;

    else if(strcmp(argv[1],"-d") == 0)
        return e_decode;

    else
        return e_unsupported;
}

int main(int argc , char *argv[])
{
    //check whether the user wants to encode or decode
    OperationType res = check_operation_type(argv);

    //for encoding process
    if(res == e_encode)
    {

        // do_encoding
        EncodeInfo encInfo;

        //Read and validate command line arguments for encoding
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            //encoding procedure
            if(do_encoding(&encInfo) == e_success)
            {
               printf(MAGENTA"\n=======================================================\n"RESET); 
               printf(YELLOW "Successfully completed encoding process\n" RESET); 
               printf(MAGENTA"\n=======================================================\n"RESET);
            }
            else
            {
                 printf(RED "Error : unable to do encoding process\n\n" RESET);
                 exit(0);
            }
        }
        else
        {
            printf(RED "Error : please enter valid arguments\n\n"RESET);
            exit(0);
        }
    }

    //for decoding process
    else if(res == e_decode)
   {

        // do_decoding
        DecodeInfo decInfo;

        //Read and validate command line arguments for encoding
        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            //encoding procedure
            if(do_decoding(&decInfo) == e_success)
            {
               printf(MAGENTA"\n=======================================================\n"RESET); 
               printf("Successfully completed decoding process\n"); 
               printf(MAGENTA"\n=======================================================\n"RESET);
            }
            else
            {
                 printf(RED "Error : unable to do decoding process\n\n" RESET);
                 exit(0);
            }
        }
        else
        {
            printf( RED "Error : please enter valid arguments\n\n" RESET);
            exit(0);
        }
    }
    //for unsuppoerted flag
    else
    {
        printf(RED "Error: Invalid argument\n\n" RESET);
        exit(0);
    }
    return 0;
}
