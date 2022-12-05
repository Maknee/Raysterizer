// ------------------------------------------------------------------------
// STRUCTURES -------------------------------------------------------------
// ------------------------------------------------------------------------

#extension GL_EXT_buffer_reference2 : require

#extension GL_EXT_scalar_block_layout : require
//#extension GL_EXT_shader_8bit_storage : require
//#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : require

#define DEFINE_PTR(base_type, type_ptr) \
    layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer type_ptr \
    { \
        base_type data[]; \
    }; \
    \


/*
layout(buffer_reference, scalar, buffer_reference_align = 1) readonly buffer UInt8Ptr
{
    uint8_t data[];
};
*/

DEFINE_PTR(uint8_t, UInt8Ptr)
DEFINE_PTR(uint16_t, UInt16Ptr)
DEFINE_PTR(uint32_t, UInt32Ptr)
DEFINE_PTR(uint64_t, UInt64Ptr)
DEFINE_PTR(vec2, Vec2Ptr)
DEFINE_PTR(vec3, Vec3Ptr)
DEFINE_PTR(vec4, Vec4Ptr)
DEFINE_PTR(ivec2, IVec2Ptr)
DEFINE_PTR(ivec3, IVec3Ptr)
DEFINE_PTR(ivec4, IVec4Ptr)
DEFINE_PTR(uvec2, UVec2Ptr)
DEFINE_PTR(uvec3, UVec3Ptr)
DEFINE_PTR(uvec4, UVec4Ptr)

#define RAYSTERIZER_DATA_TYPE_UNKNOWN 0
#define RAYSTERIZER_DATA_TYPE_INT 1
#define RAYSTERIZER_DATA_TYPE_IVEC2 2
#define RAYSTERIZER_DATA_TYPE_IVEC3 3
#define RAYSTERIZER_DATA_TYPE_IVEC4 4
#define RAYSTERIZER_DATA_TYPE_UINT 5
#define RAYSTERIZER_DATA_TYPE_UVEC2 6
#define RAYSTERIZER_DATA_TYPE_UVEC3 7
#define RAYSTERIZER_DATA_TYPE_UVEC4 8
#define RAYSTERIZER_DATA_TYPE_FLOAT 9
#define RAYSTERIZER_DATA_TYPE_VEC2 10
#define RAYSTERIZER_DATA_TYPE_VEC3 11
#define RAYSTERIZER_DATA_TYPE_VEC4 12

struct InstanceData
{
    uint64_t material_index;

    uint64_t vertices;
    uint64_t indices;

    uint64_t vertex_stride;
    uint64_t index_stride;

    uint64_t position_offset;
    uint64_t normal_offset;
    uint64_t tex_coord_offset;

    uint64_t position_data_type;
    uint64_t normal_data_type;
    uint64_t tex_coord_data_type;

    uint64_t out_color_buffer_index;
};

struct MeshData
{
    uint64_t vertices;
    uint64_t indices;
};

struct Material
{
    vec4 highlight_color;
    bool highlight;

    vec4 albedo;
    vec4 roughness_metallic;
    vec4 emissive;
};

#if defined(RAY_TRACING)
layout (set = 0, binding = 0) uniform accelerationStructureEXT u_TopLevelAS;
#endif

layout(set = 0, binding = 1) buffer InstanceDatas
{
  InstanceData instance_datas[];
};

layout(set = 0, binding = 2) buffer Materials
{
  Material materials[];
};

layout(set = 0, binding = 3) buffer RaysterizerOutColors
{
  vec4 data[];
} raysterizer_out_colors[];

struct Vertex
{
    vec4 position;
    vec2 tex_coord;
    vec3 normal;
    vec4 tangent;
    vec4 bitangent;

    bool valid_normal;
    bool valid_tex_coord;
};

struct Triangle
{
    Vertex v0;
    Vertex v1;
    Vertex v2;
};

struct Instance
{
    mat4 model_matrix;
    uint mesh_idx;
};

struct HitInfo
{
    uint mat_idx;
    uint primitive_offset;
    uint primitive_id;
};

struct SurfaceProperties
{
    Vertex vertex;
    vec4 albedo;
    vec3 emissive;
    vec3 normal;
    vec3 F0;
    float metallic;
    float roughness;   
    float alpha;
    float alpha2; 
};

// ------------------------------------------------------------------------
// DESCRIPTOR SETS --------------------------------------------------------
// ------------------------------------------------------------------------

/*
layout (set = 0, binding = 0, std430) readonly buffer MaterialBuffer 
{
    Material data[];
} Materials;

layout (set = 0, binding = 1, std430) readonly buffer InstanceBuffer 
{
    Instance data[];
} Instances;

#if defined(RAY_TRACING)
layout (set = 0, binding = 2) uniform accelerationStructureEXT u_TopLevelAS;
#endif

layout (set = 0, binding = 3, std430) readonly buffer VertexBuffer 
{
    Vertex data[];
} Vertices[1024];

layout (set = 0, binding = 4) readonly buffer IndexBuffer 
{
    uint data[];
} Indices[1024];

layout (set = 0, binding = 5) readonly buffer SubmeshInfoBuffer 
{
    uvec2 data[];
} SubmeshInfo[];

layout (set = 0, binding = 6) uniform sampler2D s_Textures[];
*/

// ------------------------------------------------------------------------
// FUNCTIONS --------------------------------------------------------------
// ------------------------------------------------------------------------

Vertex get_vertex(in InstanceData instance_data, in uint index)
{
    Vertex v;
    v.position = vec4(0/0);
    v.normal = vec3(0/0);
    v.tex_coord = vec2(0/0);
    v.valid_normal = false;
    v.valid_tex_coord = false;

    uint64_t vertices = instance_data.vertices;
    uint64_t stride = instance_data.vertex_stride;

    uint64_t position_offset = instance_data.position_offset;
    uint64_t normal_offset = instance_data.normal_offset;
    uint64_t tex_coord_offset = instance_data.tex_coord_offset;

    uint64_t position_data_type = instance_data.position_data_type;
    uint64_t normal_data_type = instance_data.normal_data_type;
    uint64_t tex_coord_data_type = instance_data.tex_coord_data_type;

    if (position_data_type == RAYSTERIZER_DATA_TYPE_VEC3 || position_data_type == RAYSTERIZER_DATA_TYPE_VEC4)
    {
        vec3 conversion_position = Vec3Ptr(vertices + uint((index * stride) + position_offset)).data[0u];
        v.position = vec4(vec3(conversion_position), 1.0);
    }
    else if (position_data_type == RAYSTERIZER_DATA_TYPE_IVEC3 || position_data_type == RAYSTERIZER_DATA_TYPE_IVEC4)
    {
        ivec3 conversion_position = IVec3Ptr(vertices + uint((index * stride) + position_offset)).data[0u];
        v.position = vec4(vec3(conversion_position), 1.0);
    }
    else if (position_data_type == RAYSTERIZER_DATA_TYPE_UVEC3 || position_data_type == RAYSTERIZER_DATA_TYPE_UVEC4)
    {
        uvec3 conversion_position = UVec3Ptr(vertices + uint((index * stride) + position_offset)).data[0u];
        v.position = vec4(vec3(conversion_position), 1.0);
    }

    if (normal_offset == -1)
    {
        v.valid_normal = false;
    }
    else if (normal_data_type == RAYSTERIZER_DATA_TYPE_VEC3 || normal_data_type == RAYSTERIZER_DATA_TYPE_VEC4)
    {
        vec3 conversion_normal = Vec3Ptr(vertices + uint((index * stride) + normal_offset)).data[0u];
        v.normal = vec3(conversion_normal);
    }
    else if (normal_data_type == RAYSTERIZER_DATA_TYPE_IVEC3 || normal_data_type == RAYSTERIZER_DATA_TYPE_IVEC4)
    {
        ivec3 conversion_normal = IVec3Ptr(vertices + uint((index * stride) + normal_offset)).data[0u];
        v.normal = vec3(conversion_normal);
    }
    else if (normal_data_type == RAYSTERIZER_DATA_TYPE_UVEC3 || normal_data_type == RAYSTERIZER_DATA_TYPE_UVEC4)
    {
        uvec3 conversion_normal = UVec3Ptr(vertices + uint((index * stride) + normal_offset)).data[0u];
        v.normal = vec3(conversion_normal);
    }

    if (tex_coord_offset == -1)
    {
        v.valid_tex_coord = false;
    }
    else if (tex_coord_data_type == RAYSTERIZER_DATA_TYPE_VEC2)
    {
        vec2 conversion_tex_coord = Vec2Ptr(vertices + uint((index * stride) + tex_coord_offset)).data[0u];
        v.tex_coord = vec2(conversion_tex_coord);
    }

    return v;
}

uint get_index(in InstanceData instance_data, in uint index)
{
    uint64_t indices = instance_data.indices;
    uint64_t stride = instance_data.index_stride;
    if (stride == 2)
    {
        uint16_t index = UInt16Ptr(indices + uint(index * stride)).data[0u];
        return uint(index);
    }
    else
    {
        uint32_t index = UInt32Ptr(indices + uint(index * stride)).data[0u];
        return uint(index);
    }
}

vec3 compute_normal_from_triangle(vec3 p1, vec3 p2, vec3 p3)
{
    vec3 normal;

    vec3 a = p2 - p1;
    vec3 b = p3 - p1;
    
    /*
    normal.x = a.y * b.z - a.z * b.y;
    normal.y = a.z * b.x - a.x * b.z;
    normal.z = a.x * b.y - a.y * b.x;
    */

    normal = normalize(cross(a, b));

    return normal;
}

// ------------------------------------------------------------------------

Triangle fetch_triangle(in InstanceData instance_data, in uint primitive_id)
{
    Triangle tri;

    //uvec3 idx = uvec3(Indices[nonuniformEXT(instance.mesh_idx)].data[3 * primitive_id], 
    //                  Indices[nonuniformEXT(instance.mesh_idx)].data[3 * primitive_id + 1],
    //                  Indices[nonuniformEXT(instance.mesh_idx)].data[3 * primitive_id + 2]);

    uvec3 idx = uvec3(get_index(instance_data, 3 * primitive_id + 0),
                    get_index(instance_data, 3 * primitive_id + 1),
                    get_index(instance_data, 3 * primitive_id + 2));

    tri.v0 = get_vertex(instance_data, idx.x);
    tri.v1 = get_vertex(instance_data, idx.y);
    tri.v2 = get_vertex(instance_data, idx.z);

    vec3 normal0 = tri.v0.normal;
    vec3 normal1 = tri.v1.normal;
    vec3 normal2 = tri.v2.normal;
    if (!tri.v0.valid_normal)
    {
        vec3 computed_normal = compute_normal_from_triangle(tri.v0.position.xyz, tri.v1.position.xyz, tri.v2.position.xyz);
        tri.v0.normal = computed_normal;
        tri.v1.normal = computed_normal;
        tri.v2.normal = computed_normal;
    }

    return tri;
}

// ------------------------------------------------------------------------

Vertex interpolated_vertex(in Triangle tri, in vec3 barycentrics)
{
    Vertex o;

    o.position = vec4(tri.v0.position.xyz * barycentrics.x + tri.v1.position.xyz * barycentrics.y + tri.v2.position.xyz * barycentrics.z, 1.0);
    o.tex_coord.xy = tri.v0.tex_coord.xy * barycentrics.x + tri.v1.tex_coord.xy * barycentrics.y + tri.v2.tex_coord.xy * barycentrics.z;
    o.normal.xyz = normalize(tri.v0.normal.xyz * barycentrics.x + tri.v1.normal.xyz * barycentrics.y + tri.v2.normal.xyz * barycentrics.z);
    o.tangent.xyz = normalize(tri.v0.tangent.xyz * barycentrics.x + tri.v1.tangent.xyz * barycentrics.y + tri.v2.tangent.xyz * barycentrics.z);
    o.bitangent.xyz = normalize(tri.v0.bitangent.xyz * barycentrics.x + tri.v1.bitangent.xyz * barycentrics.y + tri.v2.bitangent.xyz * barycentrics.z);

    return o;
}

// ------------------------------------------------------------------------

void transform_vertex(in InstanceData instance_data, inout Vertex v)
{
#ifdef RAY_TRACING_RCHIT
    mat4 model_mat = mat4(gl_ObjectToWorldEXT);
    mat3 normal_mat = mat3(model_mat);
#else
    mat4 model_mat = mat4(1.0);
    mat3 normal_mat = mat3(model_mat);
#endif

    v.position = model_mat * v.position; 
    v.normal.xyz = normalize(normal_mat * v.normal.xyz);
    v.tangent.xyz = normalize(normal_mat * v.tangent.xyz);
    v.bitangent.xyz = normalize(normal_mat * v.bitangent.xyz);
}

// ------------------------------------------------------------------------

vec3 get_normal_from_map(vec3 tangent, vec3 bitangent, vec3 normal, vec2 tex_coord, uint normal_map_idx)
{
    // Create TBN matrix.
    mat3 TBN = mat3(normalize(tangent), normalize(bitangent), normalize(normal));

    // Sample tangent space normal vector from normal map and remap it from [0, 1] to [-1, 1] range.
    //vec3 n = normalize(texture(s_Textures[nonuniformEXT(normal_map_idx)], tex_coord).rgb * 2.0 - 1.0);
    vec3 n;

    // Multiple vector by the TBN matrix to transform the normal from tangent space to world space.
    n = normalize(TBN * n);

    return n;
}

// ------------------------------------------------------------------------

vec4 fetch_albedo(in Material material, in vec2 texcoord)
{
    /*
    if (material.highlight)
    {
        return material.highlight_color;
    }
    */

#if defined(RAY_TRACING_RCHIT)
    return raysterizer_out_colors[gl_InstanceCustomIndexEXT].data[gl_PrimitiveID];
#endif

    return material.albedo;
    /*
    if (material.texture_indices0.x == -1)
        return material.albedo;
    else
        return texture(s_Textures[nonuniformEXT(material.texture_indices0.x)], texcoord);
    */
}

// ------------------------------------------------------------------------

vec3 fetch_normal(in Material material, in vec3 tangent, in vec3 bitangent, in vec3 normal, in vec2 texcoord)
{
    return normal;
    /*
    if (material.texture_indices0.y == -1)
        return normal;
    else
        return get_normal_from_map(tangent, bitangent, normal, texcoord, material.texture_indices0.y);
    */
}

// ------------------------------------------------------------------------

float fetch_roughness(in Material material, in vec2 texcoord)
{
#define MIN_ROUGHNESS 0.1f
    return max(material.roughness_metallic.r, MIN_ROUGHNESS);
    /*

    if (material.texture_indices0.z == -1)
        return max(material.roughness_metallic.r, MIN_ROUGHNESS);
    else
        return max(texture(s_Textures[nonuniformEXT(material.texture_indices0.z)], texcoord)[material.texture_indices1.z], MIN_ROUGHNESS);
    */
}

// ------------------------------------------------------------------------

float fetch_metallic(in Material material, in vec2 texcoord)
{
    return material.roughness_metallic.g;
    /*
    if (material.texture_indices0.w == -1)
        return material.roughness_metallic.g;
    else
        return texture(s_Textures[nonuniformEXT(material.texture_indices0.w)], texcoord)[material.texture_indices1.w];
    */
}

// ------------------------------------------------------------------------

vec3 fetch_emissive(in Material material, in vec2 texcoord)
{
    return material.emissive.rgb;
    /*
    if (material.texture_indices1.x == -1)
        return material.emissive.rgb;
    else
        return texture(s_Textures[nonuniformEXT(material.texture_indices1.x)], texcoord).rgb;
    */
}

// ------------------------------------------------------------------------