#include <octoon/video/render_system.h>
#include <octoon/video/camera.h>
#include <octoon/video/geometry.h>
#include <octoon/video/render_scene.h>
#include <octoon/video/text_system.h>
#include <octoon/video/text_material.h>
#include <octoon/runtime/except.h>

using namespace octoon::graphics;

namespace octoon
{
	namespace video
	{
		const char* vert =
			"#version 330\n"
			"uniform mat4 proj;\n"
			"uniform mat4 model;\n"
			"layout(location  = 0) in vec4 POSITION0;\n"
			"void main()\n"
			"{\n"
			"gl_Position = proj * model * POSITION0;\n"
			"}\n";

		const char* frag =
			"#version 330\n"
			"layout(location  = 0) out vec4 oColor;\n"
			"void main()\n"
			"{\n"
			"	oColor = vec4(0.0,1.0,0.0,0.0);\n"
			"}";

		OctoonImplementSingleton(RenderSystem)

		RenderSystem::RenderSystem() noexcept
			: width_(0)
			, height_(0)
			, fbo_(0)
			, fboMSAA_(0)
			, colorTexture_(0)
			, colorTextureMSAA_(0)
			, depthTexture_(0)
			, depthTextureMSAA_(0)
		{
		}

		RenderSystem::~RenderSystem() noexcept
		{
			this->close();
		}

		void
		RenderSystem::setup(WindHandle hwnd, std::uint32_t w, std::uint32_t h) noexcept
		{
			TextSystem::instance()->setup();

			width_ = w;
			height_ = h;
			winhandle_ = hwnd;

			graphics::GraphicsDeviceDesc deviceDesc;
			deviceDesc.setDeviceType(graphics::GraphicsDeviceType::OpenGL);
			device_ = graphics::GraphicsSystem::instance()->createDevice(deviceDesc);
			if (!device_)
				return;

			graphics::GraphicsSwapchainDesc swapchainDesc;
			swapchainDesc.setWindHandle(winhandle_);
			swapchainDesc.setWidth(w);
			swapchainDesc.setHeight(h);
			swapchainDesc.setSwapInterval(graphics::GraphicsSwapInterval::Vsync);
			swapchainDesc.setImageNums(2);
			swapchainDesc.setColorFormat(graphics::GraphicsFormat::B8G8R8A8UNorm);
			swapchainDesc.setDepthStencilFormat(graphics::GraphicsFormat::D24UNorm_S8UInt);
			swapchain_ = device_->createSwapchain(swapchainDesc);
			if (!swapchain_)
				return;

			graphics::GraphicsContextDesc contextDesc;
			contextDesc.setSwapchain(swapchain_);
			context_ = device_->createDeviceContext(contextDesc);
			if (!context_)
				return;

			graphics::GraphicsDataDesc dataDesc;
			dataDesc.setType(graphics::GraphicsDataType::StorageVertexBuffer);
			dataDesc.setStreamSize(0xFFF);
			dataDesc.setUsage(graphics::GraphicsUsageFlagBits::WriteBit);

			vbo_ = device_->createGraphicsData(dataDesc);
			if (!vbo_)
				return;

			GraphicsProgramDesc programDesc;
			programDesc.addShader(device_->createShader(GraphicsShaderDesc(GraphicsShaderStageFlagBits::VertexBit, vert, "main", GraphicsShaderLang::GLSL)));
			programDesc.addShader(device_->createShader(GraphicsShaderDesc(GraphicsShaderStageFlagBits::FragmentBit, frag, "main", GraphicsShaderLang::GLSL)));
			auto program = device_->createProgram(programDesc);

			GraphicsInputLayoutDesc layoutDesc;
			layoutDesc.addVertexLayout(GraphicsVertexLayout(0, "POSITION", 0, GraphicsFormat::R32G32B32SFloat));
			layoutDesc.addVertexBinding(GraphicsVertexBinding(0, layoutDesc.getVertexSize()));

			GraphicsDescriptorSetLayoutDesc descriptor_set_layout;
			descriptor_set_layout.setUniformComponents(program->getActiveParams());

			GraphicsStateDesc stateDesc;
			stateDesc.setPrimitiveType(GraphicsVertexType::TriangleList);
			stateDesc.setCullMode(GraphicsCullMode::None);
			stateDesc.setDepthEnable(true);

			GraphicsPipelineDesc pipeline;
			pipeline.setGraphicsInputLayout(device_->createInputLayout(layoutDesc));
			pipeline.setGraphicsState(device_->createRenderState(stateDesc));
			pipeline.setGraphicsProgram(std::move(program));
			pipeline.setGraphicsDescriptorSetLayout(device_->createDescriptorSetLayout(descriptor_set_layout));

			pipeline_ = device_->createRenderPipeline(pipeline);
			if (!pipeline_)
				return ;

			GraphicsDescriptorSetDesc descriptorSet;
			descriptorSet.setGraphicsDescriptorSetLayout(pipeline.getGraphicsDescriptorSetLayout());

			descriptorSet_ = device_->createDescriptorSet(descriptorSet);
			if (!descriptorSet_)
				return;

			auto begin = descriptorSet_->getGraphicsUniformSets().begin();
			auto end = descriptorSet_->getGraphicsUniformSets().end();

			proj_ = *std::find_if(begin, end, [](const GraphicsUniformSetPtr& set) { return set->get_name() == "proj"; });
			model_ = *std::find_if(begin, end, [](const GraphicsUniformSetPtr& set) { return set->get_name() == "model"; });

			graphics::GraphicsTextureDesc colorTextureDesc;
			colorTextureDesc.setWidth(w);
			colorTextureDesc.setHeight(h);
			colorTextureDesc.setTexFormat(graphics::GraphicsFormat::R8G8B8A8UNorm);
			colorTexture_ = device_->createTexture(colorTextureDesc);

			graphics::GraphicsTextureDesc depthTextureDesc;
			depthTextureDesc.setWidth(w);
			depthTextureDesc.setHeight(h);
			depthTextureDesc.setTexFormat(graphics::GraphicsFormat::X8_D24UNormPack32);
			depthTexture_ = device_->createTexture(depthTextureDesc);

			graphics::GraphicsFramebufferLayoutDesc framebufferLayoutDesc;
			framebufferLayoutDesc.addComponent(GraphicsAttachmentLayout(0, GraphicsImageLayout::ColorAttachmentOptimal, graphics::GraphicsFormat::R8G8B8A8UNorm));
			framebufferLayoutDesc.addComponent(GraphicsAttachmentLayout(1, GraphicsImageLayout::DepthStencilAttachmentOptimal, graphics::GraphicsFormat::X8_D24UNormPack32));

			graphics::GraphicsFramebufferDesc framebufferDesc;
			framebufferDesc.setWidth(w);
			framebufferDesc.setHeight(h);
			framebufferDesc.setGraphicsFramebufferLayout(device_->createFramebufferLayout(framebufferLayoutDesc));
			framebufferDesc.setDepthStencilAttachment(graphics::GraphicsAttachmentBinding(depthTexture_, 0, 0));
			framebufferDesc.addColorAttachment(graphics::GraphicsAttachmentBinding(colorTexture_, 0, 0));

			fbo_ = device_->createFramebuffer(framebufferDesc);

#if !defined(__linux)
			graphics::GraphicsTextureDesc colorTextureDescMSAA;
			colorTextureDescMSAA.setWidth(w);
			colorTextureDescMSAA.setHeight(h);
			colorTextureDescMSAA.setTexMultisample(4);
			colorTextureDescMSAA.setTexFormat(graphics::GraphicsFormat::R8G8B8A8UNorm);
			colorTextureMSAA_ = device_->createTexture(colorTextureDescMSAA);

			graphics::GraphicsTextureDesc depthTextureDescMSAA;
			depthTextureDescMSAA.setWidth(w);
			depthTextureDescMSAA.setHeight(h);
			depthTextureDescMSAA.setTexMultisample(4);
			depthTextureDescMSAA.setTexFormat(graphics::GraphicsFormat::X8_D24UNormPack32);
			depthTextureMSAA_ = device_->createTexture(depthTextureDescMSAA);

			graphics::GraphicsFramebufferDesc framebufferDescMSAA;
			framebufferDescMSAA.setWidth(w);
			framebufferDescMSAA.setHeight(h);
			framebufferDescMSAA.setGraphicsFramebufferLayout(device_->createFramebufferLayout(framebufferLayoutDesc));
			framebufferDescMSAA.setDepthStencilAttachment(graphics::GraphicsAttachmentBinding(depthTextureMSAA_, 0, 0));
			framebufferDescMSAA.addColorAttachment(graphics::GraphicsAttachmentBinding(colorTextureMSAA_, 0, 0));

			fboMSAA_ = device_->createFramebuffer(framebufferDescMSAA);
#endif
		}

		void
		RenderSystem::close() noexcept
		{
			TextSystem::instance()->close();
		}

		void
		RenderSystem::setFramebufferSize(std::uint32_t w, std::uint32_t h) noexcept
		{
			width_ = w;
			height_ = h;
		}

		void
		RenderSystem::getFramebufferSize(std::uint32_t& w, std::uint32_t& h) const noexcept
		{
			w = width_;
			h = height_;
		}

		void
		RenderSystem::render() noexcept
		{
			context_->renderBegin();

			for (auto& camera : video::RenderScene::instance()->getCameraList())
			{
#if (__linux)
				context_->setFramebuffer(fbo_);
#else
				context_->setFramebuffer(fboMSAA_);
#endif
				context_->setViewport(0, camera->getPixelViewport());
				context_->clearFramebuffer(0, octoon::graphics::GraphicsClearFlagBits::ColorDepthBit, camera->getClearColor(), 1.0f, 0);
				context_->setRenderPipeline(pipeline_);
				context_->setDescriptorSet(descriptorSet_);

				proj_->uniform4fmat(camera->getViewProjection());

				for (auto& object : video::RenderScene::instance()->getRenderObjects())
				{
					auto geometry = object->downcast<video::Geometry>();
					auto mesh = geometry->getMesh();
					if (!mesh)
						continue;

					auto material = geometry->getMaterial();
					if (!material)
						continue;

					auto textMaterial = std::dynamic_pointer_cast<TextMaterial>(material);
					if (!textMaterial)
						continue;

					vertices_ = mesh->getVertexArray();
					if (vbo_->getGraphicsDataDesc().getStreamSize() < vertices_.size() * sizeof(math::float3))
					{
						GraphicsDataDesc dataDesc;
						dataDesc.setType(GraphicsDataType::StorageVertexBuffer);
						dataDesc.setStream((std::uint8_t*)vertices_.data());
						dataDesc.setStreamSize(vertices_.size() * sizeof(math::float3));
						dataDesc.setUsage(vbo_->getGraphicsDataDesc().getUsage());

						vbo_ = device_->createGraphicsData(dataDesc);
						if (!vbo_)
						{
							context_->renderEnd();
							return;
						}
					}

					model_->uniform4fmat(geometry->getTransform());

					context_->setVertexBufferData(0, vbo_, 0);
					context_->draw(vertices_.size(), 1, 0, 0);
				}

				if (camera->getCameraOrder() == CameraOrder::Main)
				{
					if (winhandle_)
					{
						auto& v = camera->getPixelViewport();
#if (__linux)
						context_->blitFramebuffer(fbo_, v, nullptr, v);
#else
						context_->blitFramebuffer(fboMSAA_, v, nullptr, v);
#endif
					}
				}
			}

			context_->renderEnd();
			context_->present();
		}
	}
}