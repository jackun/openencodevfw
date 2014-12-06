//#define SATURATE_MANUALLY
//BMP is usually upside-down
#define FLIP
//#define USE_FLOAT3
#define USE_FLOAT4
//#define USE_STAGGERED

#define UpperLimit	235.0f/255.0f

#ifdef BT601_FULL

#define Ycoeff ((float4)(0.299f, 0.587f, 0.114f, 0.f))
#define Ucoeff ((float4)(-0.14713f, -0.28886f, 0.436f, 0.f))
#define Vcoeff ((float4)(0.615f, -0.51499, -0.10001f, 0.f))

//BGR
#define YcoeffB ((float4)(0.114f, 0.587f, 0.299f, 0.f))
#define UcoeffB ((float4)(0.436f, -0.28886f, -0.14713f, 0.f))
#define VcoeffB ((float4)(-0.10001f, -0.51499, 0.615f, 0.f))

#else
#ifdef BT709_FULL

#define Ycoeff ((float4)(0.2126f, 0.7152f, 0.0722f, 0.f))
#define Ucoeff ((float4)(-0.09991f, -0.33609f, 0.436f, 0.f))
#define Vcoeff ((float4)(0.615f, -0.55861f, -0.05639f, 0.f))

//BGR
#define YcoeffB ((float4)(0.0722f, 0.7152f, 0.2126f, 0.f))
#define UcoeffB ((float4)(0.436f, -0.33609f, -0.09991f, 0.f))
#define VcoeffB ((float4)(-0.05639f, -0.55861f, 0.615f, 0.f))

#else

#ifdef BT709_LIMITED

// FULL coefficients!
#define Ycoeff ((float4)(0.2126f, 0.7152f, 0.0722f, 0.f))
#define Ucoeff ((float4)(-0.09991f, -0.33609f, 0.436f, 0.f))
#define Vcoeff ((float4)(0.615f, -0.55861f, -0.05639f, 0.f))

//BGR
#define YcoeffB ((float4)(0.0722f, 0.7152f, 0.2126f, 0.f))
#define UcoeffB ((float4)(0.436f, -0.33609f, -0.09991f, 0.f))
#define VcoeffB ((float4)(-0.05639f, -0.55861f, 0.615f, 0.f))

#else //BT601_LIMITED

//http://www.mplayerhq.hu/DOCS/tech/colorspaces.txt
//Looks like Rec. 601 limited range
#define Ycoeff ((float4)(0.257f, 0.504f, 0.098f, 0.f))
#define Ucoeff ((float4)(-0.148f, -0.291f, 0.439f, 0.f))
#define Vcoeff ((float4)(0.439f, -0.368f, -0.071f, 0.f))

#define YcoeffB ((float4)(0.098f, 0.504f, 0.257f, 0.f))
#define UcoeffB ((float4)(0.439f, -0.291f, -0.148f, 0))
#define VcoeffB ((float4)(-0.071f, -0.368f, 0.439f, 0))

#endif //BT709_LIMITED
#endif //BT709_FULL
#endif //BT601_FULL

float3 RGBtoYUV(uchar R, uchar G, uchar B)
{
#ifdef COLORSPACE_LIMIT
    R = 16.f + R * UpperLimit;
    G = 16.f + G * UpperLimit;
    B = 16.f + B * UpperLimit;
#endif

    float Y = (0.257f * R) + (0.504f * G) + (0.098f * B) + 16.f;
    float V = (0.439f * R) - (0.368f * G) - (0.071f * B) + 128.f;
    float U = -(0.148f * R) - (0.291f * G) + (0.439f * B) + 128.f;

    return (float3)(Y,U,V);
}

//TODO dot() speaks only of float, float2, float4
float2 toUV(float3 RGB)
{
    return (float2) (dot(RGB, Ucoeff.xyz) + 128.f,
                    dot(RGB, Vcoeff.xyz) + 128.f);
}

float2 toUV4(float4 RGB)
{
    return (float2) (dot(RGB, Ucoeff) + 128.f,
                    dot(RGB, Vcoeff) + 128.f);
}

// Convert RGBA format to NV12
__kernel void RGBAtoNV12Combined(__global uchar4 *input,
                        __global uchar *output,
                        int alignedWidth)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    int width = get_global_size(0);
    int height = get_global_size(1);
#ifdef FLIP
    uchar4 rgba = input[id.x + width * (height - id.y - 1)];
#else
    uchar4 rgba = input[id.x + width * id.y];
#endif

    float3 YUV = RGBtoYUV(rgba.x, rgba.y, rgba.z);

    //should use convert_uchar_sat_rte but that seems to be slower on CPU atleast
    output[id.x + id.y * alignedWidth] = YUV.x; //convert_uchar_sat_rte(Y);
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2] = YUV.y;
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2 + 1] = YUV.z;
}

__kernel void BGRAtoNV12Combined(__global uchar4 *input,
                        __global uchar *output,
                        int alignedWidth)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    int width = get_global_size(0);
    int height = get_global_size(1);
#ifdef FLIP
    float4 rgba = convert_float4(input[id.x + width * (height - id.y - 1)]);
#else
    float4 rgba = convert_float4(input[id.x + width * id.y]);
#endif

#if defined(BT601_FULL) || defined(BT709_FULL)
    float range = 0.f;
#else
    float range = 16.f;
#endif

    float Y = dot(YcoeffB, rgba) + range;
    float U = dot(UcoeffB, rgba) + 128.f; //Probably should sample 2x2
    float V = dot(VcoeffB, rgba) + 128.f;

    output[id.x + id.y * alignedWidth] = Y; //convert_uchar_sat_rte(Y);
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2] = U;
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2 + 1] = V;
}

// Convert RGBA format to NV12
__kernel void RGBAtoNV12_Y(__global uchar4 *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    int width = get_global_size(0);
    int height = get_global_size(1);

    float4 rgba = convert_float4(input[id.x + width * id.y]);

#if defined(BT601_FULL) || defined(BT709_FULL)
    float range = 0.f;
#else
    float range = 16.f;
#endif

#ifdef USE_FLOAT4
    float Y = dot(rgba, Ycoeff) + range;
#else
    float Y = (0.257f * rgba.x) + (0.504f * rgba.y) + (0.098f * rgba.z) + 16.f;
#endif

#ifdef FLIP
#ifdef USE_STAGGERED
    output[id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth] = convert_uchar_sat_rte(Y);
#else
    output[id.x + (height- id.y - 1) * alignedWidth] = convert_uchar_sat_rte(Y);
#endif
#else
    output[offset + id.x + id.y * alignedWidth] = convert_uchar_sat_rte(Y);
#endif
}

// Convert only UV from RGBA format to NV12
// Run over half width/height
__kernel void RGBAtoNV12_UV(__global uchar4 *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));
    
    uint width = get_global_size(0) * 2;
    uint heightHalf = get_global_size(1);
#ifdef USE_STAGGERED
    uint height = get_global_size(1) * 4; //half size and half the samples (2*2)
#else
    uint height = get_global_size(1) * 2;
#endif

#ifdef FLIP
#ifdef USE_STAGGERED
    //id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth
    uint uv_offset = alignedWidth * height + //Skip Y bytes
                    ( (heightHalf*2 - offset) - id.y - 1) * alignedWidth + id.x * 2;
#else
    uint uv_offset = alignedWidth * height + //Skip Y bytes
                    (heightHalf - id.y - 1) * alignedWidth + id.x * 2;
#endif
#else
    uint uv_offset = offset + alignedWidth * height + id.y * alignedWidth + id.x * 2;
#endif

    uint src = id.x * 2 + width * id.y * 2;

    // sample 2x2 square
    uchar4 rgb00 = input[src];
    uchar4 rgb01 = input[src + 1];
    //next line
    uchar4 rgb10 = input[src + width];
    uchar4 rgb11 = input[src + width + 1];
    
//slower (on cpu atleast)
#ifdef USE_FLOAT4
    //1,2
    float4 RGB00 = convert_float4(rgb00);
    float4 RGB01 = convert_float4(rgb01);
    float4 RGB10 = convert_float4(rgb10);
    float4 RGB11 = convert_float4(rgb11);

    //1
    //float4 RGB = (RGB00 + RGB01 + RGB10 + RGB11) * 0.25f
    //float2 UV = (float2)(-(0.148f * RGB.x) - (0.291f * RGB.y) + (0.439f * RGB.z) + 128.f,
    //                      (0.439f * RGB.x) - (0.368f * RGB.y) - (0.071f * RGB.z) + 128.f);

    //2
    float2 UV00 = toUV4(RGB00);
    float2 UV01 = toUV4(RGB01);
    float2 UV10 = toUV4(RGB10);
    float2 UV11 = toUV4(RGB11);
    uchar2 UV = convert_uchar2_sat_rte((2.f + UV00 + UV01 + UV10 + UV11) / 4.f);

    //TODO convert_uchar2 slows this down?
    //uchar2 UV = convert_uchar2((2 + UV00 + UV01 + UV10 + UV11) / 4);

#else

    float2 UV00 =  (float2)(-(0.148f * rgb00.x) - (0.291f * rgb00.y) + (0.439f * rgb00.z) + 128.f,
                             (0.439f * rgb00.x) - (0.368f * rgb00.y) - (0.071f * rgb00.z) + 128.f);

    float2 UV01 =  (float2)(-(0.148f * rgb01.x) - (0.291f * rgb01.y) + (0.439f * rgb01.z) + 128.f,
                             (0.439f * rgb01.x) - (0.368f * rgb01.y) - (0.071f * rgb01.z) + 128.f);

    float2 UV10 =  (float2)(-(0.148f * rgb10.x) - (0.291f * rgb10.y) + (0.439f * rgb10.z) + 128.f,
                             (0.439f * rgb10.x) - (0.368f * rgb10.y) - (0.071f * rgb10.z) + 128.f);

    float2 UV11 =  (float2)(-(0.148f * rgb11.x) - (0.291f * rgb11.y) + (0.439f * rgb11.z) + 128.f,
                             (0.439f * rgb11.x) - (0.368f * rgb11.y) - (0.071f * rgb11.z) + 128.f);

    float2 UV =  (2 + UV00 + UV01 + UV10 + UV11) / 4;
#endif

    output[uv_offset]     = UV.x;
    output[uv_offset + 1] = UV.y;
}

__kernel void BGRAtoNV12_Y(const __global uchar4 *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    int width = get_global_size(0);
    int height = get_global_size(1);

    //uchar4 bgra = input[id.x + width * id.y];
    //float Y = (0.257f * bgra.z) + (0.504f * bgra.y) + (0.098f * bgra.x) + 16.f;
    float4 bgra = convert_float4(input[id.x + width * id.y]);

#if defined(BT601_FULL) || defined(BT709_FULL)
    uchar Y = convert_uchar_sat_rte(dot(YcoeffB, bgra));
#else
    uchar Y = convert_uchar_sat_rte(dot(YcoeffB, bgra) + 16.f);
#endif

    //should use convert_uchar_sat_rte but that seems to slow shit down
#ifdef FLIP
#ifdef USE_STAGGERED
    output[id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth] = Y;
#else
    output[id.x + (height- id.y - 1) * alignedWidth] = Y;
#endif
#else
    output[offset + id.x + id.y * alignedWidth] = Y;
#endif
}

__kernel void BGRAtoNV12_UV(const __global uchar4 *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    uint width = get_global_size(0) * 2;
    uint src = id.x * 2 + width * id.y * 2;
    uint heightHalf = get_global_size(1);
#ifdef USE_STAGGERED
    uint height = get_global_size(1) * 4; //half size and half the samples (2*2)
#else
    uint height = get_global_size(1) * 2;
#endif

#ifdef FLIP
#ifdef USE_STAGGERED
    uint uv_offset = alignedWidth * height + //Skip Y bytes
                    ( (heightHalf*2 - offset) - id.y - 1) * alignedWidth + id.x * 2;
#else
    uint uv_offset = alignedWidth * height + //Skip Y bytes
                    (heightHalf - id.y - 1) * alignedWidth + id.x * 2;
#endif
#else
    uint uv_offset = offset + alignedWidth * height + id.y * alignedWidth + id.x * 2;
#endif

	//Seems like no difference between dot() and plain mul/add on GPU atleast
    // sample 2x2 square
    float4 bgr00 = convert_float4(input[src]);
    float4 bgr01 = convert_float4(input[src + 1]);
    //next line
    float4 bgr10 = convert_float4(input[src + width]);
    float4 bgr11 = convert_float4(input[src + width + 1]);

    float2 UV00 =  (float2)(dot(bgr00, UcoeffB) + 128.f,
                            dot(bgr00, VcoeffB) + 128.f);

    float2 UV01 =  (float2)(dot(bgr01, UcoeffB) + 128.f,
                            dot(bgr01, VcoeffB) + 128.f);

    float2 UV10 =  (float2)(dot(bgr10, UcoeffB) + 128.f,
                            dot(bgr10, VcoeffB) + 128.f);

    float2 UV11 =  (float2)(dot(bgr11, UcoeffB) + 128.f,
                            dot(bgr11, VcoeffB) + 128.f);
    uchar2 UV =  convert_uchar2_sat_rte((2 + UV00 + UV01 + UV10 + UV11) / 4);

    output[uv_offset]     = UV.x;
    output[uv_offset + 1] = UV.y;
}

// Convert RGB format to NV12. Colors seem a little off (oversaturated).
__kernel void RGBtoNV12_Y(__global uchar *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    //Unaligned read and probably slooooow
    uchar3 rgb = vload3(id.x + width * id.y, input);

#if defined(BT601_FULL) || defined(BT709_FULL)
    float range = 0.f;
#else
    float range = 16.f;
#endif

    uchar Y = convert_uchar_sat_rte((0.257f * rgb.x) + (0.504f * rgb.y) + (0.098f * rgb.z) + range);

#ifdef FLIP
#ifdef USE_STAGGERED
    output[id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth] = Y;
#else
    output[id.x + (height- id.y - 1) * alignedWidth] = Y;
#endif
#else
    output[offset + id.x + id.y * alignedWidth] = Y;
#endif
}

// Convert only UV from RGB format to NV12
// Run over half width/height
__kernel void RGBtoNV12_UV(__global uchar *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    uint width = get_global_size(0) * 2;
    uint heightHalf = get_global_size(1);
#ifdef USE_STAGGERED
    uint height = get_global_size(1) * 4; //half size and half the samples (2*2)
#else
    uint height = get_global_size(1) * 2;
#endif

#ifdef FLIP
#ifdef USE_STAGGERED
    //id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth
    uint uv_offset = alignedWidth * height + //Skip Y bytes
                    ( (heightHalf*2 - offset) - id.y - 1) * alignedWidth + id.x * 2;
#else
    uint uv_offset = alignedWidth * height + //Skip Y bytes
                    (heightHalf - id.y - 1) * alignedWidth + id.x * 2;
#endif
#else
    uint uv_offset = offset + alignedWidth * height + id.y * alignedWidth + id.x * 2;
#endif

    uint src = id.x * 2 + width * id.y * 2;

    // sample 2x2 square
    uchar3 rgb00 = vload3(src, input);
    uchar3 rgb01 = vload3(src + 1, input);
    //next line
    uchar3 rgb10 = vload3(src + width, input);
    uchar3 rgb11 = vload3(src + width + 1, input);

    //convert_float seemed to be slower on CPU (Core2)
    float3 RGB00 = convert_float3(rgb00);
    float3 RGB01 = convert_float3(rgb01);
    float3 RGB10 = convert_float3(rgb10);
    float3 RGB11 = convert_float3(rgb11);

    float2 UV00 = toUV(RGB00);
    float2 UV01 = toUV(RGB01);
    float2 UV10 = toUV(RGB10);
    float2 UV11 = toUV(RGB11);
    float2 UV =  (2 + UV00 + UV01 + UV10 + UV11) / 4;

    output[uv_offset]     = UV.x;//convert_uchar_rte(UV.x);
    output[uv_offset + 1] = UV.y;//convert_uchar_rte(UV.y);
}

__kernel void BGRtoNV12_Y(__global uchar *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    //Unaligned read and probably slooooow
    uchar3 rgb = vload3(id.x + width * id.y, input);

#if defined(BT601_FULL) || defined(BT709_FULL)
    float range = 0.f;
#else
    float range = 16.f;
#endif

    float Y = (0.257f * rgb.z) + (0.504f * rgb.y) + (0.098f * rgb.x) + range;

#ifdef FLIP
#ifdef USE_STAGGERED
    output[id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth] = Y;//convert_uchar_sat_rte(Y);
#else
    output[id.x + (height- id.y - 1) * alignedWidth] = Y;
#endif
#else
    output[offset + id.x + id.y * alignedWidth] = Y;
#endif
}

// Run over half width/height
__kernel void BGRtoNV12_UV(__global uchar *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    uint width = get_global_size(0) * 2;
    uint heightHalf = get_global_size(1);
#ifdef USE_STAGGERED
    uint height = get_global_size(1) * 4; //half size and half the samples (2*2)
#else
    uint height = get_global_size(1) * 2;
#endif

#ifdef FLIP
#ifdef USE_STAGGERED
    uint uv_offset = alignedWidth * height + //Skip Y bytes
                    ( (heightHalf*2 - offset) - id.y - 1) * alignedWidth + id.x * 2;
#else
    uint uv_offset = alignedWidth * height + //Skip Y bytes
                    (heightHalf - id.y - 1) * alignedWidth + id.x * 2;
#endif
#else
    uint uv_offset = offset + alignedWidth * height + id.y * alignedWidth + id.x * 2;
#endif

    uint src = id.x * 2 + width * id.y * 2;

    // sample 2x2 square
    uchar3 bgr00 = vload3(src, input);
    uchar3 bgr01 = vload3(src + 1, input);
    //next line
    uchar3 bgr10 = vload3(src + width, input);
    uchar3 bgr11 = vload3(src + width + 1, input);

    float2 UV00 =  (float2)(-(0.148f * bgr00.z) - (0.291f * bgr00.y) + (0.439f * bgr00.x) + 128.f,
                             (0.439f * bgr00.z) - (0.368f * bgr00.y) - (0.071f * bgr00.x) + 128.f);

    float2 UV01 =  (float2)(-(0.148f * bgr01.z) - (0.291f * bgr01.y) + (0.439f * bgr01.x) + 128.f,
                             (0.439f * bgr01.z) - (0.368f * bgr01.y) - (0.071f * bgr01.x) + 128.f);

    float2 UV10 =  (float2)(-(0.148f * bgr10.z) - (0.291f * bgr10.y) + (0.439f * bgr10.x) + 128.f,
                             (0.439f * bgr10.z) - (0.368f * bgr10.y) - (0.071f * bgr10.x) + 128.f);

    float2 UV11 =  (float2)(-(0.148f * bgr11.z) - (0.291f * bgr11.y) + (0.439f * bgr11.x) + 128.f,
                             (0.439f * bgr11.z) - (0.368f * bgr11.y) - (0.071f * bgr11.x) + 128.f);

    float2 UV =  (2 + UV00 + UV01 + UV10 + UV11) / 4;

    output[uv_offset]     = UV.x;//convert_uchar_rte(UV.x);
    output[uv_offset + 1] = UV.y;//convert_uchar_rte(UV.y);
}

//AMD openCL frontend adds gibberish at the end, so add a comment here to ... comment it. Mind the editors that append new line (\n).
//