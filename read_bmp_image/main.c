#include <stdio.h>
#include <stdlib.h>

struct BITMAP_header{
    char name[2]; //BP BMP DIB
    unsigned int header_size;
    int reserved;
    unsigned int image_offset; // 14 bytes + dib header size
}; 

struct DIB_header{
    unsigned int header_size;
    unsigned int width;
    unsigned int height;
    unsigned short int colorplanes;
    unsigned short int bitsperpixel;
    unsigned int compression;
    unsigned int image_size;
    unsigned int horizontal_resolution;
    unsigned int vertically_resolution;
    unsigned int color_palette;
    unsigned int num_of_color;
};

struct RGB{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
};

struct Image{
    int width;
    int height;
    struct RGB **rgb;
};

struct Image readImage(FILE *fp, int width, int height){ 
    struct Image pic;
    pic.width = width;
    pic.height = height;
    pic.rgb = (struct RGB**) malloc(height*sizeof(void*));

    //////////padding for outrange
    int bytestored = ((24 * width + 31)/32)*4; // 24 bits
    printf("\n");
    
    printf("%d\n", bytestored);
    int numOfrgb = bytestored/sizeof(struct RGB)+1;
    printf("%d\n", numOfrgb);


    for(int i=height-1; i>=0; i--){ // from bottom to top because of edian
        pic.rgb[i] = (struct RGB*) malloc(numOfrgb*sizeof(struct RGB));
        fread(pic.rgb[i],1,bytestored,fp);
    }

    

    return pic;
}

void freeImage(struct Image pic){
    for(int i=pic.height-1; i>=0; i--)   free(pic.rgb[i]);
    free(pic.rgb);
}

unsigned char grayscale(struct RGB rgb){
    return ((0.3 * rgb.red) + (0.6 * rgb.green) + (0.1 * rgb.blue))/3;
}

void imageGrayscale(struct Image pic){
    for(int i=0; i<pic.height; i++){
        for(int j=0; j<pic.width; j++){
            // printf("%d, %d\n", i, j);
            pic.rgb[i][j].red = grayscale(pic.rgb[i][j]);
            pic.rgb[i][j].green = grayscale(pic.rgb[i][j]);
            pic.rgb[i][j].blue = grayscale(pic.rgb[i][j]);
            // printf("%d\n", pic.rgb[i][j].red);

        }
    }
}

void createImage(struct BITMAP_header header, struct DIB_header dibheader, struct Image pic, int select, char *fileOut){
    FILE *fpw = fopen(fileOut,"w");
    if(fpw == NULL) return;

    // select a filter
    if(select == 1) imageGrayscale(pic);

    fwrite(header.name, 2, 1, fpw);
    fwrite(&header.header_size, 3*sizeof(int),1,fpw);
    fwrite(&dibheader, sizeof(struct DIB_header),1,fpw);

    for(int i=pic.height-1; i>=0; i--){
        fwrite(pic.rgb[i],pic.width,sizeof(struct RGB),fpw);
    }
}

int openbmpfile(char *fileName, char *fileOut, int select){
    FILE *fp = fopen(fileName, "rb");
    if (fp == NULL) {
        perror("Error");
    }
    struct BITMAP_header header;
    struct DIB_header dibheader;

    fread(header.name, 2,1,fp);
    fread(&header.header_size, 3*sizeof(int),1,fp); // 13
    printf("Identify: %c%c\n", header.name[0], header.name[1]);
    printf("Size: %d\n", header.header_size);
    printf("Offset: %d\n", header.image_offset);
    fread(&dibheader, sizeof(struct DIB_header),1,fp );
    printf("Header size: %d\nWidth: %d\nHeight: %d\nColor planes: %d\nBits per pixel: %d\nCompression: %d\n",dibheader.header_size,dibheader.width,dibheader.height,dibheader.colorplanes,dibheader.bitsperpixel,dibheader.compression);

    if((header.name[0]!='B') || (header.name[1] !='M')){fclose(fp);printf("Not BM mode"); return 1;}
    if(dibheader.compression!=0){fclose(fp);printf("Compression is not 0"); return 1;} // dibheader.header_size == 40
    if(dibheader.bitsperpixel!=24){fclose(fp);printf("Bitspermode is not 24"); return 1;}

    fseek(fp, header.image_offset, SEEK_SET);
    struct Image image = readImage(fp, dibheader.width, dibheader.height);
   
    createImage(header, dibheader, image, select, fileOut);


    fclose(fp);
    freeImage(image);
    return 0;
}


int main(){
    /* 
    How to use?
    openbmpfile(process_image_name, new_image_name, filter_mode)

    selection   filter_mode
    1           grayscale
    */

    openbmpfile("image.bmp", "new_image.bmp", 1);
	

   /* 
    Ignore
    char inputFileName[50],outputFileName[50], con[3];
    int select;
    while(1){
        printf("Input a bmp file name:\n");
        scanf("%s",inputFileName);
        printf("Input a bmp file output name:\n");
        scanf("%s",outputFileName);
        printf("Select a filter: (1-9):\n");
        scanf("%d",&select);
        printf("The input file name is:%s\n",inputFileName);
        printf("The output file name is:%s\n",outputFileName);

        printf("Create Successfuly\n");
        printf("Continue or Exist (y/n)? ");
        scanf("%s", con);

        if(con[0] == 'n') break;
    }
    */

	return 0;
}