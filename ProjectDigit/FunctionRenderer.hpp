#pragma once

#include "Main.hpp"

#include <SFML/Graphics.hpp>

#include "Function.hpp"
#include "Script.hpp"
#include "CoordSystem.hpp"

using namespace std;

class FunctionRenderer {
public:

	void create(Script::DisplayFunction& func, string name, Color color = Color::White) {
		this->func = &func;
		this->color = color;
		this->name = name;

		rect = FloatRect(0.0f, 0.0f, 0.0f, 0.0f);
		unitL = 0.0;
	}

	void setParam(string name, double value) {
		params.push_back(pair<string, double>(name, value));
	}

	void update(RenderWindow& win, CoordSystem& coord) {
		Vector2f vCenter = win.getView().getCenter(), vSize = win.getView().getSize();
		FloatRect rect0 = FloatRect(vCenter.x - vSize.x / 2.0f, vCenter.y - vSize.y / 2.0f, vSize.x, vSize.y);
		double left = rect0.left / coord.unitLength - 0.0001, right = (rect0.left + rect0.width) / coord.unitLength + 0.0001;
		double top = -rect0.top / coord.unitLength + 20.0, down = -(rect0.top + rect0.height) / coord.unitLength - 20.0;
		double step = 1 / coord.unitLength;

		if (rect0 != rect || unitL != coord.unitLength) {
			rect = rect0;
			unitL = coord.unitLength;

			VertexArray vert;
			vert.setPrimitiveType(sf::PrimitiveType::LineStrip);
			vert.clear();
			verts.clear();

			for (pair<const string, shared_ptr<Variable>>& i : func->changeVal) {
				i.second->setValue(0.0);
			}

			//func->reinitalaizeConstVals();
			for (pair<string, double>& i : params) {
				func->changeVVal(i.first, i.second);
			}

			for (double i = left - step; i <= right + step; i += step) {
				func->changeXCoord(i);
				double d;
				if (!((d = func->calculate()) == NAN || d == INFINITY || d == -INFINITY || d > top || d < down)) {
					Vertex v;
					v.color = color;
					v.position = Vector2f(coord.convertCoordToDisplay(i, d));
					vert.append(v);
				}
				else {
					if (!vert.getVertexCount() == 0) {
						verts.push_back(vert);
						vert.clear();
					}
				}
			}
			if (!vert.getVertexCount() == 0)
				verts.push_back(vert);
		}
	}

	void render(RenderWindow& win, CoordSystem& coord) {
		for (VertexArray i : verts)
			win.draw(i);
	}

	void setColor(Color color) { this->color = color; }
	Color getColor() { return color; }

	string getName() { return name; }

	void forceUpdate() {
		rect = FloatRect(0.0f, 0.0f, 0.0f, 0.0f);
	}

private:

	Script::DisplayFunction* func;
	vector<pair<string, double>> params;

	vector<VertexArray> verts;
	FloatRect rect;
	double unitL;

	string name;

	Color color;

};

