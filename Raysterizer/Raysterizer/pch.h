#pragma once

#define _ENABLE_EXTENDED_ALIGNED_STORAGE

#include "include/pch.h"

using namespace RaysterizerEngine;

#include "constants.h"

#include "hooks/hook_manager.h"
#include "hooks/opengl_hooks.h"
#include "hooks/wgl_opengl_hooks.h"

#include "analysis/glslang_analyzer.h"
#include "analysis/spirv_analyzer.h"
#include "analysis/spirv_virtual_machine.h"

#include "middleware/gl_utils.h"
#include "middleware/shader_converter.h"

#include "frontend/opengl/glad.h"
#include "frontend/opengl/opengl_utils.h"

#include "frontend/opengl/opengl_buffer_manager.h"
#include "frontend/opengl/opengl_texture_manager.h"
#include "frontend/opengl/opengl_shader_manager.h"
#include "frontend/opengl/opengl_frame_buffer_manager.h"

#include "frontend/opengl/opengl_state.h"

#include "pass/common_resources.h"
#include "pass/gbuffer.h"
#include "pass/blue_noise.h"
#include "pass/ray_traced_reflections.h"
#include "pass/ray_traced_shadows.h"
#include "pass/ray_traced_ao.h"
#include "pass/ddgi.h"
#include "pass/deferred_shading.h"
#include "pass/temporal_aa.h"
#include "pass/tone_map.h"

#include "pass/brdf_integrate_lut.h"
#include "pass/cubemap_prefilter.h"
#include "pass/cubemap_sh_projection.h"
#include "pass/equirectangular_to_cubemap.h"
#include "pass/hosek_wilkie_sky_model.h"
#include "pass/sky_environment.h"

#include "middleware/draw_calls.h"
#include "middleware/pipeline_manager.h"

#include "frontend/opengl/opengl_context.h"

