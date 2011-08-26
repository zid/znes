unsigned char palette[64][3] = {
	{0x52, 0x52, 0x52},
	{0x00, 0x00, 0x80},
	{0x08, 0x08, 0x8A},
	{0x2C, 0x00, 0x7E},
	{0x4A, 0x00, 0x4E},
	{0x50, 0x00, 0x06},
	{0x44, 0x00, 0x00},
	{0x26, 0x08, 0x00},

	{0x0A, 0x20, 0x00},
	{0x00, 0x2E, 0x00},
	{0x00, 0x32, 0x00},
	{0x00, 0x26, 0x0A},
	{0x00, 0x1C, 0x48},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},

	{0xA4, 0xA4, 0xA4},
	{0x00, 0x38, 0xCE},
	{0x34, 0x16, 0xCE},
	{0x5E, 0x04, 0xDC},
	{0x8C, 0x00, 0xB0},
	{0x9A, 0x00, 0x4C},
	{0x90, 0x18, 0x00},
	{0x70, 0x36, 0x00},

	{0x4C, 0x54, 0x00},
	{0x0E, 0x6C, 0x00},
	{0x00, 0x74, 0x00},
	{0x00, 0x6C, 0x2C},
	{0x00, 0x5E, 0x84},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},

	{0xFF, 0xFF, 0xFF},
	{0x4C, 0x9C, 0xFF},
	{0x7C, 0x78, 0xFF},
	{0xA6, 0x64, 0xFF},
	{0xDA, 0x5A, 0xFF},
	{0xF0, 0x54, 0xC0},
	{0xF0, 0x6A, 0x56},
	{0xD6, 0x86, 0x10},

	{0xBA, 0xA4, 0x00},
	{0x76, 0xC0, 0x00},
	{0x46, 0xCC, 0x1A},
	{0x2E, 0xC8, 0x66},
	{0x34, 0xC2, 0xBE},
	{0x3A, 0x3A, 0x3A},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},

	{0xFF, 0xFF, 0xFF},
	{0xB6, 0xDA, 0xFF},
	{0xC8, 0xCA, 0xFF},
	{0xDA, 0xC2, 0xFF},
	{0xF0, 0xBE, 0xFF},
	{0xFC, 0xBC, 0xEE},
	{0xFA, 0xC2, 0xC0},
	{0xF2, 0xCC, 0xA2},

	{0xE6, 0xDA, 0x92},
	{0xCC, 0xE6, 0x8E},
	{0xB8, 0xEE, 0xA2},
	{0xAE, 0xEA, 0xBE},
	{0xAE, 0xE8, 0xE2},
	{0xB0, 0xB0, 0xB0},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00}
};

/* Matthew Conte
unsigned char palette[64][3] =
{
   {0x80,0x80,0x80}, {0x00,0x00,0xBB}, {0x37,0x00,0xBF}, {0x84,0x00,0xA6},
   {0xBB,0x00,0x6A}, {0xB7,0x00,0x1E}, {0xB3,0x00,0x00}, {0x91,0x26,0x00},
   {0x7B,0x2B,0x00}, {0x00,0x3E,0x00}, {0x00,0x48,0x0D}, {0x00,0x3C,0x22},
   {0x00,0x2F,0x66}, {0x00,0x00,0x00}, {0x05,0x05,0x05}, {0x05,0x05,0x05},

   {0xC8,0xC8,0xC8}, {0x00,0x59,0xFF}, {0x44,0x3C,0xFF}, {0xB7,0x33,0xCC},
   {0xFF,0x33,0xAA}, {0xFF,0x37,0x5E}, {0xFF,0x37,0x1A}, {0xD5,0x4B,0x00},
   {0xC4,0x62,0x00}, {0x3C,0x7B,0x00}, {0x1E,0x84,0x15}, {0x00,0x95,0x66},
   {0x00,0x84,0xC4}, {0x11,0x11,0x11}, {0x09,0x09,0x09}, {0x09,0x09,0x09},

   {0xFF,0xFF,0xFF}, {0x00,0x95,0xFF}, {0x6F,0x84,0xFF}, {0xD5,0x6F,0xFF},
   {0xFF,0x77,0xCC}, {0xFF,0x6F,0x99}, {0xFF,0x7B,0x59}, {0xFF,0x91,0x5F},
   {0xFF,0xA2,0x33}, {0xA6,0xBF,0x00}, {0x51,0xD9,0x6A}, {0x4D,0xD5,0xAE},
   {0x00,0xD9,0xFF}, {0x66,0x66,0x66}, {0x0D,0x0D,0x0D}, {0x0D,0x0D,0x0D},

   {0xFF,0xFF,0xFF}, {0x84,0xBF,0xFF}, {0xBB,0xBB,0xFF}, {0xD0,0xBB,0xFF},
   {0xFF,0xBF,0xEA}, {0xFF,0xBF,0xCC}, {0xFF,0xC4,0xB7}, {0xFF,0xCC,0xAE},
   {0xFF,0xD9,0xA2}, {0xCC,0xE1,0x99}, {0xAE,0xEE,0xB7}, {0xAA,0xF7,0xEE},
   {0xB3,0xEE,0xFF}, {0xDD,0xDD,0xDD}, {0x11,0x11,0x11}, {0x11,0x11,0x11}
};
*/
/* Loopy
unsigned char palette[192] = {
	0x75, 0x75, 0x75,
	0x27, 0x1B, 0x8F,
	0x00, 0x00, 0xAB,
	0x47, 0x00, 0x9F,
	0x8F, 0x00, 0x77,
	0xAB, 0x00, 0x13,
	0xA7, 0x00, 0x00,
	0x7F, 0x0B, 0x00,
	0x43, 0x2F, 0x00,
	0x00, 0x47, 0x00,
	0x00, 0x51, 0x00,
	0x00, 0x3F, 0x17,
	0x1B, 0x3F, 0x5F,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xBC, 0xBC, 0xBC,
	0x00, 0x73, 0xEF,
	0x23, 0x3B, 0xEF,
	0x83, 0x00, 0xF3,
	0xBF, 0x00, 0xBF,
	0xE7, 0x00, 0x5B,
	0xDB, 0x2B, 0x00,
	0xCB, 0x4F, 0x0F,
	0x8B, 0x73, 0x00,
	0x00, 0x97, 0x00,
	0x00, 0xAB, 0x00,
	0x00, 0x93, 0x3B,
	0x00, 0x83, 0x8B,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF,
	0x3F, 0xBF, 0xFF,
	0x5F, 0x97, 0xFF,
	0xA7, 0x8B, 0xFD,
	0xF7, 0x7B, 0xFF,
	0xFF, 0x77, 0xB7,
	0xFF, 0x77, 0x63,
	0xFF, 0x9B, 0x3B,
	0xF3, 0xBF, 0x3F,
	0x83, 0xD3, 0x13,
	0x4F, 0xDF, 0x4B,
	0x58, 0xF8, 0x98,
	0x00, 0xEB, 0xDB,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF,
	0xAB, 0xE7, 0xFF,
	0xC7, 0xD7, 0xFF,
	0xD7, 0xCB, 0xFF,
	0xFF, 0xC7, 0xFF,
	0xFF, 0xC7, 0xDB,
	0xFF, 0xBF, 0xB3,
	0xFF, 0xDB, 0xAB,
	0xFF, 0xE7, 0xA3,
	0xE3, 0xFF, 0xA3,
	0xAB, 0xF3, 0xBF,
	0xB3, 0xFF, 0xCF,
	0x9F, 0xFF, 0xF3,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00
};
*/
/* Shiru
unsigned char palette[192] =
{
	0x6C, 0x6C, 0x6C,
	0x00, 0x26, 0x8E,
	0x26, 0x00, 0xA8,
	0x40, 0x00, 0x94,
	0x70, 0x00, 0x70,
	0x78, 0x00, 0x40,
	0x57, 0x00, 0x00,
	0x5A, 0x28, 0x00,
	0x33, 0x2D, 0x00,
	0x0A, 0x3C, 0x00,
	0x00, 0x46, 0x00,
	0x00, 0x44, 0x28,
	0x00, 0x40, 0x60,
	0x00, 0x00, 0x00,
	0x08, 0x08, 0x08,
	0x08, 0x08, 0x08,
	0xC3, 0xC3, 0xC3,
	0x1E, 0x55, 0xDC,
	0x4B, 0x1E, 0xFF,
	0x80, 0x20, 0xF0,
	0xC0, 0x00, 0xC0,
	0xD0, 0x14, 0x74,
	0xC1, 0x2E, 0x1A,
	0x8C, 0x46, 0x0F,
	0x76, 0x5E, 0x00,
	0x24, 0x82, 0x00,
	0x00, 0x88, 0x00,
	0x00, 0x82, 0x4E,
	0x00, 0x74, 0x9C,
	0x20, 0x20, 0x20,
	0x08, 0x08, 0x08,
	0x08, 0x08, 0x08,
	0xFF, 0xFF, 0xFF,
	0x5A, 0xB4, 0xFF,
	0x8C, 0x8C, 0xFF,
	0xC0, 0x6C, 0xFF,
	0xFF, 0x50, 0xFF,
	0xFF, 0x64, 0xB8,
	0xFF, 0x8C, 0x78,
	0xFF, 0xAA, 0x46,
	0xDC, 0xBE, 0x00,
	0x96, 0xDA, 0x20,
	0x4A, 0xDC, 0x4A,
	0x32, 0xE6, 0x8C,
	0x1C, 0xE6, 0xDC,
	0x58, 0x58, 0x58,
	0x08, 0x08, 0x08,
	0x08, 0x08, 0x08,
	0xFF, 0xFF, 0xFF,
	0xD2, 0xF0, 0xFF,
	0xC4, 0xC4, 0xFF,
	0xE8, 0xB8, 0xFF,
	0xFF, 0xB0, 0xFF,
	0xFF, 0xB8, 0xE8,
	0xFF, 0xDC, 0xC4,
	0xFF, 0xDE, 0xA8,
	0xFF, 0xF0, 0x96,
	0xF0, 0xF4, 0xA4,
	0xC0, 0xFF, 0xC0,
	0xAC, 0xF4, 0xBE,
	0xCD, 0xF0, 0xDC,
	0xC2, 0xC2, 0xC2,
	0x20, 0x20, 0x20,
	0x08, 0x08, 0x08
} ;
*/
