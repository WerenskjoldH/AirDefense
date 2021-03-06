#include "cityPlanner.h"
#include "world.h"
#include "geography.h"
#include "cityObject.h"

#include <vector>

#define OBSERVED_CITY p->cities[observedCity]
#define PLACEMENT(i) placements.at(i)

// This way we don't have to generate 2*n city objects and waste space
struct City {
	float x;
	float y;
};

struct Placement {
	float overallWeight = 0.f;
	City* cities;

	~Placement()
	{
		delete[] cities;
	}
};

std::vector<Placement*> placements;

/// Scoring Parameters
// 1. Distanced from other cities
// 2. Not located at too high of an altitude
// 3. Connected by land 
// 4. Near water
float scoreCity(World* world, Placement *p, int numberOfCities, int observedCity)
{
	// Total score should equal out to 4.f when unweighted
	float score = 0;

	float proportionalScore = (1.f / float(numberOfCities));

	// Check altitude of city
	if (world->getAltitudeAtLocation(OBSERVED_CITY.x, OBSERVED_CITY.y) < 0.8)
		score += 3.f * proportionalScore;

	// Score distance from others and connection - This could be cleaned up and improved, but a genetic algorithm 
	for (int i = 0; i < numberOfCities; i++)
	{
		float dist = DISTANCE(OBSERVED_CITY.x, OBSERVED_CITY.y, p->cities[i].x, p->cities[i].y);
		score += (dist >= PREFERRED_CITY_DISTANCE) ? proportionalScore : (-10.f * proportionalScore);

		int step = 0;
		sf::Vector2f origin(OBSERVED_CITY.x, OBSERVED_CITY.y);
		sf::Vector2f direction(p->cities[i].x - origin.x, p->cities[i].y - origin.y);
		direction /= dist;

		float distanceCovered = 0;
		while (distanceCovered <= dist)
		{
			distanceCovered = (DISTANCE_STEP_SIZE * step);

			if (world->checkIfLandAtLocation(origin.x + direction.x * distanceCovered, origin.y + direction.y * distanceCovered) == 0 || distanceCovered > world->getWorldWidth() && distanceCovered > world->getWorldHeight())
				break;

			step++;
		}

		// Cities are connected by land -- This is a questionable approach, but on average gives a very nice layout, so it remains
		if (distanceCovered >= dist)
			score += proportionalScore;
	}

	// Score near water only check above, below, left, and right to speed things up
	if (world->checkIfLandAtLocation(OBSERVED_CITY.x + 2.f * DISTANCE_STEP_SIZE, OBSERVED_CITY.y) 
		|| world->checkIfLandAtLocation(OBSERVED_CITY.x - 2.f * DISTANCE_STEP_SIZE, OBSERVED_CITY.y)
		|| world->checkIfLandAtLocation(OBSERVED_CITY.x, OBSERVED_CITY.y + 2.f * DISTANCE_STEP_SIZE) 
		|| world->checkIfLandAtLocation(OBSERVED_CITY.x, OBSERVED_CITY.y - 2.f * DISTANCE_STEP_SIZE))
	{
		score += 1.f;
	}

	return score;
}

float scoreCities(World *world, Geography *geography, int numberOfCitiesToTest, Placement *p)
{
	float halfGeoWidth = geography->getWidth() / 2.f;
	float halfGeoHeight = geography->getHeight() / 2.f;

	float totalWeight = 0.f;

	for (int i = 0; i < numberOfCitiesToTest; i++)
	{
		// Sample a point ( either positive and negative ) based around the center of the map
		float xO = halfGeoWidth;
		float yO = halfGeoHeight;

		float r = (2.f*(float(rand())) / float(RAND_MAX)) - 1.f;
		xO *= r;

		r = (2.f*(float(rand())) / float(RAND_MAX)) - 1.f;
		yO *= r;

		p->cities[i].x = halfGeoWidth + xO;
		p->cities[i].y = halfGeoHeight + yO;

		// Check if valid position
		if (!geography->checkIfLand(p->cities[i].x, p->cities[i].y))
		{
			i--;
			continue;
		}

		totalWeight += scoreCity(world, p, numberOfCitiesToTest, i);
	}

	return totalWeight;
}

City* performTrials(World *world, Geography *geography, int numberOfCitiesToTest, int numberOfTrials)
{
	// Perform each placement and calculate total score
	for(int i = 0; i < numberOfTrials; i++)
	{
		PLACEMENT(i)->cities = (City*)malloc(numberOfCitiesToTest * sizeof(City));

		PLACEMENT(i)->overallWeight = scoreCities(world, geography, numberOfCitiesToTest, PLACEMENT(i));
		if (numberOfTrials > 0 && PLACEMENT(i)->overallWeight > PLACEMENT(0)->overallWeight)
			std::swap(PLACEMENT(i), PLACEMENT(0));
	}

	return PLACEMENT(0)->cities;
}

void placeCities(World *world, Geography *geography, int numberOfCitiesToTest, int numberOfTrials)
{
	for (int i = 0; i < numberOfTrials; i++)
	{
		placements.push_back(new Placement());
	}

	// Store the top performing cities
	City* finalCities = performTrials(world, geography, numberOfCitiesToTest, numberOfTrials);

	for (int i = 0; i < numberOfCitiesToTest; i++)
	{
		float r = (float(rand())) / float(RAND_MAX);
		world->addObject((WorldObject*)(new CityObject(finalCities[i].x, finalCities[i].y, r = POPULATION_MIN + (POPULATION_MAX - POPULATION_MIN) * r)));
	}

	// Free pointers then empty the vector
	for(std::vector<Placement*>::iterator it = placements.begin(); it != placements.end(); ++it)
		delete (*it);
	placements.clear();
}