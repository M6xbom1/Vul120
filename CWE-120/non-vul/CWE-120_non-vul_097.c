YR_API int yr_compiler_add_fd(
    YR_COMPILER* compiler,
    YR_FILE_DESCRIPTOR rules_fd,
    const char* namespace_,
    const char* file_name)
{
  int result;

  // Don't allow yr_compiler_add_fd() after
  // yr_compiler_get_rules() has been called.
  assert(compiler->rules == NULL);

  // Don't allow calls to yr_compiler_add_fd() if a previous call to
  // yr_compiler_add_XXXX failed.
  assert(compiler->errors == 0);

  if (namespace_ != NULL)
    compiler->last_error = _yr_compiler_set_namespace(compiler, namespace_);
  else
    compiler->last_error = _yr_compiler_set_namespace(compiler, "default");

  if (compiler->last_error == ERROR_SUCCESS && file_name != NULL)
    compiler->last_error = _yr_compiler_push_file_name(compiler, file_name);

  if (compiler->last_error != ERROR_SUCCESS)
    return ++compiler->errors;

  result = yr_lex_parse_rules_fd(rules_fd, compiler);

  if (file_name != NULL)
    _yr_compiler_pop_file_name(compiler);

  return result;
}