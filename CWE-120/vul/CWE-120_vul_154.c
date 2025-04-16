static ecma_value_t
ecma_builtin_array_prototype_object_slice (ecma_value_t arg1, /**< start */
                                           ecma_value_t arg2, /**< end */
                                           ecma_object_t *obj_p, /**< object */
                                           ecma_length_t len) /**< object's length */
{
  ecma_length_t start = 0, end = len;

  /* 5. 6.*/
  if (ECMA_IS_VALUE_ERROR (ecma_builtin_helper_array_index_normalize (arg1,
                                                                      len,
                                                                      &start)))
  {
    return ECMA_VALUE_ERROR;
  }

  /* 7. */
  if (ecma_is_value_undefined (arg2))
  {
    end = len;
  }
  else
  {
    /* 7. part 2, 8.*/
    if (ECMA_IS_VALUE_ERROR (ecma_builtin_helper_array_index_normalize (arg2,
                                                                        len,
                                                                        &end)))
    {
      return ECMA_VALUE_ERROR;
    }
  }

  JERRY_ASSERT (start <= len && end <= len);

  bool use_fast_path = ecma_op_object_is_fast_array (obj_p);
  ecma_length_t copied_length = (end > start) ? end - start : 0;
#if JERRY_ESNEXT
  ecma_object_t *new_array_p = ecma_op_array_species_create (obj_p, copied_length);

  if (JERRY_UNLIKELY (new_array_p == NULL))
  {
    return ECMA_VALUE_ERROR;
  }

  use_fast_path &= ecma_op_object_is_fast_array (new_array_p);
#else /* !JERRY_ESNEXT */
  ecma_object_t *new_array_p = ecma_op_new_array_object (0);
#endif /* JERRY_ESNEXT */

  if (use_fast_path && copied_length > 0)
  {
    ecma_extended_object_t *ext_from_obj_p = (ecma_extended_object_t *) obj_p;

    if (ext_from_obj_p->u.array.length_prop_and_hole_count < ECMA_FAST_ARRAY_HOLE_ONE)
    {
      if (JERRY_UNLIKELY (obj_p->u1.property_list_cp == JMEM_CP_NULL))
      {
        /**
         * Very unlikely case: the buffer copied from is a fast buffer and the property list was deleted.
         * There is no need to do any copy.
         */
        return ecma_make_object_value (new_array_p);
      }

      ecma_extended_object_t *ext_to_obj_p = (ecma_extended_object_t *) new_array_p;

#if JERRY_ESNEXT
      uint32_t target_length = ext_to_obj_p->u.array.length;
      ecma_value_t *to_buffer_p;
      JERRY_ASSERT (copied_length <= UINT32_MAX);

      if (copied_length == target_length)
      {
        to_buffer_p = ECMA_GET_NON_NULL_POINTER (ecma_value_t, new_array_p->u1.property_list_cp);
      }
      else if (copied_length > target_length)
      {
        to_buffer_p = ecma_fast_array_extend (new_array_p, (uint32_t) copied_length);
      }
      else
      {
        ecma_delete_fast_array_properties (new_array_p, (uint32_t) copied_length);
        to_buffer_p = ECMA_GET_NON_NULL_POINTER (ecma_value_t, new_array_p->u1.property_list_cp);
      }
#else /* !JERRY_ESNEXT */
      ecma_value_t *to_buffer_p = ecma_fast_array_extend (new_array_p, copied_length);
#endif /* JERRY_ESNEXT */

      ecma_value_t *from_buffer_p = ECMA_GET_NON_NULL_POINTER (ecma_value_t, obj_p->u1.property_list_cp);

      /* 9. */
      uint32_t n = 0;

      for (uint32_t k = (uint32_t) start; k < (uint32_t) end; k++, n++)
      {
#if JERRY_ESNEXT
        ecma_free_value_if_not_object (to_buffer_p[n]);
#endif /* JERRY_ESNEXT */
        to_buffer_p[n] = ecma_copy_value_if_not_object (from_buffer_p[k]);
      }

      ext_to_obj_p->u.array.length_prop_and_hole_count &= ECMA_FAST_ARRAY_HOLE_ONE - 1;

      return ecma_make_object_value (new_array_p);
    }
  }

  /* 9. */
  ecma_length_t n = 0;

  /* 10. */
  for (ecma_length_t k = start; k < end; k++, n++)
  {
    /* 10.c */
    ecma_value_t get_value = ecma_op_object_find_by_index (obj_p, k);

    if (ECMA_IS_VALUE_ERROR (get_value))
    {
      ecma_deref_object (new_array_p);
      return get_value;
    }

    if (ecma_is_value_found (get_value))
    {
      /* 10.c.ii */
      ecma_value_t put_comp;
#if JERRY_ESNEXT
      const uint32_t prop_flags = ECMA_PROPERTY_CONFIGURABLE_ENUMERABLE_WRITABLE | JERRY_PROP_SHOULD_THROW;
#else /* !JERRY_ESNEXT */
      const uint32_t prop_flags = ECMA_PROPERTY_CONFIGURABLE_ENUMERABLE_WRITABLE;
#endif /* JERRY_ESNEXT */
      put_comp = ecma_builtin_helper_def_prop_by_index (new_array_p,
                                                        n,
                                                        get_value,
                                                        prop_flags);
      ecma_free_value (get_value);

#if JERRY_ESNEXT
      if (ECMA_IS_VALUE_ERROR (put_comp))
      {
        ecma_deref_object (new_array_p);
        return put_comp;
      }
#else /* !JERRY_ESNEXT */
      JERRY_ASSERT (ecma_is_value_true (put_comp));
#endif /* JERRY_ESNEXT */
    }
  }

#if JERRY_ESNEXT
  ecma_value_t set_length_value = ecma_builtin_array_prototype_helper_set_length (new_array_p, ((ecma_number_t) n));

  if (ECMA_IS_VALUE_ERROR (set_length_value))
  {
    ecma_deref_object (new_array_p);
    return set_length_value;
  }
#endif /* JERRY_ESNEXT */

  return ecma_make_object_value (new_array_p);
}