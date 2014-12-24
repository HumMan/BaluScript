#pragma once

#include "Material.h"

class TBaluSpritePolygon
{
private:
	TBaluMaterial* material;

	TVec2 size;
	TVec2 origin;

	std::vector<TVec2> vertices;
	std::vector<TVec2> polygon_vertices;
	std::vector<TVec2> triangulated;

	TVec2 tex_coord_origin, tex_coord_scale;
	std::vector<TVec2> tex_coordinates;
	
	void SetPolygonFromTexture();
	void UpdatePolyVertices();
	void TriangulateGeometry();
public:
	TBaluSpritePolygon();

	void SetMaterial(TBaluMaterial* material);
	void SetPolygonVertices(std::vector<TVec2> polygon_vertices);
	void SetAsBox(float width, float height);
	void SetVertices(std::vector<TVec2> vertices);
	std::vector<TVec2> GetVertices();
	int GetVerticesCount();
	void SetVertex(int id, TVec2 pos);
	TVec2 GetPolygonVertex(int id);
	TVec2 GetVertex(int id);
	//void SetTexCoords(std::vector<TVec2> tex_coordinates);
	void SetTexCoordsFromVertices(TVec2 origin, TVec2 scale);
};