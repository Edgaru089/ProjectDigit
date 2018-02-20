#pragma once

#include "Main.hpp"

USING_NAMESPACE;

class CoordSystem {
public:

	Vector2f convertCoordToDisplay(double x, double y) {
		return Vector2f(x*unitLength, -y*unitLength);
	}

	Vector2d convertDisplayToCoord(float x, float y) {
		return Vector2d(x / unitLength, -y / unitLength);
	}

	void render(RenderWindow& win) {
		Vector2f vCenter = win.getView().getCenter(), vSize = win.getView().getSize();
		FloatRect rect = FloatRect(vCenter.x - vSize.x / 2.0f, vCenter.y - vSize.y / 2.0f, vSize.x, vSize.y);

		int top = -rect.top / unitLength, down = -(rect.top + rect.height) / unitLength;
		int left = rect.left / unitLength, right = (rect.left + rect.width) / unitLength;

		vector<Vertex> vert;

		//Horizional
		for (int i = down; i <= top; i++) {
			Vertex v1, v2;
			if (i == 0)
				continue;
			else
				v1.color = v2.color = coordLineColor;
			v1.position = Vector2f(rect.left, -i*unitLength);
			v2.position = Vector2f(rect.left + rect.width, -i*unitLength);
			vert.push_back(v1);
			vert.push_back(v2);
		}

		//Vertical
		for (int i = left; i <= right; i++) {
			Vertex v1, v2;
			if (i == 0)
				continue;
			else
				v1.color = v2.color = coordLineColor;
			v1.position = Vector2f(i*unitLength,rect.top);
			v2.position = Vector2f(i*unitLength,rect.top+rect.height);
			vert.push_back(v1);
			vert.push_back(v2);
		}

		//Axis
		Vertex v1, v2;
		v1.color = v2.color = coordAxisColor;
		v1.position = Vector2f(rect.left, 0.0f);
		v2.position = Vector2f(rect.left + rect.width, 0.0f);
		vert.push_back(v1);
		vert.push_back(v2);
		v1.position = Vector2f(0.0f, rect.top);
		v2.position = Vector2f(0.0f, rect.top + rect.height);
		vert.push_back(v1);
		vert.push_back(v2);

		win.draw(&vert[0], vert.size(), sf::PrimitiveType::Lines);
	}

	double unitLength;

	Color coordAxisColor;
	Color coordLineColor;

};
