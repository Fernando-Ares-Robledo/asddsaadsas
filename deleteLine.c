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

    FILE *f_temp = fopen("f_temp.txt","w");
    if (f_temp==NULL){
        fprintf(stderr,"Error al abrir el archivo: ");
        perror(NULL);
        return EXIT_FAILURE;
    };

    int contador=1;
    char line[4096];
    int linea_borrar;
    printf("line to delete: ");
    fflush(stdout);
    scanf("%i", &linea_borrar); 
    
    if(linea_borrar<0){
        fprintf(stderr, "Error en la linea a borrar cerrar: ");
        perror(NULL);
        return EXIT_FAILURE;
    };

    
    while (fgets(line,sizeof line,f)!=NULL){
        if (contador != linea_borrar){
            fputs(line,f_temp);
        };
        contador+=1;
    }





    if(fclose(f)!=0){
        fprintf(stderr, "Error al cerrar el archivo: ");
        perror(NULL);
        return EXIT_FAILURE;
    };
    if(fclose(f_temp)!=0){
        fprintf(stderr, "Error al cerrar el archivo: ");
        perror(NULL);
        return EXIT_FAILURE;
    };
    


    if(remove(buf)!=0){
        fprintf(stderr, "Error al remover el archivo: ");
        perror(NULL);
        return EXIT_FAILURE;
    };  
    
    if(rename("f_temp.txt",buf)!=0){
        fprintf(stderr, "Error al renombrar el archivo: ");
        perror(NULL);
        return EXIT_FAILURE;
    };  

    SecureZeroMemory(&contador,sizeof contador);
    SecureZeroMemory(&f_temp,sizeof f_temp);
    SecureZeroMemory(&f,sizeof f);
    SecureZeroMemory(&len, sizeof len);
    SecureZeroMemory(line, sizeof line);
    SecureZeroMemory(buf,sizeof buf);
    return EXIT_SUCCESS;
}

