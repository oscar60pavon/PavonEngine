#ifndef EDITOR_EXPORT_MODELS_H
#define EDITOR_EXPORT_MODELS_H

typedef struct Array Array;

int data_export_models_in_array(Array* array, const char* name);
int data_export_select_element(const char* name);

#endif // !EDITOR_EXPORT_MODES_H