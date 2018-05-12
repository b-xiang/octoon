#ifndef OCTOON_GL20_TEXTURE_H_
#define OCTOON_GL20_TEXTURE_H_

#include "gl20_types.h"

namespace octoon
{
	namespace graphics
	{
		class GL20Texture final : public GraphicsTexture
		{
			OctoonDeclareSubClass(GL20Texture, GraphicsTexture)
		public:
			GL20Texture() noexcept;
			~GL20Texture() noexcept;

			bool setup(const GraphicsTextureDesc& textureDesc) noexcept;
			void close() noexcept;

			bool map(std::uint32_t x, std::uint32_t y, std::uint32_t w, std::uint32_t h, std::uint32_t mipLevel, void** data) noexcept override;
			void unmap() noexcept override;

			void setSampler(const GraphicsSamplerPtr& sampler) noexcept;

			GLenum getTarget() const noexcept;
			GLuint getInstanceID() const noexcept;

			const GraphicsTextureDesc& getGraphicsTextureDesc() const noexcept override;

		private:
			static bool applySamplerWrap(GLenum target, GraphicsSamplerWrap wrap) noexcept;
			static bool applySamplerFilter(GLenum target, GraphicsSamplerFilter minFilter, GraphicsSamplerFilter magFilter) noexcept;

		private:
			friend class GL20Device;
			void setDevice(GraphicsDevicePtr device) noexcept;
			GraphicsDevicePtr getDevice() noexcept override;

		private:
			GLenum _target;
			GLuint _texture;
			GraphicsSamplerPtr _sampler;
			GraphicsTextureDesc _textureDesc;
			GraphicsDeviceWeakPtr _device;
		};
	}
}

#endif