/*
	Andrew Wetzel
	Lab3: LZW Compression & Decompression
*/
//use short int for dictionary keys
//use unsigned char[] for dictionary entries

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){

FILE 		*fpt;
FILE 		*cfpt;
FILE 		*dfpt;

unsigned char 	*dict[65536];
unsigned char 	p[100];
unsigned char	pc[100];
unsigned char 	c[2];
unsigned char	x[100];
unsigned char	y;
unsigned char 	z;


int 		i;
int 		ldict; //length of dictionary
int 		ldata; //length of data
int		cdata; //current count of data
unsigned short	cc;    //compressed short from cfpt
unsigned short	cp;    //previous compressed short
int 		mdata; //flag for more data
int 		indict;//flag for if pc is in dictionary
int 		lp;    //current length of p
int		lx;    //current length of x

//Wrong Argument #
if (argc != 3){
	printf("Usage: prog [filename] [c/d]\n");
	exit(-1);
}





// COMPRESSES **************************************************************************************
if (strcmp(argv[2],"c")==0){
	unsigned char *data;
	
	//initialize dictionary--------------------
	ldict = 0;
	for (i = 0; i < 256; i++){
		dict[i] = (unsigned char *)calloc(2,1);
		sprintf(dict[i], "%c", i);
		ldict++;
	}


	//Opens file to read from
	fpt = fopen(argv[1], "r");
	if (fpt == NULL){
		printf("Failed to open %s.\n", argv[1]);
		exit(-1);
	}
	
	
	//Opens file to send compressed data to
	cfpt = fopen("compressed.lzw", "w");
	if (cfpt == NULL){
		printf("Failed to open compressed.lzw\n");
		exit(-1);
	}
	
	
	//Gets data and length of data
	fseek(fpt, 0L, SEEK_END);
	ldata = ftell(fpt);
	rewind(fpt);
	data = (unsigned char *)calloc(ldata, sizeof(unsigned char));
	fread(data,1,ldata, fpt);
	

	//First loop iteration--------------------
	lp = 1;
	//p gets data[0]
	memcpy(p,data,lp);
	p[lp] = 0;
	//c gets data[1]
	memcpy(c,data+1,1);
	//pc gets p+c
	memcpy(pc,p,lp);
	memcpy(pc+lp,c,1);
	pc[lp+1] = 0;
	
	
	//p+c is not in dictionary (only roots)
	//prints out code of p
	for(i = 0; i < ldict; i++){
		if (memcmp(p, dict[i], strlen(p)) == 0){
			fwrite(&i,2,1,cfpt);
			i = ldict;
		}
	}


	//Adds p+c to dict
	dict[ldict] = (unsigned char *)calloc(lp+2,1);
	memcpy(dict[ldict], pc, lp+1);
	dict[ldict+1] = 0;
	ldict++;
	
	//p = c
	memcpy(p, c, 1);
	p[1] = 0;
	lp = 1;

	//sets current move through data
	cdata = 2;
	//checks if there is more data
	if (cdata < ldata)
		mdata = 1;
	else
		mdata = 0;


	//Begins Loop---------------------
	while (mdata == 1){	

		//resets indict flag
		indict = 0;
		//read C
		memcpy(c, data+cdata, 1);
		
		//creates p+c
		memcpy(pc,p,lp);
		memcpy(pc+lp,c,1);
		pc[lp+1] = 0;
		
		//checks if pc is in dict
		for(i = 0; i < ldict; i++){
			//check if pc is equal to length of dictionary first
			//p+c in dict? = yes
			if (lp+1 == strlen(dict[i]))
				if (memcmp(pc, dict[i], lp+1) == 0 && lp < 100){
					indict = 1;
				}
		}
		if (indict == 1 ){
			//p = p+c
			memcpy(p,pc, lp+1);
			p[lp+1] = 0;
			lp++;
			
		}
		
		//p+c in dict? = no
		else {
			//prints out code for p
			for(i = 0; i < ldict; i++){
				//check if p is equal to length of dictionary first
				if (lp == strlen(dict[i]))
					if (memcmp(p, dict[i], lp) == 0){
						fwrite(&i,2,1,cfpt);
						i = ldict;
					}
			}
			//adds pc to dict
			dict[ldict] = (unsigned char *)calloc(100,1);
			memcpy(dict[ldict], pc, lp+1);
			dict[ldict+1] = 0;
			ldict++;
			//p = c
			sprintf(p, "%s", c);
			lp = 1;
		}
		
		//Checks if there is more data
		if (cdata < ldata-1)
			mdata = 1;
		else
			mdata = 0;	
		
		//increments through data
		cdata++;	
	}

	//Output code for p and DONE
	for(i = 0; i < ldict; i++){
		//check if p is equal to length of dictionary first
		if (lp == strlen(dict[i]))
			if (memcmp(p, dict[i], lp) == 0){
				fwrite(&i,2,1,cfpt);
				i = ldict;
			}
	}
}



// DECOMPRESSES ************************************************************************************
else if (strcmp(argv[2],"d")==0){\

	//initialize dictionary--------------------
	ldict = 0;
	for (i = 0; i < 256; i++){
		dict[i] = (unsigned char *)calloc(100,1);
		sprintf(dict[i], "%c", i);
		ldict++;
	}

	//Opens file to decompress
	cfpt = fopen(argv[1], "rb");
	if (cfpt == NULL){
		printf("Failed to open %s.\n", argv[1]);
		exit(-1);
	}
	
	//Opens file to send decompressed data to
	dfpt = fopen("decompressed.lzw", "w");
	if (dfpt == NULL){
		printf("Failed to open decompressed.lzw.\n");
		exit(-1);
	}
	
	//Gets data and length of data
	unsigned short *data;
	fseek(cfpt, 0L, SEEK_END);
	ldata = ftell(cfpt);
	rewind(cfpt);
	data = (unsigned short *)calloc(ldata, sizeof(unsigned short));
	fread(data,2,ldata/2, cfpt);

	//Preloop----------------------------
		//dict is already initialized
	//Read C
	cc = data[0];
	
	//Output pattern for C
	//fprintf(dfpt, "%s", dict[cc]);
	fwrite(dict[cc],1,1,dfpt);
	
	//Begin Loop ------------------------
	mdata = 1;
	cdata = 1;

	while (mdata == 1){
		cp = cc;
		cc = data[cdata];
		//printf("cp: %d, cc: %d.\n", cp, cc);
		//printf(".%d.\n", cc);
		//c in dict
		if (ldict > cc){
			//printf("In Dict\n");
			//output pattern for c
			//fprintf(dfpt, "%s", dict[cc]);
			fwrite(dict[cc], strlen(dict[cc]), 1, dfpt);
			
			//let x = pattern for P
			//sprintf(x, "%s", dict[cp]);
			memcpy(x, dict[cp], 100);
			
			//let y = first char of c
			y = dict[cc][0];
			//Add x+y to dict
			dict[ldict] = (unsigned char *)calloc(100,1);
			//sprintf(dict[ldict], "%s%c", x,y);
			memcpy(dict[ldict], x, strlen(x));
			dict[ldict][strlen(x)] = y;
			dict[ldict][strlen(x)+1] = 0;
			ldict++;
			//printf(" Is in: %d\n", ldict);
		}
		
		
		//c not in dict
		else{
			//printf("Not in Dict\n");
			//Let x = pattern for p
			//sprintf(x, "%s", dict[cp]);
			memcpy(x, dict[cp], 100);
			
			//let z = first char of c
			z = dict[cp][0];
			
			//output x + z
			//fprintf(dfpt, "%s%c", x,z);
			fwrite(x, strlen(x), 1, dfpt);
			fwrite(&z, 1, 1, dfpt);
			
			//Add x+z to dict
			dict[ldict] = (unsigned char *)calloc(100,1);
			//sprintf(dict[ldict], "%s%c", x,z); // doesnt work
			memcpy(dict[ldict], x, strlen(x));
			dict[ldict][strlen(x)] = z;
			dict[ldict][strlen(x)+1] = 0;
			ldict++;
			//printf("Not in: %d\n", ldict);
		}
		//Checks if more data
		if (cdata < ldata/2-1)
			mdata = 1;
		else
			mdata = 0;
			
		cdata++;
	}
}

// Neither *****************************************************************************************

else{
	printf("Third argument need to be \"c\" to compress or \"d\" to decompress.\n");
	exit(-1);
}


return 0;
}


