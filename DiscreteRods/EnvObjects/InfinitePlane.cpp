#ifdef NEVERDEFINED
#include "InfinitePlane.h"
#include "../threadpiece_discrete.h"

InfinitePlane::InfinitePlane(const Vector3d& pos, const Vector3d& norm, float c0, float c1, float c2)
	: EnvObject(pos, Matrix3d::Identity(), c0, c1, c2, INFINITE_PLANE)
	, normal(norm)
	, side(100.0)
	, file_name("notexture")
{
	rotation_from_tangent(norm.normalized(), rotation);
}

//Image should be saved as BMP 24 bits R8 G8 B8, RGB mode
InfinitePlane::InfinitePlane(const Vector3d& pos, const Vector3d& norm, string filename)
	: EnvObject(pos, Matrix3d::Identity(), 1.0, 1.0, 1.0, INFINITE_PLANE)
	, normal(norm)
	, side(100.0)
	, file_name(filename)
{
	rotation_from_tangent(norm.normalized(), rotation);
	ilInit();
  if (! LoadImageDevIL ((char*) filename.c_str(), &texture) )
  	cerr << "Failed to load texture from filename " << filename << endl;
}

InfinitePlane::InfinitePlane(const InfinitePlane& rhs)
	: EnvObject(rhs.position, rhs.rotation, rhs.color0, rhs.color1, rhs.color2, rhs.type)
	, normal(rhs.normal)
	, side(rhs.side)
	, file_name(rhs.file_name)
{
	if (type != INFINITE_PLANE)
		cout << "error in infiniteplane" << endl; //TODO assert
	rotation_from_tangent(normal.normalized(), rotation);
	if (file_name != "notexture") {
		ilInit();
		if (! LoadImageDevIL ((char*) file_name.c_str(), &texture) )
			cerr << "Failed to load texture from filename " << file_name << endl;
	}
}

InfinitePlane::~InfinitePlane()
{
  //  Clear out the memory used by loading image files.
  if (texture.id)
    ilDeleteImages(1, &texture.id);
}

void InfinitePlane::writeToFile(ofstream& file)
{
	file << type << " ";
	for (int i=0; i<3; i++)
		file << position(i) << " ";

  file << normal(0) << " " << normal(1) << " " << normal(2) << " " << side << " " << color0 << " " << color1 << " " << color2 << " " << file_name << " ";

  file << "\n";
}

InfinitePlane::InfinitePlane(ifstream& file)
{
  type = INFINITE_PLANE;
  
	for (int i=0; i<3; i++)
		file >> position(i);

  file >> normal(0) >> normal(1) >> normal(2) >> side >> color0 >> color1 >> color2 >> file_name;
  rotation_from_tangent(normal.normalized(), rotation);
  if (file_name != "notexture") {
  	ilInit();
		if (! LoadImageDevIL ((char*) file_name.c_str(), &texture) )
			cerr << "Failed to load texture from filename " << file_name << endl;
  }
}

void InfinitePlane::recomputeFromTransform(const Vector3d& pos, const Matrix3d& rot)
{
	normal = rot.col(0);
}

void InfinitePlane::draw()
{
	Vector3d x = rotation.col(1);
	Vector3d y = rotation.col(2);

	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL);
	glColor3f(color0, color1, color2);
	glDisable(GL_CULL_FACE);
	
	bool tex = (file_name != "notexture");
	
	if (tex) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D (GL_TEXTURE_2D, 0, 3, texture.w, texture.h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texture.p);
		
		glEnable (GL_TEXTURE_2D);
	}

	glBegin(GL_QUADS);
	if (tex) glTexCoord2s(0, 0);
	glVertex3f(position(0) + side*(x(0)+y(0)), position(1) + side*(x(1)+y(1)), position(2) + side*(x(2)+y(2)));
	if (tex) glTexCoord2s(1, 0);
	glVertex3f(position(0) + side*(x(0)-y(0)), position(1) + side*(x(1)-y(1)), position(2) + side*(x(2)-y(2)));
	if (tex) glTexCoord2s(1, 1);
	glVertex3f(position(0) + side*(-x(0)-y(0)), position(1) + side*(-x(1)-y(1)), position(2) + side*(-x(2)-y(2)));
	if (tex) glTexCoord2s(0, 1);
	glVertex3f(position(0) + side*(-x(0)+y(0)), position(1) + side*(-x(1)+y(1)), position(2) + side*(-x(2)+y(2)));
	glEnd();

	if (tex)
		glDisable (GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glPopMatrix();
}

bool InfinitePlane::capsuleIntersection(int capsule_ind, const Vector3d& start, const Vector3d& end, const double radius, vector<Intersection>& intersections)
{
  Vector3d direction;
  double intersection_dist = capsuleInfinitePlaneDistance(start, end, radius, position, normal, direction);
  if(intersection_dist < 0) {
    intersections.push_back(Intersection(capsule_ind, -intersection_dist, direction));
    return true;
  }
	return false;
}

double InfinitePlane::capsuleRepulsionEnergy(const Vector3d& start, const Vector3d& end, const double radius)
{
	if (REPULSION_COEFF <= 0.0) { return 0.0; }
	Vector3d direction;
	double dist = capsuleInfinitePlaneDistance(start, end, radius, position, normal, direction);
	if (dist < 0 || dist > radius)
		return 0.0;
	return REPULSION_COEFF/2.0 * pow(dist-radius,2);
}

void InfinitePlane::capsuleRepulsionEnergyGradient(const Vector3d& start, const Vector3d& end, const double radius, Vector3d& gradient)
{
	if (REPULSION_COEFF <= 0.0) { return; }
	Vector3d direction;
	double dist = capsuleInfinitePlaneDistance(start, end, radius, position, normal, direction);
	if (dist < 0 || dist > radius)
		return;
	gradient -= REPULSION_COEFF * (radius - dist) * normal;
}

ILuint InfinitePlane::LoadImageDevIL (char *szFileName, struct TextureHandle *T)
{
    //When IL_ORIGIN_SET enabled, the origin is specified at an absolute 
    //position, and all images loaded or saved adhere to this set origin.
    ilEnable(IL_ORIGIN_SET);
    //sets the origin to be IL_ORIGIN_LOWER_LEFT when loading all images, so 
    //that any image with a different origin will be flipped to have the set 
    //origin.
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    //Now load the image file
    ILuint ImageNameID;
    ilGenImages(1, &ImageNameID);
    ilBindImage(ImageNameID);
    if (!ilLoadImage(szFileName)) return 0; // failure 

    T->id = ImageNameID;
    T->p = ilGetData(); 
    T->w = ilGetInteger(IL_IMAGE_WIDTH);
    T->h = ilGetInteger(IL_IMAGE_HEIGHT);
    
    //printf("%s %d %d %d\n",szFileName,T->id,T->w,T->h);
    return 1; // success
}
#endif
