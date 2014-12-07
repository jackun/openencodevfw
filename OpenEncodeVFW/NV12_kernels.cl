//BMP is usually upside-down
#define FLIP
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
    uchar Y = convert_uchar_sat_rte(dot(Ycoeff, rgba));
#else
    uchar Y = convert_uchar_sat_rte(dot(Ycoeff, rgba) + 16.f);
#endif

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
    float4 rgb00 = convert_float4(input[src]);
    float4 rgb01 = convert_float4(input[src + 1]);
    //next line
    float4 rgb10 = convert_float4(input[src + width]);
    float4 rgb11 = convert_float4(input[src + width + 1]);

    float2 UV00 =  (float2)(dot(rgb00, Ucoeff) + 128.f,
                            dot(rgb00, Vcoeff) + 128.f);

    float2 UV01 =  (float2)(dot(rgb01, Ucoeff) + 128.f,
                            dot(rgb01, Vcoeff) + 128.f);

    float2 UV10 =  (float2)(dot(rgb10, Ucoeff) + 128.f,
                            dot(rgb10, Vcoeff) + 128.f);

    float2 UV11 =  (float2)(dot(rgb11, Ucoeff) + 128.f,
                            dot(rgb11, Vcoeff) + 128.f);

    uchar2 UV =  convert_uchar2_sat_rte((2 + UV00 + UV01 + UV10 + UV11) / 4);

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

// Convert RGB format to NV12.
__kernel void RGBtoNV12_Y(__global uchar *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    //Unaligned read and probably slooooow
    float4 rgba = (float4)(convert_float3(vload3(id.x + width * id.y, input)), 1.0f);

#if defined(BT601_FULL) || defined(BT709_FULL)
    uchar Y = convert_uchar_sat_rte(dot(Ycoeff, rgba));
#else
    uchar Y = convert_uchar_sat_rte(dot(Ycoeff, rgba) + 16.f);
#endif

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
    float4 rgb00 = (float4)(convert_float3(vload3(src, input)), 1.0f);
    float4 rgb01 = (float4)(convert_float3(vload3(src + 1, input)), 1.0f);
    //next line
    float4 rgb10 = (float4)(convert_float3(vload3(src + width, input)), 1.0f);
    float4 rgb11 = (float4)(convert_float3(vload3(src + width + 1, input)), 1.0f);

    float2 UV00 =  (float2)(dot(rgb00, Ucoeff) + 128.f,
                            dot(rgb00, Vcoeff) + 128.f);

    float2 UV01 =  (float2)(dot(rgb01, Ucoeff) + 128.f,
                            dot(rgb01, Vcoeff) + 128.f);

    float2 UV10 =  (float2)(dot(rgb10, Ucoeff) + 128.f,
                            dot(rgb10, Vcoeff) + 128.f);

    float2 UV11 =  (float2)(dot(rgb11, Ucoeff) + 128.f,
                            dot(rgb11, Vcoeff) + 128.f);

    uchar2 UV =  convert_uchar2_sat_rte((2 + UV00 + UV01 + UV10 + UV11) / 4);

    output[uv_offset]     = UV.x;
    output[uv_offset + 1] = UV.y;
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
    float4 bgra = (float4)(convert_float3(vload3(id.x + width * id.y, input)), 1.0f);

#if defined(BT601_FULL) || defined(BT709_FULL)
    uchar Y = convert_uchar_sat_rte(dot(YcoeffB, bgra));
#else
    uchar Y = convert_uchar_sat_rte(dot(YcoeffB, bgra) + 16.f);
#endif

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
    float4 bgr00 = (float4)(convert_float3(vload3(src, input)), 1.0f);
    float4 bgr01 = (float4)(convert_float3(vload3(src + 1, input)), 1.0f);
    //next line
    float4 bgr10 = (float4)(convert_float3(vload3(src + width, input)), 1.0f);
    float4 bgr11 = (float4)(convert_float3(vload3(src + width + 1, input)), 1.0f);

    float2 UV00 =  (float2)(dot(bgr00, UcoeffB) + 128.f,
                            dot(bgr00, VcoeffB) + 128.f);

    float2 UV01 =  (float2)(dot(bgr01, UcoeffB) + 128.f,
                            dot(bgr01, VcoeffB) + 128.f);

    float2 UV10 =  (float2)(dot(bgr10, UcoeffB) + 128.f,
                            dot(bgr10, VcoeffB) + 128.f);

    float2 UV11 =  (float2)(dot(bgr11, UcoeffB) + 128.f,
                            dot(bgr11, VcoeffB) + 128.f);

    uchar2 UV = convert_uchar2_sat_rte((2 + UV00 + UV01 + UV10 + UV11) / 4);

    output[uv_offset]     = UV.x;
    output[uv_offset + 1] = UV.y;
}

//AMD openCL frontend adds gibberish at the end, so add a comment here to ... comment it. Mind the editors that append new line (\n).
//