/*
vasa: Visual Art's Script Archiver
Copyright (C) 2014-2016 DENDentar.
2017-2019 AlexHatsuyuki.
*/

int print_se_arc_info(wchar_t *file_name, int mode);
int extract_se_arc(wchar_t *file_name, wchar_t *dir_name, int game_id, bool duplicate_strings);
int create_se_arc(wchar_t *dir_name, wchar_t *out_dir_name, int compress_lv, int game_id, bool encrypt_scene);
//void clear_pck_info(ScenePCKhead scene_header, VarStruct *var1, VarStruct *var2, wchar_t **var1_name, wchar_t **var2_name, FileTable *file_table, wchar_t **file_name_str);
