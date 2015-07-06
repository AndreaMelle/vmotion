bool _big_endian()
{
	const int num = 1;
	return (*(reinterpret_cast<const char*> (&num))) != 1;
}

// swap endianess of int
void _swap_endianess(int &i)
{
	i = (i >> 24) | ((i << 8) & 0x00FF0000) | ((i >> 8) & 0x0000FF00) | (i << 24);
}

// swap endianess of float
void _swap_endianess(float &f)
{
	float swapped;
	char *f_as_char = reinterpret_cast<char*> (&f);
	char *swapped_as_char = reinterpret_cast<char*> (&swapped);

	// swap the bytes into a temporary buffer
	swapped_as_char[0] = f_as_char[3];
	swapped_as_char[1] = f_as_char[2];
	swapped_as_char[2] = f_as_char[1];
	swapped_as_char[3] = f_as_char[0];

	f = swapped;
}

// swap endianess of int array
void _swap_endianess(int arr[], int arr_length)
{
	for (int i = 0; i < arr_length; i++)
		_swap_endianess(arr[i]);
}

// swap endianess of float array
void _swap_endianess(float arr[], int arr_length)
{
	for (int i = 0; i < arr_length; i++)
		_swap_endianess(arr[i]);
}