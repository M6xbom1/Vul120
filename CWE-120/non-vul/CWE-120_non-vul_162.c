static YR_MEMORY_BLOCK* _yr_get_next_block(YR_MEMORY_BLOCK_ITERATOR* iterator)
{
  YR_MEMORY_BLOCK* result = NULL;

  YR_DEBUG_FPRINTF(
      2,
      stderr,
      "- %s() {} = %p // default iterator; single memory block, blocking\n",
      __FUNCTION__,
      result);

  return result;
}