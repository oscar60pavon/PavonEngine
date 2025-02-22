#include "level.h"
#include "editor.h"
#include <engine/level.h>
#include <editor/data.h>

void editor_level_new(const char * name){
	LOG("New level\n");

}

void editor_level_save(const char* path){
#ifdef DESKTOP	
	content_manager_create_engine_binary(path,CONTENT_TYPE_LEVEL);
#endif
}

void editor_level_open(const char *name) {
  strcpy(opened_file_name, name);

  struct timespec time1, time2;
  int temp;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

  int level_result = load_level_to_elements_array(name, actual_elements_array);
  if (level_result != 0)
    return;

  actual_model_array = &array_models_loaded;
  for (int i = 0; i < pe_arr_models_paths.count; i++) {
    char *model_path = array_get(&pe_arr_models_paths, i);
    pe_loader_model(model_path);
  }
  actual_model_array = &editor_models;

  for (int i = 0; i < pe_arr_tex_paths.count; i++) {
    load_simple_image(array_get(&pe_arr_tex_paths, i));
  }

  for_each_element_components(&init_element_component);
  
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);

  struct timespec result = time_diffence(time1, time2);
  long millisecond = result.tv_nsec / 1000000;
  LOG("Level loading time: %ld ms\n", millisecond);
}

//for level command line  util 
int main(){
//LOG("Level editor test");
printf("level\n");

save_level_data("new_level.lvl");

return 0;
}
