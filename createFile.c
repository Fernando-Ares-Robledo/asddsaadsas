#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
int main(void){
   
    printf("Creating file .txt\n");
   
    
    char buf[1024];
    printf("Input sentence: ");

    if (fgets(buf, sizeof buf, stdin)==NULL){
        fprintf(stderr, "Error sentence input\n");
        return EXIT_FAILURE;
    };
    // sanitice sentence
    size_t len= strlen(buf);
    if(len>0 && buf[len-1]=='\n'){buf[len-1]='\0';};

    FILE *f = fopen("test.txt","w");
    if(f==NULL)
    {
        fprintf(stderr,"Error con el archivo\n");
        return EXIT_FAILURE;
    };
    
    if (fprintf(f,"%s\n",buf)<0){
        fprintf(stderr,"eror al escribir en el archivo");
        fclose(f);
        return EXIT_FAILURE;
    };
    if(fclose(f)!=0){
        fprintf(stderr,"error al cerrar el archivo");
        return EXIT_FAILURE;
     
    };
    SecureZeroMemory(buf, sizeof buf);
    SecureZeroMemory(&len, sizeof len);
    return EXIT_SUCCESS;
}

