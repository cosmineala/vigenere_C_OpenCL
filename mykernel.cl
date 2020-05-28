__kernel void Encrypt_module(__global const char* mesage, __global const char* key, __global char* output)
{
	int i = get_global_id(0); 
	int mesage_size = get_global_size(0);

	char key_num = key[i] - 'a';
	char mesage_num = mesage[i] - 'a';
	char output_num = 'a' ;

	if( mesage[i] == ' ' )
	{
		output[i]=mesage[i];
	}
	else
	{
		output_num = ( mesage_num + key_num ) % 26;
		output[i] = output_num + 'a';
		
	}


}



__kernel void Decrypt_module(__global const char* mesage, __global const char* key, __global char* output)
{
	int i = get_global_id(0); 
	int mesage_size = get_global_size(0);

	char key_num = key[i] - 'a';
	char mesage_num = mesage[i] - 'a';
	char output_num = 'a' ;

	if( mesage[i] == ' ' )
	{
		output[i] = mesage[i];
	}
	else
	{
		if ( mesage_num < key_num )
		{
			output_num = 26 - key_num + mesage_num;
		}
		else
		{
			output_num = mesage_num - key_num;
		}


		output[i] = output_num + 'a';
	}

}
