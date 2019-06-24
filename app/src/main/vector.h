#ifndef VECTOR_H
#define VECTOR_H



#include "vertex.h"

#include <memory.h>

typedef struct vertex_array{    
    size_t size;
    size_t count;
    struct Vertex* vertices;
}VertexArray;

typedef struct index_array{
    size_t size;
    size_t count;
    unsigned short int * indices;
}IndexArray;

#include "model.h"

typedef struct model_array{
    size_t size;
    size_t count;
    struct Model* models;
}ModelArray;


typedef struct array{
    size_t actual_bytes_size;
    size_t count;
    size_t element_bytes_size;
    void * data;
}Array;

void init_array(Array* array, size_t element_bytes_size);
void add_element_to_array(Array* array, void* element);

int init_model_array(ModelArray* array, size_t size);
int init_vertex_array(VertexArray* array, size_t size);
int init_index_array(IndexArray* array, size_t size);

void add_vextex_to_array(VertexArray*, struct Vertex);
void add_index_to_array(IndexArray*, unsigned short int);
void add_model_to_array(ModelArray*, struct Model);

#endif // !VECTOR_H