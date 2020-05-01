#include "files_parser.h"

#define JSMN_HEADER
#include "../ThirdParty/jsmn.h"
#include "../engine/engine.h"
#include "../editor/data.h"

#include "ThirdParty/parson.h"

typedef jsmntok_t Token;

Token* tokens;
Array tokens_array_memory;

unsigned int elements_count = 0;

int element_id = 0;

const char* actual_json_file;

ComponentType current_component_type;

static inline int get_token_primitive_value(Token *token);

static inline Token* get_next_object_token(Token* token){
  while(token->type != JSMN_OBJECT){
       token += 1;
  } 
  return token;
}
static inline Token* get_next_array_token(Token* token){
  while(token->type != JSMN_ARRAY){
       token += 1;
  } 
  return token;
}


static int string_equal(Token *tok, const char *s) {
	if(tok == NULL)
		return 0;

  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(actual_json_file + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

static inline size_t get_token_size(Token *t){
  size_t length = t->end - t->start;
  return length;
}


static int dump_int(Token *next_t, Token *object_t, size_t count, int indent, Element* model) {

    if (string_equal(object_t, "id") == 0) {
        size_t size = get_token_size(next_t);
        char text[size+1];
        memcpy(&text,&actual_json_file[next_t->start],size);
        text[size] = '\0';
        unsigned int model_id = atoi(text);
        memcpy(&model->id, &model_id, sizeof(unsigned int));
      }
}

static int dump_array_ints(Token *token, size_t count, int index, float* firt_array_element) {
	
	if (token->type == JSMN_PRIMITIVE) {
    float value = get_token_primitive_value(token);
    firt_array_element += index;
    memcpy(firt_array_element, &value, sizeof(float));

		return 1;
	}
}

static inline int get_token_primitive_value(Token *token){
  int size = get_token_size(token);
  char text[size+1];
  memcpy(&text,&actual_json_file[token->start],size);
  text[size] = '\0';
  unsigned int result = atoi(text);
  return result;
}

static inline float get_token_primitive_float(Token *token){
  int size = get_token_size(token);
  char text[size+1];
  memcpy(&text,&actual_json_file[token->start],size);
  text[size] = '\0';
  float result = atof(text);
  return result;
}

static inline void get_token_string(char* out, Token* token){
  size_t size = get_token_size(token);
  char text[size+1];
  memcpy(&text,&actual_json_file[token->start],size);
  text[size] = '\0';      
  strcpy(out, text);   
}

Token* get_token_array_float_values(const char* name, Token* name_token, float* out){
  if(name_token->type != JSMN_STRING){
    LOG("No token JSMN_STRING passed\n"); 
    return NULL;
  }
    
  if( string_equal(name_token,name) != 0){
    LOG("Name \"%s\" not equal\n",name);
    return NULL;
  }

  Token* array_token = name_token+1;
  Token* array_value_token = array_token+1;
  Token* last_token_readed = NULL;
  for (int i = 0; i < array_token->size; i++) {
    last_token_readed = array_value_token+i;   
    float value = get_token_primitive_float(array_value_token+i);         
    memcpy(&out[i],&value,sizeof(float));               
  }
  return last_token_readed+1;//text element, not the las JSMN_PRIMITIVE
}

Token* get_token_array_uint_values(const char* name, Token* name_token, unsigned int* out){
  if(name_token->type != JSMN_STRING){
    LOG("No token JSMN_STRING passed\n"); 
    return NULL;
  }
    
  if( string_equal(name_token,name) != 0){
    LOG("Name \"%s\" not equal\n",name);
    return NULL;
  }

  Token* array_token = name_token+1;
  Token* array_value_token = array_token+1;
  Token* last_token_readed = NULL;
  for (int i = 0; i < array_token->size; i++) {
    last_token_readed = array_value_token+i;   
    unsigned int value = get_token_primitive_float(array_value_token+i);         
    memcpy(&out[i],&value,sizeof(unsigned int));               
  }
  return last_token_readed+1;//text element, not the las JSMN_PRIMITIVE
}

/*Level Parser Start*/
Token* get_token_array_component_transform(Token* token){
  TransformComponent* transform = get_component_from_selected_element(TRASNFORM_COMPONENT);

  Token* last_token_readed = NULL;

  last_token_readed = get_token_array_float_values("position",token,transform->position);
  if(!last_token_readed)
    debug_break();


  last_token_readed = get_token_array_float_values("rotation",last_token_readed,transform->rotation);
  if(!last_token_readed)
    debug_break();


  return last_token_readed;
}

ComponentType parse__and_add_component_type(Token* type_token){
  if( type_token->type != JSMN_PRIMITIVE)
    LOG("Get type is not a primitive value\n");
  int type = get_token_primitive_value(type_token);
  current_component_type = type;
  switch (type)
  {
  case TRASNFORM_COMPONENT:
    {
      add_transform_component_to_selected_element();
    }
    break;
  case STATIC_MESH_COMPONENT:{
      StaticMeshComponent mesh_component;
      memset(&mesh_component,0,sizeof(StaticMeshComponent));
      add_component_to_selected_element(sizeof(StaticMeshComponent),&mesh_component,STATIC_MESH_COMPONENT);
    }
    break;
  case CAMERA_COMPONENT:
  {
    add_camera_component_to_selected_element();
    break;
  }
  case SPHERE_COMPONENT:
    {
      SphereComponent sphere;
      memset(&sphere,0,sizeof(SphereComponent));
      init_sphere_component(&sphere);
      add_component_to_selected_element(sizeof(SphereComponent), &sphere, SPHERE_COMPONENT);
      break;
    }
  default:
    break;
  }
  return current_component_type;
}

Token * fill_components_values(ComponentType type, Token* token_value_name_string){
  Token * last_element_readed;
  switch (type)
  {
  case TRASNFORM_COMPONENT:
  {
    LOG("Pased Transform Component\n");
    last_element_readed = get_token_array_component_transform(token_value_name_string);
    TransformComponent* transform = get_component_from_selected_element(TRASNFORM_COMPONENT);
    glm_translate(transform->model_matrix,transform->position);
    rotate_element(selected_element,transform->rotation);
    
    return last_element_readed;
  }    
  case STATIC_MESH_COMPONENT:
  {
    LOG("Parsed Static mesh component\n");
    StaticMeshComponent* mesh = get_component_from_selected_element(STATIC_MESH_COMPONENT);
    last_element_readed = token_value_name_string;

    if( string_equal(token_value_name_string, "models") == 0){
      int model_id_count = (last_element_readed+1)->size;
      array_init(&mesh->meshes,sizeof(unsigned int),model_id_count);
      mesh->meshes.count = model_id_count;
      unsigned int * models_id_array = mesh->meshes.data;
      last_element_readed = get_token_array_uint_values("models",last_element_readed,models_id_array);

      int textures_id_count = (last_element_readed+1)->size;
      array_init(&mesh->textures,sizeof(unsigned int) , textures_id_count);
      unsigned int * textures_id_array = mesh->textures.data;
      mesh->textures.count = textures_id_count;
      last_element_readed = get_token_array_uint_values("textures",last_element_readed,textures_id_array);

      return last_element_readed;
    }
    token_value_name_string++;
    
    int mesh_path_id = get_token_primitive_value(token_value_name_string);
    int texture_id = get_token_primitive_value(token_value_name_string+2);
   
    //mesh->model_id = mesh_path_id;
    //mesh->texture_id = texture_id;
    return last_element_readed;

  }
  case LEVEL_OF_DETAIL_COMPONENT:
  {
    LOG("Parsed Level of Details component\n");
    return token_value_name_string;
  }
  case SPHERE_COMPONENT:
  {

    return token_value_name_string;
  }
  case CAMERA_COMPONENT:
  {
    Token* token = token_value_name_string;
    LOG("Parsed Camera component\n");
    if( string_equal(token,"position") != 0){
      LOG("No token \"position\" string passed\n");
      return NULL;
    }
    vec3 pos;
    Token* array_token = token+1;
    Token* array_value_token = token+2;
    Token* last_token_readed = NULL;
    for (int i = 0; i < array_token->size; i++) {
      last_token_readed = array_value_token+i;   
      float value = get_token_primitive_float(array_value_token+i);         
      memcpy(&pos[i],&value,sizeof(float));               
    }
    
    CameraComponent* camera =  get_component_from_selected_element(CAMERA_COMPONENT);
    glm_vec3_copy(pos,camera->position);   

    return last_token_readed;

  }
  break;
  }
}

Token* parse_components_objects(Token* array_components){
  Token * last_element_readed = array_components;
  for(int i= 0; i< array_components->size ; i++){
    Token* object = get_next_object_token(last_element_readed);
    Token* token_primitive_type = object+2;
    ComponentType type = parse__and_add_component_type(token_primitive_type);
    last_element_readed = fill_components_values(type,object+3);    
  }
  
  return last_element_readed;
}
Token* get_components_from_token(Token* components_array){
  int components_counts = components_array->size;
  Token* last_element_parsed = parse_components_objects(components_array);
  return last_element_parsed;
}
//return last token readed
Token* parse_element_object_token(Token* object_token){
    Token* name_token = object_token+1;
    if( name_token->type == JSMN_STRING ){
      if( string_equal(name_token,"name") == 0 ){
        char name[100];
        get_token_string(name,name_token+1);
        LOG("Name: %s\n",name);
        strcpy(selected_element->name,name);
        Token* last_element_parsed = get_components_from_token(name_token+3);
        return last_element_parsed;
      }else{
        char text[100];
        get_token_string(text,name_token);
        LOG("Found: %s\n",text);
      }
    }else{
      LOG("token name not found\n");
    }
}

Token* parse_elements(Token* array_element_token){
  
  int elements_counts = array_element_token->size;
 
  Token* last_element_parsed = array_element_token;
  for(int i = 0; i< elements_counts ; i++){
    new_empty_element();
    last_element_parsed = get_next_object_token(last_element_parsed);
    last_element_parsed = parse_element_object_token(last_element_parsed);
    element_id++;
  }
  return last_element_parsed;
}

void parse_level_tokens(Token* tokens, int count){
  Token* last_element_parsed = NULL;
  for(int i = 0; i< count ; i++){
    Token* actual_token = &tokens[i];
    if(actual_token->type == JSMN_STRING){
      if( string_equal(actual_token,"elements") == 0){
        last_element_parsed =  parse_elements(actual_token+1);
        break;
      }
    }
  }

  Token* new_object = get_next_object_token(last_element_parsed);
  for(int i = 0; i< count ; i++){
    last_element_parsed = &new_object[i];
    if(last_element_parsed->type == JSMN_STRING){
        if (string_equal(last_element_parsed, "models") == 0) {
          Token* array_value_token = last_element_parsed+2;
          for (int i = 0; i < (last_element_parsed+1)->size; i++) {   
            char text[20];
            get_token_string(text,array_value_token+i);
            array_add(&pe_arr_models_paths,text);
          }
          break;
        }

    }


  }
  
	while (string_equal(last_element_parsed,"textures") != 0)
  {
    last_element_parsed++;
  }
  
  Token* texture_array = last_element_parsed+1;
  
  for(int i = 0; i < texture_array->size; i++){
    Token* value = last_element_parsed + 2;
    char text[100];
    get_token_string(text,value+i);
    array_add(&textures_paths,text);
  }

}

int parse_tokens(const char* json_file, int json_file_size){
   
  jsmn_parser parser;
  int max_tokens = 1000;
  array_init(&tokens_array_memory,sizeof(Token),max_tokens);
  for(int i = 0; i<max_tokens ; i++){
    Token token;
    memset(&token,0,sizeof(Token));
    array_add(&tokens_array_memory,&token);
  }
  tokens = tokens_array_memory.data;

  jsmn_init(&parser);
 
  actual_json_file = json_file;

  int parse_result = jsmn_parse(&parser , actual_json_file , json_file_size , tokens,max_tokens);
  switch (parse_result)
  {
  case JSMN_ERROR_INVAL:
    LOG("ERROR in parse json file ERROR_INVAL\n");
    break;
  case JSMN_ERROR_NOMEM:
    LOG("ERROR in parse json file ERROR_NOMEM\n");
    break;
  case JSMN_ERROR_PART:
    LOG("ERROR in parse json file ERROR_PART\n");
    break;
  
  default:
    break;
  }
    
  return parse_result;
}

void load_level_elements_from_json(const char* json_file, int json_file_size){
  
  int result = parse_tokens(json_file,json_file_size);

  parse_level_tokens(tokens,result);
  
  LOG("json level parsed\n");

  array_clean(&tokens_array_memory);
}

void pe_parse_single_component_with_type_id(JSON_Object *object, int type) {
  switch (type) {

  case TRASNFORM_COMPONENT: {
    JSON_Array *position_array = json_object_get_array(object, "position");
    JSON_Array *rotation_array = json_object_get_array(object, "rotation");
    JSON_Array *scale_array = json_object_get_array(object, "scale");
    vec3 position;
    vec4 rotation;
		vec3 scale;

    for (int i = 0; i < 3; i++) {
      float n = (float)json_array_get_number(position_array, i);
      position[i] = n;
    }

    for (int i = 0; i < 3; i++) {
      float n = (float)json_array_get_number(scale_array, i);
      scale[i] = n;
    }

    for (int i = 0; i < 4; i++) {
      float n = (float)json_array_get_number(rotation_array, i);
      rotation[i] = n;
    }

    add_transform_component_to_selected_element();
    TransformComponent *transform =
        get_component_from_selected_element(TRASNFORM_COMPONENT);
    glm_vec3_copy(position, transform->position);
    glm_vec4_copy(rotation, transform->rotation);
    glm_vec3_copy(scale, transform->scale);

    glm_translate(transform->model_matrix, transform->position);
    rotate_element(selected_element, transform->rotation);
		glm_scale(transform->model_matrix,transform->scale);

  } break;
  case COMPONENT_SKINNED_MESH: {
    SkinnedMeshComponent skin_mesh_component;
	  ZERO(skin_mesh_component); 
    add_component_to_selected_element(sizeof(SkinnedMeshComponent),&skin_mesh_component,COMPONENT_SKINNED_MESH);
	break;
	}
  case STATIC_MESH_COMPONENT: {
    StaticMeshComponent mesh_component;
    ZERO(mesh_component);
    add_component_to_selected_element(sizeof(StaticMeshComponent),
                                      &mesh_component, STATIC_MESH_COMPONENT);
    StaticMeshComponent *mesh =
        get_component_from_selected_element(STATIC_MESH_COMPONENT);

    JSON_Array *models = json_object_get_array(object, "models");

    int models_ids_count = json_array_get_count(models);
    array_init(&mesh->meshes, sizeof(u8), models_ids_count);
    for (int i = 0; i < models_ids_count; i++) {
      u8 id = (u8)json_array_get_number(models, i);
      array_add(&mesh->meshes, &id);
    }

    JSON_Array *textures = json_object_get_array(object, "textures");

    int textures_ids_count = json_object_get_count(textures);
    array_init(&mesh->textures, sizeof(u8), textures_ids_count);
    for (int i = 0; i < textures_ids_count; i++) {
      u8 id = (u8)json_array_get_number(textures, i);
      array_add(&mesh->textures, &id);
    }

  } break;
  case CAMERA_COMPONENT: {
    add_camera_component_to_selected_element();
    break;
  }
  case PE_COMP_PLAYER_START: {

    PEComponentPlayerStart player_start_comp;
    ZERO(player_start_comp);
    add_component_to_selected_element(sizeof(PEComponentPlayerStart),
                                      &player_start_comp, PE_COMP_PLAYER_START);
    player_start = selected_element;
    break;
  }
  case SPHERE_COMPONENT: {
    SphereComponent sphere;
    ZERO(sphere);
    init_sphere_component(&sphere);
    add_component_to_selected_element(sizeof(SphereComponent), &sphere,
                                      SPHERE_COMPONENT);
    break;
  }
  default:
    break;
  }
}

void pe_parse_components(JSON_Array* array){
  int count = json_array_get_count(array);
  for (int i = 0; i < count; i++) {
		JSON_Object* component_obj = json_array_get_object(array,i);
		int type = json_object_get_number(component_obj,"type");		
		pe_parse_single_component_with_type_id(component_obj,type);	
	}
}

int pe_parse_level(const char *json) {
  JSON_Value *level = json_parse_string(json);
  JSON_Array *level_array = json_object_get_array(json_object(level), "level");

  JSON_Object *elements_obj = json_array_get_object(level_array, 0);
  JSON_Object *data_ob = json_array_get_object(level_array, 1);

  JSON_Array *elements_array = json_object_get_array(elements_obj, "elements");

  int elements_count = json_array_get_count(elements_array);
  for (int i = 0; i < elements_count; i++) {
    JSON_Object *element_obj = json_array_get_object(elements_array, i);
		const char* name = json_object_get_string(element_obj,"name");
		
		new_empty_element();

		strcpy(selected_element->name,name);

		JSON_Array *component_array =
        json_object_get_array(element_obj, "components");
		pe_parse_components(component_array);	

    element_id++;
  }

	JSON_Array* data_array = json_object_get_array(data_ob,"data");
	JSON_Object* models_paths_obj = json_array_get_object(data_array,0);
	JSON_Object* textures_paths_obj = json_array_get_object(data_array,1);

	JSON_Array* models_paths_array = json_object_get_array(models_paths_obj,"models");
	JSON_Array* textures_paths_array = json_object_get_array(textures_paths_obj,"textures");

	int textures_paths_count = json_array_get_count(textures_paths_array);
	int models_paths_count = json_array_get_count(models_paths_array);

	for(int i = 0; i < textures_paths_count; i++){

		const char* path = json_array_get_string(textures_paths_array,i);
		array_add(&textures_paths,path);
	}

	for(int i = 0; i < models_paths_count ; i++){
		const char* path = json_array_get_string(models_paths_array,i);
		array_add(&pe_arr_models_paths,path);
	}	

}

/*End Level Parser */


/*---------------------------------------------- */
/*UI file parser start */

void parse_ui_tokens(int count){
  Token* last_token_readed = &tokens[0];
  last_token_readed += 3;
  for(int i = 0; i< tokens[0].size ; i++){
      
    new_empty_button();
    Button* new_button = array_get(actual_buttons_array,actual_buttons_array->count-1);
    
    Token* name_value = last_token_readed;
    get_token_string(new_button->name,name_value);

    last_token_readed = get_token_array_float_values("position", name_value+1, new_button->position);
    last_token_readed = get_token_array_float_values("size", last_token_readed, new_button->size);
    int function_id = get_token_primitive_value(last_token_readed+1);
    new_button->action_function_id = function_id;
    last_token_readed += 4;
  }
}

void pe_parser_gui(const char* json){
	JSON_Value* gui_val = json_parse_string(json);
	JSON_Array* buttons_arr = 
		json_object_get_array(json_object(gui_val),"buttons");
	if(!buttons_arr){
		LOGW("no array buttons");
	}
	int buttons_count = json_array_get_count(buttons_arr);			
	//LOG("Buttons count: %i\n",buttons_count);
	for(int i = 0; i < buttons_count ; i++){
    new_empty_button();
    Button* new_button = array_get(actual_buttons_array,actual_buttons_array->count-1);
		JSON_Object* button_json_obj = json_array_get_object(buttons_arr,i);
		strcpy(new_button->name,json_object_get_string(button_json_obj,"name"));		
		JSON_Array* pos_arr = json_object_get_array(button_json_obj,"position");
		JSON_Array* size_arr = json_object_get_array(button_json_obj,"size");
		
		new_button->position[0] = (int)json_array_get_number(pos_arr,0);	
		new_button->position[1] = (int)json_array_get_number(pos_arr,1);	


		new_button->size[0] = (int)json_array_get_number(size_arr,0);	
		new_button->size[1] = (int)json_array_get_number(size_arr,1);	
	
	}
}

void parse_gui_file(const char* json_file, int json_file_size){
  int result = parse_tokens(json_file,json_file_size);
  
  parse_ui_tokens(result);

  LOG("json UI parsed\n");

  array_clean(&tokens_array_memory);

}
/*UI file parser end*/
