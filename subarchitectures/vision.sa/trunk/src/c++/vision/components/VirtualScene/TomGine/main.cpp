 /**
 * @file main.cpp
 * @author Thomas Mörwald
 * @date October 2009
 * @version 0.1
 * @brief Main file for standalone version of TomGine rendering engine.
 */
 
#include <stdio.h>

#include "tgEngine.h"
#include "tgFont.h"
#include "tgRenderModel.h"
#include "tgModelLoader.h"
#include "tgBasicGeometries.h"
#include "tgSphere.h"

using namespace TomGine;
using namespace std;

typedef vector<vec3> PointList;

int main(int argc, char *argv[])
{
	float fTime;
	
	int i;
	PointList m_points;
	vec3 v;

	tgEngine render;
	render.Init(640,480, 1.0, 0.01, "TomGine Render Engine", true);

	tgFont m_font("/usr/share/fonts/truetype/freefont/FreeSerifBold.ttf");

	// Load Model
	// for more materials visit: http://wiki.delphigl.com/index.php/Materialsammlung
	tgRenderModel::Material matSilver;
	matSilver.ambient = vec4(0.19,0.19,0.19,1.0);
	matSilver.diffuse = vec4(0.51,0.51,0.51,1.0);
	matSilver.specular = vec4(0.77,0.77,0.77,1.0);
	matSilver.shininess = 51.2;
	
	tgRenderModel::Material matRed;
	matRed.ambient = vec4(0.3,0.3,0.3,1.0);
	matRed.diffuse = vec4(1.0,0.0,0.0,1.0);
	matRed.specular = vec4(0.5,0.5,0.5,1.0);
	matRed.shininess = 10.0;
		
	tgRenderModel cylinder;
	GenCylinder(cylinder, 0.05, 0.2, 32, 1);
	cylinder.m_material = matRed;
	
	tgRenderModel camera;
	tgModelLoader loader;
	loader.LoadPly(camera, "model/camera.ply");
	camera.m_material = matRed;
	
	tgRenderModel spheremodel;
	tgSphere sphere;
	sphere.CreateSphere(spheremodel, 0.1, 2, ICOSAHEDRON);
	spheremodel.m_material = matRed;
	
	// Rendering loop
	while(render.Update(fTime)){
		spheremodel.DrawFaces();
		
		render.Activate2D();
		m_font.Print("TomGine Render Engine", 18, 5, 5);
	}
	
	return 0;
}



