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

//Wrong Argument #
if (argc != 3){
	printf("Usage: prog [filename] [c/d]\n");
	exit(-1);
}


//initialize dictionary--------------------
ldict = 0;
for (i = 0; i < 256; i++){
	dict[i] = (unsigned char *)calloc(2,1);
	sprintf(dict[i], "%c", i);
	ldict++;
}


// COMPRESSES **************************************************************************************
if (strcmp(argv[2],"c")==0){
	unsigned char *data;


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
	sprintf(p, "%.1s", data);
	sprintf(c, "%.1s", data + 1);
	sprintf(pc, "%s%s", p,c);
	//p+c is not in dictionary (only roots)
	//prints out code of p
	for(i = 0; i < ldict; i++){
		if (memcmp(p, dict[i], strlen(p)) == 0){
			fwrite(&i,2,1,cfpt);
			//fprintf(cfpt, "%d", (unsigned short)i);
			i = ldict;
		}
	}


	//Adds p+c to dict
	dict[ldict] = (unsigned char *)calloc(strlen(pc)+1,1);
	sprintf(dict[ldict], "%s", pc);
	ldict++;
	
	//p = c
	sprintf(p, "%s", c);

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
		sprintf(c, "%.1s", data+cdata);
		
		//creates p+c
		sprintf(pc, "%s%s", p,c);
		
		//checks if pc is in dict
		for(i = 0; i < ldict; i++){
			//check if pc is equal to length of dictionary first
			//p+c in dict? = yes
			if (strlen(pc) == strlen(dict[i]))
				if (memcmp(pc, dict[i], strlen(pc)) == 0 && strlen(p) < 100){
					indict = 1;
				}
		}
		if (indict == 1 ){
			//p = p+c
			sprintf(p, "%s", pc);
		}
		
		//p+c in dict? = no
		else {
			//prints out code for p
			for(i = 0; i < ldict; i++){
				//check if p is equal to length of dictionary first
				if (strlen(p) == strlen(dict[i]))
					if (memcmp(p, dict[i], strlen(p)) == 0){
						fwrite(&i,2,1,cfpt);
						i = ldict;
					}
			}
			//adds pc to dict
			dict[ldict] = (unsigned char *)calloc(100,1);
			sprintf(dict[ldict], "%s", pc);
			ldict++;
			//p = c
			sprintf(p, "%s", c);
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
		if (strlen(p) == strlen(dict[i]))
			if (memcmp(p, dict[i], strlen(p)) == 0){
				fwrite(&i,2,1,cfpt);
				i = ldict;
			}
	}
}



// DECOMPRESSES ************************************************************************************
else if (strcmp(argv[2],"d")==0){

	unsigned short *data;

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
	fprintf(dfpt, "%s", dict[cc]);
	//printf(".%d.\n", cc);
	//printf("In Dict\n");
	
	//Begin Loop ------------------------
	mdata = 1;
	cdata = 1;
	while (mdata == 1){
		cp = cc;
		cc = data[cdata];
		//printf(".%d.\n", cc);
		//c in dict
		if (ldict > cc){
			//printf("In Dict\n");
			//output pattern for c
			fprintf(dfpt, "%s", dict[cc]);//
			//let x = pattern for P
			sprintf(x, "%s", dict[cp]);
			//let y = first char of c
			y = dict[cc][0];
			//Add x+y to dict
			dict[ldict] = (unsigned char *)calloc(100,1);
			sprintf(dict[ldict], "%s%c", x,y);
			ldict++;
		}
		
		
		//c not in dict
		else{
			//printf("Not in Dict\n");
			//Let x = pattern for p
			sprintf(x, "%s", dict[cp]);
			//let z = first char of c
			z = dict[cp][0]; //segfaults
			//output x + z
			fprintf(dfpt, "%s%c", x,z);
			//Add x+y to dict
			dict[ldict] = (unsigned char *)calloc(100,1);
			sprintf(dict[ldict], "%s%c", x,z); // doesnt work
			ldict++;
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

//Prints Dictionary and index
	//printf("\n"); for (i = 0; i < ldict; i++) printf("%s %d\n", dict[i], i);
