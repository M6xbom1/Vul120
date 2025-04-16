YR_API int yr_compiler_define_integer_variable(
    YR_COMPILER* compiler,
    const char* identifier,
    int64_t value)
{
  YR_EXTERNAL_VARIABLE external;

  external.type = EXTERNAL_VARIABLE_TYPE_INTEGER;
  external.identifier = identifier;
  external.value.i = value;

  FAIL_ON_ERROR(_yr_compiler_define_variable(compiler, &external));

  return ERROR_SUCCESS;
}