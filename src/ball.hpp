#ifndef ARKANOOP_BALL_HPP_
#define ARKANOOP_BALL_HPP_
#include "keyboard.hpp"
#include "resource_manager.hpp"


namespace gp {


class Ball : public Sprite {
public:
	Ball();

	float getRadius() const;
	bool isIntersecting(const Sprite& sprite) const;

	void setRadius(float radius);
	void update(float dt);
	void reset(int sprite_index);

private:
	int m_currentSpriteIndex;
	float m_radius;
};


inline Ball::Ball() :
	Sprite(ResourceManager::getSpriteSheet("balls").getTexture()),
	m_currentSpriteIndex(0),
	m_radius(0)
{

}


inline float Ball::getRadius() const
{
	return m_radius;
}



inline void Ball::setRadius(const float radius)
{
	setSize({radius * 2.0f, radius * 2.0f});
	m_radius = radius;
}



inline void Ball::update(const float dt)
{
	if (getRight() > Display::getViewSize().x) {
		setOrigin({Display::getViewSize().x - getHalfSize().x, getOrigin().y});
		setVelocity({-std::abs(getVelocity().x), getVelocity().y});
	} else if (getLeft() < 0) {
		setOrigin({getHalfSize().x, getOrigin().y});
		setVelocity({std::abs(getVelocity().x), getVelocity().y});
	}

	if (getBottom() > Display::getViewSize().y) {
		setOrigin({getOrigin().x, Display::getViewSize().y - getHalfSize().y});
		setVelocity({getVelocity().x, -std::abs(getVelocity().y)});
		reset(++m_currentSpriteIndex);
	
		const SpriteSheet& sprites = ResourceManager::getSpriteSheet("balls");	
		if (m_currentSpriteIndex >= sprites.getSize())
			m_currentSpriteIndex = 0;

	} else if (getTop() < 0) {
		setOrigin({getOrigin().x, getHalfSize().y});
		setVelocity({getVelocity().x, std::abs(getVelocity().y)});
	}

	setOrigin(getOrigin() +  getVelocity() * dt);
}


inline bool Ball::isIntersecting(const Sprite& brick) const
{
	if (checkAABBCollision(brick)) {
		const Vec2f& ball_origin = getOrigin();
		const Vec2f& aabb_origin = brick.getOrigin();
		const Vec2f aabb_half_size = brick.getHalfSize();
		
		const Vec2f aabb_origin_to_ball_origin_diff = ball_origin - aabb_origin;
		const Vec2f closest =
		  aabb_origin + glm::clamp(aabb_origin_to_ball_origin_diff,
		                           -aabb_half_size,
			                    aabb_half_size);

		const Vec2f closest_to_ball_origin_diff = closest - ball_origin;

		return glm::length(closest_to_ball_origin_diff) < m_radius;
	}

	return false;
}


inline void Ball::reset(const int sprite_index)
{
	const Vec2f default_origin { Display::getViewSize().x / 2, Display::getViewSize().y / 2};
	const Vec2f default_velocity { 200, 200 };
	const float default_radius = 8.0f;

	const SpriteSheet& sprites = ResourceManager::getSpriteSheet("balls");
	setSprite(sprites.getSprite(sprite_index % sprites.getSize()));
	setOrigin(default_origin);
	setVelocity(default_velocity);
	setRadius(default_radius);
}


} // namespace gp
#endif

