#include <stdio.h>
#include <stdlib.h>
#include <OpenCL/opencl.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#define MY_CL_GPU 1
#define MY_CL_CPU 0

#define ENCRYPT 'E'
#define DECRYPT 'D'

#define IO_MESAGE
#define IO_KEY

#define MAX_SOURCE_SIZE (0x100000)


// void read_matix( int matrix[], int size  ){
// 	for (int i = 0; i < size; i++) {
// 		for (int j = 0; j < size; j++) {
// 			scanf("%d", &matrix[ i * size + j] );
// 		}
// 	}
// }

// void print_matix( int matrix[], int size ) {

// 	for ( int i = 0; i < size ; ++i) {
// 		for ( int j = 0; j < size; ++j) {
// 			printf("%d ", matrix[ i * size + j ]);
// 		}
// 		printf("\n");
// 	}
// 	printf("\n"); 
// }

void IO_selected_mode( char* program_opreation );
char* IO_get_mesage( int* g_size );
char* normalize_key( char* key, int key_size, int mesage_size );

int main(){

	//int compute_unit = MY_CL_CPU;
	int compute_unit = MY_CL_GPU;

	char name[128];
	dispatch_queue_t queue;

	if ( compute_unit == MY_CL_GPU ){
		queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
	}else{
		queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
	}

	if ( queue == NULL )
	{
		printf("\nCan't find OpenCL  copute unit");
	}

    cl_device_id device_id = gcl_get_device_id_with_dispatch_queue(queue);
    clGetDeviceInfo( device_id, CL_DEVICE_NAME, 128, name, NULL);
    printf("Created a dispatch queue using the %s\n\n", name);


	// printf("Enter the size of the matrix: ");
	// scanf( "%d",&mesage_size );
	// printf("\n");

	char program_opreation = '0';
	IO_selected_mode( &program_opreation );


	printf("\nInput mesage:");
	getc(stdin);
	char* mesage = NULL;// = malloc( 100 * sizeof(char) ) ;
	int mesage_size;
	mesage = IO_get_mesage( &mesage_size );
	//printf("\n Mesage is:> %s\n",mesage );
	// for (int i = 0; i <= mesage_size; ++i)
	// {
	// 	printf("%d ", mesage[i] );
	// }

	printf("\nInput key:");
	char* key = NULL;// = malloc( 100 * sizeof(char) ) ;
	int key_size; 
	key = IO_get_mesage( &key_size );
	//printf("\n Key is:> %s\n",key );
	// for (int i = 0; i <= key_size; ++i)
	// {
	// 	printf("%d ", key[i] );
	// }
	key = normalize_key( key, key_size, mesage_size );
	//printf("\n Normalized_key is:> %s\n",key );
	// for (int i = 0; i <= mesage_size; ++i)
	// {
	// 	printf("%d ", key[i] );
	// }

	char* output;
	int output_size;
	output = malloc( mesage_size * sizeof(char) );
	output_size = mesage_size;

	int mesage_data_size = mesage_size * sizeof(char);


	FILE* fp;
	char* source_str;
	size_t source_size;

	fp = fopen( "mykernel.cl", "r" );
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	cl_int ret; 

	// Create an OpenCL context
	cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

	// Create a command queue
	cl_command_queue command_queue = clCreateCommandQueue( context, device_id, 0, &ret );

	// Create memory buffers on the device for each vector 
	cl_mem mesage_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, mesage_data_size, NULL, &ret);
	cl_mem key_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, mesage_data_size, NULL, &ret);
	cl_mem output_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, mesage_data_size, NULL, &ret);

	// Copy the lists A and B to their respective memory buffers
	ret = clEnqueueWriteBuffer(command_queue, mesage_mem_obj, CL_TRUE, 0, mesage_data_size, mesage, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, key_mem_obj, CL_TRUE, 0, mesage_data_size, key, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&source_str, (const size_t*)&source_size, &ret);

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);


// Fork for encript dectript-------------???
	// Create the OpenCL kernel

	cl_kernel kernel;

	if( program_opreation == ENCRYPT )
	{
		kernel = clCreateKernel(program, "Encrypt_module", &ret);
	}
	if( program_opreation == DECRYPT )
	{
		kernel = clCreateKernel(program, "Decrypt_module", &ret);
	}
		// else{
		// 	printf("\n No operation was selected ( Encrypt, Decrypt ) ");
		// }
	//cl_kernel kernel = clCreateKernel(program, "multiplicationModule", &ret);

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&mesage_mem_obj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&key_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&output_mem_obj);

	size_t global_item_size[1];
	global_item_size[0] = mesage_size;
	//size_t local_item_size = mesage_size;

	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_item_size, NULL, 0, NULL, NULL);

	// size_t global_item_size = mesage_size; // Process the entire lists
 //    size_t local_item_size = 1; // Divide work items into groups of 64
 //    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
	

	// Read the memory buffer C on the device to the local variable C
	//int *C = new int[mesage_size * mesage_size];
	//int *C = malloc( mesage_size * mesage_size * sizeof(int) );
	ret = clEnqueueReadBuffer(command_queue, output_mem_obj, CL_TRUE, 0, output_size, output, 0, NULL, NULL);

	// Display the result to the screen
	// printf("\nThe matrix multiplication result is: \n");
	// // print_matix( C, mesage_size);
	output[ mesage_size ] = '\0';

	//printf("\nOutput mesage size is : %d\n", output_size );


	if( program_opreation == ENCRYPT )
	{
		// printf("\n>>>");
		// //printf("\nThe Encrypted mesage is:\n>>>%s\n", output);
		// for (int i = 0; i < output_size; ++i)
		// {
		// 	printf("%c", output[i] );
		// }
		// printf("\n");

		printf("\nOutput:> %s\n",output );

	}

	if( program_opreation == DECRYPT )
	{
		printf("\nThe Decrypted mesage is:%s\n", output);
		// for (int i = 0; i < output_size; ++i)
		// {
		// 	printf("%c", output[i] );
		// }
	}

	return 0;
}

void IO_selected_mode( char* program_opreation ){
	printf("\nSelect mode: for Encryption press 'E' ; for Decription press 'D' \n>>>");
	scanf( "%c", program_opreation );
}

char* IO_get_mesage( int* g_size ){

	char* mesage = malloc( sizeof(char) );
	char c = 'a';
	int size = 0;
	printf("\n>>>");
	//getc(stdin); //Posibil BUG

	while( c != '\n' ){

		c = getc( stdin );

		if ( c == '\n')
		{
			break;
		}

		mesage = (char*)realloc( mesage, ( size + 1 ) * sizeof(char) );
		
		//mesage = (char*)realloc( mesage, ( size + 1 ) * sizeof(char) );
		mesage[ size ] = c;

		size++;
	}

	mesage = (char*)realloc( mesage, ( size ) * sizeof(char) );
	mesage[size + 1 ] = '\0';

	*(g_size) = size;

	return mesage ;
}



char* normalize_key( char* key, int key_size, int mesage_size ){

	key = (char*)realloc( key, ( mesage_size + 1 ) * sizeof(char) );

	int i, j;

	for( i = 0, j = 0; i < mesage_size; i++, j++ ){
        if(j == key_size )
            j = 0;
 
        key[i] = key[j];
    }

    key[ mesage_size ] = '\0';

    return key;

}

