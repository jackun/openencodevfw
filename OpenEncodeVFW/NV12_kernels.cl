//#define SATURATE_MANUALLY
#define FLIP_RGB

#define RtoYCoeff (uint)(65.738f * 256 + 0.5f)
#define GtoYCoeff (uint)(129.057f * 256 + 0.5f)
#define BtoYCoeff (uint)(25.064f * 256 + 0.5f)

#define RtoUVCoeff ((float2)((-37.945f * 256 + 0.5f), (112.439f * 256 + 0.5f)))
#define GtoUVCoeff ((float2)((-74.494f * 256 + 0.5f), (-94.154f * 256 + 0.5f)))
#define BtoUVCoeff ((float2)((112.439f * 256 + 0.5f), (-18.285f * 256 + 0.5f)))

#define Ycoff ((float4)(0.257f, 0.504f, 0.098f, 0.f))
#define Ucoff ((float4)(-0.148f, -0.291f, 0.439f, 0.f))
#define Vcoff ((float4)(0.439f, -0.368f, -0.071f, 0.f))

#define UpperLimit	235.0f/255.0f

//#define USE_FLOAT3
//#define USE_FLOAT4
//#define USE_STAGGERED

float3 RGBtoYUV_2(uchar R, uchar G, uchar B)
{
#ifdef COLORSPACE_LIMIT
    R = 16.f + R * UpperLimit;
    G = 16.f + G * UpperLimit;
    B = 16.f + B * UpperLimit;
#endif

    //http://www.mplayerhq.hu/DOCS/tech/colorspaces.txt
    float Y = (0.257f * R) + (0.504f * G) + (0.098f * B) + 16.f;
    float V = (0.439f * R) - (0.368f * G) - (0.071f * B) + 128.f;
    float U = -(0.148f * R) - (0.291f * G) + (0.439f * B) + 128.f;

    //http://softpixel.com/~cwright/programming/colorspace/yuv/
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

float2 toUV(float3 RGB)
{
    return (float2) (-(0.148f * RGB.x) - (0.291f * RGB.y) + (0.439f * RGB.z) + 128.f,
                    (0.439f * RGB.x) - (0.368f * RGB.y) - (0.071f * RGB.z) + 128.f);
}

float2 toUV4(float4 RGB)
{
    return (float2) (-(0.148f * RGB.x) - (0.291f * RGB.y) + (0.439f * RGB.z) + 128.f,
                    (0.439f * RGB.x) - (0.368f * RGB.y) - (0.071f * RGB.z) + 128.f);
}

// Convert RGBA format to NV12
__kernel void RGBAtoNV12Combined(__global uchar4 *input,
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

    float3 YUV = RGBtoYUV_2(rgba.x, rgba.y, rgba.z);

    //should use convert_uchar_sat_rte but that seems to slow shit down
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
#ifdef FLIP_RGB
    uchar4 rgba = input[id.x + width * (height - id.y - 1)];
#else
    uchar4 rgba = input[id.x + width * id.y];
#endif

    float3 YUV = RGBtoYUV_2(rgba.z, rgba.y, rgba.x);

    //should use convert_uchar_sat_rte but that seems to slow shit down
    output[id.x + id.y * alignedWidth] = YUV.x; //convert_uchar_sat_rte(Y);
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2] = YUV.y;
    output[alignedWidth * height + (id.y >> 1) * alignedWidth + (id.x >> 1) * 2 + 1] = YUV.z;
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

    //float3 YUV = RGBtoYUV_2(rgba.x, rgba.y, rgba.z);
    float4 rgba = convert_float4(input[id.x + width * id.y]);

    //uchar Y = 16 + ((32768 + RtoYCoeff * rgba.x + GtoYCoeff * rgba.y + BtoYCoeff * rgba.z) / 65536);
    //float Y = dot(rgba, Ycoff) + 16.f;
    float Y = (0.257f * rgba.x) + (0.504f * rgba.y) + (0.098f * rgba.z) + 16.f;

    //uchar4 rgb = input[id.x + width * id.y];
    //float Y = 16.f + ((32768 + RtoYCoeff * R + GtoYCoeff * G + BtoYCoeff * B) / 65536);

    //should use convert_uchar_sat_rte but that seems to slow shit down
#ifdef FLIP_RGB
#ifdef USE_STAGGERED
    output[id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth] = Y;//convert_uchar_sat_rte(Y);
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

#ifdef FLIP_RGB
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
    float2 UV = (2.f + UV00 + UV01 + UV10 + UV11) / 4.f;
    
    //3
    //float2 UV00 = 128 + ((32768 + RtoUVCoeff * rgb00.x + GtoUVCoeff * rgb00.y + BtoUVCoeff * rgb00.z) / 65536);
    //float2 UV01 = 128 + ((32768 + RtoUVCoeff * rgb01.x + GtoUVCoeff * rgb01.y + BtoUVCoeff * rgb01.z) / 65536);
    //float2 UV10 = 128 + ((32768 + RtoUVCoeff * rgb10.x + GtoUVCoeff * rgb10.y + BtoUVCoeff * rgb10.z) / 65536);
    //float2 UV11 = 128 + ((32768 + RtoUVCoeff * rgb11.x + GtoUVCoeff * rgb11.y + BtoUVCoeff * rgb11.z) / 65536);
    //TODO convert_uchar2 slows this down?
    //uchar2 UV = convert_uchar2((2 + UV00 + UV01 + UV10 + UV11) / 4);
    
#else
#if 0
    float2 UV00 =  (float2)(dot(rgb00, Ucoff) + 128.f,
                            dot(rgb00, Vcoff) + 128.f);

    float2 UV01 =  (float2)(dot(rgb01, Ucoff) + 128.f,
                            dot(rgb01, Vcoff) + 128.f);

    float2 UV10 =  (float2)(dot(rgb10, Ucoff) + 128.f,
                            dot(rgb10, Vcoff) + 128.f);

    float2 UV11 =  (float2)(dot(rgb11, Ucoff) + 128.f,
                            dot(rgb11, Vcoff) + 128.f);
#endif

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

__kernel void BGRAtoNV12_Y(__global uchar4 *input,
                        __global uchar *output,
                        int alignedWidth,
                        int offset)
{
    int2 id = (int2)(get_global_id(0), get_global_id(1));

    int width = get_global_size(0);
    int height = get_global_size(1);

    uchar4 bgra = input[id.x + width * id.y];
    float Y = (0.257f * bgra.z) + (0.504f * bgra.y) + (0.098f * bgra.x) + 16.f;

    //should use convert_uchar_sat_rte but that seems to slow shit down
#ifdef FLIP_RGB
#ifdef USE_STAGGERED
    output[id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth] = Y;//convert_uchar_sat_rte(Y);
#else
    output[id.x + (height- id.y - 1) * alignedWidth] = Y;
#endif
#else
    output[offset + id.x + id.y * alignedWidth] = Y;
#endif
}

__kernel void BGRAtoNV12_UV(__global uchar4 *input,
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

#ifdef FLIP_RGB
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
    uchar4 bgr00 = input[src];
    uchar4 bgr01 = input[src + 1];
    //next line
    uchar4 bgr10 = input[src + width];
    uchar4 bgr11 = input[src + width + 1];
    

    float2 UV00 =  (float2)(-(0.148f * bgr00.z) - (0.291f * bgr00.y) + (0.439f * bgr00.x) + 128.f,
                             (0.439f * bgr00.z) - (0.368f * bgr00.y) - (0.071f * bgr00.x) + 128.f);

    float2 UV01 =  (float2)(-(0.148f * bgr01.z) - (0.291f * bgr01.y) + (0.439f * bgr01.x) + 128.f,
                             (0.439f * bgr01.z) - (0.368f * bgr01.y) - (0.071f * bgr01.x) + 128.f);

    float2 UV10 =  (float2)(-(0.148f * bgr10.z) - (0.291f * bgr10.y) + (0.439f * bgr10.x) + 128.f,
                             (0.439f * bgr10.z) - (0.368f * bgr10.y) - (0.071f * bgr10.x) + 128.f);

    float2 UV11 =  (float2)(-(0.148f * bgr11.z) - (0.291f * bgr11.y) + (0.439f * bgr11.x) + 128.f,
                             (0.439f * bgr11.z) - (0.368f * bgr11.y) - (0.071f * bgr11.x) + 128.f);

    float2 UV =  (2 + UV00 + UV01 + UV10 + UV11) / 4;

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

#ifdef USE_FLOAT3
    uchar Y = 16 + ((32768 + RtoYCoeff * rgb.x + GtoYCoeff * rgb.y + BtoYCoeff * rgb.z) / 65536);
#else
    float Y = (0.257f * rgb.x) + (0.504f * rgb.y) + (0.098f * rgb.z) + 16.f;
#endif

#ifdef FLIP_RGB
#ifdef USE_STAGGERED
    output[id.x + ( ((height*2  - offset)- id.y - 1) ) * alignedWidth] = Y;//convert_uchar_sat_rte(Y);
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

#ifdef FLIP_RGB
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
    
//slower (on cpu atleast)
#ifdef USE_FLOAT3
    //1,2
    //float3 RGB00 = convert_float3(rgb00);
    //float3 RGB01 = convert_float3(rgb01);
    //float3 RGB10 = convert_float3(rgb10);
    //float3 RGB11 = convert_float3(rgb11);

    //1
    //float3 RGB = (RGB00 + RGB01 + RGB10 + RGB11) * 0.25f
    //float V =  (0.439f * RGB.x) - (0.368f * RGB.y) - (0.071f * RGB.z) + 128.f;
    //float U = -(0.148f * RGB.x) - (0.291f * RGB.y) + (0.439f * RGB.z) + 128.f;

    //2
    //float2 UV00 = toUV(RGB00);
    //float2 UV01 = toUV(RGB01);
    //float2 UV10 = toUV(RGB10);
    //float2 UV11 = toUV(RGB11);
    //float2 UV = (2.f + UV00 + UV01 + UV10 + UV11) / 4.f;
    
    //3
    float2 UV00 = 128 + ((32768 + RtoUVCoeff * rgb00.x + GtoUVCoeff * rgb00.y + BtoUVCoeff * rgb00.z) / 65536);
    float2 UV01 = 128 + ((32768 + RtoUVCoeff * rgb01.x + GtoUVCoeff * rgb01.y + BtoUVCoeff * rgb01.z) / 65536);
    float2 UV10 = 128 + ((32768 + RtoUVCoeff * rgb10.x + GtoUVCoeff * rgb10.y + BtoUVCoeff * rgb10.z) / 65536);
    float2 UV11 = 128 + ((32768 + RtoUVCoeff * rgb11.x + GtoUVCoeff * rgb11.y + BtoUVCoeff * rgb11.z) / 65536);
    uchar2 UV = convert_uchar2((2 + UV00 + UV01 + UV10 + UV11) / 4);
    
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

    float Y = (0.257f * rgb.z) + (0.504f * rgb.y) + (0.098f * rgb.x) + 16.f;

#ifdef FLIP_RGB
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

#ifdef FLIP_RGB
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

//AMD openCL frontend adds gibberish at the end, so add a comment here to ... comment it. Mind the editors that append new line (\n).
//