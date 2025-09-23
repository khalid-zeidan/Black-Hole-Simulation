#pragma once
#include "allIncludes.h"
#include "BlackHole.h"
#include "Camera.h"
using namespace std;
using namespace glm;

class Scene
{
public:
	BlackHole sagittariusA;
	Camera camera;

	Scene(vec3 pos, double mass) : sagittariusA(pos, mass)
	{

	}

	void Update() 
	{

	}

	void Render() const
	{

	}
};

