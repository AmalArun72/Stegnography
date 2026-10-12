#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);

    uint size = ftell(fptr);

    rewind(fptr);

    return size;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret);

    uint required_capacity =
        54 + //Header file
        32 +  // Magic string size (int = 32 bits)
        (strlen(MAGIC_STRING) * 8) +  // Magic string
        32 +  // Extension size (int = 32 bits)
        (strlen(encInfo->extn_secret_file) * 8) +  // Extension
        32 +  // Secret data size (int = 32 bits)
        (encInfo->size_secret_file * 8);  // Secret data

    if (encInfo->image_capacity >= required_capacity)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_dest_image)
{
    char header[54];

    rewind(fptr_src_image);

    if (fread(header, 54, 1, fptr_src_image) != 1)
    {
        return e_failure;
    }

    if (fwrite(header, 54, 1, fptr_dest_image) != 1)
    {
        return e_failure;
    }

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	
    	printf("Unable to open source image\n");
        return e_failure;;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	printf("Unable to open secret file\n");
        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	printf("Unable to open stego image\n");
        return e_failure;
    }

    // No failure return e_success
    return e_success;
}


Status close_files(EncodeInfo *encInfo)
{
    if (encInfo->fptr_src_image != NULL)
    {
        fclose(encInfo->fptr_src_image);
        encInfo->fptr_src_image = NULL;
    }

    if (encInfo->fptr_secret != NULL)
    {
        fclose(encInfo->fptr_secret);
        encInfo->fptr_secret = NULL;
    }

    if (encInfo->fptr_stego_image != NULL)
    {
        fclose(encInfo->fptr_stego_image);
        encInfo->fptr_stego_image = NULL;
    }

    return e_success;
}

Status encode_size_extn(EncodeInfo *encInfo)
{
    int size = strlen(encInfo->extn_secret_file);

    return encode_secret_file_extn_size(size, encInfo);
}

Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_failure)
    {
        printf("FILE OPENING NOT SUCCESSFUL\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("FILE OPENED SUCCESSFULLY\n");

    if(check_capacity(encInfo)==e_failure)
    {
        printf("INSUFFICIENT IMAGE CAPACITY\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("IMAGE CAPACITY VALIDATED SUCCESSFULLY\n");

    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_failure)
    {
        printf("FAILED TO COPY BMP HEADER\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("BMP HEADER COPIED SUCCESSFULLY\n");
    if (encode_magic_string_size(strlen(MAGIC_STRING), encInfo) == e_failure)
    {
        printf("FAILED TO ENCODE MAGIC STRING SIZE\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("MAGIC STRING SIZE ENCODED SUCCESSFULLY\n");
    if(encode_magic_string(MAGIC_STRING, encInfo)==e_failure)
    {
        printf("FAILED TO ENCODE MAGIC STRING\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("MAGIC STRING ENCODED SUCCESSFULLY\n");
    if (encode_size_extn(encInfo) == e_failure)
    {
        printf("FAILED TO ENCODE EXTENSION SIZE\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("EXTENSION SIZE ENCODED SUCCESSFULLY\n");
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_failure)
    {
        printf("FAILED TO ENCODE SECRET FILE EXTENSION\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("SECRET FILE EXTENSION ENCODED SUCCESSFULLY\n");
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_failure)
    {
        printf("FAILED TO ENCODE SECRET FILE SIZE\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("SECRET FILE SIZE ENCODED SUCCESSFULLY\n");
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("FAILED TO ENCODE SECRET DATA\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("SECRET FILE DATA ENCODED SUCCESSFULLY\n");
    if (copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
    {
        printf("FAILED TO COPY REMAINING IMAGE DATA\n");
        close_files(encInfo);
        return e_failure;
    }
    printf("REMAINING IMAGE DATA COPIED SUCCESSFULLY\n");

    close_files(encInfo);
    return e_success;
}

 // Function to encode a byte of data into the least significant bits of the image buffer
Status encode_byte_to_lsb(char data,char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] =(image_buffer[i] & 0xFE)| ((data >> (7 - i)) & 1);
    }

    return e_success;
}
// Function to encode the size of the secret data into the least significant bits of the image buffer
Status encode_size_to_lsb(long size, char *imageBuffer)
{
   
   for (int i = 0; i < 32; i++)
    {
        imageBuffer[i]=(imageBuffer[i]&0xFE)|((size>>(31-i))&1);
    }
    return e_success;
}

Status encode_magic_string_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];

    fread(buffer, 32, 1, encInfo->fptr_src_image);

    encode_size_to_lsb(size, buffer);

    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);

    return e_success;
}
// Function to copy the remaining image data from the source image to the stego image
Status encode_magic_string(const char *magic_string,EncodeInfo *encInfo)
{
    char buffer[8];
    for (int i = 0; magic_string[i] != '\0'; i++)
    {
        fread(buffer, 8, 1,encInfo->fptr_src_image);

        encode_byte_to_lsb(magic_string[i],buffer);

        fwrite(buffer, 8, 1,encInfo->fptr_stego_image);
    }

    return e_success;
}
// Function to encode the size of the secret file extension into the least significant bits of the image buffer
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    fread(buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(size,buffer);
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    return e_success;
}
// Function to encode the secret file extension into the least significant bits of the image buffer
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];
    for(int i=0;file_extn[i]!=0;i++)
    {
        fread(buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],buffer);
        fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}
// Function to encode the size of the secret file into the least significant bits of the image buffer
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{   char buffer[32];
    fread(buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,buffer);
    fwrite(buffer,32,1,encInfo->fptr_stego_image);
    return e_success;
}
// Function to encode the secret file data into the least significant bits of the image buffer
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char data_buffer;
    char buffer[8];
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        fread(&data_buffer,1,1,encInfo->fptr_secret);
        fread(buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(data_buffer,buffer);
        fwrite(buffer,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}
// Function to copy the remaining image data from the source image to the stego image
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer;
    while(fread(&buffer,1,1,fptr_src)==1)
    {
        fwrite(&buffer,1,1,fptr_dest);
    }
    return e_success;
}