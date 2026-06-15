#include "stl.h"
#include <stdlib.h>
#include <string.h>

const char *unidades_nombre[] = {
    [MM]   = "mm",
    [CM]   = "cm",
    [M]    = "m",
    [IN]   = "pulgadas",
    [FT]   = "pies",
    [MILS] = "milis"
};

bool leer_int16_little_endian(FILE *f, int16_t *v) { 
    uint8_t bytes[2];
    if (fread(bytes, 1, 2, f) != 2) {
        return false; 
    }

    *v = (bytes[0] | (bytes[1] << 8));
    return true;
 
}

bool leer_int32_little_endian(FILE *f, int32_t *v) {  
    uint8_t bytes[4];
    if (fread(bytes, 1, 4, f) != 4) {
        return false; 
    }

    *v = (bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
    return true; 
}

bool leer_float_little_endian(FILE *f, float *v)   { 
    int32_t aux_int;
    if (!leer_int32_little_endian(f, &aux_int)) {
        return false;
    }

    memcpy(v, &aux_int, sizeof(float));
    return true; 
}

bool leer_encabezado_stl(FILE *f){     
    char tipo[3];
    int16_t reservado1, reservado2, version;
    int32_t offset;

    if (fread(tipo, 1, 3, f) != 3) return false;
    if (memcmp(tipo, "STL", 3) != 0) return false;  

    if (!leer_int16_little_endian(f, &reservado1)) return false;
    if (!leer_int16_little_endian(f, &reservado2)) return false;
    if (!leer_int16_little_endian(f, &version)) return false;
    if (version != 3) return false;  

    if (!leer_int32_little_endian(f, &offset)) return false;
    if (offset != 25) return false;  

    return true; 
}

bool leer_formato_STL(FILE *f, unidades_t *unidades, size_t *maxlong) {     
    int32_t tamanio;
    int16_t unid, dimensiones, tipo_coord, max_long;

    if (!leer_int32_little_endian(f, &tamanio)) return false;
    if (tamanio != 12) return false;

    if (!leer_int16_little_endian(f, &unid)) return false;
    if (!leer_int16_little_endian(f, &dimensiones)) return false;
    if (dimensiones != 3) return false;
    if (!leer_int16_little_endian(f, &tipo_coord)) return false;
    if (tipo_coord != 0) return false;
    if (!leer_int16_little_endian(f, &max_long)) return false;

    *unidades = unid;
    *maxlong = max_long;
    return true; 
}

bool leer_modelo_3d(FILE *f, size_t maxlong, char *etiqueta, size_t *ncoords, float **coords, size_t *nlineas, size_t **lineas){
    if (fread(etiqueta, 1, maxlong, f) != maxlong) return false;

    
    int32_t aux_ncoords;
    if (!leer_int32_little_endian(f, &aux_ncoords)) return false;
    if (aux_ncoords < 0) return false;
    *ncoords = aux_ncoords;

    *coords = malloc(sizeof(float) * 3 * (*ncoords));
    if (*coords == NULL) return false;

    for (size_t i = 0; i < (*ncoords) * 3; i++) {
        if (!leer_float_little_endian(f, &((*coords)[i]))) {
            free(*coords);
            return false;
        }
    }

   
    int32_t aux_nlineas;
    if (!leer_int32_little_endian(f, &aux_nlineas)) {
        free(*coords);
        return false;
    }
    if (aux_nlineas < 0) {
        free(*coords);
        return false;
    }
    *nlineas = aux_nlineas;

    *lineas = malloc(sizeof(size_t) * 2 * (*nlineas));
    if (*lineas == NULL) {
        free(*coords);
        return false;
    }

    for (size_t i = 0; i < (*nlineas) * 2; i++) {
        int32_t temp;
        if (!leer_int32_little_endian(f, &temp)) {
            free(*coords);
            free(*lineas);
            return false;
        }
        (*lineas)[i] = temp;
    }

    return true; 
}