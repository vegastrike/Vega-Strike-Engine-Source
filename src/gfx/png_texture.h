extern int PNG_HAS_PALETTE;
extern int PNG_HAS_COLOR;
extern int PNG_HAS_ALPHA;
typedef unsigned char * (textureTransform) (int &bpp, int &color_type, unsigned int &width, unsigned int &height, unsigned char ** row_pointers);
textureTransform heightmapTransform;
textureTransform terrainTransform;
textureTransform texTransform;
unsigned char * readImage (FILE * fp, int &bpp, int &format, unsigned int &width, unsigned int &height, unsigned char *&palette, textureTransform *tt, bool strip_16);
