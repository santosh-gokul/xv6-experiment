struct logging_params {
  char pname[16];
  uint64 syscall_num;
  uint64 integer_params[5];
  char   char_params[5][MAXPATH];
  char param_type[5];
  uint64 param_count;
};
