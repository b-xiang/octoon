#include <octoon/animator_component.h>
#include <octoon/transform_component.h>
#include <octoon/solver_component.h>
#include <octoon/timer_feature.h>

namespace octoon
{
	OctoonImplementSubClass(AnimatorComponent, GameComponent, "Animator")

	AnimatorComponent::AnimatorComponent() noexcept
		: enableAnimation_(true)
		, enableAnimOnVisableOnly_(false)
	{
	}

	AnimatorComponent::AnimatorComponent(animation::Animation<float>&& animation, GameObjects&& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(std::move(avatar));
		this->setAnimation(std::move(animation));
	}

	AnimatorComponent::AnimatorComponent(animation::Animation<float>&& animation, const GameObjects& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(avatar);
		this->setAnimation(std::move(animation));
	}

	AnimatorComponent::AnimatorComponent(const animation::Animation<float>& animation, GameObjects&& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(std::move(avatar));
		this->setAnimation(animation);
	}

	AnimatorComponent::AnimatorComponent(const animation::Animation<float>& animation, const GameObjects& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(avatar);
		this->setAnimation(animation);
	}

	AnimatorComponent::AnimatorComponent(animation::Animation<float>&& animation) noexcept
		: AnimatorComponent()
	{
		animation_ = std::move(animation);
	}

	AnimatorComponent::AnimatorComponent(const animation::Animation<float>& animation) noexcept
		: AnimatorComponent()
	{
		animation_ = animation;
	}

	AnimatorComponent::AnimatorComponent(GameObjects&& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(std::move(avatar));
	}

	AnimatorComponent::AnimatorComponent(const GameObjects& avatar) noexcept
		: AnimatorComponent()
	{
		this->setAvatar(avatar);
	}

	AnimatorComponent::~AnimatorComponent() noexcept
	{
	}

	bool
	AnimatorComponent::play(const std::string& status) noexcept
	{
		this->setName(status);
		this->addComponentDispatch(GameDispatchType::FixedUpdate);

		enableAnimation_ = true;
		return enableAnimation_;
	}

	void
	AnimatorComponent::pause() noexcept
	{
		enableAnimation_ = false;
		this->removeComponentDispatch(GameDispatchType::FixedUpdate);
	}

	void
	AnimatorComponent::stop() noexcept
	{
		this->setTime(0.0f);
		this->sample();
		this->removeComponentDispatch(GameDispatchType::FixedUpdate);
	}

	void
	AnimatorComponent::setTime(float time) noexcept
	{
		animation_.setTime(time);
	}

	void
	AnimatorComponent::sample(float delta) noexcept
	{
		if (delta != 0.0f)
			animation_.evaluate(delta);

		if (avatar_.empty())
			this->updateAnimation();
		else
			this->updateAvatar();
	}

	void
	AnimatorComponent::setAnimation(animation::Animation<float>&& clips) noexcept
	{
		animation_ = std::move(clips);
	}

	void
	AnimatorComponent::setAnimation(const animation::Animation<float>& clips) noexcept
	{
		animation_ = clips;
	}

	const animation::Animation<float>&
	AnimatorComponent::getAnimation() const noexcept
	{
		return animation_;
	}

	void
	AnimatorComponent::setAvatar(GameObjects&& avatar) noexcept
	{
		avatar_ = std::move(avatar);
		this->onAttachAvatar(avatar_);
	}

	void
	AnimatorComponent::setAvatar(const GameObjects& avatar) noexcept
	{
		avatar_ = avatar;
		this->onAttachAvatar(avatar_);
	}

	const GameObjects&
	AnimatorComponent::getAvatar() const noexcept
	{
		return avatar_;
	}

	const animation::AnimatorStateInfo&
	AnimatorComponent::getCurrentAnimatorStateInfo() const noexcept
	{
		return animation_.state;
	}

	GameComponentPtr
	AnimatorComponent::clone() const noexcept
	{
		auto instance = std::make_shared<AnimatorComponent>();
		instance->setName(this->getName());
		instance->setAvatar(this->getAvatar());
		instance->setAnimation(this->getAnimation());

		return instance;
	}

	void 
	AnimatorComponent::onActivate() except
	{
	}

	void
	AnimatorComponent::onDeactivate() noexcept
	{
		this->removeComponentDispatch(GameDispatchType::FixedUpdate);
	}

	void
	AnimatorComponent::onFixedUpdate() noexcept
	{
		if (enableAnimation_)
		{
			auto timeFeature = this->getFeature<TimerFeature>();
			if (timeFeature)
				this->sample(timeFeature->getTimeInterval());
		}
	}

	void
	AnimatorComponent::onAttachAvatar(const GameObjects& avatar) noexcept
	{
		bindpose_.resize(avatar.size());

		for (std::size_t i = 0; i < avatar.size(); i++)
			bindpose_[i] = avatar[i]->getComponent<TransformComponent>()->getLocalTranslate();
	}

	void
	AnimatorComponent::updateAvatar(float delta) noexcept
	{
		if (this->getCurrentAnimatorStateInfo().finish)
			return;

		for (std::size_t i = 0; i < animation_.clips.size(); i++)
		{
			auto transform = avatar_[i]->getComponent<TransformComponent>();

			auto scale = transform->getLocalScale();
			auto quat = transform->getLocalQuaternion();
			auto translate = transform->getLocalTranslate();
			auto euler = math::eulerAngles(quat);

			for (auto& curve : animation_.clips[i].curves)
			{
				if (curve.first == "LocalPosition.x")
					translate.x = curve.second.value + bindpose_[i].x;
				else if (curve.first == "LocalPosition.y")
					translate.y = curve.second.value + bindpose_[i].y;
				else if (curve.first == "LocalPosition.z")
					translate.z = curve.second.value + bindpose_[i].z;
				else if (curve.first == "LocalScale.x")
					scale.x = curve.second.value;
				else if (curve.first == "LocalScale.y")
					scale.y = curve.second.value;
				else if (curve.first == "LocalScale.z")
					scale.z = curve.second.value;
				else if (curve.first == "LocalRotation.x")
					quat.x = curve.second.value;
				else if (curve.first == "LocalRotation.y")
					quat.y = curve.second.value;
				else if (curve.first == "LocalRotation.z")
					quat.z = curve.second.value;
				else if (curve.first == "LocalRotation.w")
					quat.w = curve.second.value;
				else if (curve.first == "LocalEulerAnglesRaw.x")
					euler.x = curve.second.value;
				else if (curve.first == "LocalEulerAnglesRaw.y")
					euler.y = curve.second.value;
				else if (curve.first == "LocalEulerAnglesRaw.z")
					euler.z = curve.second.value;
			}

			transform->setLocalScale(scale);
			transform->setLocalTranslate(translate);
			transform->setLocalQuaternion(math::Quaternion(euler));
		}

		this->sendMessage("octoon:animation:update");
	}

	void
	AnimatorComponent::updateAnimation(float delta) noexcept
	{
		for (auto& clip : animation_.clips)
		{
			if (clip.finish)
				continue;

			auto transform = this->getComponent<TransformComponent>();
			auto scale = transform->getLocalScale();
			auto quat = transform->getLocalQuaternion();
			auto translate = transform->getLocalTranslate();
			auto euler = math::eulerAngles(quat);
			auto move = 0.0f;

			for (auto& curve : clip.curves)
			{
				if (curve.first == "LocalScale.x")
					scale.x = curve.second.value;
				else if (curve.first == "LocalScale.y")
					scale.y = curve.second.value;
				else if (curve.first == "LocalScale.z")
					scale.z = curve.second.value;
				else if (curve.first == "LocalPosition.x")
					translate.x = curve.second.value;
				else if (curve.first == "LocalPosition.y")
					translate.y = curve.second.value;
				else if (curve.first == "LocalPosition.z")
					translate.z = curve.second.value;
				else if (curve.first == "LocalRotation.x")
					quat.x = curve.second.value;
				else if (curve.first == "LocalRotation.y")
					quat.y = curve.second.value;
				else if (curve.first == "LocalRotation.z")
					quat.z = curve.second.value;
				else if (curve.first == "LocalRotation.w")
					quat.w = curve.second.value;
				else if (curve.first == "LocalEulerAnglesRaw.x")
					euler.x = curve.second.value;
				else if (curve.first == "LocalEulerAnglesRaw.y")
					euler.y = curve.second.value;
				else if (curve.first == "LocalEulerAnglesRaw.z")
					euler.z = curve.second.value;
				else if (curve.first == "Transform:move")
					move = curve.second.value;
				else
					this->sendMessage(curve.first, curve.second.value);
			}

			auto rotation = math::Quaternion(euler);

			transform->setLocalScale(scale);
			transform->setLocalTranslate(translate + math::rotate(rotation, math::float3::Forward) * move);
			transform->setLocalQuaternion(rotation);
		}

		this->sendMessage("octoon:animation:update");
	}
}