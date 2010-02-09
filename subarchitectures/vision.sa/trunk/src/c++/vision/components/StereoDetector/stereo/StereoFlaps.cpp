/**
 * @file StereoFlaps.cpp
 * @author Andreas Richtsfeld
 * @date October 2009
 * @version 0.2
 * @brief Stereo matching of flaps..
 */

#include "StereoFlaps.h"

namespace Z
{

//-----------------------------------------------------------------//
//---------------------------- TmpFlap ----------------------------//
//-----------------------------------------------------------------//
/**
 * @brief Constructor TmpFlap
 * @param flap vs3 flap
 */
TmpFlap::TmpFlap(Flap *flap)
{
  surf[0].Init(flap->clos[0]);
  surf[1].Init(flap->clos[1]);
}

/**
 * @brief Rectify TmpFlap
 * @param cam Stereo camera parameters and functions.
 * @param side LEFT / RIGHT side of stereo
 */
void TmpFlap::Rectify(StereoCamera *stereo_cam, int side)
{
  surf[0].Rectify(stereo_cam, side);
  surf[1].Rectify(stereo_cam, side);
}

/**
 * @brief Refine TmpFlap
 */
void TmpFlap::Refine()
{
  surf[0].Refine();
  surf[1].Refine();
}

/**
 * @brief Returns true, if flap is at the x/y-position in the image.
 * @param x X-coordinate in image pixel.
 * @param y Y-coordinate in image pixel.
 * @return Returns true, if flap is at this position.
 */
bool TmpFlap::IsAtPosition(int x, int y) const
{
  return surf[0].IsAtPosition(x, y) || surf[1].IsAtPosition(x, y);
}

/**
 * @brief If a flap from the right image was matched with a flap from the left image we
 * typically have to shift the point arrays of the right surfaces to align with
 * the left points and maybe swap surface 0 and 1 of the right flap.
 * Note that we first shift points and then swap surfaces.
 * @param off0 TODO Shift points ???
 * @param off1 TODO Shift points ???
 * @param swap Swap the the two surfaces 
 */
void TmpFlap::Fuddle(unsigned off0, unsigned off1, bool swap)
{
  surf[0].ShiftPointsLeft(off0);
  surf[1].ShiftPointsLeft(off1);
  if(swap)
  {
    TmpSurf t = surf[1];
    surf[1] = surf[0];
    surf[0] = t;
  }
}


//----------------------------------------------------------------//
//---------------------------- Flap3D ----------------------------//
//----------------------------------------------------------------//

/**
 * @brief Reconstruct the flap in 3D
 * @param left Left TmpFlap
 * @param right Right TmpFlap
 * @param cam Stereo camera parameters and functions.
 */
bool Flap3D::Reconstruct(StereoCamera *stereo_cam, TmpFlap &left, TmpFlap &right)
{
  bool ok0 = surf[0].Reconstruct(stereo_cam, left.surf[0], right.surf[0], true);
  bool ok1 = surf[1].Reconstruct(stereo_cam, left.surf[1], right.surf[1], true);
  return ok0 && ok1;
}



//------------------------------------------------------------------//
//--------------------------- StereoFlaps --------------------------//
//------------------------------------------------------------------//
/**
 * @brief Constructor of StereoFlaps: Calculate stereo matching of flaps
 * @param vc Vision core of calculated LEFT and RIGHT stereo image
 * @param sc Stereo camera parameters
 */
StereoFlaps::StereoFlaps(VisionCore *vc[2], StereoCamera *sc) : StereoBase()
{
	vcore[LEFT] = vc[LEFT];
	vcore[RIGHT] = vc[RIGHT];
	stereo_cam = sc;
  flapMatches = 0;
}

/**
 * @brief Number of Surfaces in 2D 
 * @param side LEFT/RIGHT side of stereo rig.
 */
// int StereoFlaps::NumSurfaces2D(int side)
// {
//   assert(side == LEFT || side == RIGHT);
//   return surfs[side].Size();
// }

/**
 * @brief Delivers 2D tmp. surface.
 * @param side LEFT/RIGHT side of stereo rig.
 * @param i Position of the surface in the array.
 */
// const TmpSurf &StereoFlaps::Surfaces2D(int side, int i)
// {
//   assert(side == LEFT || side == RIGHT);
//   return surfs[side][i];
// }

/**
 * @brief Number of Flaps in 2D 
 * @param side LEFT/RIGHT side of stereo rig.
 */
int StereoFlaps::NumFlaps2D(int side)
{
  assert(side == LEFT || side == RIGHT);
  return flaps[side].Size();
}

/**
 * @brief Delivers 2D tmp. flap.
 * @param side LEFT/RIGHT side of stereo rig.
 * @param i Position of the flap in the array
 */
const TmpFlap &StereoFlaps::Flaps2D(int side, int i)
{
  assert(side == LEFT || side == RIGHT);
  return flaps[side][i];
}

/**
 * @brief Draw flaps as overlay.
 * @param side Left or right side of the stereo images.
 * @param masked Draw masked features.
 */
void StereoFlaps::Draw(int side, bool masked)
{
	SetColor(RGBColor::blue);
	int nrFlaps = 0;
	if(side == LEFT) nrFlaps = NumFlapsLeft2D();
	else nrFlaps = NumFlapsRight2D();

printf("StereoFlaps::Draw: %u\n", nrFlaps);
	for(int i=0; i<nrFlaps; i++)
	{
		if(masked)
			vcore[side]->Gestalts(Gestalt::FLAP, i)->Draw();	
		else
			if (vcore[side]->Gestalts(Gestalt::FLAP, i)->IsUnmasked())
				vcore[side]->Gestalts(Gestalt::FLAP, i)->Draw();	
	}
}

/**
 * @brief Draw matched flaps as overlay.
 * @param side Left or right side of the stereo images.
 */
void StereoFlaps::DrawMatched(int side)
{
printf("StereoFlaps::DrawMatched: %u\n", flapMatches);
	for(int i=0; i< flapMatches; i++)
	{
		flaps[side][i].surf[0].Draw(RGBColor::red);
		flaps[side][i].surf[1].Draw(RGBColor::red);
	}
}

/**
 * @brief Convert flap from object detector to working memory's visual object.
 * @param obj Visual object to create.
 * @param id ID of the object detector flap.
 * @return Return true for success.
 */
bool StereoFlaps::StereoGestalt2VisualObject(VisionData::VisualObjectPtr &obj, int id)
{
	obj->model = new VisionData::GeometryModel;
	Flap3D flap = Flaps(id);

	// Recalculate pose of vertices (relative to the pose of the flap == COG)
	Pose3 pose;
	RecalculateCoordsystem(flap, pose);

	// add center point to the model
	cogx::Math::Pose3 cogxPose;
	cogxPose.pos.x = pose.pos.x;
	cogxPose.pos.y = pose.pos.y;
	cogxPose.pos.z = pose.pos.z;
	obj->pose = cogxPose;

	// create vertices (relative to the 3D center point)
	for(unsigned i=0; i<=1; i++)	// LEFT/RIGHT rectangle of flap
	{
		VisionData::Face f;

		for(unsigned j=0; j<flap.surf[i].vertices.Size(); j++)
		{
			VisionData::Vertex v;
			v.pos.x = flap.surf[i].vertices[j].p.x;
			v.pos.y = flap.surf[i].vertices[j].p.y;
			v.pos.z = flap.surf[i].vertices[j].p.z;
			obj->model->vertices.push_back(v);

			f.vertices.push_back(j+(i*4));
		}

		obj->model->faces.push_back(f);
		f.vertices.clear();
	}

	obj->detectionConfidence = 1.0;						// TODO detection confidence is always 1

	return true;
}

/**
 * TODO: 
 * Es wird der Schwerpunkt des Flaps als Zentrum des Flap-Koordinatensystem verwendet und die Pose zur Kamera errechnet.
 * @brief Try to find a "natural" looking coordinate system for a flap.
 * The coordinate system is really arbitrary, there is no proper implicitly defined coordinate system.
 * We take the (geometrical) center of gravity of the corner points as position and set orientation to identity.
 * @param flap 3D Flap
 * @param pose pose
 */
void StereoFlaps::RecalculateCoordsystem(Flap3D &flap, Pose3 &pose)
{
  Vector3 c(0., 0., 0.);
  int cnt = 0;
  // find the center of gravity
  for(int i = 0; i <= 1; i++)
  {
    for(unsigned j = 0; j < flap.surf[i].vertices.Size(); j++)
    {
      c += flap.surf[i].vertices[j].p;
      cnt++;
    }
  }
  c /= (double)cnt;
  pose.pos.x = c.x;
  pose.pos.y = c.y;
  pose.pos.z = c.z;

	// set the orientation to identity, i.e. parallel to world coordinate system
  pose.rot.x = 0.;
  pose.rot.y = 0.;
  pose.rot.z = 0.;

  // invert to get pose of world w.r.t. flap
  Pose3 inv = pose.Inverse();

	// recalculate the vectors to the vertices from new center point
  for(int i = 0; i <= 1; i++)
  {
    for(unsigned j = 0; j < flap.surf[i].vertices.Size(); j++)
    {
      Vector3 p(flap.surf[i].vertices[j].p.x,
                flap.surf[i].vertices[j].p.y,
                flap.surf[i].vertices[j].p.z);
			flap.surf[i].vertices[j].p = inv.Transform(p);
    }
	}
}


/**
 * @brief Calculate matching score for flaps
 * @param left_flap Left tmp. flap
 * @param right_flap Right tmp. flap
 * @param off_0 TODO Offset ???
 * @param off_1 TODO Offset ???
 * @param cross true, if match is "crossed" and not "straight"
 */
double StereoFlaps::MatchingScore(TmpFlap &left_flap, TmpFlap &right_flap, unsigned &off_0, unsigned &off_1, bool &cross)
{
  // _s .. straight (left flap surf 0 matches right flap surf 0)
  // _x .. crossed (left flap surf 0 matches right flap surf 1)
  unsigned off_s0, off_s1, off_x0, off_x1;
  double sc_s = MatchingScoreSurf(left_flap.surf[0], right_flap.surf[0], off_s0) +
                MatchingScoreSurf(left_flap.surf[1], right_flap.surf[1], off_s1);
  double sc_x = MatchingScoreSurf(left_flap.surf[0], right_flap.surf[1], off_x1) +
                MatchingScoreSurf(left_flap.surf[1], right_flap.surf[0], off_x0);

// printf("	Matching score: %4.2f - %4.2f\n", sc_s, sc_x);
  // if flaps match "straight"
  if(sc_s < sc_x)
  {
    cross = false;
    off_0 = off_s0;
    off_1 = off_s1;
    return sc_s;
  }
  // else if flaps match "crossed"
  else
  {
    cross = true;
    off_0 = off_x0;
    off_1 = off_x1;
    return sc_x;
  }
}

/**																																			/// TODO StereoFlaps verschieben
 * @brief Find right best matching flap for given left flaps, begining at position l of right flap array.
 * @param left_flap Tmp. flap of left stereo image.
 * @param right_flaps Array of all flaps from right stereo image.
 * @param l Begin at position l of right flap array
 * @return Returns position of best matching right flap from the right flap array.
 */
unsigned StereoFlaps::FindMatchingFlap(TmpFlap &left_flap, Array<TmpFlap> &right_flaps, unsigned l)
{
// printf("    FindMatchingFlap:\n");
  double match, best_match = HUGE;
  unsigned j, j_best = UNDEF_ID;
  unsigned off_0, off_1, off_0_best = 0, off_1_best = 0;
  bool cross = false, cross_best = false;
  for(j = l; j < right_flaps.Size(); j++)
  {
    match = MatchingScore(left_flap, right_flaps[j], off_0, off_1, cross);
    if(match < best_match)
    {
      best_match = match;
      j_best = j;
      cross_best = cross;
      off_0_best = off_0;
      off_1_best = off_1;
    }
  }
  if(j_best != UNDEF_ID)
  {
    right_flaps[j_best].Fuddle(off_0_best, off_1_best, cross_best);
  }
  return j_best;
}


/**
 * @brief Match left and right flaps from an stereo image pair and get it sorted to the beginning of the array.
 * @param left_flaps Array of all flaps from left stereo image (matching flaps get sorted to the beginning of the array.)
 * @param right_flaps Array of all flaps from right stereo image.
 * @param matches Number of matched flaps (sorted to the beginning of the arrays).
 */
void StereoFlaps::MatchFlaps(Array<TmpFlap> &left_flaps, Array<TmpFlap> &right_flaps, int &matches)
{
  unsigned j, l = 0, u = left_flaps.Size();
  for(; l < u && l < right_flaps.Size();)
  {
    j = FindMatchingFlap(left_flaps[l], right_flaps, l);
    // found a matching right, move it to same index position as left
    if(j != UNDEF_ID)
    {
      right_flaps.Swap(l, j);
      l++;
    }
    // found no right, move left to end and decrease end
    else
    {
      left_flaps.Swap(l, u-1);
      u--;
    }
  }
  u = min(u, right_flaps.Size());
  matches = u;
}


/**
 * @brief Calculate 3D flaps from matched flaps.
 * @param left_flaps Array of all flaps from left stereo image.
 * @param right_flaps Array of all flaps from right stereo image.
 * @param matches Number of matched flaps.
 * @param flap3ds Array of calculated 3d flaps.
 */
void StereoFlaps::Calculate3DFlaps(Array<TmpFlap> &left_flaps, Array<TmpFlap> &right_flaps, int &matches, Array<Flap3D> &flap3ds)
{
  unsigned u = matches;
  for(unsigned i = 0; i < u;)
  {
    Flap3D flap3d;
    bool ok0 = flap3d.surf[0].Reconstruct(stereo_cam, left_flaps[i].surf[0], right_flaps[i].surf[0], true);
    bool ok1 = flap3d.surf[1].Reconstruct(stereo_cam, left_flaps[i].surf[1], right_flaps[i].surf[1], true);
    if(ok0 && ok1)
    {
      flap3ds.PushBack(flap3d);
      i++;
    }
    // move unacceptable flaps to the end
    else
    {
      left_flaps.Swap(i, u-1);
      right_flaps.Swap(i, u-1);
      u--;
    }
  }
  matches = u;
}

/**
 * @brief Delete all arrays ...
 */
void StereoFlaps::ClearResults()
{
	flaps[LEFT].Clear();
	flaps[RIGHT].Clear();
	flap3ds.Clear();

	flapMatches = 0;
}

/**
 * @brief Match and calculate 3D flaps from 2D flaps.
 * @param side LEFT/RIGHT image of stereo.images.
 */
void StereoFlaps::Process()
{
  for(int side = LEFT; side <= RIGHT; side++)
  {
		// note: the awkward Gestalt::FLAP thingy is necessary because the global		TODO ARI: Why?
		// NumFlaps() and Flaps() collide with StereoCores respective methods.
		for(unsigned i = 0; i < vcore[side]->NumGestalts(Gestalt::FLAP); i++)
		{
			Flap *core_flap = (Flap*)vcore[side]->Gestalts(Gestalt::FLAP, i);
			if(!vcore[side]->use_masking || !core_flap->IsMasked())
			{
				TmpFlap flap(core_flap);
				if(flap.IsValid())
					flaps[side].PushBack(flap);
			}
		}
		for(unsigned i = 0; i < flaps[side].Size(); i++)
			flaps[side][i].Rectify(stereo_cam, side);
		for(unsigned i = 0; i < flaps[side].Size(); i++)
			flaps[side][i].Refine();
	}

  // do stereo matching and depth calculation
  flapMatches = 0;
  MatchFlaps(flaps[LEFT], flaps[RIGHT], flapMatches);
  Calculate3DFlaps(flaps[LEFT], flaps[RIGHT], flapMatches, flap3ds);
}


}








