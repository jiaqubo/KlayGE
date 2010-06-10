// OGLES2Mapping.cpp
// KlayGE RenderEngine和OpenGL ES 2本地之间的映射 实现文件
// Ver 3.10.0
// 版权所有(C) 龚敏敏, 2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// 初次建立 (2010.1.22)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Vector.hpp>
#include <KlayGE/Matrix.hpp>
#include <KlayGE/Color.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderLayout.hpp>

#include <boost/assert.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGLES2/OGLES2Texture.hpp>
#include <KlayGE/OpenGLES2/OGLES2Mapping.hpp>

namespace KlayGE
{
	// 从KlayGE的Color转换到float[4]
	/////////////////////////////////////////////////////////////////////////////////
	void OGLES2Mapping::Mapping(GLfloat* clr4, Color const & clr)
	{
		clr4[0] = clr.r();
		clr4[1] = clr.g();
		clr4[2] = clr.b();
		clr4[3] = clr.a();
	}

	// 从RenderEngine::CompareFunction转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLES2Mapping::Mapping(CompareFunction func)
	{
		switch (func)
		{
		case CF_AlwaysFail:
			return GL_NEVER;

		case CF_AlwaysPass:
			return GL_ALWAYS;

		case CF_Less:
			return GL_LESS;

		case CF_LessEqual:
			return GL_LEQUAL;

		case CF_Equal:
			return GL_EQUAL;

		case CF_NotEqual:
			return GL_NOTEQUAL;

		case CF_GreaterEqual:
			return GL_GEQUAL;

		case CF_Greater:
			return GL_GREATER;

		default:
			BOOST_ASSERT(false);
			return GL_EQUAL;
		};
	}

	// 从RenderEngine::AlphaBlendFactor转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLES2Mapping::Mapping(AlphaBlendFactor factor)
	{
		switch (factor)
		{
		case ABF_Zero:
			return GL_ZERO;

		case ABF_One:
			return GL_ONE;

		case ABF_Src_Alpha:
			return GL_SRC_ALPHA;

		case ABF_Dst_Alpha:
			return GL_DST_ALPHA;

		case ABF_Inv_Src_Alpha:
			return GL_ONE_MINUS_SRC_ALPHA;

		case ABF_Inv_Dst_Alpha:
			return GL_ONE_MINUS_DST_ALPHA;

		case ABF_Src_Color:
			return GL_SRC_COLOR;

		case ABF_Dst_Color:
			return GL_DST_COLOR;

		case ABF_Inv_Src_Color:
			return GL_ONE_MINUS_SRC_COLOR;

		case ABF_Inv_Dst_Color:
			return GL_ONE_MINUS_DST_COLOR;

		case ABF_Src_Alpha_Sat:
			return GL_SRC_ALPHA_SATURATE;

		default:
			BOOST_ASSERT(false);
			return GL_ZERO;
		}
	}

	// 从RenderEngine::StencilOperation转换到GLenum
	/////////////////////////////////////////////////////////////////////////////////
	GLenum OGLES2Mapping::Mapping(StencilOperation op)
	{
		switch (op)
		{
		case SOP_Keep:
			return GL_KEEP;

		case SOP_Zero:
			return GL_ZERO;

		case SOP_Replace:
			return GL_REPLACE;

		case SOP_Increment:
			return GL_INCR;

		case SOP_Decrement:
			return GL_DECR;

		case SOP_Invert:
			return GL_INVERT;

		default:
			BOOST_ASSERT(false);
			return GL_KEEP;
		};
	}

	GLenum OGLES2Mapping::Mapping(BlendOperation bo)
	{
		switch (bo)
		{
		case BOP_Add:
			return GL_FUNC_ADD;

		case BOP_Sub:
			return GL_FUNC_SUBTRACT;

		case BOP_Rev_Sub:
			return GL_FUNC_REVERSE_SUBTRACT;

		case BOP_Min:
			if (glloader_GLES_EXT_blend_minmax())
			{
				return GL_MIN_EXT;
			}
			else
			{
				THR(boost::system::posix_error::not_supported);
			}

		case BOP_Max:
			if (glloader_GLES_EXT_blend_minmax())
			{
				return GL_MAX_EXT;
			}
			else
			{
				THR(boost::system::posix_error::not_supported);
			}

		default:
			BOOST_ASSERT(false);
			return GL_FUNC_ADD;
		}
	}

	GLint OGLES2Mapping::Mapping(TexAddressingMode mode)
	{
		switch (mode)
		{
		case TAM_Wrap:
			return GL_REPEAT;

		case TAM_Mirror:
			return GL_MIRRORED_REPEAT;

		case TAM_Clamp:
			return GL_CLAMP_TO_EDGE;

		case TAM_Border:
			return GL_CLAMP_TO_EDGE;

		default:
			BOOST_ASSERT(false);
			return GL_REPEAT;
		}
	}

	void OGLES2Mapping::Mapping(GLenum& primType, uint32_t& primCount, RenderLayout const & rl)
	{
		uint32_t const vertexCount = static_cast<uint32_t>(rl.UseIndices() ? rl.NumIndices() : rl.NumVertices());
		primType = GL_POINTS;
		primCount = vertexCount;
		switch (rl.TopologyType())
		{
		case RenderLayout::TT_PointList:
			primType = GL_POINTS;
			primCount = vertexCount;
			break;

		case RenderLayout::TT_LineList:
			primType = GL_LINES;
			primCount = vertexCount / 2;
			break;

		case RenderLayout::TT_LineStrip:
			primType = GL_LINE_STRIP;
			primCount = vertexCount - 1;
			break;

		case RenderLayout::TT_TriangleList:
			primType = GL_TRIANGLES;
			primCount = vertexCount / 3;
			break;

		case RenderLayout::TT_TriangleStrip:
			primType = GL_TRIANGLE_STRIP;
			primCount = vertexCount - 2;
			break;

		default:
			THR(boost::system::posix_error::not_supported);
		}
	}

	void OGLES2Mapping::MappingFormat(GLint& internalFormat, GLenum& glformat, GLenum& gltype, ElementFormat ef)
	{
		switch (ef)
		{
		case EF_A8:
			internalFormat = GL_ALPHA;
			glformat = GL_ALPHA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case EF_R8:
			internalFormat = GL_LUMINANCE;
			glformat = GL_LUMINANCE;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case EF_SIGNED_R8:
			internalFormat = GL_LUMINANCE;
			glformat = GL_LUMINANCE;
			gltype = GL_BYTE;
			break;

		case EF_BGR8:
			internalFormat = GL_RGB;
			glformat = GL_RGB;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case EF_ARGB8:
			if (glloader_GLES_EXT_texture_format_BGRA8888())
			{
				internalFormat = GL_BGRA_EXT;
				glformat = GL_BGRA_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(boost::system::posix_error::not_supported);
			}
			break;

		case EF_ABGR8:
			internalFormat = GL_RGBA;
			glformat = GL_RGBA;
			gltype = GL_UNSIGNED_BYTE;
			break;

		case EF_A2BGR10:
			internalFormat = GL_RGBA;
			glformat = GL_RGBA;
			gltype = GL_UNSIGNED_INT_2_10_10_10_REV_EXT;
			break;

		case EF_SIGNED_A2BGR10:
			internalFormat = GL_RGBA;
			glformat = GL_RGBA;
			gltype = GL_UNSIGNED_INT_2_10_10_10_REV_EXT;
			break;

		case EF_R16:
			internalFormat = GL_LUMINANCE;
			glformat = GL_LUMINANCE;
			gltype = GL_UNSIGNED_SHORT;
			break;

		case EF_SIGNED_R16:
			internalFormat = GL_LUMINANCE;
			glformat = GL_LUMINANCE;
			gltype = GL_SHORT;
			break;

		case EF_BGR16:
			internalFormat = GL_RGB;
			glformat = GL_RGB;
			gltype = GL_UNSIGNED_SHORT;
			break;

		case EF_SIGNED_BGR16:
			internalFormat = GL_RGB;
			glformat = GL_RGB;
			gltype = GL_SHORT;
			break;

		case EF_ABGR16:
			internalFormat = GL_RGBA;
			glformat = GL_RGBA;
			gltype = GL_UNSIGNED_SHORT;
			break;

		case EF_SIGNED_ABGR16:
			internalFormat = GL_RGBA;
			glformat = GL_RGBA;
			gltype = GL_SHORT;
			break;

		case EF_R16F:
			internalFormat = GL_LUMINANCE;
			glformat = GL_LUMINANCE;
			gltype = GL_HALF_FLOAT_OES;
			break;

		case EF_GR16F:
			internalFormat = GL_LUMINANCE_ALPHA;
			glformat = GL_LUMINANCE_ALPHA;
			gltype = GL_FLOAT;
			break;

		case EF_BGR16F:
			internalFormat = GL_RGB;
			glformat = GL_RGB;
			gltype = GL_HALF_FLOAT_OES;
			break;

		case EF_ABGR16F:
			internalFormat = GL_RGBA;
			glformat = GL_RGBA;
			gltype = GL_HALF_FLOAT_OES;
			break;

		case EF_R32F:
			internalFormat = GL_LUMINANCE;
			glformat = GL_LUMINANCE;
			gltype = GL_FLOAT;
			break;

		case EF_GR32F:
			internalFormat = GL_LUMINANCE_ALPHA;
			glformat = GL_LUMINANCE_ALPHA;
			gltype = GL_FLOAT;
			break;

		case EF_BGR32F:
			internalFormat = GL_RGB;
			glformat = GL_RGB;
			gltype = GL_FLOAT;
			break;

		case EF_ABGR32F:
			internalFormat = GL_RGBA;
			glformat = GL_RGBA;
			gltype = GL_FLOAT;
			break;

		case EF_BC1:
			if (glloader_GLES_EXT_texture_compression_dxt1())
			{
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				glformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				gltype = GL_UNSIGNED_BYTE;
			}
			else
			{
				THR(boost::system::posix_error::not_supported);
			}
			break;

		case EF_D16:
			internalFormat = GL_DEPTH_COMPONENT16;
			glformat = GL_DEPTH_COMPONENT;
			gltype = GL_UNSIGNED_SHORT;
			break;

		default:
			THR(boost::system::posix_error::not_supported);
		}
	}
}
