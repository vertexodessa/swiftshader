//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// utilities.cpp: Conversion functions and other utility routines.

#include "utilities.h"

#include "mathutil.h"
#include "Context.h"
#include "common/debug.h"

#include <limits>
#include <stdio.h>
#include <windows.h>

namespace gl
{
	int UniformComponentCount(GLenum type)
	{
		switch(type)
		{
		case GL_BOOL:
		case GL_FLOAT:
		case GL_INT:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
			return 1;
		case GL_BOOL_VEC2:
		case GL_FLOAT_VEC2:
		case GL_INT_VEC2:
			return 2;
		case GL_INT_VEC3:
		case GL_FLOAT_VEC3:
		case GL_BOOL_VEC3:
			return 3;
		case GL_BOOL_VEC4:
		case GL_FLOAT_VEC4:
		case GL_INT_VEC4:
		case GL_FLOAT_MAT2:
			return 4;
		case GL_FLOAT_MAT3:
			return 9;
		case GL_FLOAT_MAT4:
			return 16;
		default:
			UNREACHABLE();
		}

		return 0;
	}

	GLenum UniformComponentType(GLenum type)
	{
		switch(type)
		{
		case GL_BOOL:
		case GL_BOOL_VEC2:
		case GL_BOOL_VEC3:
		case GL_BOOL_VEC4:
			return GL_BOOL;
		case GL_FLOAT:
		case GL_FLOAT_VEC2:
		case GL_FLOAT_VEC3:
		case GL_FLOAT_VEC4:
		case GL_FLOAT_MAT2:
		case GL_FLOAT_MAT3:
		case GL_FLOAT_MAT4:
			return GL_FLOAT;
		case GL_INT:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
		case GL_INT_VEC2:
		case GL_INT_VEC3:
		case GL_INT_VEC4:
			return GL_INT;
		default:
			UNREACHABLE();
		}

		return GL_NONE;
	}

	size_t UniformTypeSize(GLenum type)
	{
		switch(type)
		{
		case GL_BOOL:  return sizeof(GLboolean);
		case GL_FLOAT: return sizeof(GLfloat);
		case GL_INT:   return sizeof(GLint);
		}

		return UniformTypeSize(UniformComponentType(type)) * UniformComponentCount(type);
	}

	int VariableRowCount(GLenum type)
	{
		switch(type)
		{
		case GL_NONE:
			return 0;
		case GL_BOOL:
		case GL_FLOAT:
		case GL_INT:
		case GL_BOOL_VEC2:
		case GL_FLOAT_VEC2:
		case GL_INT_VEC2:
		case GL_INT_VEC3:
		case GL_FLOAT_VEC3:
		case GL_BOOL_VEC3:
		case GL_BOOL_VEC4:
		case GL_FLOAT_VEC4:
		case GL_INT_VEC4:
			return 1;
		case GL_FLOAT_MAT2:
			return 2;
		case GL_FLOAT_MAT3:
			return 3;
		case GL_FLOAT_MAT4:
			return 4;
		default:
			UNREACHABLE();
		}

		return 0;
	}

	int VariableColumnCount(GLenum type)
	{
		switch(type)
		{
		case GL_NONE:
			return 0;
		case GL_BOOL:
		case GL_FLOAT:
		case GL_INT:
			return 1;
		case GL_BOOL_VEC2:
		case GL_FLOAT_VEC2:
		case GL_INT_VEC2:
		case GL_FLOAT_MAT2:
			return 2;
		case GL_INT_VEC3:
		case GL_FLOAT_VEC3:
		case GL_BOOL_VEC3:
		case GL_FLOAT_MAT3:
			return 3;
		case GL_BOOL_VEC4:
		case GL_FLOAT_VEC4:
		case GL_INT_VEC4:
		case GL_FLOAT_MAT4:
			return 4;
		default:
			UNREACHABLE();
		}

		return 0;
	}

	int AllocateFirstFreeBits(unsigned int *bits, unsigned int allocationSize, unsigned int bitsSize)
	{
		ASSERT(allocationSize <= bitsSize);

		unsigned int mask = std::numeric_limits<unsigned int>::max() >> (std::numeric_limits<unsigned int>::digits - allocationSize);

		for(unsigned int i = 0; i < bitsSize - allocationSize + 1; i++)
		{
			if((*bits & mask) == 0)
			{
				*bits |= mask;
				return i;
			}

			mask <<= 1;
		}

		return -1;
	}

	GLsizei ComputePitch(GLsizei width, GLenum format, GLenum type, GLint alignment)
	{
		ASSERT(alignment > 0 && isPow2(alignment));

		GLsizei rawPitch = ComputePixelSize(format, type) * width;
		return (rawPitch + alignment - 1) & ~(alignment - 1);
	}

	GLsizei ComputeCompressedPitch(GLsizei width, GLenum format)
	{
		return ComputeCompressedSize(width, 1, format);
	}

	GLsizei ComputeCompressedSize(GLsizei width, GLsizei height, GLenum format)
	{
		switch(format)
		{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			break;
		default:
			return 0;
		}

		return 8 * (GLsizei)ceil((float)width / 4.0f) * (GLsizei)ceil((float)height / 4.0f);
	}

	bool IsCompressed(GLenum format)
	{
		if(format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
		   format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// Returns the size, in bytes, of a single texel in an Image
	int ComputePixelSize(GLenum format, GLenum type)
	{
		switch(type)
		{
		case GL_UNSIGNED_BYTE:
			switch(format)
			{
			case GL_ALPHA:           return sizeof(unsigned char);
			case GL_LUMINANCE:       return sizeof(unsigned char);
			case GL_LUMINANCE_ALPHA: return sizeof(unsigned char) * 2;
			case GL_RGB:             return sizeof(unsigned char) * 3;
			case GL_RGBA:            return sizeof(unsigned char) * 4;
			case GL_BGRA_EXT:        return sizeof(unsigned char) * 4;
			default: UNREACHABLE();
			}
			break;
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_5_6_5:
			return sizeof(unsigned short);
		case GL_FLOAT:
			switch(format)
			{
			case GL_ALPHA:           return sizeof(float);
			case GL_LUMINANCE:       return sizeof(float);
			case GL_LUMINANCE_ALPHA: return sizeof(float) * 2;
			case GL_RGB:             return sizeof(float) * 3;
			case GL_RGBA:            return sizeof(float) * 4;
			default: UNREACHABLE();
			}
			break;
		case GL_HALF_FLOAT_OES:
			switch(format)
			{
			case GL_ALPHA:           return sizeof(unsigned short);
			case GL_LUMINANCE:       return sizeof(unsigned short);
			case GL_LUMINANCE_ALPHA: return sizeof(unsigned short) * 2;
			case GL_RGB:             return sizeof(unsigned short) * 3;
			case GL_RGBA:            return sizeof(unsigned short) * 4;
			default: UNREACHABLE();
			}
			break;
		default: UNREACHABLE();
		}

		return 0;
	}

	bool IsCubemapTextureTarget(GLenum target)
	{
		return (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	}

	CubeFace ConvertCubeFace(GLenum cubeFace)
	{
		CubeFace face = CUBEMAP_FACE_POSITIVE_X;

		// Map a cube map texture target to the corresponding Device surface index. Note that the
		// Y faces are swapped because the Y coordinate to the texture lookup intrinsic functions
		// are negated in the pixel shader.
		switch(cubeFace)
		{
		case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
			face = CUBEMAP_FACE_POSITIVE_X;
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
			face = CUBEMAP_FACE_NEGATIVE_X;
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
			face = CUBEMAP_FACE_NEGATIVE_Y;
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
			face = CUBEMAP_FACE_POSITIVE_Y;
			break;
		case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
			face = CUBEMAP_FACE_POSITIVE_Z;
			break;
		case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
			face = CUBEMAP_FACE_NEGATIVE_Z;
			break;
		default: UNREACHABLE();
		}

		return face;
	}

	bool IsTextureTarget(GLenum target)
	{
		return target == GL_TEXTURE_2D || IsCubemapTextureTarget(target);
	}

	// Verify that format/type are one of the combinations from table 3.4.
	bool CheckTextureFormatType(GLenum format, GLenum type)
	{
		switch(type)
		{
		case GL_UNSIGNED_BYTE:
			switch(format)
			{
			case GL_RGBA:
			case GL_BGRA_EXT:
			case GL_RGB:
			case GL_ALPHA:
			case GL_LUMINANCE:
			case GL_LUMINANCE_ALPHA:
				return true;
			default:
				return false;
			}
		case GL_FLOAT:
		case GL_HALF_FLOAT_OES:
			switch(format)
			{
			case GL_RGBA:
			case GL_RGB:
			case GL_ALPHA:
			case GL_LUMINANCE:
			case GL_LUMINANCE_ALPHA:
				return true;
			default:
				return false;
			}
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
			return (format == GL_RGBA);
		case GL_UNSIGNED_SHORT_5_6_5:
			return (format == GL_RGB);
		default:
			return false;
		}
	}

	bool IsColorRenderable(GLenum internalformat)
	{
		switch(internalformat)
		{
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGB565:
		case GL_RGB8_OES:
		case GL_RGBA8_OES:
			return true;
		case GL_DEPTH_COMPONENT16:
		case GL_STENCIL_INDEX8:
		case GL_DEPTH24_STENCIL8_OES:
			return false;
		default:
			UNIMPLEMENTED();
		}

		return false;
	}

	bool IsDepthRenderable(GLenum internalformat)
	{
		switch(internalformat)
		{
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH24_STENCIL8_OES:
			return true;
		case GL_STENCIL_INDEX8:
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGB565:
		case GL_RGB8_OES:
		case GL_RGBA8_OES:
			return false;
		default:
			UNIMPLEMENTED();
		}

		return false;
	}

	bool IsStencilRenderable(GLenum internalformat)
	{
		switch(internalformat)
		{
		case GL_STENCIL_INDEX8:
		case GL_DEPTH24_STENCIL8_OES:
			return true;
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGB565:
		case GL_RGB8_OES:
		case GL_RGBA8_OES:
		case GL_DEPTH_COMPONENT16:
			return false;
		default:
			UNIMPLEMENTED();
		}

		return false;
	}
}

namespace es2sw
{
	sw::Context::DepthCompareMode ConvertDepthComparison(GLenum comparison)
	{
		switch(comparison)
		{
		case GL_NEVER:    return sw::Context::DEPTH_NEVER;
		case GL_ALWAYS:   return sw::Context::DEPTH_ALWAYS;
		case GL_LESS:     return sw::Context::DEPTH_LESS;
		case GL_LEQUAL:   return sw::Context::DEPTH_LESSEQUAL;
		case GL_EQUAL:    return sw::Context::DEPTH_EQUAL;
		case GL_GREATER:  return sw::Context::DEPTH_GREATER;
		case GL_GEQUAL:   return sw::Context::DEPTH_GREATEREQUAL;
		case GL_NOTEQUAL: return sw::Context::DEPTH_NOTEQUAL;
		default: UNREACHABLE();
		}

		return sw::Context::DEPTH_ALWAYS;
	}

	sw::Context::StencilCompareMode ConvertStencilComparison(GLenum comparison)
	{
		switch(comparison)
		{
		case GL_NEVER:    return sw::Context::STENCIL_NEVER;
		case GL_ALWAYS:   return sw::Context::STENCIL_ALWAYS;
		case GL_LESS:     return sw::Context::STENCIL_LESS;
		case GL_LEQUAL:   return sw::Context::STENCIL_LESSEQUAL;
		case GL_EQUAL:    return sw::Context::STENCIL_EQUAL;
		case GL_GREATER:  return sw::Context::STENCIL_GREATER;
		case GL_GEQUAL:   return sw::Context::STENCIL_GREATEREQUAL;
		case GL_NOTEQUAL: return sw::Context::STENCIL_NOTEQUAL;
		default: UNREACHABLE();
		}

		return sw::Context::STENCIL_ALWAYS;
	}

	sw::Color<float> ConvertColor(gl::Color color)
	{
		return sw::Color<float>(color.red, color.green, color.blue, color.alpha);
	}

	sw::Context::BlendFactor ConvertBlendFunc(GLenum blend)
	{
		switch(blend)
		{
		case GL_ZERO:                     return sw::Context::BLEND_ZERO;
		case GL_ONE:                      return sw::Context::BLEND_ONE;
		case GL_SRC_COLOR:                return sw::Context::BLEND_SOURCE;
		case GL_ONE_MINUS_SRC_COLOR:      return sw::Context::BLEND_INVSOURCE;
		case GL_DST_COLOR:                return sw::Context::BLEND_DEST;
		case GL_ONE_MINUS_DST_COLOR:      return sw::Context::BLEND_INVDEST;
		case GL_SRC_ALPHA:                return sw::Context::BLEND_SOURCEALPHA;
		case GL_ONE_MINUS_SRC_ALPHA:      return sw::Context::BLEND_INVSOURCEALPHA;
		case GL_DST_ALPHA:                return sw::Context::BLEND_DESTALPHA;
		case GL_ONE_MINUS_DST_ALPHA:      return sw::Context::BLEND_INVDESTALPHA;
		case GL_CONSTANT_COLOR:           return sw::Context::BLEND_CONSTANT;
		case GL_ONE_MINUS_CONSTANT_COLOR: return sw::Context::BLEND_INVCONSTANT;
		case GL_CONSTANT_ALPHA:           return sw::Context::BLEND_CONSTANTALPHA;
		case GL_ONE_MINUS_CONSTANT_ALPHA: return sw::Context::BLEND_INVCONSTANTALPHA;
		case GL_SRC_ALPHA_SATURATE:       return sw::Context::BLEND_SRCALPHASAT;
		default: UNREACHABLE();
		}

		return sw::Context::BLEND_ZERO;
	}

	sw::Context::BlendOperation ConvertBlendOp(GLenum blendOp)
	{
		switch(blendOp)
		{
		case GL_FUNC_ADD:              return sw::Context::BLENDOP_ADD;
		case GL_FUNC_SUBTRACT:         return sw::Context::BLENDOP_SUB;
		case GL_FUNC_REVERSE_SUBTRACT: return sw::Context::BLENDOP_INVSUB;
		default: UNREACHABLE();
		}

		return sw::Context::BLENDOP_ADD;
	}

	sw::Context::StencilOperation ConvertStencilOp(GLenum stencilOp)
	{
		switch(stencilOp)
		{
		case GL_ZERO:      return sw::Context::OPERATION_ZERO;
		case GL_KEEP:      return sw::Context::OPERATION_KEEP;
		case GL_REPLACE:   return sw::Context::OPERATION_REPLACE;
		case GL_INCR:      return sw::Context::OPERATION_INCRSAT;
		case GL_DECR:      return sw::Context::OPERATION_DECRSAT;
		case GL_INVERT:    return sw::Context::OPERATION_INVERT;
		case GL_INCR_WRAP: return sw::Context::OPERATION_INCR;
		case GL_DECR_WRAP: return sw::Context::OPERATION_DECR;
		default: UNREACHABLE();
		}

		return sw::Context::OPERATION_KEEP;
	}

	sw::AddressingMode ConvertTextureWrap(GLenum wrap)
	{
		switch(wrap)
		{
		case GL_REPEAT:            return sw::ADDRESSING_WRAP;
		case GL_CLAMP_TO_EDGE:     return sw::ADDRESSING_CLAMP;
		case GL_MIRRORED_REPEAT:   return sw::ADDRESSING_MIRROR;
		default: UNREACHABLE();
		}

		return sw::ADDRESSING_WRAP;
	}

	sw::Context::CullMode ConvertCullMode(GLenum cullFace, GLenum frontFace)
	{
		switch(cullFace)
		{
		case GL_FRONT:
			return (frontFace == GL_CCW ? sw::Context::CULL_CLOCKWISE : sw::Context::CULL_COUNTERCLOCKWISE);
		case GL_BACK:
			return (frontFace == GL_CCW ? sw::Context::CULL_COUNTERCLOCKWISE : sw::Context::CULL_CLOCKWISE);
		case GL_FRONT_AND_BACK:
			return sw::Context::CULL_NONE;   // culling will be handled during draw
		default: UNREACHABLE();
		}

		return sw::Context::CULL_COUNTERCLOCKWISE;
	}

	unsigned int ConvertColorMask(bool red, bool green, bool blue, bool alpha)
	{
		return (red   ? 0x00000001 : 0) |
			   (green ? 0x00000002 : 0) |
			   (blue  ? 0x00000004 : 0) |
			   (alpha ? 0x00000008 : 0);
	}

	sw::FilterType ConvertMagFilter(GLenum magFilter)
	{
		switch(magFilter)
		{
		case GL_NEAREST: return sw::FILTER_POINT;
		case GL_LINEAR:  return sw::FILTER_LINEAR;
		default: UNREACHABLE();
		}

		return sw::FILTER_POINT;
	}

	void ConvertMinFilter(GLenum texFilter, sw::FilterType *minFilter, sw::MipmapType *mipFilter)
	{
		switch(texFilter)
		{
		case GL_NEAREST:
			*minFilter = sw::FILTER_POINT;
			*mipFilter = sw::MIPMAP_NONE;
			break;
		case GL_LINEAR:
			*minFilter = sw::FILTER_LINEAR;
			*mipFilter = sw::MIPMAP_NONE;
			break;
		case GL_NEAREST_MIPMAP_NEAREST:
			*minFilter = sw::FILTER_POINT;
			*mipFilter = sw::MIPMAP_POINT;
			break;
		case GL_LINEAR_MIPMAP_NEAREST:
			*minFilter = sw::FILTER_LINEAR;
			*mipFilter = sw::MIPMAP_POINT;
			break;
		case GL_NEAREST_MIPMAP_LINEAR:
			*minFilter = sw::FILTER_POINT;
			*mipFilter = sw::MIPMAP_LINEAR;
			break;
		case GL_LINEAR_MIPMAP_LINEAR:
			*minFilter = sw::FILTER_LINEAR;
			*mipFilter = sw::MIPMAP_LINEAR;
			break;
		default:
			*minFilter = sw::FILTER_POINT;
			*mipFilter = sw::MIPMAP_NONE;
			UNREACHABLE();
		}
	}

	bool ConvertPrimitiveType(GLenum primitiveType, GLsizei elementCount,  gl::PrimitiveType &swPrimitiveType, int &primitiveCount)
	{
		switch(primitiveType)
		{
		case GL_POINTS:
			swPrimitiveType = gl::DRAW_POINTLIST;
			primitiveCount = elementCount;
			break;
		case GL_LINES:
			swPrimitiveType = gl::DRAW_LINELIST;
			primitiveCount = elementCount / 2;
			break;
		case GL_LINE_LOOP:
			swPrimitiveType = gl::DRAW_LINELOOP;
			primitiveCount = elementCount;
			break;
		case GL_LINE_STRIP:
			swPrimitiveType = gl::DRAW_LINESTRIP;
			primitiveCount = elementCount - 1;
			break;
		case GL_TRIANGLES:
			swPrimitiveType = gl::DRAW_TRIANGLELIST;
			primitiveCount = elementCount / 3;
			break;
		case GL_TRIANGLE_STRIP:
			swPrimitiveType = gl::DRAW_TRIANGLESTRIP;
			primitiveCount = elementCount - 2;
			break;
		case GL_TRIANGLE_FAN:
			swPrimitiveType = gl::DRAW_TRIANGLEFAN;
			primitiveCount = elementCount - 2;
			break;
		default:
			return false;
		}

		return true;
	}

	sw::Format ConvertRenderbufferFormat(GLenum format)
	{
		switch(format)
		{
		case GL_RGBA4:
		case GL_RGB5_A1:
		case GL_RGBA8_OES:            return sw::FORMAT_A8R8G8B8;
		case GL_RGB565:               return sw::FORMAT_R5G6B5;
		case GL_RGB8_OES:             return sw::FORMAT_X8R8G8B8;
		case GL_DEPTH_COMPONENT16:
		case GL_STENCIL_INDEX8:       
		case GL_DEPTH24_STENCIL8_OES: return sw::FORMAT_D24S8;
		default: UNREACHABLE();       return sw::FORMAT_A8R8G8B8;
		}
	}
}

namespace sw2es
{
	unsigned int GetStencilSize(sw::Format stencilFormat)
	{
		switch(stencilFormat)
		{
		case sw::FORMAT_D24FS8:
		case sw::FORMAT_D24S8:
			return 8;
	//	case sw::FORMAT_D24X4S4:
	//		return 4;
	//	case sw::FORMAT_D15S1:
	//		return 1;
	//	case sw::FORMAT_D16_LOCKABLE:
		case sw::FORMAT_D32:
		case sw::FORMAT_D24X8:
		case sw::FORMAT_D32F_LOCKABLE:
		case sw::FORMAT_D16:
			return 0;
	//	case sw::FORMAT_D32_LOCKABLE:  return 0;   // DirectX 9Ex only
	//	case sw::FORMAT_S8_LOCKABLE:   return 8;   // DirectX 9Ex only
		default:
			return 0;
		}
	}

	unsigned int GetAlphaSize(sw::Format colorFormat)
	{
		switch(colorFormat)
		{
		case sw::FORMAT_A16B16G16R16F:
			return 16;
		case sw::FORMAT_A32B32G32R32F:
			return 32;
		case sw::FORMAT_A2R10G10B10:
			return 2;
		case sw::FORMAT_A8R8G8B8:
			return 8;
		case sw::FORMAT_A1R5G5B5:
			return 1;
		case sw::FORMAT_X8R8G8B8:
		case sw::FORMAT_R5G6B5:
			return 0;
		default:
			return 0;
		}
	}

	unsigned int GetRedSize(sw::Format colorFormat)
	{
		switch(colorFormat)
		{
		case sw::FORMAT_A16B16G16R16F:
			return 16;
		case sw::FORMAT_A32B32G32R32F:
			return 32;
		case sw::FORMAT_A2R10G10B10:
			return 10;
		case sw::FORMAT_A8R8G8B8:
		case sw::FORMAT_X8R8G8B8:
			return 8;
		case sw::FORMAT_A1R5G5B5:
		case sw::FORMAT_R5G6B5:
			return 5;
		default:
			return 0;
		}
	}

	unsigned int GetGreenSize(sw::Format colorFormat)
	{
		switch(colorFormat)
		{
		case sw::FORMAT_A16B16G16R16F:
			return 16;
		case sw::FORMAT_A32B32G32R32F:
			return 32;
		case sw::FORMAT_A2R10G10B10:
			return 10;
		case sw::FORMAT_A8R8G8B8:
		case sw::FORMAT_X8R8G8B8:
			return 8;
		case sw::FORMAT_A1R5G5B5:
			return 5;
		case sw::FORMAT_R5G6B5:
			return 6;
		default:
			return 0;
		}
	}

	unsigned int GetBlueSize(sw::Format colorFormat)
	{
		switch(colorFormat)
		{
		case sw::FORMAT_A16B16G16R16F:
			return 16;
		case sw::FORMAT_A32B32G32R32F:
			return 32;
		case sw::FORMAT_A2R10G10B10:
			return 10;
		case sw::FORMAT_A8R8G8B8:
		case sw::FORMAT_X8R8G8B8:
			return 8;
		case sw::FORMAT_A1R5G5B5:
		case sw::FORMAT_R5G6B5:
			return 5;
		default:
			return 0;
		}
	}

	unsigned int GetDepthSize(sw::Format depthFormat)
	{
		switch(depthFormat)
		{
	//	case sw::FORMAT_D16_LOCKABLE:  return 16;
		case sw::FORMAT_D32:           return 32;
	//	case sw::FORMAT_D15S1:         return 15;
		case sw::FORMAT_D24S8:         return 24;
		case sw::FORMAT_D24X8:         return 24;
	//	case sw::FORMAT_D24X4S4:       return 24;
		case sw::FORMAT_D16:           return 16;
		case sw::FORMAT_D32F_LOCKABLE: return 32;
		case sw::FORMAT_D24FS8:        return 24;
	//	case sw::FORMAT_D32_LOCKABLE:  return 32;
	//	case sw::FORMAT_S8_LOCKABLE:   return 0;
		default:                   return 0;
		}
	}

	GLenum ConvertBackBufferFormat(sw::Format format)
	{
		switch(format)
		{
		case sw::FORMAT_A4R4G4B4: return GL_RGBA4;
		case sw::FORMAT_A8R8G8B8: return GL_RGBA8_OES;
		case sw::FORMAT_A1R5G5B5: return GL_RGB5_A1;
		case sw::FORMAT_R5G6B5:   return GL_RGB565;
		case sw::FORMAT_X8R8G8B8: return GL_RGB8_OES;
		default:
			UNREACHABLE();
		}

		return GL_RGBA4;
	}

	GLenum ConvertDepthStencilFormat(sw::Format format)
	{
		switch(format)
		{
		case sw::FORMAT_D16:
		case sw::FORMAT_D24X8:
		case sw::FORMAT_D32:
			return GL_DEPTH_COMPONENT16;
		case sw::FORMAT_D24S8:
			return GL_DEPTH24_STENCIL8_OES;
		default:
			UNREACHABLE();
		}

		return GL_DEPTH24_STENCIL8_OES;
	}
}

std::string getTempPath()
{
    char path[MAX_PATH];
    DWORD pathLen = GetTempPathA(sizeof(path) / sizeof(path[0]), path);
    if(pathLen == 0)
    {
        UNREACHABLE();
        return std::string();
    }

    UINT unique = GetTempFileNameA(path, "sh", 0, path);
    if(unique == 0)
    {
        UNREACHABLE();
        return std::string();
    }
    
    return path;
}

void writeFile(const char* path, const void* content, size_t size)
{
    FILE* file = fopen(path, "w");
    if(!file)
    {
        UNREACHABLE();
        return;
    }

    fwrite(content, sizeof(char), size, file);
    fclose(file);
}
