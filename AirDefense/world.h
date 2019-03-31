#ifndef WORLD_H
#define WORLD_H

#define IM InputManager::getInstance()

#include "inputManager.h"
#include "objectManager.h"
#include "geography.h"

class World {
public:

	World(int worldWidth, int worldHeight)
	{
		geography = new Geography(worldWidth, worldHeight, 6, 0.9, 1, 0.4);

		initialize();
	}

	~World() 
	{
		delete geography;
	}

	void update(sf::RenderWindow* window, float dt);
	void draw(sf::RenderWindow* window);

	void addObject(WorldObject *obj);

	bool checkIfLandAtMouse() { return geography->checkIfLand(IM.mousePosition().x, IM.mousePosition().y); }
	void regenerateGeography(float seed) { geography->regenerate(seed); }

private:
	Geography *geography;

	ObjectManager objectManager;

	void initialize();
};

#endif