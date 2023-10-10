// This will apply the sobel filter and return the PSNR between the golden sobel and the produced sobel
// sobelized image
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define SIZE	4096
#define INPUT_FILE	"input.grey"
#define OUTPUT_FILE	"output_sobel.grey"
#define GOLDEN_FILE	"golden.grey"

/* The horizontal and vertical operators to be used in the sobel filter */
const char horiz_operator[3][3] = {{-1, 0, 1}, //6:assist compiler
                             {-2, 0, 2}, 
                             {-1, 0, 1}};
const char vert_operator[3][3] = {{1, 2, 1},  //6:assist compiler
                            {0, 0, 0}, 
                            {-1, -2, -1}};

double sobel(unsigned char *input, unsigned char *output, unsigned char *golden);

/* The arrays holding the input image, the output image and the output used *
 * as golden standard. The luminosity (intensity) of each pixel in the      *
 * grayscale image is represented by a value between 0 and 255 (an unsigned *
 * character). The arrays (and the files) contain these values in row-major *
 * order (element after element within each row and row after row. 			*/
unsigned char input[SIZE*SIZE], output[SIZE*SIZE], golden[SIZE*SIZE];

/* The main computational function of the program. The input, output and *
 * golden arguments are pointers to the arrays used to store the input   *
 * image, the output produced by the algorithm and the output used as    *
 * golden standard for the comparisons.									 */
double sobel(unsigned char *input, unsigned char *output, unsigned char *golden)
{
	double PSNR = 0, t;
	register int i, j; //6:assist compiler
	unsigned int p, p1, p2, p3;
	int res, res1, res2, res3;
	struct timespec  tv1, tv2;
	FILE *f_in, *f_out, *f_golden;

	/* The first and last row of the output array, as well as the first  *
     * and last element of each column are not going to be filled by the *
     * algorithm, therefore make sure to initialize them with 0s.		 */
	memset(output, 0, SIZE*sizeof(unsigned char));
	memset(&output[SIZE*(SIZE-1)], 0, SIZE*sizeof(unsigned char));
	for (i = 1; i < SIZE-1; i++) {
		output[i*SIZE] = 0;
		output[i*SIZE + SIZE - 1] = 0;
	}

	/* Open the input, output, golden files, read the input and golden    *
     * and store them to the corresponding arrays.						  */
	f_in = fopen(INPUT_FILE, "r");
	if (f_in == NULL) {
		printf("File " INPUT_FILE " not found\n");
		exit(1);
	}
  
	f_out = fopen(OUTPUT_FILE, "wb");
	if (f_out == NULL) {
		printf("File " OUTPUT_FILE " could not be created\n");
		fclose(f_in);
		exit(1);
	}  
  
	f_golden = fopen(GOLDEN_FILE, "r");
	if (f_golden == NULL) {
		printf("File " GOLDEN_FILE " not found\n");
		fclose(f_in);
		fclose(f_out);
		exit(1);
	}    

	fread(input, sizeof(unsigned char), SIZE*SIZE, f_in);
	fread(golden, sizeof(unsigned char), SIZE*SIZE, f_golden);
	fclose(f_in);
	fclose(f_golden);
  
	/* This is the main computation. Get the starting time. */
	clock_gettime(CLOCK_MONOTONIC_RAW, &tv1);
	/* For each pixel of the output image */
	//1: Loop interchange
	for (i=1; i<SIZE-1; i+=2) { //5:Loop unrolling
		for (j=1; j<SIZE-1; j+=2 ) { //4:Loop unrolling
			
			/* Apply the sobel filter and calculate the magnitude *
			 * of the derivative.								  */
			
			/* Implement a 2D convolution of the matrix with the operator */
			/* posy and posx correspond to the vertical and horizontal disposition of the *
			 * pixel we process in the original image, input is the input image and       *
			 * operator the operator we apply (horizontal or vertical). The function ret. *
			 * value is the convolution of the operator with the neighboring pixels of the*
			 * pixel we process.
													*/
			//3: Loop interchange, function inling, loop fusion	
			int m, n;							  
			register int conv_horiz = 0, conv_vert = 0, conv_horiz_1 = 0, conv_vert_1 = 0, conv_horiz_2 = 0, conv_vert_2 = 0, conv_horiz_3 = 0, conv_vert_3 = 0; //6:assist compiler
			for (m = -1; m <= 1; m++) {
				for (n = -1; n <= 1; n++) {
					conv_horiz += input[(i + m)*SIZE + j + n] * horiz_operator[m+1][n+1];
					conv_horiz_1 += input[(i + m)*SIZE + j+1 + n] * horiz_operator[m+1][n+1]; //4:Loop unrolling
					
					conv_horiz_2 += input[(i+1 + m)*SIZE + j + n] * horiz_operator[m+1][n+1];
					conv_horiz_3 += input[(i+1 + m)*SIZE + j+1 + n] * horiz_operator[m+1][n+1]; //5:Loop unrolling
					
					conv_vert += input[(i + m)*SIZE + j + n] * vert_operator[m+1][n+1];
					conv_vert_1 += input[(i + m)*SIZE + j+1 + n] * vert_operator[m+1][n+1];	//4:Loop unrolling
					
					conv_vert_2 += input[(i+1 + m)*SIZE + j + n] * vert_operator[m+1][n+1];
					conv_vert_3 += input[(i+1 + m)*SIZE + j+1 + n] * vert_operator[m+1][n+1];	//5:Loop unrolling
				}
			}
			
			p = pow(conv_horiz, 2) + 
				pow(conv_vert, 2); 	
				
			p1 = pow(conv_horiz_1, 2) + 
				pow(conv_vert_1, 2);	//4:Loop unrolling
			
			p2 = pow(conv_horiz_2, 2) + 
				pow(conv_vert_2, 2);	//5:Loop unrolling
			
			p3 = pow(conv_horiz_3, 2) + 
				pow(conv_vert_3, 2);	//5:Loop unrolling	
				
				
			res = (int)sqrt(p);
			res1 = (int)sqrt(p1);	//4:Loop unrolling
			res2 = (int)sqrt(p2);	//5:Loop unrolling
			res3 = (int)sqrt(p3);	//5:Loop unrolling
			
			/* If the resulting value is greater than 255, clip it *
			 * to 255.											   */
			if (res > 255)
				output[i*SIZE + j] = 255;      
			else
				output[i*SIZE + j] = (unsigned char)res;
			
			if (res1 > 255)
				output[i*SIZE + j+1] = 255;      
			else
				output[i*SIZE + j+1] = (unsigned char)res1;
			
			if (res2 > 255)
				output[(i+1)*SIZE + j] = 255;      
			else
				output[(i+1)*SIZE + j] = (unsigned char)res2;
			
			if (res3 > 255)
				output[(i+1)*SIZE + j+1] = 255;      
			else
				output[(i+1)*SIZE + j+1] = (unsigned char)res3;
			
			/* Now run through the output and the golden output to calculate *
			 * the MSE and then the PSNR.	
			*/
			//2: Loop fusion
			t = pow((output[i*SIZE+j] - golden[i*SIZE+j]),2);
			PSNR += t;
			t = pow((output[i*SIZE+j+1] - golden[i*SIZE+j+1]),2);
			PSNR += t;
			t = pow((output[(i+1)*SIZE+j] - golden[(i+1)*SIZE+j]),2);
			PSNR += t;
			t = pow((output[(i+1)*SIZE+j+1] - golden[(i+1)*SIZE+j+1]),2);
			PSNR += t;
			
		}
	}


	PSNR /= (double)(SIZE*SIZE);
	PSNR = 10*log10(65536/PSNR);

	/* This is the end of the main computation. Take the end time,  *
	 * calculate the duration of the computation and report it. 	*/
	clock_gettime(CLOCK_MONOTONIC_RAW, &tv2);

	printf ("Total time = %10g seconds\n",
			(double) (tv2.tv_nsec - tv1.tv_nsec) / 1000000000.0 +
			(double) (tv2.tv_sec - tv1.tv_sec));

  
	/* Write the output file */
	fwrite(output, sizeof(unsigned char), SIZE*SIZE, f_out);
	fclose(f_out);
  
	return PSNR;
}


int main(int argc, char* argv[])
{
	double PSNR;
	PSNR = sobel(input, output, golden);
	printf("PSNR of original Sobel and computed Sobel image: %g\n", PSNR);
	printf("A visualization of the sobel filter can be found at " OUTPUT_FILE ", or you can run 'make image' to get the jpg\n");

	return 0;
}

