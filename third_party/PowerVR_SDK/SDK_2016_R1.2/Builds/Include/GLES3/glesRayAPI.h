#if defined(__linux__) || defined(__QNXNTO__)
#undef GL_API_EXT
#define GL_API_EXT __attribute__((visibility("hidden")))
#elif defined (_WIN32)
#undef GL_API_EXT
#define GL_API_EXT __declspec(dllexport)
#endif
#if defined _MSC_VER
#pragma push_macro("GL_APICALL")
#endif
#undef GL_APICALL
#define GL_APICALL GL_API_EXT
#define GL_APIENTRYP GL_APIENTRY*
#include <GLES3/gl31.h>

//Shaders
#define GL_RAY_SHADER                               0x7000
#define GL_RAY_SHADER_BIT                           0x00000040
#define GL_FRAME_SHADER                             0x7002
#define GL_FRAME_SHADER_BIT                         0x00000080
#define GL_REFERENCED_BY_RAY_SHADER                 0x7004
#define GL_REFERENCED_BY_FRAME_SHADER               0x7005

#define GL_MAX_RAY_UNIFORM_BLOCKS                   0x7006
#define GL_MAX_RAY_SHADER_STORAGE_BLOCKS            0x7043
#define GL_MAX_RAY_TEXTURE_IMAGE_UNITS              0x7007
#define GL_MAX_RAY_IMAGE_UNIFORMS                   0x7008
#define GL_MAX_RAY_UNIFORM_COMPONENTS               0x7009
#define GL_MAX_RAY_ATOMIC_COUNTER_BUFFERS           0x700A
#define GL_MAX_RAY_ATOMIC_COUNTERS                  0x700B
#define GL_MAX_COMBINED_RAY_UNIFORM_COMPONENTS      0x700C

#define GL_MAX_FRAME_UNIFORM_BLOCKS                 0x700D
#define GL_MAX_FRAME_SHADER_STORAGE_BLOCKS          0x7044
#define GL_MAX_FRAME_TEXTURE_IMAGE_UNITS            0x700E
#define GL_MAX_FRAME_IMAGE_UNIFORMS                 0x700F
#define GL_MAX_FRAME_UNIFORM_COMPONENTS             0x7010
#define GL_MAX_FRAME_ATOMIC_COUNTER_BUFFERS         0x7011
#define GL_MAX_FRAME_ATOMIC_COUNTERS                0x7012
#define GL_MAX_COMBINED_FRAME_UNIFORM_COMPONENTS    0x7013
#define GL_MAX_RAY_TYPES                            0x7014

//Limit on number of EmitRay() calls per class/type for each shader invocation
#define GL_MAX_OUTPUT_RAYS_PER_RAY_TYPE             0x7015

//On cores without variable ray size, making the value used
//for ray_size HW register and this GL limit the same will be the
//easiest option.
//Adding to a scene group would require a complete scene
//rebuild (not just a group rebuild) if shaders with larger ray sizes
//are added to the group. Rebuilding scene & groups is left entirely to
//the application in this proposal.
// Max ray size in components.
#define GL_MAX_RAY_COMPONENTS                       0x7016 // Builtin state does not count against this limit.
// Max ray size in bytes
#define GL_MAX_RAY_TYPE_ALLOCATION                  0x7039 // Builtin state does not count against this limit.

//For khr_debug ObjectLabel/GetObjectLabel <identifier> parameter
#define GL_SCENE_ARRAY                              0x7017
#define GL_COMPONENT_GROUP                          0x7018
#define GL_COMPONENT                                0x7019

//Initial value for RayBounceLimit is either 1 or the max supported by HW.
GL_APICALL void GL_APIENTRY glRayBounceLimit(GLuint limit);
#define GL_RAY_BOUNCE_LIMIT                          0x701A //GetInteger
#define GL_MAX_RAY_BOUNCE_LIMIT                      0x701B //GetInteger. Query the max limit HW supports. glRayBounceLimit clamps to this limit.

// Can be passed into glGetIntegeri_v to query the current bound scene array.
#define GL_SCENE_ARRAY_BINDING                      0x701C
// Scene arrays represent a set of "active" scenes that can be rendered with. Similar to vertex array objects.
// Scene arrays are not used to parent objects and are not used for validation purposes when creating other objects.
GL_APICALL GLuint GL_APIENTRY glCreateSceneArray();
GL_APICALL void GL_APIENTRY glDeleteSceneArray(GLuint sceneArray);
GL_APICALL GLboolean GL_APIENTRY glIsSceneArray(GLuint sceneArray);

// Set the size of each ray type. Applies to subsequent glDispatchRays calls that use this command.
GL_APICALL void GL_APIENTRY glSceneArrayRayTypeSize(GLuint sceneArray, GLuint index, GLsizei size);

// Binding a scene array 'installs' its scenes under the Root BID.
// DispatchRays commands use this state.
GL_APICALL void GL_APIENTRY glBindSceneArray(GLuint sceneArray);

#define GL_NO_WAIT_BIT                              0x701D // Don't put a dependency on outstanding component group builds.
// Triggers the execution of a frame shader. Typically this will wait on outstanding builds, but it can be told to not wait.
GL_APICALL void GL_APIENTRY glDispatchRays(GLuint id, GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield flags);

// Query scene array state
#define GL_SCENE_ARRAY_COMPONENT_GROUP_BINDING_IMG   0x7037
#define GL_SCENE_ARRAY_RAY_TYPE_SIZE_IMG             0x7038
GL_APICALL void GL_APIENTRY glGetSceneArrayInfoiIMG(GLuint sceneArray, GLenum pname, GLuint index, GLint* params);

// Create component groups
GL_APICALL void GL_APIENTRY glCreateComponentGroups(GLsizei n, GLuint *componentGroups);
// Delete component groups
GL_APICALL void GL_APIENTRY glDeleteComponentGroups(GLsizei n, GLuint *componentGroups);
// Set extents for a component group
GL_APICALL void GL_APIENTRY glComponentGroupExtents(GLuint componentGroup, const GLfloat* min, const GLfloat* max);

GL_APICALL GLboolean GL_APIENTRY glIsComponentGroup(GLuint componentGroup);

// Gets a GPU handle for the component group.
GL_APICALL GLuint64 GL_APIENTRY glGetComponentGroupHandle(GLuint componentGroup);

/*
Components can be built into component groups.
This is the point at which the states of components are finalised/latched.
Building with zero components clears a previous build.
*/
#define GL_MAX_COMPONENT_GROUP_BUILD_SIZE           0x701E //GetInteger. Maximum number of components per-build.
GL_APICALL void GL_APIENTRY glBuildComponentGroup(GLuint id, GLuint componentGroup, GLuint n, const void* build);

/*
Merge multiple component groups into a single component group.
Previously merged component groups can be remerged, overwriting previous merges, and unparenting child component groups.
    Previously merged children have an internal ref count decreased by one.
    Newly merged children have an internal ref count increased by one.
The top merged component group can only support a limited number of component groups.
    GL_MAX_COMPONENT_GROUP_MERGE_SOURCES
    Future HW will allow more component groups.
    The parent has a source count which is increased by the sum of all source’s source counts. NB: This is always set to 1 as the result of a non-empty Build command.
Building with zero components clears a previous merge.
Quality sets the quality of merge functions with this component group as the destination.
    Only a ToT merge quality is exposed in first version (e.g. "trivial" or "fastest").
    Note: driver currently requires this to be set to GL_DONT_CARE.
*/
#define GL_MAX_COMPONENT_GROUP_MERGE_SOURCES        0x701F //GetInteger. Maximum number of sources per merge. Min value is <TBD - 4?>.
GL_APICALL void GL_APIENTRY glMergeComponentGroups(GLuint componentGroup, GLuint n, const void* merge, GLenum quality);

// Bind component groups to the scene array as a scene. Up to GL_MAX_SCENE_ARRAY_COMPONENT_GROUP_BINDINGS binding points.
// The same component group can be attached to multiple different arrays, which is potentially useful when streaming in world data.
#define GL_MAX_SCENE_ARRAY_COMPONENT_GROUP_BINDINGS                0x7021 // GetInteger. Maximum number of binding points in the scene array.
GL_APICALL void GL_APIENTRY glBindSceneArrayComponentGroup(GLuint sceneArray, GLuint index, GLuint componentGroup);

// Query the component group extents
GL_APICALL void GL_APIENTRY glGetComponentGroupExtentsfv(GLuint componentGroup, GLfloat* min, GLfloat* max);

// Query the component group extent precision.
GL_APICALL void GL_APIENTRY glGetComponentGroupExtentPrecision(GLint* range, GLint* precision);

// Scene components
GL_APICALL void GL_APIENTRY glCreateComponents(GLsizei n, GLuint *components);
GL_APICALL void GL_APIENTRY glDeleteComponents(GLsizei n, const GLuint* components);
GL_APICALL GLboolean GL_APIENTRY glIsComponent(GLuint component);

GL_APICALL void GL_APIENTRY glComponentOccluder(GLuint component, GLboolean occluder);
GL_APICALL void GL_APIENTRY glComponentRayTypeVisibility(GLuint component, GLuint index, GLboolean visibility);
//Front face vertex winding order.
GL_APICALL void GL_APIENTRY glComponentFrontFace(GLuint component, GLenum dir);
//GL_FRONT then only front face intersections will cause shader execution (back face when gl_OutRay.flipFacing is true)
//GL_BACK then only back face intersections will cause shader execution (front face when gl_OutRay.flipFacing is true)
//GL_FRONT_AND_BACK double-sided intersections
GL_APICALL void GL_APIENTRY glComponentVisibleFace(GLuint component, GLenum mode);
GL_APICALL void GL_APIENTRY glComponentVertexArray(GLuint component, GLuint vertexArray);
// These next two functions override each other.
GL_APICALL void GL_APIENTRY glComponentGeometry(GLuint component, GLenum mode, GLint first, GLsizei count);
GL_APICALL void GL_APIENTRY glComponentIndexedGeometry(GLuint component,
                                                       GLenum mode,
                                                       GLsizei count,
                                                       GLenum type,
                                                       const GLintptr indices,
                                                       GLint baseVertex);
GL_APICALL void GL_APIENTRY glComponentBufferRange(GLuint component, GLenum target, GLuint index,
                                                   GLuint buffer, GLintptr offset, GLsizeiptr size);
// Creates a component/program handle. Makes both the program and handle immutable.
GL_APICALL GLuint64 GL_APIENTRY glGetComponentProgramHandle(GLuint component, GLuint program);
// Creates a component/rayprogram/vertexprogram handle. All (used) programs are made immutable. 
// One ray program and one vertex shader must be supplied. The last one in the array wins, so specifying ray program twice will mean the first program is ignored.
GL_APICALL GLuint64 GL_APIENTRY glGetComponentProgramStagesHandle(GLuint component, GLsizei n, GLenum* stages, GLuint* programs); 

#define GL_COMPONENT_RAY_TYPE_VISIBILITY                      0x7022
#define GL_COMPONENT_RAY_OCCLUDER_IMG                         0x7023
#define GL_COMPONENT_VISIBLE_FACE                             0x7024
#define GL_COMPONENT_GEOMETRY_INDEXED_IMG						0x703D
#define GL_COMPONENT_GEOMETRY_MODE_IMG							0x703E
#define GL_COMPONENT_GEOMETRY_COUNT_IMG							0x703F
#define GL_COMPONENT_GEOMETRY_BASE_VERTEX_IMG					0x7040
#define GL_COMPONENT_GEOMETRY_INDEX_TYPE_IMG					0x7041
#define GL_COMPONENT_GEOMETRY_INDEX_OFFSET_IMG					0x7042
//Already defined:
//GL_FRONT_FACE
//GL_VERTEX_ARRAY_BINDING
//GL_SHADER_STORAGE_BUFFER_BINDING
//GL_SHADER_STORAGE_BUFFER_START
//GL_SHADER_STORAGE_BUFFER_SIZE
//GL_UNIFORM_BUFFER_BINDING
//GL_UNIFORM_BUFFER_START
//GL_UNIFORM_BUFFER_SIZE
GL_APICALL void GL_APIENTRY glGetComponentInfoiv(GLuint component, GLenum pname, GLint* params);
GL_APICALL void GL_APIENTRY glGetComponentInfoi_v(GLuint component, GLenum pname, GLuint index, GLint* params);

#define GL_SYNC_RTU_COMMANDS_COMPLETE               0x7025 //Use with glFenceSync to determine if all prior DispatchRays operations have finished.
#define GL_SYNC_SH_COMMANDS_COMPLETE                0x7026 //Use to wait on all scene builds prior to the point glFenceSync is called.

#define GL_MAX_RAY_PROGRAM_HANDLES                  0x7027 // Use with glGet* functions to determine max number of bindless ray programs that can exist.
#define GL_MAX_DISPATCH_RAYS_ID_IMG                 0x7029
#define GL_MAX_BUILD_ID_IMG                         0x7001

// Following functions only work if program is separable, only contains a ray shader and is bindless:

// Standard bindless stuff. Get the handle.
GL_APICALL GLuint64 GL_APIENTRY glGetRayProgramHandleIMG(GLuint rayProgram);

// Set buffer resources for a bindless ray program.
GL_APICALL void GL_APIENTRY glRayProgramBufferRangeIMG(GLuint program, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

// Query indexed parameters of the program - currently only values set by glRayProgramBufferRangeIMG
GL_APICALL void GL_APIENTRY glGetProgrami_vIMG(GLuint program, GLenum pname, GLuint index, GLint *params);
//#define GL_SHADER_STORAGE_BUFFER_BINDING  0x90D3
//#define GL_SHADER_STORAGE_BUFFER_START    0x90D4
//#define GL_SHADER_STORAGE_BUFFER_SIZE     0x90D5
//#define GL_UNIFORM_BUFFER_BINDING         0x8A28
//#define GL_UNIFORM_BUFFER_START           0x8A29
//#define GL_UNIFORM_BUFFER_SIZE            0x8A2A

//The values are per-program instead of per-component mostly because that means a single functions can
//set the values for both frame and ray shaders.
//Having a program with both a frame and ray shader in it must be an error.
//ray shaders: change takes affect when the component is next built.
//ray shaders: change affects next DispatchRays
//How about glProgramParameteri_v instead?
//GL_APICALL void GL_APIENTRY glProgramParameteri_v(GLuint program, GLenum pname, GLuint index, GLint value)
GL_API_EXT void GL_APIENTRY glProgramMaxRayEmits(GLuint program,
	/*GLenum stage,*/
    GLuint index,
    GLuint value);

#define GL_ACCUMULATE_ONLY 0x7034

// Returned from glGetProgramResource when querying uniform types.
#define GL_RAY_PROGRAM_HANDLE_IMG                0x7035

// Used by glGetProgramInterface and glGetProgramResource
#define GL_RAY_TYPE_IMG                          0x703A
#define GL_RAY_TYPE_VARIABLE_IMG                 0x703B
#define GL_RAY_TYPE_SIZE_IMG                     0x703C