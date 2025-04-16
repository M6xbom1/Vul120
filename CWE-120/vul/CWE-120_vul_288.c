int URIParser::parse(const char *str, ParsedURI& uri)
{
	uri.state = URI_STATE_INVALID;

	int start_idx[URI_PART_ELEMENTS] = {0};
	int end_idx[URI_PART_ELEMENTS] = {0};
	int pre_state = URI_SCHEME;;
	int i;
	bool in_ipv6 = false;

	for (i = 0; str[i]; i++)
	{
		if (str[i] == ':')
		{
			end_idx[URI_SCHEME] = i++;
			break;
		}
	}

	if (end_idx[URI_SCHEME] == 0)
		return -1;

	if (str[i] == '/' && str[i + 1] == '/')
	{
		pre_state = URI_HOST;
		i += 2;
		if (str[i] == '[')
			in_ipv6= true;
		else
			start_idx[URI_USERINFO] = i;

		start_idx[URI_HOST] = i;
	}
	else
	{
		pre_state = URI_PATH;
		start_idx[URI_PATH] = i;
	}

	bool skip_path = false;
	if (start_idx[URI_PATH] == 0)
	{
		for (; ; i++)
		{
			switch (authority_map[(unsigned char)str[i]])
			{
				case 0:
					continue;

				case URI_USERINFO:
					if (str[i + 1] == '[')
						in_ipv6 = true;

					end_idx[URI_USERINFO] = i;
					start_idx[URI_HOST] = i + 1;
					pre_state = URI_HOST;
					continue;

				case URI_HOST:
					if (str[i - 1] == ']')
						in_ipv6 = false;

					if (!in_ipv6)
					{
						end_idx[URI_HOST] = i;
						start_idx[URI_PORT] = i + 1;
						pre_state = URI_PORT;
					}
					continue;

				case URI_QUERY:
					end_idx[pre_state] = i;
					start_idx[URI_QUERY] = i + 1;
					pre_state = URI_QUERY;
					skip_path = true;
					continue;

				case URI_FRAGMENT:
					end_idx[pre_state] = i;
					start_idx[URI_FRAGMENT] = i + 1;
					end_idx[URI_FRAGMENT] = i + strlen(str + i);
					pre_state = URI_PART_ELEMENTS;
					skip_path = true;
					break;

				case URI_PATH:
					if (skip_path)
						continue;

					start_idx[URI_PATH] = i;
					break;

				case URI_PART_ELEMENTS:
					skip_path = true;
					break;
			}

			break;
		}
	}
	if (pre_state != URI_PART_ELEMENTS)
		end_idx[pre_state] = i;

	if (!skip_path)
	{
		pre_state = URI_PATH;
		for (; str[i]; i++)
		{
			if (str[i] == '?')
			{
				end_idx[URI_PATH] = i;
				start_idx[URI_QUERY] = i + 1;
				pre_state = URI_QUERY;
				while (str[i + 1])
				{
					if (str[++i] == '#')
						break;
				}
			}

			if (str[i] == '#')
			{
				end_idx[pre_state] = i;
				start_idx[URI_FRAGMENT] = i + 1;
				pre_state = URI_FRAGMENT;
				break;
			}
		}
		end_idx[pre_state] = i + strlen(str + i);
	}

	for (int i = 0; i < URI_QUERY; i++)
	{
		for (int j = start_idx[i]; j < end_idx[i]; j++)
		{
			if (!valid_char[i][(unsigned char)str[j]])
				return -1;//invalid char
		}
	}

	char **dst[URI_PART_ELEMENTS] = {&uri.scheme, &uri.userinfo, &uri.host, &uri.port,
					 &uri.query, &uri.fragment, &uri.path};

	for (int i = 0; i < URI_PART_ELEMENTS; i++)
	{
		if (end_idx[i] > start_idx[i])
		{
			size_t len = end_idx[i] - start_idx[i];

			*dst[i] = (char *)realloc(*dst[i], len + 1);
			if (*dst[i] == NULL)
			{
				uri.state = URI_STATE_ERROR;
				uri.error = errno;
				return -1;
			}

			if (i == URI_HOST && str[start_idx[i]] == '[')
			{
				len -= 2;
				memcpy(*dst[i], str + start_idx[i] + 1, len);
			}
			else
				memcpy(*dst[i], str + start_idx[i], len);
			(*dst[i])[len] = '\0';
		}
		else
		{
			free(*dst[i]);
			*dst[i] = NULL;
		}
	}

	uri.state = URI_STATE_SUCCESS;
	return 0;
}