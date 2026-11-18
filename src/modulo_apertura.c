#include "modulo_apertura.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*-------FUNCIONES PORQUE SI NO ATHENEA SE CONFUNDE-------------*/
static bool extensionValida(const char* ext) //verifica que la extensión se valida, osea solamente nasm
{
    if(strcmp(ext, ".asm") == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool ExtensionArchivo(const char* ruta_archivo)
{
    if(ruta_archivo==NULL||*ruta_archivo=='\0')
        return false;//verifica que no este vacio o que de plano no exista el archivo

    const char* punto=strrchr(ruta_archivo,'.');//busca que haya un punto en el nombre del archivo para verificar la extensión
    if (punto==NULL|| *(punto+1)=='\0') //verifica que si haya un punto y que despues del punto si haya algo
        return false;

    //si todo sale bien verifica que la extensión se valida
    return extensionValida (punto);
}

//abre el archivo
FILE* abrirFlujoLectura(const char* ruta_archivo) 
{
    if (ruta_archivo == NULL)
        return NULL; //regresa null su no existe el archivo o no hay ruta valida
    return fopen(ruta_archivo, "r");
}

// regresa el tamaño del archivo
long calcularTamañoArchivo(FILE* archivo)
{
    
    if (archivo==NULL)
        return -1;
    
    if (fseek(archivo,0,SEEK_END)!=0) //usa fseek para buscar el final del archivo
        return -1;
    //ftell toma  la posición actual del indicador de posición actual dentro de un archivo
    long tamaño= ftell(archivo); //guarda la posición del final del archivo en tamaño
    rewind(archivo);//regrea el constador interno al byte 0

    return tamaño;
}

//booleano que valida que el archivo no es este pues, vacio. Lo dice en el nombre :P
bool validarArchivoNoVacio (long tamaño_bytes)
{
    return tamaño_bytes >0;
}

void gestionarErrorApertura(int codigo_error, const char* ruta_archivo)
{
    const char* ruta=(ruta_archivo!=NULL) ? ruta_archivo : "(desconocida)"; //hace una variable ruta con la ruta dodne sucede el error, si no lo encuentra pone un bonito (desconocida)
    switch (codigo_error)
    {
         case 1: //error de no se encontro el archivo :(
            fprintf(stderr, "ERROR: Archivo no encontrado: '%s'\n", ruta);
            break;
        case 2: //error de no tienes los permisos para leer el archivo :(
            fprintf(stderr, "ERROR: Permiso denegado: '%s'\n", ruta);
            break;
        case 3://error de la ruta que pusiste es una carpeta y no un archivo :(
            fprintf(stderr, "ERROR: La ruta es un directorio: '%s'\n", ruta);
            break;
        case 4: //error de demasiados archivos abiertos, la verdad no sabia que esto se podia pero segun lo investigado puede pasar y es malo :(
            fprintf(stderr, "ERROR: Demasiados archivos abiertos.\n");
            break;
        default: //error base, esta mal en algo:(
            fprintf(stderr, "ERROR: No se pudo abrir '%s': %s\n",ruta, strerror(codigo_error));
            break;
    }
}
//usa todas las cosas planteadas antes para abrir el archivo, este es el main
FILE* obtenerArchivoVerificado(const char* ruta_archivo,bool* estado_verificacion)
{
    if (estado_verificacion!=NULL)//esta varaible mantiene registro de como va la verificacion, si para este punto ya fue regrea un 0
    {
        *estado_verificacion=false;
    }
    if(ruta_archivo==NULL)// que encuentre la ruta vdd
        return NULL;
    if(!ExtensionArchivo(ruta_archivo))
    {
        fprintf(stderr,"error de extension");//que sea un asm
        return NULL;
    }

    FILE* archivo= abrirFlujoLectura(ruta_archivo);
    if(archivo == NULL )
    {
        gestionarErrorApertura(1,ruta_archivo);//que se abra bien vdd
        return NULL;
    }

    long tamaño= calcularTamañoArchivo(archivo);
    if (tamaño<0) //que no este inleible vdd
    {
        fprintf(stderr,"error, no se pudo calcular el tamaño del archivo");
        fclose(archivo);
        return NULL;
    } 
    if(!validarArchivoNoVacio(tamaño))//que no este vacio vdd
    {
        fprintf(stderr,"error, el archivo esta vacio");
        fclose(archivo);
        return NULL;
    }
    //si todo sale bien 
    if (estado_verificacion!=NULL)
    {
        *estado_verificacion=true;
    }
    return archivo;

    
}
int leerLineaArchivo(FILE* archivo, char* buffer_linea, int tamano_max)
    {
        if (archivo == NULL || buffer_linea == NULL || tamano_max <= 0)
            return -1;

        if (fgets(buffer_linea, tamano_max, archivo) == NULL)
        {
            if (feof(archivo))
                return 0;   //no hay error todo bien
            return -1;      // ya fue, persinense
        }
        return (int)strlen(buffer_linea);
    }
