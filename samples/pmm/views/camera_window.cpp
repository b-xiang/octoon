#include "camera_window.h"

#include <octoon/octoon.h>
#include <octoon/ui/imgui.h>
#include <octoon/editor_camera_component.h>

namespace octoon
{
	namespace editor
	{
		OctoonImplementSubClass(CameraWindow, GameComponent, "CameraWindow")

		CameraWindow::CameraWindow() noexcept
			: framebufferSizeW_(0)
			, framebufferSizeH_(0)
			, isShowedCameraWindow_(true)
		{
		}

		CameraWindow::~CameraWindow() noexcept
		{
		}

		void
		CameraWindow::onActivate() noexcept
		{
			this->addComponentDispatch(GameDispatchType::Gui);

			this->addMessageListener("editor:camera:set", std::bind(&CameraWindow::setCamera, this, std::placeholders::_1));
			this->addMessageListener("editor:camera:fov", std::bind(&CameraWindow::setFov, this, std::placeholders::_1));
			this->addMessageListener("editor:camera:position", std::bind(&CameraWindow::setPosition, this, std::placeholders::_1));
			this->addMessageListener("editor:camera:rotation", std::bind(&CameraWindow::setRotation, this, std::placeholders::_1));
			this->addMessageListener("editor:camera:distance", std::bind(&CameraWindow::setDistance, this, std::placeholders::_1));
		}

		void
		CameraWindow::onDeactivate() noexcept
		{
			this->removeComponentDispatchs();
			this->removeMessageListener("editor:camera:set", std::bind(&CameraWindow::setCamera, this, std::placeholders::_1));
			this->removeMessageListener("editor:camera:fov", std::bind(&CameraWindow::setFov, this, std::placeholders::_1));
			this->removeMessageListener("editor:camera:position", std::bind(&CameraWindow::setPosition, this, std::placeholders::_1));
			this->removeMessageListener("editor:camera:rotation", std::bind(&CameraWindow::setRotation, this, std::placeholders::_1));
			this->removeMessageListener("editor:camera:distance", std::bind(&CameraWindow::setDistance, this, std::placeholders::_1));

			if (camera_)
				camera_.reset();
		}

		void
		CameraWindow::onGui() noexcept
		{
		}

		void 
		CameraWindow::setCamera(const runtime::any& data) noexcept
		{
			if (data.type() == typeid(GameObjectPtr))
				camera_ = runtime::any_cast<GameObjectPtr>(data);
		}

		void 
		CameraWindow::setFov(const runtime::any& data) noexcept
		{
			if (camera_)
			{
				if (data.type() == typeid(float))
					camera_->getComponent<PerspectiveCameraComponent>()->setAperture(runtime::any_cast<float>(data));
			}
		}

		void 
		CameraWindow::setPosition(const runtime::any& data) noexcept
		{
			if (camera_)
				camera_->getComponent<Transform>()->setTranslate(runtime::any_cast<math::float3>(data));
		}

		void 
		CameraWindow::setRotation(const runtime::any& data) noexcept
		{
			if (camera_)
				camera_->getComponent<Transform>()->setQuaternion(runtime::any_cast<math::Quaternion>(data));
		}

		void
		CameraWindow::setDistance(const runtime::any& data) noexcept
		{
			if (camera_)
			{
				if (data.type() == typeid(std::uint8_t))
					camera_->getComponent<EditorCameraComponent>()->moveCamera((float)runtime::any_cast<std::uint8_t>(data));
				else if (data.type() == typeid(std::uint16_t))
					camera_->getComponent<EditorCameraComponent>()->moveCamera((float)runtime::any_cast<std::uint16_t>(data));
				else if (data.type() == typeid(std::uint32_t))
					camera_->getComponent<EditorCameraComponent>()->moveCamera((float)runtime::any_cast<std::uint32_t>(data));
				else if (data.type() == typeid(float))
					camera_->getComponent<EditorCameraComponent>()->moveCamera(runtime::any_cast<float>(data));
			}
		}

		GameComponentPtr
		CameraWindow::clone() const noexcept
		{
			return std::make_shared<CameraWindow>();
		}
	}
}