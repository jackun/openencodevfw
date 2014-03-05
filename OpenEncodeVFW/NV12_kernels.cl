// AMD APP 2.8 SDK or 13.6beta6/13.8beta cl compiler complains about PAD() etc. being undefined :S
//#define SATURATE_MANUALLY
#define FLIP_RGB

#define RtoYCoeff	65.738f  * 256 + 0.5f
#define GtoYCoeff	129.057f * 256 + 0.5f
#define BtoYCoeff	5.064f   * 256 + 0.5f

#define RtoUCoeff	-37.945f * 256 + 0.5f
#define GtoUCoeff	-74.494f * 256 + 0.5f
#define BtoUCoeff	112.439f * 256 + 0.5f

#define RtoVCoeff	112.439f * 256 + 0.5f
#define GtoVCoeff	-94.154f * 256 + 0.5f
#define BtoVCoeff	-18.285f * 256 + 0.5f
#define UpperLimit	235.0f/255.0f

// Based on http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC30
// I dunno, all pink
float3 RGBtoYUV(float R, float G, float B)
{
	float Y = 16.f + (( RtoYCoeff * R + GtoYCoeff * G + BtoYCoeff * B) / 65536);
	float U = 128.f + (( RtoUCoeff * R + GtoUCoeff * G + BtoUCoeff * B) / 65536);
	float V = 128.f + (( RtoVCoeff * R + GtoVCoeff * G + BtoVCoeff * B) / 65536);
	return (float3)(Y,U,V);
}

float3 RGBtoYUV_2(float R, float G, float B)
{
#ifdef COLORSPACE_LIMIT
	R = 16.f + R * UpperLimit;
	G = 16.f + G * UpperLimit;
	B = 16.f + B * UpperLimit;
#endif
	float Y = (0.257f * R) + (0.504f * G) + (0.098f * B) + 16.f;
	float V = (0.439f * R) - (0.368f * G) - (0.071f * B) + 128.f;
	float U = -(0.148f * R) - (0.291f * G) + (0.439f * B) + 128.f;

	//http://softpixel.com/~cwright/programming/colorspace/yuv/ still reddish
	//float Y = R *  .299000f + G *  .587000f + B *  .114000f;
	//float U = R * -.168736f + G * -.331264f + B *  .500000f + 128.f;
	//float V = R *  .500000f + G * -.418688f + B * -.081312f + 128.f;

	//#2
	//float Y = 0.299f * R + 0.587f * G + 0.114f * B;
	//float U = -0.14713f * R - 0.28886f * G + 0.436f * B + 128;
	//float V = 0.615f * R - 0.51499f * G - 0.10001f * B + 128;
	//float V = (R - Y) * 0.713 + 128;
	//float U = (B - Y) * 0.564 + 128;

	//float Scale_factor = 0.65f;
	//U = (U - 128.f) * Scale_factor + 128.f;
	//V = (V - 128.f) * Scale_factor + 128.f;
	return (float3)(Y,U,V);
}

// Convert RGBA format to NV12
__kernel void RGBAtoNV12(__global uchar4 *input,
						__global uchar *output,
						int alignedWidth)
{
	int2 id = (int2)(get_global_id(0), get_global_id(1));

	int width = get_global_size(0);
	int height = get_global_size(1);
#ifdef FLIP_RGB
	uchar4 rgba = input[id.x + width * (height - id.y - 1)];
#else
	uchar4 rgba = input[id.x + width * id.y];
#endif

	float R = convert_float(rgba.s0);
	float G = convert_float(rgba.s1);
	float B = convert_float(rgba.s2);

	float3 YUV = RGBtoYUV_2(R, G, B);

	output[id.x + id.y * alignedWidth] = convert_uchar(YUV.s0 > 255 ? 255 : YUV.s0); //Y
	output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2] = convert_uchar(YUV.s2 > 255 ? 255 : YUV.s2); //V
	output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2 + 1] = convert_uchar(YUV.s1 > 255 ? 255 : YUV.s1); //U
}

// Convert RGB format to NV12. Colors seem a little off (oversaturated).
__kernel void RGBtoNV12(__global uchar *input,
                        __global uchar *output,
						int alignedWidth)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));
    
    uint width = get_global_size(0);
    uint height = get_global_size(1);

	//Unaligned read and probably slooooow
#ifdef FLIP_RGB
    uchar3 rgb = vload3(id.x + width * (height-id.y-1), input);
#else
    uchar3 rgb = vload3(id.x + width * id.y, input);
#endif
    
    float R = convert_float(rgb.s0);
    float G = convert_float(rgb.s1);
    float B = convert_float(rgb.s2);

	float3 YUV = RGBtoYUV_2(R,G,B);
                
    output[id.x + id.y * alignedWidth] = convert_uchar(YUV.s0); //Y
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2] = convert_uchar(YUV.s2 > 255 ? 255 : YUV.s2) ; //V
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2 + 1] = convert_uchar(YUV.s1 > 255 ? 255 : YUV.s1); //U
}

__kernel void RGBBlend(__global uchar *input1,
						__global uchar *output,
						int alignedWidth,
                       __global uchar *input2)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));
    
    uint width = get_global_size(0);
    uint height = get_global_size(1);

	//Unaligned read and probably slooooow
#ifdef FLIP_RGB
    float3 rgb1 = convert_float3(vload3(id.x + width * (height-id.y-1), input1));
	float3 rgb2 = convert_float3(vload3(id.x + width * (height-id.y-1), input2));
#else
    float3 rgb1 = convert_float3(vload3(id.x + width * id.y, input1));
	float3 rgb2 = convert_float3(vload3(id.x + width * id.y, input2));
#endif
    
	rgb1 = (rgb1+rgb2) / 2;

    float R = rgb1.s0;
    float G = rgb1.s1;
    float B = rgb1.s2;

    float Y = (0.257f * R) + (0.504f * G) + (0.098f * B) + 16.f;
    float U = (0.439f * R) - (0.368f * G) - (0.071f * B) + 128.f;
    float V = -(0.148f * R) - (0.291f * G) + (0.439f * B) + 128.f;
                
    output[id.x + id.y * alignedWidth] = convert_uchar(Y);
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2] = convert_uchar(U > 255 ? 255 : U) ;
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2 + 1] = convert_uchar(V > 255 ? 255 : V);
}

// Remove pitch kernel
__kernel void removePitch(__global uchar* input,
                          __global uchar *output,
                          int video_pitch)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    int pos_output = x + y * get_global_size(0);
    int pos_input = x + y * video_pitch;
    
    output[pos_output] = input[pos_input];
}


// Convert NV12 format to RGBA
__kernel void NV12toRGBA(__global uchar *input,
                        __global uchar4 *output)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));
    
    uint width = get_global_size(0);
    uint height = get_global_size(1);
    
    float Y = convert_float(input[id.x + id.y * width]);
    float U = convert_float(input[width * height + (id.y >> 1) * width + (id.x >> 1) * 2]);
    float V = convert_float(input[width * height + (id.y >> 1) * width + (id.x >> 1) * 2 + 1]);

    float B = (1.164f*(Y-16.0f) + 2.018f*(U-128.0f));
    float G = (1.164f*(Y-16.0f) - 0.813f*(V-128.0f) - 0.391f*(U-128.0f));
    float R = (1.164f*(Y-16.0f) + 1.596f*(V-128.0f));

#ifdef SATURATE_MANUALLY
    if(B < 0)
        B = 0;
    if(B > 255)
        B = 255;
        
    if(G < 0)
        G = 0;
    if(G > 255)
        G = 255;
        
    if(R < 0)
        R = 0;
    if(R > 255)
        R = 255;
    
    uchar BLUE = convert_uchar(B);
    uchar GREEN = convert_uchar(G);
    uchar RED = convert_uchar(R);
#else
    uchar BLUE = convert_uchar_sat(B);
    uchar GREEN = convert_uchar_sat(G);
    uchar RED = convert_uchar_sat(R);
#endif

#ifdef FLIP_RGB
    output[id.x + width * (height - id.y - 1)] = (uchar4)(RED, GREEN, BLUE, 255);
#else
    output[id.x + width * id.y] = (uchar4)(RED, GREEN, BLUE, 255);
#endif
}


// Convert NV12 format to RGB
__kernel void NV12toRGB(__global uchar *input,
                        __global uchar *output)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));
	
    uint width = get_global_size(0);
    uint height = get_global_size(1);
    
    float Y = convert_float(input[id.x + id.y * width]);
    float U = convert_float(input[width * height + (id.y >> 1) * width + (id.x >> 1) * 2]);
    float V = convert_float(input[width * height + (id.y >> 1) * width + (id.x >> 1) * 2 + 1]);

    float B = (1.164f*(Y-16.0f) + 2.018f*(U-128.0f));
    float G = (1.164f*(Y-16.0f) - 0.813f*(V-128.0f) - 0.391f*(U-128.0f));
    float R = (1.164f*(Y-16.0f) + 1.596f*(V-128.0f));

#ifdef SATURATE_MANUALLY
    if(B < 0)
        B = 0;
    if(B > 255)
        B = 255;
        
    if(G < 0)
        G = 0;
    if(G > 255)
        G = 255;
        
    if(R < 0)
        R = 0;
    if(R > 255)
        R = 255;
    
    uchar BLUE = convert_uchar(B);
    uchar GREEN = convert_uchar(G);
    uchar RED = convert_uchar(R);
#else
    uchar BLUE = convert_uchar_sat(B);
    uchar GREEN = convert_uchar_sat(G);
    uchar RED = convert_uchar_sat(R);
#endif

#ifdef FLIP_RGB
    //output[id.x + width * (height - id.y - 1)] = (uchar3)(RED, GREEN, BLUE);
    uchar3 out = (uchar3)(RED, GREEN, BLUE);
    vstore3(out, id.x + width * (height - id.y - 1),  output);
#else
    //doesn't seem to do unaligned
    //output[id.x + width * id.y] = (uchar3)(RED, GREEN, BLUE);
    //so use vstore3?
    uchar3 out = (uchar3)(RED, GREEN, BLUE);
    vstore3(out, id.x + width * id.y,  output);
#endif
}

//AMD openCL frontend adds gibberish at the end, so add a comment here to ... comment it. Mind the editors that append carrier return (\r).
//