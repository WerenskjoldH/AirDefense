#include "projectileObject.h"

ProjectileObject::ProjectileObject(float x, float y, CityObject * target) : WorldObject("ProjectileObject", sf::Vector2f(x, y), DEFAULT_PROJECTILE_SIZE)
{
	circle.setRadius(boundaryRadius);
	circle.setOrigin(sf::Vector2f(boundaryRadius, boundaryRadius));
	circle.setFillColor(sf::Color(150, 0, 60, 255));

	speed = DEFAULT_PROJECTILE_SPEED;

	direction = vu::unit(target->getPosition() - sf::Vector2f(x,y));

	targetCity = target;
}

ProjectileObject::~ProjectileObject()
{

}

void ProjectileObject::update(World * world, float dt)
{
	position += speed * direction * dt;

	if (intersects((WorldObject*)targetCity))
	{
		if (!targetCity->isDestroyed())
		{
			spawner->incrementDestroyedCities();

			// Destroy the city, if it is not destroyed yet
			targetCity->destroyCity();
		}

		// Destroy this missile
		this->setObjectDead();
	}
}

void ProjectileObject::draw(sf::RenderWindow * window)
{
	circle.setPosition(position);
	window->draw(circle);
}

void ProjectileObject::setSpawner(ProjectileSpawnerObject* s)
{
	spawner = s;
}
