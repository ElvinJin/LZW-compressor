/*
* CSCI3280 Introduction toMultimedia Systems
*
* --- Declaration --- 
*
* I declare that the assignment here submitted is original except for source
* material explicitly acknowledged. I also acknowledge that I am aware of
* University policy and regulations on honesty in academic work, and of the
* disciplinary guidelines and procedures applicable to breaches of such policy
* and regulations, as contained in the website
* http://www.cuhk.edu.hk/policy/academichonesty/ 
*
* Assignment 2 
* Name: JIN, Peng
* Student ID : 1155014559
* Email Addr : pjin.elvin@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define CODE_SIZE  12
#define TRUE 1
#define FALSE 0


/* function prototypes */
unsigned int read_code(FILE*, unsigned int); 
void write_code(FILE*, unsigned int, unsigned int); 
void writefileheader(FILE *,char**,int);
void readfileheader(FILE *,char**,int *);
void compress(FILE*, FILE*);
void decompress(FILE*, FILE*);

int main(int argc, char **argv)
{
    int printusage = 0;
    int	no_of_file;
    char **input_file_names;    
	char *output_file_names;
    FILE *lzw_file;

    if (argc >= 3)
    {
		if ( strcmp(argv[1],"-c") == 0)
		{		
			/* compression */
			lzw_file = fopen(argv[2] ,"wb");
        
			/* write the file header */
			input_file_names = argv + 3;
			no_of_file = argc - 3;
			writefileheader(lzw_file,input_file_names,no_of_file);
        	        	
			/* ADD CODES HERE */
			for (int i = 0; i < no_of_file; ++i)
			{
				FILE *file_to_compress = fopen(input_file_names[i], "rb");
				compress(file_to_compress, lzw_file);
				fclose(file_to_compress);
			}
			write_code(lzw_file, 0, CODE_SIZE);

			fclose(lzw_file);        	
		} else
		if ( strcmp(argv[1],"-d") == 0)
		{	
			/* decompress */
			lzw_file = fopen(argv[2] ,"rb");
			
			/* read the file header */
			no_of_file = 0;			
			readfileheader(lzw_file,&output_file_names,&no_of_file);
			
			char *current_file_name = NULL;
			/* ADD CODES HERE */
			for (int i = 0; i < no_of_file; ++i)
			{
				if (current_file_name == NULL)
					current_file_name = strtok(output_file_names, "\n");
				else
					current_file_name = strtok(NULL, "\n");
				printf(current_file_name);
				FILE *file_to_output = fopen(current_file_name, "wb");
				decompress(lzw_file, file_to_output);
				fclose(file_to_output);
			}
			
			fclose(lzw_file);
			free(output_file_names);
		}else
			printusage = 1;
    }else
		printusage = 1;

	if (printusage)
		printf("Usage: %s -<c/d> <lzw filename> <list of files>\n",argv[0]);
 	
	return 0;
}

/*****************************************************************
 *
 * writefileheader() -  write the lzw file header to support multiple files
 *
 ****************************************************************/
void writefileheader(FILE *lzw_file,char** input_file_names,int no_of_files)
{
	int i;
	/* write the file header */
	for ( i = 0 ; i < no_of_files; i++) 
	{
		fprintf(lzw_file,"%s\n",input_file_names[i]);	
			
	}
	fputc('\n',lzw_file);

}

/*****************************************************************
 *
 * readfileheader() - read the fileheader from the lzw file
 *
 ****************************************************************/
void readfileheader(FILE *lzw_file,char** output_filenames,int * no_of_files)
{
	int noofchar;
	char c,lastc;

	noofchar = 0;
	lastc = 0;
	*no_of_files=0;
	/* find where is the end of double newline */
	while((c = fgetc(lzw_file)) != EOF)
	{
		noofchar++;
		if (c =='\n')
		{
			if (lastc == c )
				/* found double newline */
				break;
			(*no_of_files)++;
		}
		lastc = c;
	}

	if (c == EOF)
	{
		/* problem .... file may have corrupted*/
		*no_of_files = 0;
		return;
	
	}
	/* allocate memeory for the filenames */
	*output_filenames = (char *) malloc(sizeof(char)*noofchar);
	/* roll back to start */
	fseek(lzw_file,0,SEEK_SET);

	fread((*output_filenames),1,(size_t)noofchar,lzw_file);
	
	return;
}

/*****************************************************************
 *
 * read_code() - reads a specific-size code from the code file
 *
 ****************************************************************/
unsigned int read_code(FILE *input, unsigned int code_size)
{
    unsigned int return_value;
    static int input_bit_count = 0;
    static unsigned long input_bit_buffer = 0L;

    /* The code file is treated as an input bit-stream. Each     */
    /*   character read is stored in input_bit_buffer, which     */
    /*   is 32-bit wide.                                         */

    /* input_bit_count stores the no. of bits left in the buffer */

    while (input_bit_count <= 24) {
        input_bit_buffer |= (unsigned long) getc(input) << (24-input_bit_count);
        input_bit_count += 8;
    }
    
    return_value = input_bit_buffer >> (32 - code_size);
    input_bit_buffer <<= code_size;
    input_bit_count -= code_size;
    
    return(return_value);
}

/*****************************************************************
 *
 * write_code() - write a code (of specific length) to the file 
 *
 ****************************************************************/
void write_code(FILE *output, unsigned int code, unsigned int code_size)
{
    static int output_bit_count = 0;
    static unsigned int output_bit_buffer = 0;

    /* Each output code is first stored in output_bit_buffer,    */
    /*   which is 32-bit wide. Content in output_bit_buffer is   */
    /*   written to the output file in bytes.                    */

    /* output_bit_count stores the no. of bits left              */    

    output_bit_buffer |= (unsigned) code << (32-code_size-output_bit_count);
    output_bit_count += code_size;

    while (output_bit_count >= 8) {
        putc(output_bit_buffer >> 24, output);
        output_bit_buffer <<= 8;
        output_bit_count -= 8;
    }


    /* only < 8 bits left in the buffer                          */    

}

/*****************************************************************
 *
 * compress() - compress the source file and output the coded text
 *
 ****************************************************************/

typedef struct com_node_struct {
	struct com_node_struct *child[256];
} com_node;

void compress(FILE *input, FILE *output)
{
	/* ADD CODES HERE */
	static com_node dic_root;
	int c;
	static com_node *p = &dic_root;
	static int used_index = -1;

	static com_node node_array[4096];

	if (used_index == -1)
	{
		for (int i = 0; i < 256; ++i)
		{
			dic_root.child[i] = &node_array[i];
		}
		used_index = 255;
	}

	// Start loop
	c = fgetc(input);

	while (c != EOF) {
		if (p->child[c] == NULL)
		{
			// Output p
			write_code(output, p - node_array, CODE_SIZE);
			// Add (p,c) to dictionary/tree
			// clear tree if it's full

			if (used_index >= 4095)
			{
				memset(node_array, 0, 4096*sizeof(com_node));
				used_index = 255;// should be 255
			}
			else 
			{
				used_index++;
				p->child[c] = &node_array[used_index];
			}

			// p point to c
			p = dic_root.child[c];
		} 
		else 
		{
			p = p->child[c];
		}

		c = fgetc(input);
	}

	write_code(output, p - node_array, CODE_SIZE);
	write_code(output, 4095, CODE_SIZE);
	p = &dic_root;
}


/*****************************************************************
 *
 * decompress() - decompress a compressed file to the orig. file
 *
 ****************************************************************/
typedef struct de_node_struct {
	char character;
	struct de_node_struct *parent;
} de_node;

char reverse_put_c(FILE *output, de_node *node_p, de_node *dic_root_p)
{
	char root_char;

	if (node_p->parent != dic_root_p) 
		root_char = reverse_put_c(output, node_p->parent, dic_root_p);
	else
		root_char = node_p->character;

	fputc(node_p->character, output);

	return root_char;
}

void decompress(FILE *input, FILE *output)
{	
	static de_node node_array[4096];
	static de_node dic_root;
	static int de_used_index = -1;

	// init the dic
	if (de_used_index == -1)
	{
		for (int i = 0; i < 256; ++i)
		{
			node_array[i].character = (char)i;
			node_array[i].parent = &dic_root;
		}
		de_used_index = 255;
	}

	unsigned int cW = read_code(input, CODE_SIZE);
	fputc((char)cW, output);
	unsigned int pW;
	unsigned int C;

	while (cW != 4095)
	{
		pW = cW;
		cW = read_code(input, CODE_SIZE);

		if (de_used_index == 4094)
		{
			memset(node_array, 0, 4096 * sizeof(de_node));
			for (int i = 0; i < 256; ++i)
			{
				node_array[i].character = (char)i;
				node_array[i].parent = &dic_root;
			}
			de_used_index = 255;

			pW = -1;
		}

		if (node_array[cW].parent != NULL)
		{
			C = (unsigned int)reverse_put_c(output, &node_array[cW], &dic_root);
			

			if (pW != -1) {
				de_used_index++;
				node_array[de_used_index].character = (char)C;
				node_array[de_used_index].parent = &node_array[pW];
			}
		} else {

			C = (unsigned int)reverse_put_c(output, &node_array[pW], &dic_root);
			fputc((char)C, output);

			if (pW != -1) {
				de_used_index++;
				node_array[de_used_index].character = (char)C;
				node_array[de_used_index].parent = &node_array[pW];
			}
		}
	}

	// fputc(EOF, output);
}
