#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>

int main(void){

    printf("Enter file name: \n");
    fflush(stdout);
    char buf[1024];

    if(fgets(buf,sizeof buf,stdin)==NULL){
        fprintf(stderr, "Error nombre archivo: ");
        perror(NULL);
        return EXIT_FAILURE;
    };

    //sanear nombre
    size_t len = strlen(buf);
    if(len>0 && buf[len-1]=='\n'){buf[len-1]='\0';};

    FILE *f = fopen(buf,"r");
    if (f==NULL){
        fprintf(stderr,"Error al abrir el archivo: ");
        perror(NULL);
        return EXIT_FAILURE;
    };

    char line[4096];
    while(fgets(line, sizeof line, f)!= NULL){
        fputs(line,stdout);  
    };

    if(fclose(f)!=0){
        fprintf(stderr, "Error al cerrar el archivo: ");
        perror(NULL);
        return EXIT_FAILURE;
    };
    
    SecureZeroMemory(&f,sizeof f);
    SecureZeroMemory(&len, sizeof len);
    SecureZeroMemory(line, sizeof line);
    SecureZeroMemory(buf,sizeof buf);
    return EXIT_SUCCESS;
}
