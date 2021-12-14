#ifndef COMPONENT_STATIC_MESH
#define COMPONENT_STATIC_MESH
#include "components.h"
#include <engine/renderer/material.h>
typedef struct StaticMeshComponent{
    Array meshes;
    Array distances;
    Array textures;
    Array models_p;
    vec3 center;
    vec3 bounding_box[2];
    PMaterial material;
    
}StaticMeshComponent;

void pe_comp_static_mesh_init(ComponentDefinition*);
void pe_comp_static_mesh_update(ComponentDefinition*);

#endif
