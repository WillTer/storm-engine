#pragma once

// Use default alignment for vectors and matrices as we need them to be aligned by 16 bytes
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

// Disable UB-stuff from glm
// This will also disable SIMD-operations though
#define GLM_FORCE_XYZW_ONLY

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
