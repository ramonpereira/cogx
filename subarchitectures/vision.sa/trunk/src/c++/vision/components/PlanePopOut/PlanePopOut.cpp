/**
 * @author Kai ZHOU
 * @date June 2009
 */

#include <GL/freeglut.h>
#include <cogxmath.h>
#include "PlanePopOut.h"
#include <stack>
#include <vector>
#include <VideoUtils.h>
#include <cast/architecture/ChangeFilterFactory.hpp>

#ifdef FEAT_VISUALIZATION
#include <time.h>
long long gethrtime(void)
{
  struct timespec sp;
  int ret;
  long long v;
#ifdef CLOCK_MONOTONIC_HR
  ret=clock_gettime(CLOCK_MONOTONIC_HR, &sp);
#else
  ret=clock_gettime(CLOCK_MONOTONIC, &sp);
#endif
  if(ret!=0) return 0;
  v=1000000000LL; /* seconds->nanonseconds */
  v*=sp.tv_sec;
  v+=sp.tv_nsec;
  return v;
}
#endif

#define USE_PSO	0	//0=use RANSAC, 1=use PSO to estimate multiple planes

#define Shrink_SOI 1
#define Upper_BG 1.5
#define Lower_BG 1.1	// 1.1-1.5 radius of BoundingSphere
#define min_height_of_obj 0.03	//unit cm, due to the error of stereo, >0.01 is suggested
#define rate_of_centers 0.4	//compare two objs, if distance of centers of objs more than rate*old radius, judge two objs are different
#define ratio_of_radius 0.5	//compare two objs, ratio of two radiuses
#define Torleration 5		// Torleration error, even there are "Torleration" frames without data, previous data will still be used
				//this makes stable obj
#define MAX_V 0.1
#define label4initial		-3
#define label4plane		0	//0, -10, -20, -30... for multiple planes
#define label4objcandidant	-2
#define label4objs		1	//1,2,3,4.....for multiple objs
#define label4ambiguousness	-1

/**
 * The function called to create a new instance of our component.
 */
extern "C"
{
  cast::CASTComponentPtr newComponent()
  {
    return new cast::PlanePopOut();
  }
}

namespace cast
{
using namespace std;
using namespace Stereo;
using namespace cogx;
using namespace cogx::Math;
using namespace VisionData;
//using namespace navsa;
using namespace cdl;

int win;
int objnumber = 0;
double cam_trans[3];
double cam_rot[2];
int mouse_x, mouse_y;
int mouse_butt;
int butt_state;
Vector3 view_point, view_dir, view_up, view_normal;
GLfloat col_background[4];
GLfloat col_surface[4];
GLfloat col_overlay[4];
GLfloat col_highlight[4];

VisionData::SurfacePointSeq points;
VisionData::SurfacePointSeq pointsN;

VisionData::ObjSeq mObjSeq;
VisionData::Vector3Seq mConvexHullPoints;
Vector3 mCenterOfHull;
double mConvexHullRadius;
double mConvexHullDensity;

Vector3 pre_mCenterOfHull;
double pre_mConvexHullRadius;
std::string pre_id;

vector <int> points_label;  //0->plane; 1~999->objects index; -1->discarded points



double A, B, C, D;
int N;  // 1/N points will be used
bool mbDrawWire;
bool doDisplay;
int m_torleration;
Vector3 v3dmax;
Vector3 v3dmin;


void InitWin()
{
  GLfloat light_ambient[] = {0.4, 0.4, 0.4, 1.0};
  GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat light_specular[] = {0.0, 0.0, 0.0, 1.0};

  glClearColor(col_background[0], col_background[1], col_background[2],
      col_background[3]);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);

  // setup lighting
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // setup view point stuff
  cam_trans[0] = cam_trans[1] = cam_trans[2] = 0.;
  cam_rot[0] = cam_rot[1] = 0.;
  mouse_x = mouse_y = 0;
  mouse_butt = 0;
  butt_state = 0;

  // look in z direcction with y pointing downwards
  view_point = vector3(0.0, 0.0, 0.0);
  view_dir = vector3(0.0, 0.0, 1.0);
  view_up = vector3(0.0, -1.0, 0.0);
  view_normal = cross(view_dir, view_up);

  // black background
  col_background[0] = 0.0;
  col_background[1] = 0.0;
  col_background[2] = 0.0;
  col_background[3] = 1.0;

  // surfaces in white
  col_surface[0] = 1.0;
  col_surface[1] = 1.0;
  col_surface[2] = 1.0;
  col_surface[3] = 1.0;

  // highlighted things in light blue
  col_highlight[0] = 0.2;
  col_highlight[1] = 0.2;
  col_highlight[2] = 1.0;
  col_highlight[3] = 1.0;

  // overlay thingies (e.g. coordinate axes) in yellow
  col_overlay[0] = 1.0;
  col_overlay[1] = 1.0;
  col_overlay[2] = 0.0;
  col_overlay[3] = 1.0;

  mbDrawWire = true;
}

void ResizeWin(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45., (double)w/(double)h, 0.001, 10000.);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void DrawText3D(const char *text, double x, double y, double z)
{
  glRasterPos3d(x, y, z);
  while(*text != '\0')
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *text++);
}
/**
 * Draw things like coord frames
 */
void DrawOverlays()
{
  glColor4fv(col_overlay);

  // draw coordinate axes
  glBegin(GL_LINES);
  glVertex3d(-1000., 0., 0.);
  glVertex3d(1000., 0., 0.);
  glVertex3d(0., -1000., 0.);
  glVertex3d(0., 1000., 0.);
  glVertex3d(0., 0., -1000.);
  glVertex3d(0., 0., 1000.);
  glEnd();
  DrawText3D("x", 0.1, 0.02, 0.);
  DrawText3D("y", 0., 0.1, 0.02);
  DrawText3D("z", 0.02, 0., 0.1);

  // draw tics every m, up to 10 m
  const double tic_size = 0.05;
  glBegin(GL_LINES);
  for(int i = -10; i < 10; i++)
  {
    if(i != 0)
    {
      glVertex3d((double)i, 0, 0.);
      glVertex3d((double)i, tic_size, 0.);
      glVertex3d(0., (double)i, 0.);
      glVertex3d(0., (double)i, tic_size);
      glVertex3d(0., 0., (double)i);
      glVertex3d(tic_size, 0., (double)i);
    }
  }
  glEnd();
  char buf[100];
  for(int i = -10; i < 10; i++)
  {
    if(i != 0)
    {
      snprintf(buf, 100, "%d", i);
      DrawText3D(buf, (double)i, 2.*tic_size, 0.);
      DrawText3D(buf, 0., (double)i, 2.*tic_size);
      DrawText3D(buf, 2.*tic_size, 0., (double)i);
    }
  }
}

void DrawPlaneGrid()
{
	glLineWidth(1);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
///////////////////////////////////////////////////////////
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0,1.0,1.0);
	glVertex3f(v3dmax.x, v3dmax.y, v3dmax.z);
	glVertex3f(v3dmin.x, v3dmax.y, v3dmax.z);
	glVertex3f(v3dmin.x, v3dmin.y, v3dmin.z);
	glVertex3f(v3dmax.x, v3dmin.y, v3dmin.z);
	glVertex3f(v3dmin.x, v3dmax.y, v3dmax.z);
	glVertex3f(v3dmin.x, v3dmin.y, v3dmin.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glColor3f(1.0,1.0,1.0);
	glVertex3f(v3dmax.x, v3dmax.y, v3dmax.z);
	glVertex3f(v3dmin.x, v3dmax.y, v3dmax.z);
	glVertex3f(v3dmin.x+A, v3dmin.y+B, v3dmin.z+C);
	glVertex3f(v3dmax.x+A, v3dmax.y+B, v3dmax.z+C);
	glVertex3f(v3dmin.x, v3dmax.y, v3dmax.z);
	glVertex3f(v3dmin.x+A, v3dmin.y+B, v3dmin.z+C);
	glEnd();

	glDisable(GL_BLEND);
}

void DrawPointb(Vector3 v3p, GLbyte red, GLbyte green, GLbyte blue)
{
	glPointSize(2);
	glBegin(GL_POINTS);
	glColor3b(red,green,blue);
	glVertex3f(v3p.x, v3p.y, v3p.z);
	glEnd();
}
void DrawPointf(Vector3 v3p, GLfloat red, GLfloat green, GLfloat blue)
{
	glPointSize(2);
	glBegin(GL_POINTS);
	glColor3f(red,green,blue);
	glVertex3f(v3p.x, v3p.y, v3p.z);
	glEnd();
}

void DrawPoints()
{
  //cout<<"Drawing......"<<endl;
  glPointSize(2);
  glBegin(GL_POINTS);
  for(size_t i = 0; i < pointsN.size(); i++)
  {
	if (points_label.at(i) == 0)   		glColor3f(1.0,0.0,0.0); 
	else if (points_label.at(i) == -10)  	glColor3f(0.0,1.0,0.0);
	else if (points_label.at(i) == -20)  	glColor3f(0.0,0.0,1.0);
	else if (points_label.at(i) > 0)  	glColor3f(0.2,1.0,0.2);
	glVertex3f(pointsN[i].p.x, pointsN[i].p.y, pointsN[i].p.z);
  }
  glEnd();
/*
glPointSize(20);
glColor3ub(255, 255, 255);
glBegin(GL_POINTS);
glVertex3f(0.0, 0.0, 0.0);
glEnd();
*/
}

void DisplayWin()
{
	GLfloat light_position[] = {2.0, -2.0, 1.0, 1.0};

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(view_point.x, view_point.y, view_point.z,
	view_point.x + view_dir.x, view_point.y + view_dir.y,
	view_point.z + view_dir.z,
	view_up.x, view_up.y, view_up.z);
	glRotated(cam_rot[0], 0., 1., 0.);
	glRotated(-cam_rot[1], 1., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glDisable(GL_LIGHTING);
	DrawOverlays();
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	DrawPoints();

	glDisable(GL_COLOR_MATERIAL);
	glutSwapBuffers();

}

void KeyPress(unsigned char key, int x, int y)
{
  switch(key)
  {/*
    case 'q':
      // a slightly harsh way to end a program ...
      exit(EXIT_SUCCESS);
      break;*/
    case 's':
	{if (mbDrawWire) mbDrawWire = false;
	 else mbDrawWire = true;}
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

void MousePress(int button, int state, int x, int y)
{
  mouse_x = x;
  mouse_y = y;
  mouse_butt = button;
  butt_state = state;
}

void MouseMove(int x, int y)
{
  double trans_scale = 0.005, rot_scale = 1.;
  double delta_x = (double)(x - mouse_x);
  double delta_y = (double)(y - mouse_y);

  if(mouse_butt == GLUT_LEFT_BUTTON)
  {
    view_point += (view_up*delta_y - view_normal*delta_x)*trans_scale;
  }
  else if(mouse_butt == GLUT_MIDDLE_BUTTON)
  {
    view_point -= view_dir*delta_y*trans_scale;
  }
  else if(mouse_butt == GLUT_RIGHT_BUTTON)
  {
    cam_rot[0] += (GLfloat)delta_x/rot_scale;
    cam_rot[1] += (GLfloat)delta_y/rot_scale;
  }
  mouse_x = x;
  mouse_y = y;
  glutPostRedisplay();
}

void show_window()
{
    glutPostRedisplay();
}


void PlanePopOut::configure(const map<string,string> & _config)
{
  // first let the base classes configure themselves
  configureStereoCommunication(_config);

  map<string,string>::const_iterator it;

  useGlobalPoints = true;
  doDisplay = false;
  if((it = _config.find("--globalPoints")) != _config.end())
  {
    istringstream str(it->second);
    str >> boolalpha >> useGlobalPoints;
  }
  if((it = _config.find("--display")) != _config.end())
  {
	doDisplay = true;
  }
  println("use global points: %d", (int)useGlobalPoints);
  m_torleration = 0;
  mConvexHullDensity = 0.0;
  pre_mCenterOfHull.x = pre_mCenterOfHull.y = pre_mCenterOfHull.z = 0.0;
  pre_mConvexHullRadius = 0.0;
  pre_id = "";

#ifdef FEAT_VISUALIZATION
  m_display.configureDisplayClient(_config);
#endif
}

#ifdef FEAT_VISUALIZATION
	#define ID_POPOUT_POINTS "popout.show.points"
	#define ID_POPOUT_PLANEGRID "popout.show.planegrid"
	#define ID_POPOUT_IMAGE "popout.show.image"
#endif

void PlanePopOut::start()
{
  startStereoCommunication(*this);

  int argc = 1;
  char argv0[] = "PlanePopOut";
  char *argv[1] = {argv0};

  if (doDisplay)
  {
  glutInit(&argc, argv);
  win = glutCreateWindow("points");
  InitWin();
  glutKeyboardFunc(KeyPress);
  glutMouseFunc(MousePress);
  glutMotionFunc(MouseMove);
  glutReshapeFunc(ResizeWin);
  glutDisplayFunc(DisplayWin);
  }
#ifdef FEAT_VISUALIZATION
  m_bSendPoints = false;
  m_bSendPlaneGrid = false;
  m_bSendImage = true;
  m_display.connectIceClient(*this);
  m_display.setClientData(this);
  m_display.installEventReceiver();
  m_display.addCheckBox("PlanePopout", ID_POPOUT_POINTS, "Show 3D points");
  m_display.addCheckBox("PlanePopout", ID_POPOUT_PLANEGRID, "Show plane grid");
  m_display.addCheckBox("PlanePopout", ID_POPOUT_IMAGE, "Show image");
#endif
}

#ifdef FEAT_VISUALIZATION
void PlanePopOut::CDisplayClient::handleEvent(const Visualization::TEvent &event)
{
	if (!pPopout) return;
	if (event.sourceId == ID_POPOUT_POINTS) {
		if (event.data == "0" || event.data=="") pPopout->m_bSendPoints = false;
		else pPopout->m_bSendPoints = true;
	}
	else if (event.sourceId == ID_POPOUT_PLANEGRID) {
		if (event.data == "0" || event.data=="") pPopout->m_bSendPlaneGrid = false;
		else pPopout->m_bSendPlaneGrid = true;
	}
	else if (event.sourceId == ID_POPOUT_IMAGE) {
		if (event.data == "0" || event.data=="") pPopout->m_bSendImage = false;
		else pPopout->m_bSendImage = true;
	}
}

std::string PlanePopOut::CDisplayClient::getControlState(const std::string& ctrlId)
{
	if (!pPopout) return "";
	if (ctrlId == ID_POPOUT_POINTS) {
		if (pPopout->m_bSendPoints) return "2";
		else return "0";
	}
	if (ctrlId == ID_POPOUT_PLANEGRID) {
		if (pPopout->m_bSendPlaneGrid) return "2";
		else return "0";
	}
	if (ctrlId == ID_POPOUT_IMAGE) {
		if (pPopout->m_bSendImage) return "2";
		else return "0";
	}
	return "";
}

void SendImage(VisionData::SurfacePointSeq points, std::vector <int> &labels, Video::Image img, cogx::display::CDisplayClient& m_display, PlanePopOut *powner)
{
    IplImage *iplImg = convertImageToIpl(img);
    Video::CameraParameters c = img.camPars;

    for (unsigned int i=0 ; i<points.size() ; i++)
    {
	int m_label = labels.at(i);
	switch (m_label)
	{
	  case 0:	cvCircle(iplImg, powner->ProjectPointOnImage(points.at(i).p,c), 2, CV_RGB(255,0,0)); break;
	  case -10:	cvCircle(iplImg, powner->ProjectPointOnImage(points.at(i).p,c), 2, CV_RGB(0,255,0)); break;
	  case -20:	cvCircle(iplImg, powner->ProjectPointOnImage(points.at(i).p,c), 2, CV_RGB(0,0,255)); break;
	}
    }
    cvSaveImage("/tmp/planes_image.jpg", iplImg);
    //m_display.setImage("PlanePopout", iplImg);
    cvReleaseImage(&iplImg);
}

void SendPoints(const VisionData::SurfacePointSeq& points, std::vector<int> &labels,
  cogx::display::CDisplayClient& m_display, PlanePopOut *powner)
{
	long long t0 = gethrtime();
	std::ostringstream str;
	str << "function render()\nglPointSize(2)\nglBegin(GL_POINTS)\n";
	int plab = -9999;
	for(size_t i = 0; i < points.size(); i++)
	{
		int lab = labels.at(i);
		if (plab != lab) {
			plab = lab;
			switch (lab) {
				case 0: str << "glColor(1.0,0.0,0.0)\n"; break;
				case -10: str << "glColor(0.0,1.0,0.0)\n"; break;
				case -20: str << "glColor(0.0,0.0,1.0)\n"; break;
				case -30: str << "glColor(0.0,1.0,1.0)\n"; break;
				default: str << "glColor(1.0,1.0,0.0)\n"; break;
			}
		}
		const VisionData::SurfacePoint &p = points[i];
		str << "glVertex(" << p.p.x << "," << p.p.y << "," << p.p.z << ")\n";
	}
	str << "glEnd()\nend\n";
	long long t1 = gethrtime();
	double dt = (t1 - t0) * 1e-6;
	powner->log("*****: %d points; Time to create script %lfms", points.size(), dt);
	m_display.setLuaGlObject("PlanePopout", "3D points", str.str());
	t1 = gethrtime();
	dt = (t1 - t0) * 1e-6;
	powner->log("*****GL: %ld points sent after %lfms", points.size(), dt);
}

void SendPlaneGrid(cogx::display::CDisplayClient& m_display, PlanePopOut *powner)
{
	long long t0 = gethrtime();
	std::ostringstream str;
	str << "function render()\nglPointSize(2)\nglBegin(GL_POINTS)\n";

	// See: DrawPlaneGrid
	str << "glBegin(GL_LINE_LOOP);\n";
	str << "glColor(1.0,1.0,1.0);\n";
	str << "glVertex(" << v3dmax.x << "," << v3dmax.y << "," << v3dmax.z << ");\n";
	str << "glVertex(" << v3dmin.x << "," << v3dmax.y << "," << v3dmax.z << ");\n";
	str << "glVertex(" << v3dmin.x << "," << v3dmin.y << "," << v3dmin.z << ");\n";
	str << "glVertex(" << v3dmax.x << "," << v3dmin.y << "," << v3dmin.z << ");\n";
	str << "glVertex(" << v3dmin.x << "," << v3dmax.y << "," << v3dmax.z << ");\n";
	str << "glVertex(" << v3dmin.x << "," << v3dmin.y << "," << v3dmin.z << ");\n";
	str << "glEnd();\n";

	str << "glBegin(GL_LINE_LOOP);\n";
	str << "glColor(1.0,1.0,1.0);\n";
	str << "glVertex(" << v3dmax.x << "," << v3dmax.y << "," << v3dmax.z << ");\n";
	str << "glVertex(" << v3dmin.x << "," << v3dmax.y << "," << v3dmax.z << ");\n";
	str << "glVertex(" << v3dmin.x+A << "," << v3dmin.y+B << "," << v3dmin.z+C << ");\n";
	str << "glVertex(" << v3dmax.x+A << "," << v3dmax.y+B << "," << v3dmax.z+C << ");\n";
	str << "glVertex(" << v3dmin.x << "," << v3dmax.y << "," << v3dmax.z << ");\n";
	str << "glVertex(" << v3dmin.x+A << "," << v3dmin.y+B << "," << v3dmin.z+C << ");\n";
	str << "glEnd();\n";

	str << "end\n";

	m_display.setLuaGlObject("PlanePopout", "PlaneGrid", str.str());
	long long t1 = gethrtime();
	double dt = (t1 - t0) * 1e-6;
	powner->log("*****GL: Plane grid sent after %lfms", dt);
}

void SendOverlays(cogx::display::CDisplayClient& m_display, PlanePopOut *powner)
{
  std::ostringstream str;
  str << "function render()\n";
  str << "glColor(1.0,1.0,0.0)\n";
  str << "glBegin(GL_LINES)\n";
  str << "glVertex(-1000., 0., 0.)\n";
  str << "glVertex(1000., 0., 0.)\n";
  str << "glVertex(0., -1000., 0.)\n";
  str << "glVertex(0., 1000., 0.)\n";
  str << "glVertex(0., 0., -1000.)\n";
  str << "glVertex(0., 0., 1000.)\n";
  str << "glEnd()\n";
  //str << "DrawText3D(\"x\", 0.1, 0.02, 0.)\n";
  //str << "DrawText3D(\"y\", 0., 0.1, 0.02)\n";
  //str << "DrawText3D(\"z\", 0.02, 0., 0.1)\n";

  // draw tics every m, up to 10 m
  str << "tic_size = 0.05\n";
  str << "glBegin(GL_LINES)\n";
  str << "for i=-10,10 do\n";
    str << "if i ~= 0 then\n";
      str << "glVertex(i, 0, 0.)\n";
      str << "glVertex(i, tic_size, 0.)\n";
      str << "glVertex(0., i, 0.)\n";
      str << "glVertex(0., i, tic_size)\n";
      str << "glVertex(0., 0., i)\n";
      str << "glVertex(tic_size, 0., i)\n";
    str << "end\n";
  str << "end\n";
  str << "glEnd()\n";
  //str << "for(int i = -10; i < 10; i++)\n";
  //  str << "if(i != 0)
  //    snprintf(buf, 100, "%d", i);
  //    DrawText3D(buf, (double)i, 2.*tic_size, 0.);
  //    DrawText3D(buf, 0., (double)i, 2.*tic_size);
  //    DrawText3D(buf, 2.*tic_size, 0., (double)i);
  //  }
  //}
  str << "end\n";
  m_display.setLuaGlObject("PlanePopout", "Overlays", str.str());
}
#endif

void PlanePopOut::runComponent()
{
  sleepComponent(1000);
#ifdef FEAT_VISUALIZATION
  //SendOverlays(m_display, this);
#endif
  while(isRunning())
  {
	VisionData::SurfacePointSeq tempPoints = points;
	points.resize(0);

	getPoints(useGlobalPoints, points);
	
	Video::Image image;
	getRectImage(LEFT, image);
	if (points.size() == 0)
	{
		points = tempPoints;
		log("Attention: getpoints() gets ZERO point!!");
	}
	else
	{	//cout<<"we get "<<points.size()<<" points"<<endl;
		tempPoints.clear();
		pointsN.clear();
		objnumber = 0;
		N = (int)points.size()/2500;
		for (VisionData::SurfacePointSeq::iterator it=points.begin(); it<points.end(); it+=N)
			pointsN.push_back(*it);
		points_label.clear();
		points_label.assign(pointsN.size(), -3);

		bool executeflag = false;
		if (USE_PSO == 0)		executeflag = RANSAC(pointsN,points_label);
		else if (USE_PSO == 1)		executeflag = PSO_Label(pointsN,points_label);
		if (executeflag = true)
		{	//cout<<"after ransac we have "<<points.size()<<" points"<<endl;
			SplitPoints(pointsN,points_label);
			if (objnumber != 0)
			{
				ConvexHullOfPlane(pointsN,points_label);
  				BoundingPrism(pointsN,points_label);
				DrawCuboids(pointsN,points_label); //cal bounding Cuboids and centers of the points cloud
 				BoundingSphere(pointsN,points_label); // get bounding spheres, SOIs and ROIs
				//cout<<"m_bSendImage is "<<m_bSendImage<<endl;
#ifdef FEAT_VISUALIZATION
				if (m_bSendImage) 
				{
				    m_bSendPoints = false;
				    m_bSendPlaneGrid = false;
				    SendImage(pointsN,points_label,image, m_display, this);
				    //cout<<"send Imgs"<<endl;
				}
#endif
			}
			else
			{
				v3size.clear();
				v3center.clear();
				vdradius.clear();
				//cout<<"there is no objects, now strating cal convex hull"<<endl;
				ConvexHullOfPlane(pointsN,points_label);
			}
			if (doDisplay)
			{
				//glutIdleFunc(show_window);
				glutPostRedisplay();
				glutMainLoopEvent();
			}
			AddConvexHullinWM();
#ifdef FEAT_VISUALIZATION
			if (m_bSendPoints) SendPoints(pointsN, points_label, m_display, this);
			if (m_bSendPlaneGrid) SendPlaneGrid(m_display, this);
#endif
		}
		else	log("Wrong with the execution of Plane fitting!");
	}
	if (para_a!=0.0 || para_b!=0.0 || para_c!=0.0 || para_d!=0.0)
	{
		CurrentObjList.clear();
		Pre2CurrentList.clear();
		for(unsigned int i=0; i<v3center.size(); i++)  //create objects
		{
			ObjPara OP;
			OP.c = v3center.at(i);
			OP.s = v3size.at(i);
			OP.r = vdradius.at(i);
			OP.id = "";
			OP.bComCurrentPre = false;
			OP.bInWM = false;
			OP.count = 0;
			OP.pointsInOneSOI = SOIPointsSeq.at(i);
			OP.BGInOneSOI = BGPointsSeq.at(i);
			OP.EQInOneSOI = EQPointsSeq.at(i);
			CurrentObjList.push_back(OP);
		}
		if (PreviousObjList.empty())
		{
			for(unsigned int i=0; i<CurrentObjList.size(); i++)
			{	
			  CurrentObjList.at(i).count++;
			  PreviousObjList.push_back(CurrentObjList.at(i));
			}
		}
		else
		{
		    for (unsigned int k=0; k<PreviousObjList.size(); k++)
			log("Previous objects center are: object %u center is (%f, %f, %f)", k, PreviousObjList.at(k).c.x, PreviousObjList.at(k).c.y, PreviousObjList.at(k).c.z);
		    for (unsigned int j=0; j<PreviousObjList.size(); j++)
		    {
			bool deleteObjFlag = true; // if this flag is still true after compare, then this object should be deleted from the WM
			for(unsigned int i=0; i<CurrentObjList.size(); i++)
			{
			    if (CurrentObjList.at(i).bComCurrentPre == false)
			    {
				if(Compare2SOI(CurrentObjList.at(i), PreviousObjList.at(j)) == true)
				{
				    deleteObjFlag = false;
				    CurrentObjList.at(i).bComCurrentPre = true;
				    if(PreviousObjList.at(j).bInWM == true)
				    {
					CurrentObjList.at(i).bInWM = true;
					CurrentObjList.at(i).id = PreviousObjList.at(j).id;
					CurrentObjList.at(i).count = PreviousObjList.at(j).count;
					if (dist(CurrentObjList.at(i).c, PreviousObjList.at(j).c)/norm(CurrentObjList.at(i).c) > 1/2)
					  //(abs((CurrentObjList.at(i).c.y-PreviousObjList.at(j).c.y)/CurrentObjList.at(i).c.y)>0.2)
					{
					    //cout<<"Current = "<<CurrentObjList.at(i).c.y<<"  Previous = "<<PreviousObjList.at(j).c.y<<endl;
					    CurrentObjList.at(i).c = PreviousObjList.at(j).c*4/5 + CurrentObjList.at(i).c/5;					
					    SOIPtr obj = createObj(CurrentObjList.at(i).c, CurrentObjList.at(i).s, CurrentObjList.at(i).r,CurrentObjList.at(i).pointsInOneSOI, CurrentObjList.at(i).BGInOneSOI, CurrentObjList.at(i).EQInOneSOI);
					    overwriteWorkingMemory(CurrentObjList.at(i).id, obj);
					    //cout<<"Overwrite!! ID of the overwrited SOI = "<<CurrentObjList.at(i).id<<endl;
					}
					else
					{
					    CurrentObjList.at(i).c = PreviousObjList.at(j).c;
					}
				    }
				    else
				    {
					CurrentObjList.at(i).count = PreviousObjList.at(j).count+1;
					if (CurrentObjList.at(i).count >= Torleration)
					{
					    CurrentObjList.at(i).bInWM =true;
					    CurrentObjList.at(i).id = newDataID();
					    SOIPtr obj = createObj(CurrentObjList.at(i).c, CurrentObjList.at(i).s, CurrentObjList.at(i).r, CurrentObjList.at(i).pointsInOneSOI, CurrentObjList.at(i).BGInOneSOI, CurrentObjList.at(i).EQInOneSOI);
					    addToWorkingMemory(CurrentObjList.at(i).id, obj);
					    log("Add an New Object in the WM, id is %s", CurrentObjList.at(i).id.c_str());
					    log("objects number = %u",objnumber);
					    //cout<<"New!! ID of the added SOI = "<<CurrentObjList.at(i).id<<endl;
					}
				    }
				    break;
				}
			    }
			}
			if (deleteObjFlag == true)
			{
			    if (PreviousObjList.at(j).bInWM == true)
			    {
				PreviousObjList.at(j).count = PreviousObjList.at(j).count-1;
				if(PreviousObjList.at(j).count > 0) Pre2CurrentList.push_back(PreviousObjList.at(j));
				else 
				{
				  //cout<<"count of obj = "<<PreviousObjList.at(j).count<<endl;
				  deleteFromWorkingMemory(PreviousObjList.at(j).id);
				  //cout<<"Delete!! ID of the deleted SOI = "<<PreviousObjList.at(j).id<<endl;
				}
			    }
			}
		    }
		    PreviousObjList.clear();
		    for (unsigned int i=0; i<CurrentObjList.size(); i++)
		    {
			if (CurrentObjList.at(i).bComCurrentPre == false)  CurrentObjList.at(i).count ++;
			PreviousObjList.push_back(CurrentObjList.at(i));
		    }
		    if (Pre2CurrentList.size()>0)
			for (unsigned int i=0; i<Pre2CurrentList.size(); i++)
			    PreviousObjList.push_back(Pre2CurrentList.at(i));
		}
	}

//cout<<"SOI in the WM = "<<PreviousObjList.size()<<endl;
    // wait a bit so we don't hog the CPU
    sleepComponent(50);
  }
}

void PlanePopOut::CalRadiusCenter4BoundingSphere(VisionData::SurfacePointSeq points, Vector3 &c, double &r)
{
    for (unsigned int i = 0 ; i<points.size() ; i++)
    {
	c = c+points.at(i).p;
    }
    c = c/points.size();
    for (unsigned int i = 0 ; i<points.size() ; i++)
    {
	double d = dist(points.at(i).p,c);
	if (d>r)	r = d;
    }
}

vector<double> PlanePopOut::Hypo2ParaSpace(vector<Vector3> vv3Hypo)
{
     if (vv3Hypo.size() != 3)
     {
	// need three points to determine a plane
	std::cout << " size =  " <<vv3Hypo.size()<< std::endl;
	vector<double> r;
	r.assign(4,0.0);
	return(r);
     }

    double para_a, para_b, para_c, para_d;
    para_a = ( (vv3Hypo.at(1).y-vv3Hypo.at(0).y)*(vv3Hypo.at(2).z-vv3Hypo.at(0).z)-(vv3Hypo.at(1).z-vv3Hypo.at(0).z)*(vv3Hypo.at(2).y-vv3Hypo.at(0).y) );
    para_b = ( (vv3Hypo.at(1).z-vv3Hypo.at(0).z)*(vv3Hypo.at(2).x-vv3Hypo.at(0).x)-(vv3Hypo.at(1).x-vv3Hypo.at(0).x)*(vv3Hypo.at(2).z-vv3Hypo.at(0).z) );
    para_c = ( (vv3Hypo.at(1).x-vv3Hypo.at(0).x)*(vv3Hypo.at(2).y-vv3Hypo.at(0).y)-(vv3Hypo.at(1).y-vv3Hypo.at(0).y)*(vv3Hypo.at(2).x-vv3Hypo.at(0).x) );
    para_d = ( 0-(para_a*vv3Hypo.at(0).x+para_b*vv3Hypo.at(0).y+para_c*vv3Hypo.at(0).z) );
    double temp = sqrt(para_a*para_a+para_b*para_b+para_c*para_c);
    
    vector<double> ABCD;
    ABCD.push_back(para_a/temp);
    ABCD.push_back(para_b/temp);
    ABCD.push_back(para_c/temp);
    ABCD.push_back(para_d/temp);
    
    return ABCD;
}

CvPoint PlanePopOut::ProjectPointOnImage(Vector3 p, const Video::CameraParameters &cam)
{
    cogx::Math::Vector2 p2 = projectPoint(cam, p);
    int x = p2.x;
    int y = p2.y;
    CvPoint re;
    re.x = x; re.y = y;
    return re;
}

Vector3 PlanePopOut::ProjectPointOnPlane(Vector3 p, double A, double B, double C, double D)
{
    if (A*p.x+B*p.y+C*p.z+D == 0)
	return p;
    Vector3 r;
    r.x = ((B*B+C*C)*p.x-A*(B*p.y+C*p.z+D))/(A*A+B*B+C*C);
    r.y = ((A*A+C*C)*p.y-B*(A*p.x+C*p.z+D))/(A*A+B*B+C*C);
    r.z = ((A*A+B*B)*p.z-C*(A*p.x+B*p.y+D))/(A*A+B*B+C*C);
    return r;
}

double PlanePopOut::DistOfParticles(Particle p1, Particle p2, Vector3 c, double r, bool& bParallel)
{    
    Vector3 i1 = ProjectPointOnPlane(c,p1.p.at(0),p1.p.at(1),p1.p.at(2),p1.p.at(3));
    Vector3 i2 = ProjectPointOnPlane(c,p2.p.at(0),p2.p.at(1),p2.p.at(2),p2.p.at(3));
    Vector3 v1 = i1-c;		//cout<<"v1 = "<<v1<<endl;
    Vector3 v2 = i2-c;		//cout<<"v2 = "<<v2<<endl;
    double d1 = length(v1);	//cout<<"d1 = "<<d1<<endl;	cout<<"r = "<<r<<endl;
    double d2 = length(v2);
    double costheta = dot(v1,v2)/d1/d2;
    if (costheta == 0)	return abs(p1.p.at(0)*p2.p.at(3)/p2.p.at(0)-p1.p.at(3))/2/r;
    double sintheta = sin(acos(costheta));
    Vector2 l1i1, l1i2, l2i1, l2i2;
    l1i1.x = d1;  l1i1.y = sqrt(r*r-d1*d1);
    l1i2.x = d1;  l1i2.y = -sqrt(r*r-d1*d1);
    double k = -costheta/sintheta;
    double b = d2*sqrt(k*k+1);
                //cout<<"b^2-4ac = "<<4*k*k*b*b-4*(1+k*k)*(b*b-r*r)<<endl;
    l2i1.x = (-2*k*b+sqrt(4*k*k*b*b-4*(1+k*k)*(b*b-r*r)))/2/(1+k*k); l2i1.y = k*l2i1.x+b;
    l2i2.x = (-2*k*b-sqrt(4*k*k*b*b-4*(1+k*k)*(b*b-r*r)))/2/(1+k*k); l2i2.y = k*l2i2.x+b;

    double d13, d23, d24, d14;
    d13 = dist(l1i1, l2i1);  d23 = dist(l1i2, l2i1);  d24 = dist(l1i2, l2i2);  d14 = dist(l1i1, l2i2);
    //cout<<"dist = ["<<d13<<" "<<d23<<" "<<d24<<" "<<d14<<"]"<<endl;
    vector <double> list;
    list.assign(4,0.0);	list.at(0)=d13; list.at(1)=d23; list.at(2)=d24; list.at(3)=d14;
    sort(list.begin(),list.end());
    double re;
    if (list.at(0)==d13)	re = d24/2/r;
    if (list.at(0)==d23)	re = d14/2/r;
    if (list.at(0)==d24)	re = d13/2/r;
    if (list.at(0)==d14)	re = d23/2/r;
    
    if (abs(costheta)>0.85) bParallel = true;
    return re;
}

double PlanePopOut::PSO_EvaluateParticle(Particle OneParticle, vector <Particle> optima_found, VisionData::SurfacePointSeq points, Vector3 cc, double rr)
{
      double dSumError = 0.0;
      double lambda = 50;
      for(unsigned int i=0; i<points.size(); i++)
      {
	      double dNormDist = abs(OneParticle.p.at(0)*points.at(i).p.x+OneParticle.p.at(1)*points.at(i).p.y+OneParticle.p.at(2)*points.at(i).p.z+OneParticle.p.at(3))
				/sqrt(OneParticle.p.at(0)*OneParticle.p.at(0)+OneParticle.p.at(1)*OneParticle.p.at(1)+OneParticle.p.at(2)*OneParticle.p.at(2));
	      if(dNormDist == 0.0)
	      continue;
	      if(dNormDist > min_height_of_obj)
		  dNormDist = min_height_of_obj;
	      dSumError += dNormDist;
      }
      if (optima_found.size() == 0)
	  return dSumError;
      else
      {
	  vector <double> weight;
	  weight.assign(optima_found.size(),0.0);
	  bool bP = false;
	  for (unsigned int i = 0; i<optima_found.size(); i++)
	  {
	      weight.at(i) = DistOfParticles(OneParticle,optima_found.at(i),cc,rr,bP);
	      weight.at(i) =lambda*exp(-lambda*weight.at(i))+1;
	  }
	  double r = 1.0;
	  for (unsigned int i = 0; i<optima_found.size(); i++)
	      r = r*weight.at(i);
	  return r*dSumError;
      }
}

vector<double> PlanePopOut::UpdatePosition(vector<double> p, vector<double> v)
{
    if (p.size() != v.size())
    {
	cout<<"the dimentions of position and velocity are different = "<<endl;//error
	exit(0);
    }
    
    vector<double> r = p;
    for (unsigned int i=0; i<r.size(); i++)
    {
	r.at(i) = p.at(i)+v.at(i);
    }
    return r;
}

vector<double> PlanePopOut::UpdateVelocity(vector<double> p, vector<double> v, vector<double> pbest, vector<double> gbest, float chi, float c1, float c2, float w)
{
    if (p.size() != v.size())
    {
	cout<<"the dimentions of position and velocity are different = "<<endl;//error
	exit(0);
    }
    
    vector<double> r = v;
    double r1, r2;
    for (unsigned int i=0; i<r.size(); i++)
    {
	r1 = rand()/(double)RAND_MAX;
	srand((unsigned) time (NULL));
	r2 = rand()/(double)RAND_MAX;
	r.at(i) = chi*(w*v.at(i)+c1*r1*(pbest.at(i)-p.at(i))+c2*r2*(gbest.at(i)-p.at(i)));
	if (r.at(i)>MAX_V) r.at(i) = MAX_V;
    }

    return r;
}

void PlanePopOut::Reinitialise_Parallel(vector<Particle>& vPar, vector<Particle>& vT, vector<Particle> vFO, VisionData::SurfacePointSeq points, Vector3 cc, double rr)
{
    if (vFO.at(0).p.at(3)<0)
    {vFO.at(0).p.at(0)=-vFO.at(0).p.at(0); vFO.at(0).p.at(1)=-vFO.at(0).p.at(1); vFO.at(0).p.at(2)=-vFO.at(0).p.at(2); vFO.at(0).p.at(3)=-vFO.at(0).p.at(3);}
    Vector3 vnorm; vnorm.x=vFO.at(0).p.at(0); vnorm.y=vFO.at(0).p.at(1); vnorm.z=vFO.at(0).p.at(2); 
    double min_dist = 9999999.0;
    for (unsigned int i = 0; i<points.size(); i++)
    {
	double dd = -dot(vnorm,points.at(i).p);
	if (dd<min_dist)	min_dist = dd;
    }
    for (unsigned int i = 0; i<vPar.size(); i++)
    {
	vPar.at(i).p.at(0)=vFO.at(0).p.at(0); vPar.at(i).p.at(1)=vFO.at(0).p.at(1); vPar.at(i).p.at(2)=vFO.at(0).p.at(2);
	vPar.at(i).p.at(3) = (vFO.at(0).p.at(3)-min_dist)*(i+1)/vPar.size()+min_dist;
	vPar.at(i).v.at(0)=0.0; vPar.at(i).v.at(1)=0.0; vPar.at(i).v.at(2)=0.0; 
	srand((unsigned) time (NULL));
	vPar.at(i).v.at(3)=-1+rand()%200/100;
	vPar.at(i).fCurr = PSO_EvaluateParticle(vPar.at(i),vFO,points,cc,rr);
	vPar.at(i).pbest = vPar.at(i).p;
	if (vPar.at(i).fCurr<vT.at(vT.size()-1).fCurr)
	{
	    vT.at(vT.size()-1)=vPar.at(i);
	    sort(vT.begin(),vT.end());
	}
    }
}

void PlanePopOut::PSO_internal(vector < vector<double> > init_positions,
						   VisionData::SurfacePointSeq &points, 
						   std::vector <int> &labels)
{
     int N_iter = 200; 				// iteration number
     int N_particle = init_positions.size();	// particle number
     float w = 1;				// inertia weight
     float c1 = 2.0;				// cognitive factor
     float c2 = 2.1;				// social factor
     float chi = 0.792;				// constriction factor
     
     int N_tour = 3;				// number of tournament best particles
     vector <Particle> mvFoundOptima;		// vector including all the optima found
     Vector3 cc;  cc.x = 0; cc.y = 0; cc.z = 0;
     double rr = 0;
     CalRadiusCenter4BoundingSphere(points,cc,rr);
     unsigned int nPoints = points.size();
     /* initialise velocity and position for each particle */
     //std::cout << "  start the particle swarm optimization" << std::endl;
     vector <Particle> mvParticle;
     mvParticle.assign(N_particle,InitialParticle());
     vector <Particle> mTournament;
     mTournament.assign(N_tour,InitialParticle());

     for (int i=0; i<N_particle; i++)
     {
	mvParticle.at(i).p = init_positions.at(i);
	vector <double> init_velocity;
	for (unsigned int j = 0; j<4; j++)
	{
	    srand((unsigned) time (NULL));
	    init_velocity.push_back(-1+rand()%200/100); //-1~1 rand number
	}
	mvParticle.at(i).v = init_velocity;	
	mvParticle.at(i).fCurr = PSO_EvaluateParticle(mvParticle.at(i),mvFoundOptima,points,cc,rr);
	mvParticle.at(i).fbest = mvParticle.at(i).fCurr;
	mvParticle.at(i).pbest = mvParticle.at(i).p;
	if (mvParticle.at(i).fCurr<mTournament.at(N_tour-1).fCurr)
	{
	    mTournament.at(N_tour-1)=mvParticle.at(i);
	    sort(mTournament.begin(),mTournament.end());
	}
     }
     vector <Particle> mvParticle_bak;
     mvParticle_bak.assign(N_particle,InitialParticle());
     mvParticle_bak = mvParticle;
     
     //std::cout << "  finish the initialisation" << std::endl;
     /* PSO iterations */
     int count = 0;
     Particle p_previous = InitialParticle();
     double dist_threshold = 0.1;	 	// to measure the distance between optima found of continuous iterations
     int min_iter_time_4_optima_found = 60;	// to determine the stability of the optimum found
     for (int i=0; i<N_iter; i++)
     {
	p_previous = mTournament.at(0);	
	for (int j=0; j<N_particle; j++)
	{	
	  //cout<<"before update velocity, v ="<<mvParticle.at(j).v.at(0)<<endl;
	    mvParticle.at(j).v = UpdateVelocity(mvParticle.at(j).p, mvParticle.at(j).v, mvParticle.at(j).pbest, mTournament.at(0).p, chi, c1, c2, w);
	  //cout<<"after update velocity, v ="<<mvParticle.at(j).v.at(0)<<endl;
	    mvParticle.at(j).p = UpdatePosition(mvParticle.at(j).p, mvParticle.at(j).v);
	    //cout<<"after update Position, p ="<<mvParticle.at(j).p.at(0)<<endl;
	    mvParticle.at(j).fCurr = PSO_EvaluateParticle(mvParticle.at(j),mvFoundOptima,points,cc,rr);
	    //cout<<"fitness value of "<<j<<" particle ="<<mvParticle.at(j).fCurr<<endl;
	    if (mvParticle.at(j).fCurr < mvParticle.at(j).fbest)	// update pbest
	    {
		mvParticle.at(j).fbest = mvParticle.at(j).fCurr;
		mvParticle.at(j).pbest = mvParticle.at(j).p;
	    }
	    if (mvParticle.at(j).fCurr < mTournament.at(N_tour-1).fCurr)	//update gbest
	    {
		for (unsigned int k = 0 ; k<mTournament.size() ; k++)
		{
		    bool bP = false;
		    double ddist = DistOfParticles(mvParticle.at(j),mTournament.at(k),cc,rr,bP);
		    if (ddist>dist_threshold && bP == true)
		    {
			mTournament.at(k)=mvParticle.at(j);
			sort(mTournament.begin(),mTournament.end());
			break;
		    }
		}
	    }
	}
	//cout<<"fitness of the best particle in Tournament vector is "<<mTournament.at(0).fCurr<<endl;
	//cout<<"fitness of the previous particle is "<<p_previous.fCurr<<endl;
	//cout<<"count = "<<count<<endl;
	//cout<<"dist between previous particle and current best particle is "<<DistOfParticles(p_previous,mTournament.at(0),cc,rr)<<endl;
	bool bP = false;
	if (DistOfParticles(p_previous,mTournament.at(0),cc,rr,bP)<dist_threshold/10) //stability verification
	    count++;
	if (count >= min_iter_time_4_optima_found)
	{
	    mvFoundOptima.push_back(mTournament.at(0));
	    mTournament.assign(N_tour,InitialParticle());
	    Reinitialise_Parallel(mvParticle,mTournament,mvFoundOptima,points,cc,rr);
	    count = 0;
	    //cout<<"iteration number = "<<i<<endl;
	}
     }
     /*
     for(unsigned int m = 0 ; m<mTournament.size() ; m++)
     cout<<"mTournament parameter "<<m<<" is ["<<mTournament.at(m).p.at(0)<<" "<<mTournament.at(m).p.at(1)<<" "<<mTournament.at(m).p.at(2)<<" "<<mTournament.at(m).p.at(3)<<"]"<<endl;
     */
     if (mvFoundOptima.size()>0)	// find at least one dominant plane
     {
	for (unsigned int i = 0 ; i<mvFoundOptima.size() ; i++)
	{
	      A = mvFoundOptima.at(i).p.at(0);
	      B = mvFoundOptima.at(i).p.at(1);
	      C = mvFoundOptima.at(i).p.at(2);
	      D = mvFoundOptima.at(i).p.at(3);
	      if (D<0)
	      {
		    A = -A;
		    B = -B;
		    C = -C;
		    D = -D;
	      }
	      //cout<<"parameters are ["<<A<<" "<<B<<" "<<C<<" "<<D<<"]"<<endl;
	      for(unsigned int j=0; j<nPoints; j++)
	      {
		    double d_parameter = -(A*points.at(j).p.x+B*points.at(j).p.y+C*points.at(j).p.z);
		    double dNormDist = abs(d_parameter-D)/sqrt(A*A+B*B+C*C);
		    if(dNormDist < min_height_of_obj)
		    {
			    labels.at(j) = i*(-10); // dominant plane i
		    }
	      }
	}
     }
     for(unsigned int j=0; j<nPoints; j++)	//non-planes, good, they are objects candidants
     {
	if (labels.at(j) == -3)
	    labels.at(j) = -2;
     }

     //cout<<"there are "<<mvFoundOptima.size()<<" planes found!"<<endl;

}

bool PlanePopOut::PSO_Label(VisionData::SurfacePointSeq &points, std::vector <int> &labels)
{
     int N_particle = 25; 			// particle number

      VisionData::SurfacePointSeq P_points = points;
      unsigned int nPoints = P_points.size();
      if(nPoints < 10)
      {
	      //std::cout << "  too few points to calc plane" << std::endl;
	      return false;
      }
      
      vector < vector<double> > vvd_particles;
      vector<Vector3> vv3Hypo;
      //vvd_particles.reserve(N_particle);
      for(int i=0; i<N_particle; i++)
      {
	  int nA, nB, nC;
	  Vector3 v3Normal;
	  do
	  {
		  nA = rand()%nPoints;
		  nB = nA;
		  nC = nA;
		  while(nB == nA)
			  nB = rand()%nPoints;
		  while(nC == nA || nC==nB)
			  nC = rand()%nPoints;
		  Vector3 v3CA = P_points.at(nC).p  - P_points.at(nA).p;
		  Vector3 v3BA = P_points.at(nB).p  - P_points.at(nA).p;
		  v3Normal = cross(v3CA, v3BA);
		  if (norm(v3Normal) != 0)
			  normalise(v3Normal);
		  else v3Normal = 99999.9*v3Normal;
	  } while (fabs(v3Normal.x/(dot(v3Normal,v3Normal)+1))>0.01); //the plane should parallel with the initialisation motion of camera
	  vv3Hypo.clear();
	  vv3Hypo.push_back(P_points.at(nA).p); vv3Hypo.push_back(P_points.at(nB).p); vv3Hypo.push_back(P_points.at(nC).p);
	  vvd_particles.push_back(Hypo2ParaSpace(vv3Hypo));
      }
      //std::cout << " After random selection, PSO is running... " << std::endl;
      PSO_internal(vvd_particles, points, labels);
// find multiple dominant planes, now label the inliers for each plane

      return true;
}


bool PlanePopOut::RANSAC(VisionData::SurfacePointSeq &points, std::vector <int> &labels)
{
	VisionData::SurfacePointSeq R_points = points;
	unsigned int nPoints = R_points.size();
	if(nPoints < 10)
	{
		//std::cout << "  too few points to calc plane" << std::endl;
		return false;
	};
	int nRansacs = 100;
	int point1 = 0;
	int point2 = 0;
	int point3 = 0;
	Vector3 v3BestMean;
	Vector3 v3BestNormal;
	double dBestDistSquared = 9999999999999999.9;

	for(int i=0; i<nRansacs; i++)
	{
		int nA, nB, nC;
		Vector3 v3Normal;
		do
		{
			nA = rand()%nPoints;
			nB = nA;
			nC = nA;
			while(nB == nA)
				nB = rand()%nPoints;
			while(nC == nA || nC==nB)
				nC = rand()%nPoints;
			Vector3 v3CA = R_points.at(nC).p  - R_points.at(nA).p;
			Vector3 v3BA = R_points.at(nB).p  - R_points.at(nA).p;
			v3Normal = cross(v3CA, v3BA);
			if (norm(v3Normal) != 0)
				normalise(v3Normal);
			else v3Normal = 99999.9*v3Normal;
		} while (fabs(v3Normal.x/(dot(v3Normal,v3Normal)+1))>0.01); //the plane should parallel with the initialisation motion of camera

		Vector3 v3Mean = 0.33333333 * (R_points.at(nA).p + R_points.at(nB).p + R_points.at(nC).p);
		double dSumError = 0.0;
		for(unsigned int i=0; i<nPoints; i++)
		{
			Vector3 v3Diff = R_points.at(i).p - v3Mean;
			double dDistSq = dot(v3Diff, v3Diff);
			if(dDistSq == 0.0)
			continue;
			double dNormDist = fabs(dot(v3Diff, v3Normal));
			if(dNormDist > min_height_of_obj)
			dNormDist = min_height_of_obj;
			dSumError += dNormDist;
		}
		if(dSumError < dBestDistSquared)
		{
			dBestDistSquared = dSumError;
			v3BestMean = v3Mean;
			v3BestNormal = v3Normal;
			point1 = nA;
			point2 = nB;
			point3 = nC;
		}
	}
////////////////////////////////use three points to cal plane////
	para_a = ( (R_points.at(point2).p.y-R_points.at(point1).p.y)*(R_points.at(point3).p.z-R_points.at(point1).p.z)-(R_points.at(point2).p.z-R_points.at(point1).p.z)*(R_points.at(point3).p.y-R_points.at(point1).p.y) );
	para_b = ( (R_points.at(point2).p.z-R_points.at(point1).p.z)*(R_points.at(point3).p.x-R_points.at(point1).p.x)-(R_points.at(point2).p.x-R_points.at(point1).p.x)*(R_points.at(point3).p.z-R_points.at(point1).p.z) );
	para_c = ( (R_points.at(point2).p.x-R_points.at(point1).p.x)*(R_points.at(point3).p.y-R_points.at(point1).p.y)-(R_points.at(point2).p.y-R_points.at(point1).p.y)*(R_points.at(point3).p.x-R_points.at(point1).p.x) );
	para_d = ( 0-(para_a*R_points.at(point1).p.x+para_b*R_points.at(point1).p.y+para_c*R_points.at(point1).p.z) );
/////////////////////////////////end parameters calculation/////////////
// Done the ransacs, now collect the supposed inlier set
	if (para_d<0)
	{
		para_a = -para_a;
		para_b = -para_b;
		para_c = -para_c;
		para_d = -para_d;
	}
	if (para_a*para_a+para_b*para_b+para_c*para_c != 0)
	{
		A = para_a;
		B = para_b;
		C = para_c;
		D = para_d;
	}

	double dmin = 9999.0;
	double dmax = 0.0;

	if (v3BestMean.x != 0 || v3BestMean.y != 0 || v3BestMean.z != 0)
	{
		for(unsigned int i=0; i<nPoints; i++)
		{
			Vector3 v3Diff = R_points.at(i).p - v3BestMean;
			double dNormDist = fabs(dot(v3Diff, v3BestNormal));
			if(dNormDist < min_height_of_obj)
			{
				labels.at(i) = 0; // dominant plane
				double ddist = dot(R_points.at(i).p,R_points.at(i).p);
				if (ddist > dmax) {dmax = ddist; v3dmax = R_points.at(i).p;}
				else if (ddist < dmin) {dmin = ddist; v3dmin = R_points.at(i).p;}
			}
			else
			{
				double d_parameter = -(A*R_points.at(i).p.x+B*R_points.at(i).p.y+C*R_points.at(i).p.z);
				if (d_parameter > 0 && d_parameter < D && fabs(d_parameter-D) > sqrt(A*A+B*B+C*C)*min_height_of_obj)
					labels.at(i) = -2; // objects
				if (d_parameter > 0 && d_parameter < D && fabs(d_parameter-D) <= sqrt(A*A+B*B+C*C)*min_height_of_obj)
					labels.at(i) = -1; // cannot distingush
			}
		}
	}
	return true;
}

void PlanePopOut::SplitPoints(VisionData::SurfacePointSeq &points, std::vector <int> &labels)
{
	std::vector<int> candidants;
	std::vector <int> S_label = labels;
	for (unsigned int i=0; i<S_label.size(); i++)
	{
		if (S_label.at(i) == -2) //not belong to the dominant plane cluster
			candidants.push_back(i);
	}
//cout<<"candidants.size() =  "<<candidants.size()<<endl;
	std::vector<int> one_obj;
	objnumber = 1;
	unsigned int points_of_one_object = 0;
	std::stack <int> objstack;
	double split_threshold = Calc_SplitThreshold(points, labels);
	unsigned int obj_number_threshold;
	obj_number_threshold = 20;
	while(!candidants.empty())
	{
		S_label.at(*candidants.begin()) = objnumber;
		objstack.push(*candidants.begin());
		points_of_one_object++;
		candidants.erase(candidants.begin());
		while(!objstack.empty())
		{
			int seed = objstack.top();
			objstack.pop();
			for(std::vector<int>::iterator it=candidants.begin(); it<candidants.end(); it++)
			{
				if (dist(points.at(seed).p, points.at(*it).p)<split_threshold)
				{
					S_label.at(*it) = S_label.at(seed);
					objstack.push(*it);
					points_of_one_object++;
					candidants.erase(it);
				}
			}
		}//cout<<"points_of_one_object =  "<<points_of_one_object<<endl;cout<<"candidants.size() =  "<<candidants.size()<<endl;
		if (points_of_one_object>obj_number_threshold)
		{
			labels = S_label;
			objnumber++;
		}
		else
			S_label = labels;
		points_of_one_object = 0;
	}
	objnumber--;
}

double PlanePopOut::Calc_SplitThreshold(VisionData::SurfacePointSeq &points, std::vector <int> &labels)
{
	double max_x = -99999.0;
	double min_x = 99999.0;
	double max_y = -99999.0;
	double min_y = 99999.0;
	double max_z = -99999.0;
	double min_z = 99999.0;

	for(unsigned int i=0; i<points.size(); i++)
	{
		if (labels.at(i) == -2)
		{
			Vector3 v3Obj = points.at(i).p;
			if (v3Obj.x>max_x) max_x = v3Obj.x;
			if (v3Obj.x<min_x) min_x = v3Obj.x;
			if (v3Obj.y>max_y) max_y = v3Obj.y;
			if (v3Obj.y<min_y) min_y = v3Obj.y;
			if (v3Obj.z>max_z) max_z = v3Obj.z;
			if (v3Obj.z<min_z) min_z = v3Obj.z;
		}
	}
	return sqrt((max_x-min_x)*(max_x-min_x)+(max_y-min_y)*(max_y-min_y)+(max_z-min_z)*(max_z-min_z))/20;
}

SOIPtr PlanePopOut::createObj(Vector3 center, Vector3 size, double radius, VisionData::SurfacePointSeq psIn1SOI, VisionData::SurfacePointSeq BGpIn1SOI, VisionData::SurfacePointSeq EQpIn1SOI)
{
	debug("create an object at (%f, %f, %f) now", center.x, center.y, center.z);
	VisionData::SOIPtr obs = new VisionData::SOI;
	obs->status = 0;
	obs->boundingBox.pos.x = obs->boundingSphere.pos.x = center.x;
	obs->boundingBox.pos.y = obs->boundingSphere.pos.y = center.y;
	obs->boundingBox.pos.z = obs->boundingSphere.pos.z = center.z;
	obs->boundingBox.size.x = size.x;
	obs->boundingBox.size.y = size.y;
	obs->boundingBox.size.z = size.z;	//cout<<"radius in SOI = "<<radius<<endl;
	obs->boundingSphere.rad = radius;       //cout<<"radius in SOI (obs) = "<<obs->boundingSphere.rad<<endl;
	obs->time = getCASTTime();
	obs->points = psIn1SOI;//cout<<"points in 1 SOI = "<<obs->points.at(1).p<<obs->points.at(2).p<<obs->points.at(10).p<<endl;
	obs->BGpoints =	BGpIn1SOI;
	obs->EQpoints =	EQpIn1SOI;//cout<<"EQ points in 1 SOI = "<<obs->EQpoints.size()<<endl;

	return obs;
}

bool PlanePopOut::Compare2SOI(ObjPara obj1, ObjPara obj2)
{
	if (dist(obj1.c,obj2.c)<rate_of_centers*obj1.r)
		return true; //the same object
	else
		return false; //not the same one
}

void PlanePopOut::AddConvexHullinWM()
{
	double T_CenterHull = 0.5 * mConvexHullRadius;
	VisionData::ConvexHullPtr CHPtr = new VisionData::ConvexHull;
	Pose3 p3;
	setIdentity(p3);
	Vector3 v3;
	setZero(v3);
	
	if (pre_mConvexHullRadius == 0.0)
	{ 
	    if (mConvexHullPoints.size()>0)
	    {
		debug("There are %u points in the convex hull", mConvexHullPoints.size());
		CHPtr->PointsSeq = mConvexHullPoints;
		CHPtr->time = getCASTTime();
		p3.pos = mCenterOfHull;
		
		CHPtr->center = p3;
		CHPtr->radius = mConvexHullRadius;
		CHPtr->density = mConvexHullDensity;
		CHPtr->Objects = mObjSeq;
		CHPtr->plane.a = A; CHPtr->plane.b = B; CHPtr->plane.c = C; CHPtr->plane.d = D;
		pre_id = newDataID();
		addToWorkingMemory(pre_id,CHPtr);
		
		pre_mConvexHullRadius = mConvexHullRadius;
		pre_mCenterOfHull = mCenterOfHull;
	    }
	}
	else
	{
	    if (mConvexHullPoints.size()>0)
	    {
		    debug("There are %u points in the convex hull", mConvexHullPoints.size());
		    CHPtr->PointsSeq = mConvexHullPoints;
		    CHPtr->time = getCASTTime();
		    p3.pos = mCenterOfHull;
		    
		    CHPtr->center = p3;
		    CHPtr->radius = mConvexHullRadius;
		    CHPtr->density = mConvexHullDensity;
		    CHPtr->Objects = mObjSeq;
		    CHPtr->plane.a = A; CHPtr->plane.b = B; CHPtr->plane.c = C; CHPtr->plane.d = D;
		    if (dist(pre_mCenterOfHull, mCenterOfHull) > T_CenterHull)
		    {
			  //cout<<"dist = "<<dist(pre_mCenterOfHull, mCenterOfHull)<<"  T = "<<T_CenterHull<<endl;
			  debug("add sth into WM");
			  pre_id = newDataID();
			  addToWorkingMemory(pre_id,CHPtr);
			  pre_mConvexHullRadius = mConvexHullRadius;
			  pre_mCenterOfHull = mCenterOfHull;  
		    }
		    else
		    {
			  overwriteWorkingMemory(pre_id, CHPtr);
		    }		    
	    }
	}

	mConvexHullPoints.clear();
	mObjSeq.clear();
	mCenterOfHull.x = mCenterOfHull.y = mCenterOfHull.z = 0.0;
	mConvexHullRadius = 0.0;
	mConvexHullDensity = 0.0;
}
void PlanePopOut::DrawOneCuboid(Vector3 Max, Vector3 Min)
{
	glLineWidth(1);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//////////////////////top/////////////////////////////////////
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0,1.0,1.0);
	glVertex3f(Max.x, Max.y, Max.z);
	glVertex3f(Min.x, Max.y, Max.z);
	glVertex3f(Min.x, Min.y, Max.z);
	glVertex3f(Max.x, Min.y, Max.z);
	glEnd();
/////////////////////////////bottom///////////////////////
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0,1.0,1.0);
	glVertex3f(Min.x, Min.y, Min.z);
	glVertex3f(Max.x, Min.y, Min.z);
	glVertex3f(Max.x, Max.y, Min.z);
	glVertex3f(Min.x, Max.y, Min.z);
	glEnd();
//////////////////////verticle lines//////////////////////////
	glBegin(GL_LINES);
	glVertex3f(Min.x, Min.y, Min.z);
	glVertex3f(Min.x, Min.y, Max.z);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(Max.x, Min.y, Min.z);
	glVertex3f(Max.x, Min.y, Max.z);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(Max.x, Max.y, Min.z);
	glVertex3f(Max.x, Max.y, Max.z);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(Min.x, Max.y, Min.z);
	glVertex3f(Min.x, Max.y, Max.z);
	glEnd();

	glDisable(GL_BLEND);
}

void PlanePopOut::DrawCuboids(VisionData::SurfacePointSeq &points, std::vector <int> &labels)
{
	VisionData::SurfacePointSeq Max;
	VisionData::SurfacePointSeq Min;
	Vector3 initial_vector;
	initial_vector.x = -9999;
	initial_vector.y = -9999;
	initial_vector.z = -9999;
	VisionData::SurfacePoint InitialStructure;
	InitialStructure.p = initial_vector;
	InitialStructure.c.r = InitialStructure.c.g = InitialStructure.c.b = 0;
	Max.assign(objnumber, InitialStructure);
	initial_vector.x = 9999;
	initial_vector.y = 9999;
	initial_vector.z = 9999;
	InitialStructure.p = initial_vector;
	Min.assign(objnumber, InitialStructure);

	for(unsigned int i = 0; i<points.size(); i++)
	{
		Vector3 v3Obj = points.at(i).p;
		int label = labels.at(i);
		if (label > 0)
		{
			if (v3Obj.x>Max.at(label-1).p.x) Max.at(label-1).p.x = v3Obj.x;
			if (v3Obj.x<Min.at(label-1).p.x) Min.at(label-1).p.x = v3Obj.x;

			if (v3Obj.y>Max.at(label-1).p.y) Max.at(label-1).p.y = v3Obj.y;
			if (v3Obj.y<Min.at(label-1).p.y) Min.at(label-1).p.y = v3Obj.y;

			if (v3Obj.z>Max.at(label-1).p.z) Max.at(label-1).p.z = v3Obj.z;
			if (v3Obj.z<Min.at(label-1).p.z) Min.at(label-1).p.z = v3Obj.z;
		}
	}
	v3size.clear();
	vdradius.clear();
	for (int i=0; i<objnumber; i++)
	{
		Vector3 s;
		s.x = (Max.at(i).p.x-Min.at(i).p.x)/2;
		s.y = (Max.at(i).p.y-Min.at(i).p.y)/2;
		s.z = (Max.at(i).p.z-Min.at(i).p.z)/2;
		v3size.push_back(s);
		double rad = norm(s);
		vdradius.push_back(rad);
	}
/*
	for (int i = 0; i<objnumber; i++)
	{
		DrawOneCuboid(Max.at(i),Min.at(i));
	}
*/
	Max.clear();
	Min.clear();
}

void PlanePopOut::DrawWireSphere(Vector3 center, double radius)
{
	glColor3f(1.0,1.0,1.0);
	glTranslatef(center.x, center.y, center.z);
	glutWireSphere(radius,10,10);
	glTranslatef(-center.x, -center.y, -center.z);
}

Vector3 PlanePopOut::ProjectOnDominantPlane(Vector3 InputP)
{
	Vector3 OutputP;
	OutputP.x = ((B*B+C*C)*InputP.x-A*(B*InputP.y+C*InputP.z+D))/(A*A+B*B+C*C);
	OutputP.y = ((A*A+C*C)*InputP.y-B*(A*InputP.x+C*InputP.z+D))/(A*A+B*B+C*C);
	OutputP.z = ((B*B+A*A)*InputP.z-C*(B*InputP.y+A*InputP.x+D))/(A*A+B*B+C*C);

	return OutputP;
}

Matrix33 PlanePopOut::GetAffineRotMatrix()
{
	Vector3 vb; //translation vector
	Vector3 v3normal;  //normal vector of dominant plane
	v3normal.x = A;	v3normal.y = B;	v3normal.z = C;
	normalise(v3normal);
	if(v3normal.z < 0)
	v3normal *= -1.0;

	Matrix33 rot;
	setIdentity(rot);
	setColumn(rot,2,v3normal);
	vb.x = 1;	vb.y = 0;	vb.z = 0;
	vb = vb-(v3normal*v3normal.x);
	normalise(vb);
	setRow(rot,0,vb);
	setZero(vb);
	vb = cross(getRow(rot,2),getRow(rot,0));
	setRow(rot,1,vb);
	setZero(vb);

	return rot;
}

Vector3 PlanePopOut::GetAffineTransVec(Vector3 v3p) // translation vector from p to original point
{
	Matrix33 m33 = GetAffineRotMatrix();
	return -(m33*v3p);
}

inline Vector3 PlanePopOut::AffineTrans(Matrix33 m33, Vector3 v3)
{
	return m33*v3;
}

void PlanePopOut::ConvexHullOfPlane(VisionData::SurfacePointSeq &points, std::vector <int> &labels)
{
	CvPoint* points2D = (CvPoint*)malloc( points.size() * sizeof(points2D[0]));
	vector<Vector3> PlanePoints3D;
	Matrix33 AffineM33 = GetAffineRotMatrix();
	int j = 0;

	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* ptseq = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
                                     sizeof(CvPoint), storage );


	for(unsigned int i = 0; i<points.size(); i++)
	{
		Vector3 v3Obj = points.at(i).p;
		int label = labels.at(i);
		if (label == 0) // collect points seq for drawing convex hull of the dominant plane
		{
			CvPoint cvp;
			Vector3 v3AfterAffine = AffineTrans(AffineM33, v3Obj);
			cvp.x =100.0*v3AfterAffine.x; cvp.y =100.0*v3AfterAffine.y;
			points2D[j] = cvp;
			cvSeqPush( ptseq, &cvp );
			j++;
			PlanePoints3D.push_back(v3Obj);
		}
	}
	// calculate convex hull
	if (j>0)
	{
		//cout<<"2d points number ="<<j-1<<endl;
		int* hull = (int*)malloc( (j-1) * sizeof(hull[0]));
		CvSeq* cvhull;


		CvMat pointMat = cvMat( 1, j-1, CV_32SC2, points2D);
		CvMat hullMat = cvMat( 1, j-1, CV_32SC1, hull);
		cvConvexHull2(&pointMat, &hullMat, CV_CLOCKWISE, 0);
		cvhull = cvConvexHull2( ptseq, 0, CV_CLOCKWISE, 1);


		//draw the hull
		if (hullMat.cols != 0)
		{
			//cout<<"points in the hull ="<<hullMat.cols<<endl;
// 			if (mbDrawWire)	glBegin(GL_LINE_LOOP); else glBegin(GL_POLYGON);
// 			glColor3f(1.0,1.0,1.0);
			Vector3 v3OnPlane;
			for (int i = 0; i<hullMat.cols; i++)
			{
				v3OnPlane = ProjectOnDominantPlane(PlanePoints3D.at(hull[i]));
				mConvexHullPoints.push_back(v3OnPlane);
				mCenterOfHull += v3OnPlane;
			}
			mConvexHullDensity = PlanePoints3D.size() / fabs(cvContourArea(cvhull));//cout<<"mConvexHullDensity = "<<mConvexHullDensity<<endl;
			mCenterOfHull /= hullMat.cols;
			mConvexHullRadius = sqrt((v3OnPlane.x-mCenterOfHull.x)*(v3OnPlane.x-mCenterOfHull.x)+(v3OnPlane.y-mCenterOfHull.y)*(v3OnPlane.y-mCenterOfHull.y)+(v3OnPlane.z-mCenterOfHull.z)*(v3OnPlane.z-mCenterOfHull.z));
			//cout<<"mConvexHullRadius = "<<mConvexHullRadius<<endl;
			// 			glEnd();
		}
		free( hull );
		cvClearSeq(cvhull);
	}
	cvClearMemStorage( storage );
	cvReleaseMemStorage(&storage);
	cvClearSeq(ptseq);
	PlanePoints3D.clear();
	free( points2D );
}

void PlanePopOut::DrawOnePrism(vector <Vector3> ppSeq, double hei, Vector3& v3c)
{
	double dd = D - hei*sqrt(A*A+B*B+C*C);
	double half_dd = D- 0.5*hei*sqrt(A*A+B*B+C*C);
	Vector3 v3core;
	v3core.x =((C*C+B*B)*v3c.x-A*(C*v3c.z+B*v3c.y+half_dd))/(A*A+B*B+C*C);
	v3core.y =((A*A+C*C)*v3c.y-B*(A*v3c.x+C*v3c.z+half_dd))/(A*A+B*B+C*C);
	v3core.z =((A*A+B*B)*v3c.z-C*(A*v3c.x+B*v3c.y+half_dd))/(A*A+B*B+C*C);
	v3c = v3core;
	vector <Vector3> pphSeq;
	VisionData::OneObj OObj;
	for (unsigned int i = 0; i<ppSeq.size(); i++)
	{
		Vector3 v;
		v.x =((C*C+B*B)*ppSeq.at(i).x-A*(C*ppSeq.at(i).z+B*ppSeq.at(i).y+dd))/(A*A+B*B+C*C);
		v.y =((A*A+C*C)*ppSeq.at(i).y-B*(A*ppSeq.at(i).x+C*ppSeq.at(i).z+dd))/(A*A+B*B+C*C);
		v.z =((A*A+B*B)*ppSeq.at(i).z-C*(A*ppSeq.at(i).x+B*ppSeq.at(i).y+dd))/(A*A+B*B+C*C);
		pphSeq.push_back(v);
		OObj.pPlane.push_back(ppSeq.at(i)); OObj.pTop.push_back(v);
	}
	mObjSeq.push_back(OObj);
	
/*
	glBegin(GL_POLYGON);
	glColor3f(1.0,1.0,1.0);
	for (unsigned int i = 0; i<ppSeq.size(); i++)
		glVertex3f(ppSeq.at(i).x,ppSeq.at(i).y,ppSeq.at(i).z);
	glEnd();
	glBegin(GL_POLYGON);
	for (unsigned int i = 0; i<ppSeq.size(); i++)
		glVertex3f(pphSeq.at(i).x,pphSeq.at(i).y,pphSeq.at(i).z);
	glEnd();
	for (unsigned int i = 0; i<ppSeq.size(); i++)
	{
		glBegin(GL_LINES);
		glVertex3f(ppSeq.at(i).x,ppSeq.at(i).y,ppSeq.at(i).z);
		glVertex3f(pphSeq.at(i).x,pphSeq.at(i).y,pphSeq.at(i).z);
		glEnd();
	}
*/
}

void PlanePopOut::BoundingPrism(VisionData::SurfacePointSeq &pointsN, std::vector <int> &labels)
{
	CvPoint* points2D = (CvPoint*)malloc( pointsN.size() * sizeof(points2D[0]));

	Matrix33 AffineM33 = GetAffineRotMatrix();
	Matrix33 AffineM33_1;
	inverse(AffineM33,AffineM33_1);
	vector < CvPoint* > objSeq;
	objSeq.assign(objnumber,points2D);
	vector < int > index;
	index.assign(objnumber,0);
	vector < double > height;
	height.assign(objnumber,0.0);
	vector < vector<Vector3> > PlanePoints3DSeq;
	Vector3 v3init; v3init.x = v3init.y = v3init.z =0.0;
	vector<Vector3> vv3init; vv3init.assign(1,v3init);
	PlanePoints3DSeq.assign(objnumber, vv3init);


	for(unsigned int i = 0; i<pointsN.size(); i++)
	{
		Vector3 v3Obj = pointsN.at(i).p;
		int label = labels.at(i);
		if (label < 1)	continue;
		CvPoint cvp;
		Vector3 v3AfterAffine = AffineTrans(AffineM33, v3Obj);
		cvp.x =1000.0*v3AfterAffine.x; cvp.y =1000.0*v3AfterAffine.y;
		objSeq.at(label-1)[index.at(label-1)] = cvp;
		PlanePoints3DSeq.at(label-1).push_back(v3Obj);
		index.at(label-1)++;
		if (fabs(A*v3Obj.x+B*v3Obj.y+C*v3Obj.z+D)/sqrt(A*A+B*B+C*C) > height.at(label-1))
			height.at(label-1) = fabs(A*v3Obj.x+B*v3Obj.y+C*v3Obj.z+D)/sqrt(A*A+B*B+C*C);
	}
	// calculate convex hull
	if (index.at(0)>0)
	{
		int* hull = (int*)malloc( pointsN.size() * sizeof(hull[0]));
		Vector3 temp_v; temp_v.x = temp_v.y = temp_v.z = 0.0;
		v3center.assign(objnumber, temp_v);
		for (int i = 0; i < objnumber; i++)
		{

			CvMat pointMat = cvMat( 1, index.at(i)-1, CV_32SC2, objSeq.at(i));
			CvMat hullMat = cvMat( 1, index.at(i)-1, CV_32SC1, hull);
			cvConvexHull2(&pointMat, &hullMat, CV_CLOCKWISE, 0);
			//calculate convex hull points on the plane
			std::vector <Vector3> v3OnPlane;
			v3OnPlane.assign(hullMat.cols, v3init);
			for (int j = 0; j<hullMat.cols; j++)
				v3OnPlane.at(j) =ProjectOnDominantPlane(PlanePoints3DSeq.at(i).at(hull[j]+1));

			for (unsigned int k = 0; k<v3OnPlane.size(); k++)
			{
			    v3center.at(i) = v3center.at(i) + v3OnPlane.at(k);
			}
			v3center.at(i) = v3center.at(i)/ v3OnPlane.size();			
			
 			DrawOnePrism(v3OnPlane, height.at(i), v3center.at(i));


			v3OnPlane.clear();
		}
		free( hull );
	}
	free( points2D );
}

void PlanePopOut::BoundingSphere(VisionData::SurfacePointSeq &points, std::vector <int> &labels)
{
	VisionData::SurfacePointSeq center;
	Vector3 initial_vector;
	initial_vector.x = 0;
	initial_vector.y = 0;
	initial_vector.z = 0;
	VisionData::SurfacePoint InitialStructure;
	InitialStructure.p = initial_vector;
	center.assign(objnumber,InitialStructure);
	
	for (unsigned int i = 0 ; i<v3center.size() ; i++)
	{
	    center.at(i).p = v3center.at(i);
	}

	std::vector<double> radius_world;
	radius_world.assign(objnumber,0);
	VisionData::SurfacePointSeq pointsInOneSOI;
	SOIPointsSeq.clear();
	SOIPointsSeq.assign(objnumber, pointsInOneSOI);
	BGPointsSeq.clear();
	BGPointsSeq.assign(objnumber, pointsInOneSOI);
	EQPointsSeq.clear();
	EQPointsSeq.assign(objnumber, pointsInOneSOI);

	////////////////////calculte radius in the real world//////////////////
		for(unsigned int i = 0; i<points.size(); i++)
		{
			Vector3 v3Obj = points.at(i).p;
			int label = labels.at(i);
			if (label > 0 && dist(v3Obj,center.at(label-1).p) > radius_world.at(label-1))
				radius_world.at(label-1) = dist(v3Obj,center.at(label-1).p);
		}
/*		//vdradius.clear();
		for (int i=0; i<objnumber; i++)
		{
			//vdradius.push_back(radius_world.at(i));
			cout<<"in Bounding box, radius of "<<i<<" object is "<<vdradius.at(i)<<endl;
			cout<<"world radius of "<<i<<" object is "<<radius_world.at(i)<<endl;
		}
	
*/
	for (int i = 0; i<objnumber; i++)
	{
		//if (mbDrawWire)	DrawWireSphere(center.at(i).p,radius_world.at(i));
		Vector3 Center_DP = ProjectOnDominantPlane(center.at(i).p);//cout<<" center on DP ="<<Center_DP<<endl;
		for (unsigned int j = 0; j<points.size(); j++)
		{
			VisionData::SurfacePoint PushStructure;
			PushStructure.p = points.at(j).p;
			PushStructure.c = points.at(j).c;	//cout<<"in BG"<<PushStructure.c.r<<PushStructure.c.g<<PushStructure.c.b<<endl;
			Vector3 Point_DP = ProjectOnDominantPlane(PushStructure.p);
			int label = labels.at(j);
			if (label > 0 && dist(Point_DP,Center_DP) < Shrink_SOI*vdradius.at(i))
				SOIPointsSeq.at(label-1).push_back(PushStructure);

			if (label == -1 && dist(Point_DP,Center_DP) < Lower_BG*vdradius.at(i)) // equivocal points
				EQPointsSeq.at(i).push_back(PushStructure);

			if (label == 0 && dist(Point_DP,Center_DP) < Upper_BG*radius_world.at(i) && dist(Point_DP,Center_DP) > Lower_BG*vdradius.at(i)) //BG nearby also required
				BGPointsSeq.at(i).push_back(PushStructure);

		}
	}

	center.clear();
	radius_world.clear();
	pointsInOneSOI.clear();
}


}
