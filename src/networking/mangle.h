#define BITMASK(bits) ((1 << bits) - 1)
#define SIGN_MASK    0x80000000
#define EXP_MASK     0x7f800000
#define SIGNIF_MASK  0x007fffff
#define EXP_BITS     8
#define SIGNIF_BITS  23


unsigned int float_to_uint32(float value, char exponent, char significand);
float uint32_to_float(unsigned int value, char exponent, char significand);

