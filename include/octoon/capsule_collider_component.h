#ifndef OCTOON_CAPSULE_COLLIDER_COMPONENT_H_
#define OCTOON_CAPSULE_COLLIDER_COMPONENT_H_

#include <octoon/collider_component.h>
#include <octoon/physics/physics_capsule_shape.h>

namespace octoon
{
	class OCTOON_EXPORT CapsuleColliderComponent final : public ColliderComponent
	{
		OctoonDeclareSubClass(CapsuleColliderComponent, ColliderComponent)
	public:
		CapsuleColliderComponent() noexcept;
		CapsuleColliderComponent(float radius, float height) noexcept;
		virtual ~CapsuleColliderComponent();

		void setRadius(float radius) noexcept;
		void setHeight(float height) noexcept;

		float getRadius() const noexcept;
		float getHeight() const noexcept;

		void setCenter(const math::float3& center) noexcept override;
		const math::float3& getCenter() const noexcept override;

		GameComponentPtr clone() const noexcept;

	private:
		friend class RigidbodyComponent;
		std::shared_ptr<physics::PhysicsShape> getShape() noexcept override;

	private:
		void onActivate() noexcept override;
		void onDeactivate() noexcept override;

	private:
		float radius_;
		float height_;

		math::float3 center_;

		std::shared_ptr<physics::PhysicsCapsuleShape> shape_;
	};
}

#endif