#include<stdio.h>
#include<string.h>
#include "decode.h"
#include "common.h"

// Function Definitions
Status do_decoding(DecodeInfo *decInfo)                                                         
{
    if(open_decode_files(decInfo)==e_failure)                                                   
    {
        printf("FILE OPENING FAILED\n"); 
        close_decode_files(decInfo);
        return e_failure;                                                                       
    }
    printf("FILE OPENED SUCCESSFULLY\n");

    if (decode_magic_string_size(decInfo) == e_failure)
    {
    printf("FAILED TO DECODE MAGIC STRING SIZE\n");
    close_decode_files(decInfo);
    return e_failure;
    }

    printf("MAGIC STRING SIZE DECODED SUCCESSFULLY\n");

    if (decode_magic_string(decInfo) == e_failure)
    {
    printf("FAILED TO DECODE MAGIC STRING\n");
    close_decode_files(decInfo);
    return e_failure;
    }

    printf("MAGIC STRING DECODED SUCCESSFULLY\n");

    // Decode the secret file extension size
    if(decode_secret_file_extn_size(decInfo)==e_failure)                                         
    {
        printf("FAILED TO DECODE EXTENSION SIZE\n");
        close_decode_files(decInfo);
        return e_failure;                                                                        
    }
    printf("EXTENSION SIZE DECODED SUCCESSFULLY\n");

     // Decode the secret file extension
    if(decode_secret_file_extn(decInfo)==e_failure)                                                                 
    {
        printf("FAILED TO DECODE FILE EXTENSION\n");
        close_decode_files(decInfo);
        return e_failure;                                                                        
    }
    printf("FILE EXTENSION DECODED SUCCESSFULLY\n");
    // Decode the secret file size
    if(decode_secret_file_size(decInfo)==e_failure)                                                
    {
        printf("FAILED TO DECODE FILE SIZE\n");
        close_decode_files(decInfo);
        return e_failure;                                                                          
    }
    printf("FILE SIZE DECODED SUCCESSFULLY\n");
    
     // Decode the secret file data
    if(decode_secret_file_data(decInfo)==e_failure)                                                
    {
        printf("FAILED TO DECODE FILE DATA\n");
        close_decode_files(decInfo);
        return e_failure;                                                                           
    }
    printf("FILE DATA DECODED SUCCESSFULLY\n");
    return e_success;
}
 
Status open_decode_files(DecodeInfo *decInfo)                                     
{
    decInfo->fptr_stego_image=fopen(decInfo->stego_image_fname,"r");  // Open the stego image in read mode            

    if(decInfo->fptr_stego_image==NULL)                                            
    {
        printf("unable to open the file %s\n",decInfo->stego_image_fname);
        return e_failure;                                                          
    }

    decInfo->fptr_output=fopen(decInfo->output_fname,"w");   // Open the output file in write mode                       
    
    if(decInfo->fptr_output==NULL)                                                  
    {
        printf("unable to open the file %s\n",decInfo->output_fname);
        return e_failure;                                                            
    }
        return e_success;                                                            
}

Status close_decode_files(DecodeInfo *decInfo)
{
    if (decInfo->fptr_stego_image != NULL)
    {
        fclose(decInfo->fptr_stego_image);
        decInfo->fptr_stego_image = NULL;
    }

    if (decInfo->fptr_secret != NULL)
    {
        fclose(decInfo->fptr_secret);
        decInfo->fptr_secret = NULL;
    }

    if (decInfo->fptr_output != NULL)
    {
        fclose(decInfo->fptr_output);
        decInfo->fptr_output = NULL;
    }

    return e_success;
}
// Function to decode a byte from the least significant bits of the image buffer
Status decode_byte_from_lsb(char *buffer, char *data)                                        
{
    *data = 0;             
    for (int i=0;i<8;i++)                                                              
    {
        *data=(*data<<1) | (buffer[i]&1);                                               
    }
    return e_success;                                                                         
}
// Function to decode the size of the secret file extension from the least significant bits of the image buffer
Status decode_size_from_lsb(char *buffer, int *size)                                           
{
    *size = 0;
    for (int i = 0; i < 32; i++)                                                                
    {
        *size=(*size<<1) | (buffer[i]&1);                                                 
    }
    return e_success;                                                                           
}
Status decode_magic_string_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if (fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
        return e_failure;

    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return e_failure;

    decode_size_from_lsb(buffer, &decInfo->magic_string_size);

    printf("Decoded magic string size = %d\n",
           decInfo->magic_string_size);

    return e_success;
}
Status decode_magic_string(DecodeInfo *decInfo)                                      
{           
    char buffer[8];  // Buffer to hold 8 bytes of pixel data                                                                
    char decoded_char; // Variable to hold the decoded character                                                              
    char magic[10];// Buffer to hold the decoded magic string                                                                   

    int len = decInfo->magic_string_size;                                                   
    for (int i=0;i<len;i++)                                                          
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image); // Read 8 bytes of pixel data from the stego image                            
        decode_byte_from_lsb(buffer, &decoded_char);    // Decode the character from the least significant bits of the pixel data                             
        magic[i] = decoded_char;  // Store the decoded character in the magic string buffer                                                    
    }

    magic[len]='\0';                                                                   

    if (strcmp(magic, MAGIC_STRING) != 0)                                              
    {
        return e_failure;                                                               
    }
    return e_success;                                                                   
}
// Function to decode the size of the secret file extension from the least significant bits of the image buffer
Status decode_secret_file_extn_size(DecodeInfo *decInfo)                                                              
{
    char buffer[32];

    fread(buffer, 32, 1, decInfo->fptr_stego_image);                                   
    decode_size_from_lsb(buffer, &decInfo->extn_size);                                 

    return e_success;                                                                   
}
// Function to decode the secret file extension from the least significant bits of the image buffer
Status decode_secret_file_extn(DecodeInfo *decInfo)                                                                  
{
    char buffer[8];                                                                     
    char decoded_char;                                                                  
    char extn[20];                                                                      

    for (int i = 0; i < decInfo->extn_size; i++)   // Loop to decode each character of the secret file extension                                     
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);                                  
        decode_byte_from_lsb(buffer, &decoded_char);                                  
        extn[i] = decoded_char;       // Store the decoded character in the extension buffer                                                   
    }
    extn[decInfo->extn_size] = '\0';                                                     

    strcpy(decInfo->secret_fname, decInfo->output_fname);

    if (strstr(decInfo->output_fname, extn) == NULL)
    {
        strcat(decInfo->secret_fname, extn);
    }                                                  

    decInfo->fptr_secret = fopen(decInfo->secret_fname, "wb");                          

    if (decInfo->fptr_secret == NULL)                                                       
    {
        printf("Unable to create output file\n");
        return e_failure;                                                                  
    }
    return e_success;                                                                       
}
//
Status decode_secret_file_size(DecodeInfo *decInfo)                                         
{
    char buffer[32];                                                                        
    fread(buffer, 32, 1, decInfo->fptr_stego_image);                                         
    decode_size_from_lsb(buffer, &decInfo->size_secret_file);                                
    return e_success;                                                                        
}
// Function to decode the secret file data from the least significant bits of the image buffer
Status decode_secret_file_data(DecodeInfo *decInfo)                                          
{
    char buffer[8];                                                                           
    char decoded_char;                                                                       

    for (int i=0;i<decInfo->size_secret_file;i++)                                       
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);                                      
        decode_byte_from_lsb(buffer, &decoded_char);                                         
        fwrite(&decoded_char, 1, 1, decInfo->fptr_secret);                                   
    }
    return e_success;                                                                        
}