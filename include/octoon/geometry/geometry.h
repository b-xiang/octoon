#ifndef OCTOON_GEOMETRY_H_
#define OCTOON_GEOMETRY_H_

#include <octoon/mesh/mesh.h>
#include <octoon/material/material.h>
#include <octoon/video/render_object.h>
#include <octoon/hal/graphics_data.h>

namespace octoon::geometry
{
	class OCTOON_EXPORT Geometry final : public video::RenderObject
	{
		OctoonDeclareSubClass(Geometry, RenderObject)
	public:
		Geometry() noexcept;
		~Geometry() noexcept;

		void setCastShadow(bool enable) noexcept;
		bool getCastShadow() const noexcept;

		void setReceiveShadow(bool enable) noexcept;
		bool getReceiveShadow() const noexcept;

		void setNumVertices(std::uint32_t numVertice) noexcept;
		std::uint32_t getNumVertices() const noexcept;

		void setNumIndices(std::uint32_t numIndices) noexcept;
		std::uint32_t getNumIndices() const noexcept;

		void setVertexBuffer(const hal::GraphicsDataPtr& data) noexcept;
		const hal::GraphicsDataPtr& getVertexBuffer() const noexcept;

		void setIndexBuffer(const hal::GraphicsDataPtr& data) noexcept;
		const hal::GraphicsDataPtr& getIndexBuffer() const noexcept;

		void setMaterial(std::shared_ptr<material::Material>&& material) noexcept;
		void setMaterial(const std::shared_ptr<material::Material>& material) noexcept;
		const std::shared_ptr<material::Material>& getMaterial() const noexcept;

	private:
		bool isCastShadow_;
		bool isReceiveShadow_;

		std::uint32_t numVertices_;
		std::uint32_t numIndices_;

		hal::GraphicsDataPtr vertices_;
		hal::GraphicsDataPtr indices_;

		std::shared_ptr<material::Material> material_;
	};
}

#endif