#ifndef SKELETAL_EDITOR_H
#define SKELETAL_EDITOR_H

#include "../engine/skeletal.h"
#include <engine/elements.h>
#include <engine/components//skinned_mesh_component.h>

void pe_skeletal_update_draw_vertices(SkinnedMeshComponent*);
void pe_skeletal_editor_create_vertices(SkinnedMeshComponent* skin_component);
void pe_skeletal_editor_init_for(SkinnedMeshComponent* skin);
void pe_skeletal_update_draw_vertices_target(SkinnedMeshComponent* skin_component, mat4 target_model_mat);
void pe_debug_skeletal_show_bones(Element* element);

bool pe_bool_can_draw_skeletal;

void draw_skeletal_bones();
void init_skeletal_editor();

void init_skeletal_gizmo();
void update_joints_vertex();

void update_skeletal_vertices_gizmo(mat4 global, int i, Node* current_joint);
void clear_skeletal_vertices();
#endif // !SKELETAL_EDITOR_H
